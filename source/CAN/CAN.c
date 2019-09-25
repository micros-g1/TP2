/*
 * CAN.c
 *
 *  Created on: 24 Sep 2019
 *      Author: grein
 */

#include <CAN/CAN.h>
#include <CAN/MCP25625/MCP25625_driver.h>

// Target bitrate: 125kbit/seg == 8us/bit
// TBIT = [SYNC_T + PSEG_T + PHSEG1_T + PHSEG2_T ] = N x TQ
// TQ = 2 x (BRP<5:0> + 1)/FOSC = K/FOSC => TBIT = N x K / FOSC
// Then K = TBIT x FOSC / N
// SYNC_T = 1xTQ

#define PROPSEG_QUANTA 4
#define PHSEG1_QUANTA 1
#define PHSEG2_QUANTA 2

#define ID_MASK ~0x07	//3 lsb can take any value.
#define ID_FILTER 0		//Combined with ID_MASK, all bits except the first 3 bits must be zero.
#define DATA_MASK	0	//Do not apply filter to data
#define DATA_FILTER	0	//Don't care.

//Since SYNC_T + PSEG_T + PHSEG1_T + PHSEG2_T  = 8 x TQ and FOSC = 16MHz
//Then K = 8us * 16MHz / 8 = 16 => BRP<5:0> = K/2-1 = 7
#define BRP_VALUE 7

//Was last message rolled over?
static bool rolled_over;

//Callbacks (for user)
static CAN_rx_buffer_overflow_callback_t callback_overflow;
static CAN_message_callback_t callback_message;

//Callback (for interrupts)
static void can_controller_isr(void);

//Convert raw (mcp25625 format) to can message.
static void convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message);

//Convert can message to raw (mcp25625 format)
static void convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package);

//Create id for filter / mask
// + for target_frames == CAN_STANDARD_FRAME, id_filter_mask is the 11 bits of the id of the message (will be cropped if > 11 bits)
// and data_filter_mask represents the filter / mask to apply to the first two bytes of the message
// + for target_frames == CAN_EXTENDED_FRAME, id_filter_mask is the 29 bits of the id of the message (will be cropped if > 11 bits)
// and, in this case, data_filter_mask is ignored.
static mcp25625_id_t generate_filter_mask_id(uint32_t id_filter_mask, uint16_t data_filter_mask, can_frame_t target_frames);

void CAN_init()
{
	static bool init = false;
	if(init)
		return;
	init = true;
	callback_message = NULL;
	callback_overflow = NULL;
	//Reset
	mcp25625_reset();
	//Configure Timing (and set BLTMODE so that value of PHSEG2 is set by PHSEG2 bits in CNF3)
	mcp25625_bit_modify(CNF1_ADDR, BRP, BRP_VALUE<<BRP_POS);
	mcp25625_bit_modify(CNF2_ADDR, BLTMODE | PHSEG1 | PRSEG, BLTMODE | (PHSEG1_QUANTA-1)<<PHSEG1_POS | (PROPSEG_QUANTA-1)<<PRSEG);
	mcp25625_bit_modify(CNF3_ADDR, PHSEG2, (PHSEG2_QUANTA-1) << PHSEG2_POS);
	//Set reception filters and masks
	mcp25625_id_t filter = generate_filter_mask_id(ID_FILTER,DATA_FILTER,CAN_STANDARD_FRAME);
	mcp25625_id_t mask = generate_filter_mask_id(ID_MASK,DATA_MASK,CAN_STANDARD_FRAME);
	//Write all masks and filters
	mcp25625_write(RXF0_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF1_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF2_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF3_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF4_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF5_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXM0_ADDR, sizeof(mask), (uint8_t*) &mask);
	mcp25625_write(RXM1_ADDR, sizeof(mask), (uint8_t*) &mask);
	//Set reception mode to "receive valid messages", and enable roll over
	mcp25625_write_register(RXB0_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS | BUKT);
	mcp25625_write_register(RXB1_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS);
	rolled_over = false;
	//Clear CANINTF Flags
	mcp25625_write_register(CANINTF_ADDR, 0);
	//Configure interrupt pin
	//TODO: Configure interrupt and callback
	//Enable interrupts in MCP25625
	mcp25625_write_register(CANINTE_ADDR, RX0IE | RX1IE);
	//Set to normal mode, and disable clock, which is not needed.
	//TODO: Change to NORMAL_OPERATION after testing
	mcp25625_bit_modify(CANSTAT_ADDR, OPMOD | CLKEN, LOOPBACK_MODE << OPMOD_POS | 0 << CLKEN_POS);
	//While until normal operation mode is set.
	while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != NORMAL_OPERATION);
}

