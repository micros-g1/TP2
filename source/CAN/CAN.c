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

#define PROPSEG_QUANTA 4u
#define PHSEG1_QUANTA 1u
#define PHSEG2_QUANTA 2u

#define DATA_MASK	0	//Do not apply filter to data
#define DATA_FILTER	0	//Don't care.

//Since SYNC_T + PSEG_T + PHSEG1_T + PHSEG2_T  = 8 x TQ and FOSC = 16MHz
//Then K = 8us * 16MHz / 8 = 16 => BRP<5:0> = K/2-1 = 7
#define BRP_VALUE 7

//Operation mode to set after restart and init
#define DEFAULT_OP_MODE LOOPBACK_MODE

//roll-over flag
static bool rolled_over = false;

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

void CAN_init(uint32_t id_mask, uint32_t id_filter)
{
	static bool init = false;
	if(init)
		return;
	init = true;
	//Init Driver
	mcp25625_driver_init();
	//Reset
	mcp25625_reset();
	//Now, configure driver callback
	mcp25625_driver_set_callback(can_controller_isr);
	//Configure Timing (and set BLTMODE so that value of PHSEG2 is set by PHSEG2 bits in CNF3)
	mcp25625_bit_modify(CNF1_ADDR, BRP, BRP_VALUE<<BRP_POS);
	mcp25625_bit_modify(CNF2_ADDR, BLTMODE | PHSEG1 | PRSEG, BLTMODE | (PHSEG1_QUANTA-1)<<PHSEG1_POS | (PROPSEG_QUANTA-1)<<PRSEG_POS);
	mcp25625_bit_modify(CNF3_ADDR, PHSEG2, (PHSEG2_QUANTA-1) << PHSEG2_POS);
	//Set reception filters and masks
	mcp25625_id_t filter = generate_filter_mask_id(id_filter,DATA_FILTER,CAN_STANDARD_FRAME);
	mcp25625_id_t mask = generate_filter_mask_id(id_mask,DATA_MASK,CAN_STANDARD_FRAME);
	//Write all masks and filters. Same filter and mask for both input buffers
	//RXB0 has higher priority. Therefore, RXB1 will not be used.
	mcp25625_write(RXF0_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF1_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF2_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF3_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF4_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF5_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXM0_ADDR, sizeof(mask), (uint8_t*) &mask);
	mcp25625_write(RXM1_ADDR, sizeof(mask), (uint8_t*) &mask);
	//Set reception mode to "receive valid messages", enable roll-over
	mcp25625_write_register(RXB0_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS | BUKT);
	mcp25625_write_register(RXB1_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS);
	rolled_over = false;
	//Clear CANINTF Flags
	mcp25625_write_register(CANINTF_ADDR, 0);
	//Enable interrupts, MCP25625. Interrupts: Freed transmit buffer, full receive buffer, error.
	mcp25625_write_register(CANINTE_ADDR, ERRIE | TX2IE | TX1IE | TX0IE | RX0IE | RX1IE);
	//Set to default operation mode, and disable clock, which is not needed.
	mcp25625_bit_modify(CANCTRL_ADDR, REQOP | CLKEN, DEFAULT_OP_MODE << REQOP_POS | 0 << CLKEN_POS);
	//While until operation mode is set.
	while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != DEFAULT_OP_MODE);
}

void CAN_change_filter_config(uint32_t id_mask, uint32_t id_filter)
{
	mcp25625_bit_modify(CANCTRL_ADDR, REQOP, CONFIG_MODE << REQOP_POS);
	while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != CONFIG_MODE);
	mcp25625_id_t filter = generate_filter_mask_id(id_filter,DATA_FILTER,CAN_STANDARD_FRAME);
	mcp25625_id_t mask = generate_filter_mask_id(id_mask,DATA_MASK,CAN_STANDARD_FRAME);
	//Write all masks and filters. Same filter and mask for both input buffers
	//RXB0 has higher priority. Therefore, RXB1 will not be used.
	mcp25625_write(RXF0_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF1_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF2_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF3_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF4_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXF5_ADDR, sizeof(filter), (uint8_t*) &filter);
	mcp25625_write(RXM0_ADDR, sizeof(mask), (uint8_t*) &mask);
	mcp25625_write(RXM1_ADDR, sizeof(mask), (uint8_t*) &mask);
	mcp25625_bit_modify(CANCTRL_ADDR, REQOP, DEFAULT_OP_MODE << REQOP_POS);
	while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != DEFAULT_OP_MODE);
}

bool CAN_message_available()
{
	return mcp25625_read_rx_status().msg_info != NO_RX_MSG;
}

bool CAN_send(const can_message_t *p_message)
{
	mcp25625_status_t status = mcp25625_read_status();
	mcp25625_txb_rts_flag_t rts_flag = 0;
	mcp25625_txb_id_t txb_to_write = TXB0;
	bool tx_buffer_free = false;
	if(!status.txreq0)
	{
		rts_flag = TXB0_RTS;
		txb_to_write = TXB0;
		tx_buffer_free = true;
	}
	else if(!status.txreq1)
	{
		rts_flag = TXB1_RTS;
		txb_to_write = TXB1;
		tx_buffer_free = true;
	}
	else if(!status.txreq2)
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
		p_can_message->fir.rtr = p_raw_package->dlc.rtr;
	}
	else
	{
		p_can_message->fir.frame_type = CAN_STANDARD_FRAME;
		p_can_message->message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->message_id <<= 3;
		p_can_message->message_id |= p_raw_package->id.sidl.sid_l;
		p_can_message->fir.rtr = p_raw_package->id.sidl.ssr;
	}
	unsigned int n_msg = p_raw_package->dlc.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	//RTR frames do not have data
	if(!p_can_message->fir.rtr)
		for(int i = 0 ; i < n_msg ; i++)
			p_can_message->data[i] = p_raw_package->data.buffer[i];
	p_can_message->fir.dlc = n_msg;
}


void convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package)
{
	p_raw_package->id.sidl.ide = p_can_message->fir.frame_type == CAN_EXTENDED_FRAME;
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
	//id.sidl.ssr is not valid for transmission
	//But it is set here so that it is possible to convert messages to raw and back to CAN message
	//or to can message and back to raw.
	//Transmission will ignore this bit.
	p_raw_package->id.sidl.ssr = p_can_message->fir.rtr;
	//This is the important one for transmission:
	p_raw_package->dlc.rtr = p_can_message->fir.rtr;
	unsigned int n_msg = p_can_message->fir.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	//RTR frames do not have data
	if(!p_can_message->fir.rtr)
		for(int i = 0 ; i < n_msg ; i++)
			p_raw_package->data.buffer[i] = p_can_message->data[i];
	p_raw_package->dlc.dlc = n_msg;
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
	//DEBUG
}
