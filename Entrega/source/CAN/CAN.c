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
//Since SYNC_T + PSEG_T + PHSEG1_T + PHSEG2_T  = 8 x TQ and FOSC = 16MHz
//Then K = 8us * 16MHz / 8 = 16 => BRP<5:0> = K/2-1 = 7
#define BRP_VALUE 7

//Do not filter data
#define DATA_MASK	0	//Do not apply filter to data
#define DATA_FILTER	0	//Don't care.

//Operation mode to set after restart and init
#define DEFAULT_OP_MODE NORMAL_OPERATION
static mcp25625_opmode_t current_op_mode;

//roll-over flag (for managing both reception buffers)
static bool rolled_over = false;

//Initialized?
static bool initialized = false;

//Callback (for interrupts)
static void _CAN_controller_isr(void);

//Convert raw (mcp25625 format) to can message.
static void _CAN_convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message);

//Convert can message to raw (mcp25625 format)
static void _CAN_convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package);

//Sends filter to mcp25625. Device must be in config mode.
static void _CAN_send_filter_config(const can_filter_t * can_filter);

//Create id for filter / mask
// + for target_frames == CAN_STANDARD_FRAME, id_filter_mask is the 11 bits of the id of the message (will be cropped if > 11 bits)
// and data_filter_mask represents the filter / mask to apply to the first two bytes of the message
// + for target_frames == CAN_EXTENDED_FRAME, id_filter_mask is the 29 bits of the id of the message (will be cropped if > 11 bits)
// and, in this case, data_filter_mask is ignored.
static mcp25625_id_t _CAN_generate_filter_mask_id(uint32_t id_filter_mask, uint16_t data_filter_mask, can_frame_t target_frames);

//Buffers priorities
static mcp25625_priority_t txb0_prior,txb1_prior,txb2_prior;

//BUFFERS
#define CAN_RX_BUFFER_LENGTH 3
#define CAN_TX_BUFFER_LENGTH 3
static mcp25625_id_data_t tx_buffer[CAN_TX_BUFFER_LENGTH];
static mcp25625_id_data_t rx_buffer[CAN_RX_BUFFER_LENGTH];
static int tx_index_in = 0;
static int rx_index_in = 0;
static int tx_index_out = 0;
static int rx_index_out = 0;
static bool tx0_free = true;
static bool tx1_free = true;
static bool tx2_free = true;
static bool got_error = false;

void CAN_init()
{
	if(initialized)
		return;
	initialized = true;
	tx_index_in = 0;
	rx_index_in = 0;
	tx_index_out = 0;
	rx_index_out = 0;
	got_error = false;
	tx0_free = true;
	tx1_free = true;
	tx2_free = true;
	//Init Driver
	mcp25625_driver_init();
	//Reset
	mcp25625_reset();
	//Reset values are:
	txb0_prior = txb1_prior = txb2_prior = LOWEST_PRIOIRTY;
	current_op_mode = CONFIG_MODE;
	//Now, configure driver callback
	mcp25625_driver_set_callback(_CAN_controller_isr);
	//Configure Timing (and set BLTMODE so that value of PHSEG2 is set by PHSEG2 bits in CNF3)
	mcp25625_bit_modify(CNF1_ADDR, BRP, BRP_VALUE<<BRP_POS);
	mcp25625_bit_modify(CNF2_ADDR, BLTMODE | PHSEG1 | PRSEG, BLTMODE | (PHSEG1_QUANTA-1)<<PHSEG1_POS | (PROPSEG_QUANTA-1)<<PRSEG_POS);
	mcp25625_bit_modify(CNF3_ADDR, PHSEG2, (PHSEG2_QUANTA-1) << PHSEG2_POS);
	//Set reception filters and masks
	can_filter_t filter;
	filter.id = 0;
	filter.mask = 0;
	filter.frame_type = CAN_STANDARD_FRAME;
	_CAN_send_filter_config(&filter);
	//Set reception mode to "receive valid messages", enable roll-over
	mcp25625_write_register(RXB0_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS | BUKT);
	mcp25625_write_register(RXB1_ADDR+CTRL_OFFSET, RECEIVE_VALID_MESSAGES << RXM_POS);
	rolled_over = false;
	//Clear CANINTF Flags
	mcp25625_write_register(CANINTF_ADDR, 0);
	//Enable interrupts, MCP25625. Interrupts: Freed transmit buffer, full receive buffer, error.
	mcp25625_write_register(CANINTE_ADDR, ERRIE | TX2IE | TX1IE | TX0IE | RX0IE | RX1IE);
	//Set to sleep mode, and disable clock, which is not needed.
	mcp25625_bit_modify(CANCTRL_ADDR, CLKEN, 0 << CLKEN_POS);
	//Allow for further config (filters)
	//do not change op mode.
}

