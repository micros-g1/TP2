/*
 * MCP25626_driver.c
 *
 *  Created on: 24 Sep 2019
 *      Author: grein
 */
#include <CAN/MCP25625/MCP25625_driver.h>
#include <string.h>

//This function represents an spi function to be implemented in the future
#define DEBUG_LENGTH 1024
static int i_deb = 0;
static uint8_t a_deb[DEBUG_LENGTH];
void spi_transaction(void *p_in, void *p_out, size_t length)
{if (i_deb+length+1>DEBUG_LENGTH) while(1); a_deb[i_deb++] = length ; memcpy(a_deb+i_deb,p_in,length) ; i_deb+= length;}

#define TEMP_ARRAY_BUFFER_LENGTH 128+2	//All memory + 1 read / write instruction + address worst case.
//Array used for spi transactions.
static uint8_t temp_array[TEMP_ARRAY_BUFFER_LENGTH];

/**
 * @enum mcp25625_instruction_t
 * @brief MCP25625 SPI Instructions.
 * @details Instruction Bytes for all operations
 */
typedef enum {

    MCP_RESET				= 0xC0,	///< Resets the internal registers to
									///		the default state, sets Configuration mode.
	MCP_READ				= 0x03,	///< Reads data from the register beginning
									///		at the selected address
	MCP_READ_RX_BUFFER		= 0x90,	///< When reading a receive buffer,
									///		reduces the overhead of a normal READ
									///		command by placing the Address Pointer at one
									///		of four locations, as indicated by ‘nm’
									/// 	(1001 0nm0)
	MCP_WRITE				= 0x02,	///< Writes data to the register beginning at the
									///		selected address.
	MCP_LOAD_TX_BUFFER		= 0x40,	///< When loading a transmit buffer, reduces the
									///		overhead of a normal WRITE command by placing
									///		the Address Pointer at one of six locations, as
									///		indicated by ‘abc’. (0100 0abc)
	MCP_RTS 				= 0x80,	///< Inst	ructs the controller to begin the message
									///		transmission sequence for any of the transmit
									///		buffers. 1000 0nnn -> nnn=(TXB2,TXB1,TXB0)
	MCP_READ_STATUS			= 0xA0,	///< Quick polling command that reads several Status
									///		bits for transmit and receive functions.
	MCP_RX_STATUS			= 0xB0,	///< Quick polling command that indicates a filter
									///		match and message type 	(standard, extended
									///		and/or remote) of the received message.
	MCP_BIT_MODIFY			= 0x05,	///< Allows the user to set or clear individual bits
									/// 	in a particular register. Not for all
									///		registers.
}mcp25625_instruction_t;

/**
 * @enum mcp25625_rx_read_locations_t
 * @brief Argument for MCP_READ_RX_BUFFER
 */
typedef enum
{
	RXB0_ID = 0,
	RXB0_DATA = 2,
	RXB1_ID = 4,
	RXB1_DATA = 6
}mcp25625_rx_read_locations_t;

/**
 * @enum mcp25625_tx_load_locations_t
 * @brief Argument for MCP_LOAD_TX_BUFFER
 */
typedef enum
{
	TXB0_ID = 0,
	TXB0_DATA = 1,
	TXB1_ID = 2,
	TXB1_DATA = 3,
	TXB2_ID = 4,
	TXB2_DATA = 5
}mcp25625_tx_load_locations_t;


void mcp25625_reset(void)
{
	uint8_t instruction = MCP_RESET;
	spi_transaction(&instruction, NULL, 1);
}

void mcp25625_write(mcp25625_addr_t addr, size_t length, const uint8_t *p_data)
{
	if(length > TEMP_ARRAY_BUFFER_LENGTH-1)
		length = TEMP_ARRAY_BUFFER_LENGTH-1;	//Crop amount of data to write. Sorry.
	temp_array[0] = MCP_WRITE;
	temp_array[1] = addr;
	memcpy(&temp_array[2], p_data, length);
	spi_transaction(&temp_array,NULL,2+length);
}

void mcp25625_read(mcp25625_addr_t addr, size_t length, uint8_t *p_data)
{
	if(length > TEMP_ARRAY_BUFFER_LENGTH-1)
		length = TEMP_ARRAY_BUFFER_LENGTH-1;	//Crop amount of data to write. Sorry.
	temp_array[0] = MCP_WRITE;
	temp_array[1] = addr;
	spi_transaction(&temp_array,&temp_array,2+length);
	memcpy(p_data,&temp_array[2],length);
}

