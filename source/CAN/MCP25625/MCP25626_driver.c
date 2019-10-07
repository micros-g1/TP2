/*
 * MCP25626_driver.c
 *
 *  Created on: 24 Sep 2019
 *      Author: grein
 */
#include <CAN/MCP25625/MCP25625_driver.h>
#include <string.h>
#include <SPI/spi_driver.h>
#include <gpio.h>
#include <hardware.h>
#include <Interrupts/interrupts.h>

//Array used for spi transactions.
//Size: Memory size + 1 read / write instruction + address worst case.
#define TEMP_ARRAY_BUFFER_LENGTH 128+2
static uint8_t temp_array[TEMP_ARRAY_BUFFER_LENGTH];
//Global flag to indicate if mcp25625 interrupt handling is enabled.
static bool interrupts_enabled = false;

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

static void mcp25625_internal_ISR(void);
mcp25625_driver_callback_t callback_isr;

static bool initialized = false;
void mcp25625_driver_init()
{
	if(initialized)
		return;
	initialized = true;
	//Set callback to null
	mcp25625_driver_set_callback(NULL);
	//Initialize interrupts
	interrupts_init();
	//Configure interrupt pin
	gpioMode(MCP25625_INTREQ_PIN, INPUT);
	mcp25625_driver_enable_interrupt_handling(true);
	//Configure SPI
	spi_driver_init();
	spi_set_clock_polarity(SPI_SCK_INACTIVE_LOW);
	spi_set_clock_phase(SPI_CPHA_CAP_IN_LEAD_CHANGE_FOLLOWING);
	spi_set_transfer_order(SPI_MSB_FIRST);
	spi_set_baud_rate_scaler(0);
	spi_set_baud_rate_prescaler(0x03);
	spi_set_double_baud_rate(true);
}

void mcp25625_reset(void)
{
	if(!initialized)
		return;
	uint8_t instruction = MCP_RESET;
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	spi_master_transfer_blocking((uint8_t*)&instruction, NULL, 1);
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_write(mcp25625_addr_t addr, size_t length, const uint8_t *p_data)
{
	if(!initialized)
			return;
	if(length <= TEMP_ARRAY_BUFFER_LENGTH-1)
	{
		bool int_bkp = interrupts_enabled;
		if(interrupts_enabled)
			mcp25625_driver_enable_interrupt_handling(false);
		temp_array[0] = MCP_WRITE;
		temp_array[1] = addr;
		memcpy(&temp_array[2], p_data, length);
		spi_master_transfer_blocking((uint8_t*)&temp_array,NULL,2+length);
		if(int_bkp)
			mcp25625_driver_enable_interrupt_handling(int_bkp);
	}
}

void mcp25625_read(mcp25625_addr_t addr, size_t length, uint8_t *p_data)
{
	if(!initialized)
			return;
	if(length <= TEMP_ARRAY_BUFFER_LENGTH-1)
	{
		bool int_bkp = interrupts_enabled;
		if(interrupts_enabled)
			mcp25625_driver_enable_interrupt_handling(false);
		temp_array[0] = MCP_READ;
		temp_array[1] = addr;
		spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,2+length);
		memcpy(p_data,&temp_array[2],length);
		if(int_bkp)
			mcp25625_driver_enable_interrupt_handling(int_bkp);
	}
}

void mcp25625_write_register(mcp25625_addr_t addr, uint8_t data)
{
	if(!initialized)
			return;
	mcp25625_write(addr, 1, &data);
}

uint8_t mcp25625_read_register(mcp25625_addr_t addr)
{
	if(!initialized)
			return 0x00;
	uint8_t data;
	mcp25625_read(addr, 1, &data);
	return data;
}