void CAN_start()
{
	if(initialized && current_op_mode != DEFAULT_OP_MODE)
	{
		//Clear all interrupt flags (free all buffers and clear errors)
		mcp25625_write_register(CANINTF_ADDR, 0);
		//Clear internal buffers
		tx_index_in = 0;
		rx_index_in = 0;
		tx_index_out = 0;
		rx_index_out = 0;
		tx0_free = true;
		tx1_free = true;
		tx2_free = true;
		//Set operation to default operation mode
		mcp25625_bit_modify(CANCTRL_ADDR, REQOP , DEFAULT_OP_MODE << REQOP_POS);
		//Wait operation mode changes
		while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != DEFAULT_OP_MODE);
		current_op_mode = DEFAULT_OP_MODE;
	}
}

void CAN_stop()
{
	if(initialized && current_op_mode == DEFAULT_OP_MODE)
	{
		//Set operation to default operation mode
		mcp25625_bit_modify(CANCTRL_ADDR, REQOP | CLKEN, CONFIG_MODE << REQOP_POS);
		//Wait operation mode changes
		current_op_mode = CONFIG_MODE;
		while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != CONFIG_MODE);
	}
}

void CAN_set_filter_config(can_filter_t filter)
{
	if(initialized)
	{
		if(current_op_mode != CONFIG_MODE)
		{
			mcp25625_bit_modify(CANCTRL_ADDR, REQOP, CONFIG_MODE << REQOP_POS);
			while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != CONFIG_MODE);
		}
		_CAN_send_filter_config(&filter);
		if(current_op_mode != CONFIG_MODE)
		{
			mcp25625_bit_modify(CANCTRL_ADDR, REQOP, current_op_mode << REQOP_POS);
			while(((mcp25625_canstat_t) mcp25625_read_register(CANSTAT_ADDR)).opmode != current_op_mode);
		}
	}
}

bool CAN_message_available()
{
	return rx_index_in != rx_index_out;
}

bool CAN_send(const can_message_t *p_message)
{
	mcp25625_driver_enable_interrupt_handling(false);
	int next_element = tx_index_in + 1;
	next_element = next_element == CAN_TX_BUFFER_LENGTH? 0 : next_element;
	bool buffer_not_full = next_element != tx_index_out;
	if(buffer_not_full)
	{
		_CAN_convert_can_message_to_raw(p_message, &tx_buffer[tx_index_in]);
		tx_index_in = next_element;
		//If all buffers are free, call irq so that message is sent.
		//This will start domino effect. Next time tx free interrupt will take care.
		//(unless all buffers become free again)
		if(tx0_free && tx1_free && tx2_free )
			_CAN_controller_isr();
	}
	mcp25625_driver_enable_interrupt_handling(true);
	return buffer_not_full;
}


bool CAN_get(can_message_t *p_message)
{
	bool got_message = false;
	mcp25625_driver_enable_interrupt_handling(false);
	got_message = rx_index_in != rx_index_out;
	if(got_message)
	{
		_CAN_convert_raw_to_can_message(&rx_buffer[rx_index_out], p_message);
		rx_index_out++;
		rx_index_out = rx_index_out == CAN_RX_BUFFER_LENGTH ? 0 : rx_index_out;
	}
	mcp25625_driver_enable_interrupt_handling(true);
	return got_message;
}