bool CAN_message_available()
{
	return mcp25625_read_rx_status().msg_info != NO_RX_MSG;
}

bool CAN_send(const can_message_t *p_message)
{
	mcp25625_status_t rx_status = mcp25625_read_status();
	mcp25625_txb_rts_flag_t rts_flag = 0;
	mcp25625_txb_id_t txb_to_write = TXB0;
	bool tx_buffer_free = false;
	if(!rx_status.txreq0)
	{
		rts_flag = TXB0_RTS;
		txb_to_write = TXB0;
		tx_buffer_free = true;
	}
	else if(!rx_status.txreq1)
	{
		rts_flag = TXB1_RTS;
		txb_to_write = TXB1;
		tx_buffer_free = true;
	}
	else if(!rx_status.txreq2)
	{
		rts_flag = TXB2_RTS;
		txb_to_write = TXB2;
		tx_buffer_free = true;
	}
	if(tx_buffer_free)
	{
		mcp25625_id_data_t package_raw;
		convert_can_message_to_raw(p_message,&package_raw);
		mcp25625_load_tx_buffer_id_data(txb_to_write, &package_raw);
		mcp25625_tx_request_to_send(rts_flag);
	}
	return tx_buffer_free;
}


bool CAN_get(can_message_t *p_message)
{
	mcp25625_rx_status_t rx_status = mcp25625_read_rx_status();
	bool message_available = rx_status.msg_info != NO_RX_MSG;
	mcp25625_id_data_t package_raw;
	if(message_available)
	{
		mcp25625_rxb_id_t rxb_to_read = RXB0;
		switch(rx_status.msg_info)
		{
			case RXB0_MSG:
				//Only one message in RXB0. No roll over.
				rolled_over = false;
				break;
			case RXB1_MSG:
				//Only message in RXB1. This message rolled over,
				//but now it is the only message left, no roll over message
				//left behind
				rolled_over = false;
				rxb_to_read = RXB1;
				break;
			case BOTH_RX_MSG:
				//Did I already have a rolled over message?
				if(rolled_over)
				{
					//Yes. Read RXB1 before TXB0
					//RBX0 was freed and reused after message in TXB1 was received.
					rxb_to_read = RXB1;
					rolled_over = false;
				}
				else
				{
					//No. Then, first check message in RXB0
					//And next message to read is message in RXB1
					rolled_over = true;
				}
			case NO_RX_MSG:
				break;
		}
		//Get data.
		mcp25625_read_rx_buffer_id_data(rxb_to_read,&package_raw);
		convert_raw_to_can_message(&package_raw, p_message);
	}
	return message_available;
}


void CAN_set_rx_buffer_overflow_callback(CAN_rx_buffer_overflow_callback_t callback)
{
	callback_overflow = callback;
}


void CAN_set_message_callback(CAN_message_callback_t callback)
{
	callback_message = callback;
}


static void convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message)
{
	if(p_raw_package->id.sidl.ide)
	{
		p_can_message->fir.frame_type = CAN_EXTENDED_FRAME;
		p_can_message->message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->message_id <<= 3;
		p_can_message->message_id |= p_raw_package->id.sidl.sid_l;
		p_can_message->message_id <<= 2;
		p_can_message->message_id |= p_raw_package->id.sidl.eid_hh;
		p_can_message->message_id <<= 8;
		p_can_message->message_id |= p_raw_package->id.eid8.eid_h;
		p_can_message->message_id <<= 8;
		p_can_message->message_id |= p_raw_package->id.eid0.eid_l;
		p_can_message->fir.rtr = p_raw_package->data.dlc.rtr;
	}
	else
	{
		p_can_message->fir.frame_type = CAN_STANDARD_FRAME;
		p_can_message->message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->message_id <<= 3;
		p_can_message->message_id |= p_raw_package->id.sidl.sid_l;
		p_can_message->fir.rtr = p_raw_package->id.sidl.ssr;
	}
	unsigned int n_msg = p_raw_package->data.dlc.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	for(int i = 0 ; i < n_msg ; i++)
		p_can_message->data[i] = p_raw_package->data.buffer[i];
	p_can_message->fir.dlc = n_msg;
}


void convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package)
{
	switch(p_can_message->fir.frame_type)
	{
		case CAN_STANDARD_FRAME:
			p_raw_package->id.sidl.sid_l = (uint8_t) (p_can_message->message_id & 0x07);
			p_raw_package->id.sidh.sid_h = (uint8_t) ((p_can_message->message_id >> 3) & 0x0FF);
			break;
		case CAN_EXTENDED_FRAME:
			p_raw_package->id.eid0.eid_l = (uint8_t) (p_can_message->message_id & 0x0FF);
			p_raw_package->id.eid8.eid_h = (uint8_t) ((p_can_message->message_id >> 8) & 0x0FF);
			p_raw_package->id.sidl.sid_l = (uint8_t) ((p_can_message->message_id >> 16) & 0x07);
			p_raw_package->id.sidh.sid_h = (uint8_t) ((p_can_message->message_id >> 19) & 0x0FF);
			break;
	}
	p_raw_package->id.sidl.ssr = p_can_message->fir.rtr;
	p_raw_package->data.dlc.rtr = p_can_message->fir.rtr;
	unsigned int n_msg = p_can_message->fir.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	for(int i = 0 ; i < n_msg ; i++)
		p_raw_package->data.buffer[i] = p_can_message->data[i];
	p_raw_package->data.dlc.dlc = n_msg;
}


mcp25625_id_t generate_filter_mask_id(uint32_t id_filter_mask, uint16_t data_filter_mask, can_frame_t target_frames)
{
	mcp25625_id_t filter_mask;
	//Used for filters, ignored for masks.
	filter_mask.sidl.ide = (target_frames == CAN_EXTENDED_FRAME);
	filter_mask.sidl.ssr = 0;	//Does not apply, set to zero
	switch(target_frames)
	{
		case CAN_STANDARD_FRAME:
			filter_mask.sidh.sid_h = (uint8_t) 0x0FF & (id_filter_mask >> 3);
			filter_mask.sidl.sid_l = (uint8_t) 0x07 & id_filter_mask;
			filter_mask.sidl.eid_hh = 0; //The two MSbs’ (EID17 and EID16) mask and filter bits are not used.
			filter_mask.eid8.eid_h = (uint8_t) 0x0FF & (data_filter_mask >> 8);
			filter_mask.eid0.eid_l = (uint8_t) 0x0FF & data_filter_mask;
			break;
		case CAN_EXTENDED_FRAME:
			//data_filter_mask parameter is ignored.
			filter_mask.sidh.sid_h = (uint8_t) 0x0FF & (id_filter_mask >> 21);
			filter_mask.sidl.sid_l = (uint8_t) 0x07 & (id_filter_mask >> 18);
			filter_mask.sidl.eid_hh = (uint8_t) 0x03 & (id_filter_mask >> 16);
			filter_mask.eid8.eid_h = (uint8_t) 0x0FF & (data_filter_mask >> 8);
			filter_mask.eid0.eid_l = (uint8_t) 0x0FF & data_filter_mask;
			break;
	}
	return filter_mask;
}

static void can_controller_isr(void)
{
	//Get flags
	mcp25625_canintf_t canintf = (mcp25625_canintf_t) mcp25625_read_register(CANINTF_ADDR);
	if(canintf.errif)
	{
		mcp25625_eflg_t eflg = (mcp25625_eflg_t) mcp25625_read_register(EFLG_ADDR);
		//Free flag so that further interrupts can be attended separatedly
		if(eflg.rx1ovr)
		{
			mcp25625_bit_modify(EFLG_ADDR, RX1OVR, 0);
			mcp25625_bit_modify(CANINTF_ADDR, RX1OVR, 0);
			if(callback_overflow != NULL)
				callback_overflow();
		}
	}
	if(canintf.rx1if)
	{
		mcp25625_bit_modify(CANINTF_ADDR, RX1IF, 0);
		if(callback_message != NULL)
		{
			can_message_t can_message;
			if(CAN_get(&can_message))	//Should always be true
				callback_message(&can_message);
		}
	}
}