void mcp25625_bit_modify(mcp25625_addr_t addr, uint8_t mask, uint8_t data)
{
	if(!initialized)
			return;
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_BIT_MODIFY;
	temp_array[1] = addr;
	temp_array[2] = mask;
	temp_array[3] = data;
	spi_master_transfer_blocking((uint8_t*)&temp_array,NULL,4);
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_read_rx_buffer_id(mcp25625_rxb_id_t buffer_id, mcp25625_id_t *p_id)
{
	if(!initialized)
			return;
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
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,1+sizeof(*p_id));
	memcpy(p_id, &temp_array[1], sizeof(*p_id));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_load_tx_buffer_id(mcp25625_txb_id_t buffer_id, const mcp25625_id_t *p_id)
{
	if(!initialized)
			return;
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
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_id, sizeof(*p_id));
	spi_master_transfer_blocking((uint8_t*)&temp_array,NULL,1+sizeof(*p_id));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_read_rx_buffer_data(mcp25625_rxb_id_t buffer_id, mcp25625_data_t *p_data)
{
	if(!initialized)
			return;
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
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,1+sizeof(*p_data));
	memcpy(p_data, &temp_array[1], sizeof(*p_data));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_load_tx_buffer_data(mcp25625_txb_id_t buffer_id, const mcp25625_data_t *p_data)
{
	if(!initialized)
			return;
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
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_data, sizeof(*p_data));
	spi_master_transfer_blocking((uint8_t*)&temp_array,NULL,1+sizeof(*p_data));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_read_rx_buffer_id_data(mcp25625_rxb_id_t buffer_id, mcp25625_id_data_t *p_id_data)
{
	if(!initialized)
			return;
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
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_READ_RX_BUFFER + location;
	//we can't know beforehand how many bytes we need. grab them all.
	spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,1+sizeof(*p_id_data));
	memcpy(p_id_data, &temp_array[1], sizeof(*p_id_data));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_load_tx_buffer_id_data(mcp25625_txb_id_t buffer_id, const mcp25625_id_data_t *p_id_data)
{
	if(!initialized)
			return;
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
	//No need to transfer all bytes. Read frame info
	//and determine how many bytes are needed.
	size_t bytes_to_transfer = p_id_data->dlc.rtr? 0 : p_id_data->dlc.dlc;
	bytes_to_transfer = bytes_to_transfer >= 8? 8 : bytes_to_transfer;
	bytes_to_transfer += sizeof(mcp25625_id_t) + 1;
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_LOAD_TX_BUFFER + location;
	memcpy(&temp_array[1], p_id_data, bytes_to_transfer);
	spi_master_transfer_blocking((uint8_t*)&temp_array,NULL,1+bytes_to_transfer);
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

void mcp25625_tx_request_to_send(mcp25625_txb_rts_flag_t tx_rts_flags)
{
	if(!initialized)
			return;
	uint8_t instruction = MCP_RTS + tx_rts_flags;
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	spi_master_transfer_blocking(&instruction, NULL, 1);
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
}

mcp25625_status_t mcp25625_read_status(void)
{
	mcp25625_status_t status;
	if(!initialized)
	{
		status.rx0if = 0;
		status.rx1if = 0;
		status.tx0if = 0;
		status.tx1if = 0;
		status.tx2if = 0;
		status.txreq0 = 0;
		status.txreq1 = 0;
		status.txreq2 = 0;
		return status;
	}
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_READ_STATUS;
	spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,1+sizeof(status));
	memcpy(&status, &temp_array[1], sizeof(status));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
	return status;
}

mcp25625_rx_status_t mcp25625_read_rx_status(void)
{
	mcp25625_rx_status_t rx_status;
	if(!initialized)
	{
		rx_status.filter_match = 0;
		rx_status.msg_info = 0;
		rx_status.msg_type = 0;
		return rx_status;
	};
	bool int_bkp = interrupts_enabled;
	if(interrupts_enabled)
		mcp25625_driver_enable_interrupt_handling(false);
	temp_array[0] = MCP_RX_STATUS;
	spi_master_transfer_blocking((uint8_t*)&temp_array,(uint8_t*)&temp_array,1+sizeof(rx_status));
	memcpy(&rx_status, &temp_array[1], sizeof(rx_status));
	if(int_bkp)
		mcp25625_driver_enable_interrupt_handling(int_bkp);
	return rx_status;
}

void mcp25625_driver_enable_interrupt_handling(bool enabled)
{
	if(!initialized)
		return;
	if(enabled)
		gpioIRQ(MCP25625_INTREQ_PIN, GPIO_IRQ_MODE_LOGIC_0, &mcp25625_internal_ISR);
	else
		gpioIRQ(MCP25625_INTREQ_PIN, GPIO_IRQ_MODE_DISABLE, NULL);
	interrupts_enabled = enabled;
}

void mcp25625_driver_set_callback(mcp25625_driver_callback_t callback)
{
	//Set callback
	callback_isr = callback;
}

bool mcp25625_get_IRQ_flag(void)
{
	if(!initialized)
		return false;
	return !gpioRead(MCP25625_INTREQ_PIN);
}

void mcp25625_internal_ISR()
{
	//IRQ may go low and high during ISR. Prevent multiple ISR calls
	//Now, enable interrupts (callback may be time-demanding...)
	//__enable_irq();
	//Interrupt flag cleared by gpio.
	if(callback_isr != NULL)
		//Execute callback if callback given
		callback_isr();
	//Re-enable interrupts
}