static void _CAN_convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message)
{
	if(p_raw_package->id.sidl.ide)
	{
		p_can_message->header.frame_type = CAN_EXTENDED_FRAME;
		p_can_message->header.message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->header.message_id <<= 3;
		p_can_message->header.message_id |= p_raw_package->id.sidl.sid_l;
		p_can_message->header.message_id <<= 2;
		p_can_message->header.message_id |= p_raw_package->id.sidl.eid_hh;
		p_can_message->header.message_id <<= 8;
		p_can_message->header.message_id |= p_raw_package->id.eid8.eid_h;
		p_can_message->header.message_id <<= 8;
		p_can_message->header.message_id |= p_raw_package->id.eid0.eid_l;
		p_can_message->header.rtr = p_raw_package->dlc.rtr;
	}
	else
	{
		p_can_message->header.frame_type = CAN_STANDARD_FRAME;
		p_can_message->header.message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->header.message_id <<= 3;
		p_can_message->header.message_id |= p_raw_package->id.sidl.sid_l;
		p_can_message->header.rtr = p_raw_package->id.sidl.ssr;
	}
	unsigned int n_msg = p_raw_package->dlc.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	//RTR frames do not have data
	if(!p_can_message->header.rtr)
		for(int i = 0 ; i < n_msg ; i++)
			p_can_message->data[i] = p_raw_package->data.buffer[i];
	p_can_message->header.dlc = n_msg;
}

void _CAN_convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package)
{
	p_raw_package->id.sidl.ide = p_can_message->header.frame_type == CAN_EXTENDED_FRAME;
	switch(p_can_message->header.frame_type)
	{
		case CAN_STANDARD_FRAME:
			p_raw_package->id.sidl.sid_l = (uint8_t) (p_can_message->header.message_id & 0x07);
			p_raw_package->id.sidh.sid_h = (uint8_t) ((p_can_message->header.message_id >> 3) & 0x0FF);
			break;
		case CAN_EXTENDED_FRAME:
			p_raw_package->id.eid0.eid_l = (uint8_t) (p_can_message->header.message_id & 0x0FF);
			p_raw_package->id.eid8.eid_h = (uint8_t) ((p_can_message->header.message_id >> 8) & 0x0FF);
			p_raw_package->id.sidl.sid_l = (uint8_t) ((p_can_message->header.message_id >> 16) & 0x07);
			p_raw_package->id.sidh.sid_h = (uint8_t) ((p_can_message->header.message_id >> 19) & 0x0FF);
			break;
	}
	//id.sidl.ssr is not valid for transmission
	//But it is set here so that it is possible to convert messages to raw and back to CAN message
	//or to can message and back to raw.
	//Transmission will ignore this bit.
	p_raw_package->id.sidl.ssr = p_can_message->header.rtr;
	//This is the important one for transmission:
	p_raw_package->dlc.rtr = p_can_message->header.rtr;
	unsigned int n_msg = p_can_message->header.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	//RTR frames do not have data
	if(!p_can_message->header.rtr)
		for(int i = 0 ; i < n_msg ; i++)
			p_raw_package->data.buffer[i] = p_can_message->data[i];
	p_raw_package->dlc.dlc = n_msg;
}


mcp25625_id_t _CAN_generate_filter_mask_id(uint32_t id_filter_mask, uint16_t data_filter_mask, can_frame_t target_frames)
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

void _CAN_send_filter_config(const can_filter_t * can_filter)
{
	mcp25625_id_t filter = _CAN_generate_filter_mask_id(can_filter->id,DATA_FILTER,can_filter->frame_type);
	mcp25625_id_t mask = _CAN_generate_filter_mask_id(can_filter->mask,DATA_MASK,can_filter->frame_type);
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
}