void mcp25625_write_register(mcp25625_addr_t addr, uint8_t data)
{
	mcp25625_write(addr, 1, &data);
}

uint8_t mcp25625_read_register(mcp25625_addr_t addr)
{
	uint8_t data;
	mcp25625_read(addr, 1, &data);
	return data;
}

void mcp25625_bit_modify(mcp25625_addr_t addr, uint8_t mask, uint8_t data)
{
	temp_array[0] = MCP_BIT_MODIFY;
	temp_array[1] = addr;
	temp_array[2] = mask;
	temp_array[3] = data;
	spi_transaction(&temp_array,NULL,4);
}

void mcp25625_read_rx_buffer_id(mcp25625_rxb_id_t buffer_id, mcp25625_id_t *p_id)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case RXB0:
			location = RXB0_ID;
			break;
		case RXB1:
			location = RXB1_ID;
			break;
	}
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	spi_transaction(&temp_array,&temp_array,1+sizeof(*p_id));
	memcpy(&temp_array[1], p_id, sizeof(*p_id));
}

void mcp25625_load_tx_buffer_id(mcp25625_txb_id_t buffer_id, const mcp25625_id_t *p_id)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case TXB0:
			location = TXB0_ID;
			break;
		case TXB1:
			location = TXB1_ID;
			break;
		case TXB2:
			location = TXB2_ID;
			break;
	}
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_id, sizeof(*p_id));
	spi_transaction(&temp_array,NULL,1+sizeof(*p_id));
}

void mcp25625_read_rx_buffer_data(mcp25625_rxb_id_t buffer_id, mcp25625_data_t *p_data)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case RXB0:
			location = RXB0_DATA;
			break;
		case RXB1:
			location = RXB1_DATA;;
			break;
	}
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	spi_transaction(&temp_array,&temp_array,1+sizeof(*p_data));
	memcpy(&temp_array[1], p_data, sizeof(*p_data));
}

void mcp25625_load_tx_buffer_data(mcp25625_txb_id_t buffer_id, const mcp25625_data_t *p_data)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case TXB0:
			location = TXB0_DATA;
			break;
		case TXB1:
			location = TXB1_DATA;
			break;
		case TXB2:
			location = TXB2_DATA;
			break;
	}
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_data, sizeof(*p_data));
	spi_transaction(&temp_array,NULL,1+sizeof(*p_data));
}

void mcp25625_read_rx_buffer_id_data(mcp25625_rxb_id_t buffer_id, mcp25625_id_data_t *p_id_data)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case RXB0:
			location = RXB0_ID;
			break;
		case RXB1:
			location = RXB1_ID;
			break;
	}
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	spi_transaction(&temp_array,&temp_array,1+sizeof(*p_id_data));
	memcpy(&temp_array[1], p_id_data, sizeof(*p_id_data));
}

void mcp25625_load_tx_buffer_id_data(mcp25625_txb_id_t buffer_id, const mcp25625_id_data_t *p_id_data)
{
	mcp25625_rx_read_locations_t location;
	switch(buffer_id)
	{
		case TXB0:
			location = TXB0_ID;
			break;
		case TXB1:
			location = TXB1_ID;
			break;
		case TXB2:
			location = TXB2_ID;
			break;
	}
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_id_data, sizeof(*p_id_data));
	spi_transaction(&temp_array,NULL,1+sizeof(*p_id_data));
}

void mcp25625_tx_request_to_send(mcp25625_txb_rts_flag_t tx_rts_flags)
{
	uint8_t instruction = MCP_RTS + tx_rts_flags;
	spi_transaction(&instruction, NULL, 1);
}

mcp25625_status_t mcp25625_read_status(void)
{
	mcp25625_status_t status;
	temp_array[0] = MCP_READ_STATUS;
	spi_transaction(&temp_array,&temp_array,1+sizeof(status));
	memcpy(&temp_array[2], &status, sizeof(status));
	return status;
}

mcp25625_rx_status_t mcp25625_read_rx_status(void)
{
	mcp25625_rx_status_t rx_status;
	temp_array[0] = MCP_READ_STATUS;
	spi_transaction(&temp_array,&temp_array,1+sizeof(rx_status));
	memcpy(&temp_array[2], &rx_status, sizeof(rx_status));
	return rx_status;
}
