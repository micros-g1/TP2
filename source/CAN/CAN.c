/*
 * CAN.c
 *
 *  Created on: 24 Sep 2019
 *      Author: grein
 */

#include <CAN/CAN.h>
#include <CAN/MCP25625/MCP25625_driver.h>

static bool rolled_over = false;

static void convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message);
static void convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package);

void CAN_init()
{
	mcp25625_cnf1_t cnf1;
	mcp25625_cnf2_t cnf2;
	mcp25625_cnf3_t cnf3;
	mcp25625_rxb0ctrl_t rxb0ctrl;
	mcp25625_rxb0ctrl_t rxb1ctrl;
	mcp25625_canintf_t canintf;
	mcp25625_caninte_t caninte;
	//Reset
	mcp25625_reset();
	//Set bit time, 125kbit/sec target -> 8us Target bit length
	//TBIT = N*TQ. Let K =  2*(BRP<5:0> + 1), then
	//TQ = Tb/N = K/FOSC => K*N = TBIT*FOSC = 8us*16MHz = 128.
	//SYNC = 1 / PRSEG = 4 / PHSEG1 = 1 / PHSEG2 = 2 -> N = 8
	//Then K = 16. K = 2 * (BRP<5:0> + 1) => BRP<5:0> = K/2-1 = 7.
	//Set PRSEG time to 4xTQ
	cnf2.prseg = 3;	//(PRSEG<2:0> + 1) x TQ
	//Set PHSEG1 to 1xTQ
	cnf2.phseg1 = 0; //(PHSEG1<2:0> + 1) x TQ
	//Set PHSEG2 = 2;
	cnf2.btlmode = true;	//Length of PS2 is determined by the PHSEG2<2:0> bits of CNF3
	cnf3.phseg2 = 1;	//(PHSEG2<2:0> + 1) x TQ. Mimimum 2xTQ
	//Set Baud Rate Prescaler bits BRP<5:0> = 7.
	cnf1.brp = 7;

	//Write CNF1, CNF2, CNF3
	mcp25625_bit_modify(CNF1_ADDR, BRP, cnf1.register_byte);
	mcp25625_bit_modify(CNF2_ADDR, BLTMODE | PHSEG1 | PRSEG, cnf2.register_byte);
	mcp25625_bit_modify(CNF3_ADDR, PHSEG2, cnf3.register_byte);

	//Set reception filters and masks (reimplement this...) //Create driver function!
	can_message_t temp_can_msg;
	mcp25625_id_data_t temp_raw;
	temp_can_msg.fir.dlc = 0;
	temp_can_msg.fir.frame_type = CAN_STANDARD_FRAME;
	temp_can_msg.fir.rtr = 0;
	//Configure Mask
	temp_can_msg.message_id = ~0x07;
	convert_can_message_to_raw(&temp_can_msg, &temp_raw);
	//Write Masks
	mcp25625_write(RXM0_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXM1_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	temp_can_msg.message_id = 0;
	convert_can_message_to_raw(&temp_can_msg, &temp_raw);
	mcp25625_write(RXF0_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXF1_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXF2_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXF3_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXF4_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);
	mcp25625_write(RXF5_ADDR, sizeof(temp_raw.id), (uint8_t*) &temp_raw.id);

	//Only accept 0x000 to 0x007

	//TODO: What should I do here??

	//Set reception mode
	rxb0ctrl.register_byte = 0;
	rxb0ctrl.rxm = RECEIVE_VALID_MESSAGES;
	rxb1ctrl.register_byte = 0;
	rxb1ctrl.rxm = RECEIVE_VALID_MESSAGES;

	//Enable message roll over
	rxb0ctrl.bukt = true;
	rolled_over = false;

	//Write RXB0CTRL and RXB1CTRL
	mcp25625_write(RXB0_ADDR+CTRL_OFFST, 1, &rxb0ctrl.register_byte);
	mcp25625_write(RXB1_ADDR+CTRL_OFFST, 1, &rxb1ctrl.register_byte);

	//Clear flags, write CANINTF
	canintf.register_byte = 0;
	mcp25625_write(CANINTF_ADDR, 1, &canintf.register_byte);

	//Configure interrupt pin
	//TODO: Configure interrput

	//Enable interrupts in MCP25625, write CANINTE
	caninte.register_byte = 0;
	caninte.rx0ie = true;
	caninte.rx1ie = true;
	mcp25625_write(CANINTE_ADDR, 1, &caninte.register_byte);

	//Set to normal mode
	mcp25625_canstat_t canstat;
	canstat.opmode = NORMAL_OPERATION;
	mcp25625_bit_modify(CANSTAT_ADDR, OPMOD, canstat.register_byte);

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
				rolled_over = false;
				break;
			case RXB1_MSG:
				rolled_over = false;
				rxb_to_read = RXB1;
				break;
			case BOTH_RX_MSG:
				if(rolled_over)
				{
					rxb_to_read = RXB1;
					rolled_over = false;
				}
				else
				{
					rolled_over = true;
				}
			case NO_RX_MSG:
				break;
		}
		mcp25625_read_rx_buffer_id_data(rxb_to_read,&package_raw);
		convert_raw_to_can_message(&package_raw, p_message);
	}
	return message_available;
}

void CAN_set_rx_buffer_overflow_callback(CAN_rx_buffer_overflow_callback_t callback)
{

}

void CAN_set_message_callback(CAN_message_callback_t callback)
{

}

static void convert_raw_to_can_message(const mcp25625_id_data_t *p_raw_package, can_message_t *p_can_message)
{
	if(p_raw_package->id.sidl.ide)
	{
		p_can_message->fir.frame_type = CAN_EXTENDED_FRAME;
		p_can_message->message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->message_id <<= 3;
		p_can_message->message_id += p_raw_package->id.sidl.sid_l;
		p_can_message->message_id <<= 2;
		p_can_message->message_id += p_raw_package->id.sidl.eid_hh;
		p_can_message->message_id <<= 8;
		p_can_message->message_id += p_raw_package->id.eid8.eid_h;
		p_can_message->message_id <<= 8;
		p_can_message->message_id += p_raw_package->id.eid0.eid_l;
		p_can_message->fir.rtr = p_raw_package->data.dlc.rtr;
	}
	else
	{
		p_can_message->fir.frame_type = CAN_STANDARD_FRAME;
		p_can_message->message_id = p_raw_package->id.sidh.sid_h;
		p_can_message->message_id <<= 3;
		p_can_message->message_id += p_raw_package->id.sidl.sid_l;
		p_can_message->fir.rtr = p_raw_package->id.sidl.ssr;
	}
	unsigned int n_msg = p_raw_package->data.dlc.dlc;
	n_msg = n_msg <= 8? n_msg : 8;
	for(int i = 0 ; i < n_msg ; i++)
		p_can_message->data[i] = p_raw_package->data.buffer[i];
	p_can_message->fir.dlc = n_msg;
}

static void convert_can_message_to_raw(const can_message_t *p_can_message, mcp25625_id_data_t *p_raw_package)
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