static void _CAN_controller_isr(void)
{
	if(current_op_mode == DEFAULT_OP_MODE)
	{
		//Do not leave ISR until IRQ is gone
		do
		{
			mcp25625_txb_rts_flag_t rts_flags = 0;
			//Read interrupt flags, if IRQ requested
			mcp25625_canintf_t canintf = (mcp25625_canintf_t) mcp25625_read_register(CANINTF_ADDR);
			//Update tx free flags
			tx0_free |= canintf.tx0if;
			tx1_free |= canintf.tx1if;
			tx2_free |= canintf.tx2if;
			//Anything to receive? Got space in buffer
			int next = rx_index_in+1;
			next = next == CAN_RX_BUFFER_LENGTH? 0 : next;
			bool rx_buff_full = (next == rx_index_in);
			while((canintf.rx0if || canintf.rx1if) && !rx_buff_full)
			{
				mcp25625_rxb_id_t rxb_to_read = RXB0;
				if(canintf.rx0if && canintf.rx1if)
				{
					if(rolled_over)
					{
						//Yes. Read RXB1 before TXB0
						//RBX0 was freed and reused after message in TXB1 was received.
						rxb_to_read = RXB1;
						rolled_over = false;
						canintf.rx1if = false;
					}
					else
					{
						//No. Then, first check message in RXB0
						//And next message to read is message in RXB1
						rolled_over = true;
						canintf.rx0if = false;
					}
				}
				else if(canintf.rx0if)
				{
					//Only one message in RXB0. No roll over.
					rolled_over = false;
					canintf.rx0if = false;

				}
				else
				{
					//Only message in RXB1. This message rolled over,
					//but now it is the only message left, no roll over message
					//left behind
					rolled_over = false;
					rxb_to_read = RXB1;
					canintf.rx1if = false;
				}
				//This will clear corresponding flag...
				mcp25625_read_rx_buffer_id_data(rxb_to_read,&rx_buffer[rx_index_in]);
				rx_index_in = next;
				//Prepare for next iteration
				next = rx_index_in+1;
				next = next == CAN_RX_BUFFER_LENGTH? 0 : next;
				rx_buff_full = (next == rx_index_in);
			}
			//Anything to transfer? got free transmit buffer?
			if(tx_index_out != tx_index_in && (tx0_free || tx1_free || tx2_free))
			{
				//Set free buffers priorities to low
				if(tx0_free && txb0_prior != LOWEST_PRIOIRTY)
				{
					txb0_prior = LOWEST_PRIOIRTY;
					mcp25625_bit_modify(TXB0_ADDR + CTRL_OFFSET, TXP, LOWEST_PRIOIRTY << TXP_POS);
				}
				if(tx1_free && txb1_prior != LOWEST_PRIOIRTY)
				{
					txb1_prior = LOWEST_PRIOIRTY;
					mcp25625_bit_modify(TXB1_ADDR + CTRL_OFFSET, TXP, LOWEST_PRIOIRTY << TXP_POS);
				}
				if(tx2_free && txb2_prior != LOWEST_PRIOIRTY)
				{
					txb2_prior = LOWEST_PRIOIRTY;
					mcp25625_bit_modify(TXB2_ADDR + CTRL_OFFSET, TXP, LOWEST_PRIOIRTY << TXP_POS);
				}
				//Determine new priorities
				mcp25625_priority_t txb0_prior_new=txb0_prior;
				mcp25625_priority_t txb1_prior_new=txb1_prior;
				mcp25625_priority_t txb2_prior_new=txb2_prior;
				//Promote TXB0, TXB1 and/or TXB2 to highest, if must do so
				if(!tx0_free && (tx1_free || txb0_prior >= txb1_prior) && (tx2_free || txb0_prior >= txb2_prior))
				{
					txb0_prior_new = HIGHEST_PRIORITY;
					mcp25625_bit_modify(TXB0_ADDR+CTRL_OFFSET, TXP, HIGHEST_PRIORITY << TXP_POS);
				}
				if(!tx1_free && (tx0_free || txb1_prior >= txb0_prior) && (tx2_free || txb1_prior >= txb2_prior))
				{
					txb1_prior_new = HIGHEST_PRIORITY;
					mcp25625_bit_modify(TXB1_ADDR+CTRL_OFFSET, TXP, HIGHEST_PRIORITY << TXP_POS);
				}
				if(!tx2_free && (tx0_free || txb2_prior >= txb0_prior) && (tx1_free || txb2_prior >= txb1_prior))
				{
					txb2_prior_new = HIGHEST_PRIORITY;
					mcp25625_bit_modify(TXB2_ADDR+CTRL_OFFSET, TXP, HIGHEST_PRIORITY << TXP_POS);
				}
				//Updates priorities variables
				txb0_prior = txb0_prior_new;txb1_prior = txb1_prior_new;txb2_prior = txb2_prior_new;
				//If lowest priority is in use by any transmitting buffer, upgrade priorities
				bool lowest_in_use_by_transmitting = !tx0_free && txb0_prior == LOWEST_PRIOIRTY;
				lowest_in_use_by_transmitting |=  !tx1_free && txb1_prior == LOWEST_PRIOIRTY;
				lowest_in_use_by_transmitting |=  !tx2_free && txb2_prior == LOWEST_PRIOIRTY;
				if(lowest_in_use_by_transmitting)
				{
					//Update all Low to High
					if(!tx0_free && txb0_prior==LOW_PRIORITY)
						mcp25625_bit_modify(TXB0_ADDR+CTRL_OFFSET, TXP, HIGH_PRIORITY << TXP_POS);
					if(!tx1_free && txb1_prior==LOW_PRIORITY)
						mcp25625_bit_modify(TXB1_ADDR+CTRL_OFFSET, TXP, HIGH_PRIORITY << TXP_POS);
					if(!tx2_free && txb2_prior==LOW_PRIORITY)
						mcp25625_bit_modify(TXB2_ADDR+CTRL_OFFSET, TXP, HIGH_PRIORITY << TXP_POS);
					//Update all Lowest to low
					if(!tx0_free && txb0_prior==LOWEST_PRIOIRTY)
						mcp25625_bit_modify(TXB0_ADDR+CTRL_OFFSET, TXP, LOW_PRIORITY << TXP_POS);
					if(!tx1_free && txb1_prior==LOWEST_PRIOIRTY)
						mcp25625_bit_modify(TXB0_ADDR+CTRL_OFFSET, TXP, LOW_PRIORITY << TXP_POS);
					if(!tx2_free && txb2_prior==LOWEST_PRIOIRTY)
						mcp25625_bit_modify(TXB0_ADDR+CTRL_OFFSET, TXP, LOW_PRIORITY << TXP_POS);
				}
				//Great! Priorities are ok now!
				//Transmit data while we can...
				while((tx0_free || tx1_free ||tx2_free) && (tx_index_out != tx_index_in))
				{
					mcp25625_txb_id_t tx2use;
					tx2use = tx0_free?TXB0:tx1_free?TXB1:TXB2;
					switch(tx2use)
					{
						case TXB0: tx0_free = false; rts_flags |= TXB0_RTS; break;
						case TXB1: tx1_free = false; rts_flags |= TXB1_RTS; break;
						case TXB2: tx2_free = false; rts_flags |= TXB2_RTS; break;
					}
					mcp25625_load_tx_buffer_id_data(tx2use, &tx_buffer[tx_index_out]);
					tx_index_out++;
					tx_index_out = tx_index_out == CAN_TX_BUFFER_LENGTH? 0 : tx_index_out;
				}
			}
			if(canintf.errif)
			{
				//Implement error handling if needed
				mcp25625_eflg_t eflg = (mcp25625_eflg_t) mcp25625_read_register(EFLG_ADDR);
				got_error = true;
			}
			//Clear interrupt flags if any
			if(canintf.tx0if || canintf.tx1if || canintf.tx2if || canintf.errif)
			{
				uint8_t mask = canintf.tx2if << 2 | canintf.tx1if <<1 | canintf.tx0if;
				mcp25625_bit_modify(CANINTF_ADDR, (mask << TX0IF_POS) | (canintf.errif << ERRIF_POS), 0);
			}
			//Request to send?
			if(rts_flags != 0)
				mcp25625_tx_request_to_send(rts_flags);
		}while(mcp25625_get_IRQ_flag());
		//TXxIF cleared by code, ERRIF cleared by code, RXxIF creared by READ RX BUFFER command
	}
}
