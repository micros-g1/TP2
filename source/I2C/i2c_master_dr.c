/*
 * i2c_master.c
 *
 *  Created on: 27 sep. 2019
 *      Author: Tomas
 */

#include <I2C/i2c_master_dr.h>
#include "MK64F12.h"
#include "stdlib.h"

#define I2C_CLK_FREQ	50000000

i2c_service_callback_t interruption_callback = NULL;

void i2c_dr_master_init(i2c_service_callback_t callback){
	SIM->SCGC4 |= SIM_SCGC4_I2C0(1);		//clock gating, feed clock to the module
	I2C_Type* i2c0_pos = (I2C_Type*) I2C0_BASE;
	//clock module is set to 50Mhz!!
	i2c0_pos->F = 0x2D;		//set baud rate to 78125Hz. mult == 0x0-> mult=1; scl div ==2D-> scl div=640; !!!
	i2c0_pos->C1 = 0xF8;	//IICEN = 1; IICIE = 1; MST = 1; TX = 1; TXAK = 1; RSTA = 0; WUEN = 0; DMAEN = 0.
	interruption_callback = callback;
}

void I2C0_IRQHandler(){
	interruption_callback();
}

/**************************************
****************I2Cx_S field***********
***************************************
*7		6		5	4	3		2	1		0
*TCF, IAAS, BUSY, ARBL, RAM, SRW, IICIF, RXAK
*/

bool i2c_dr_get_transfer_complete(){
	/*TCF
	 * Acknowledges a byte transfer; TCF is set on the completion of a byte transfer. This bit is valid only during
	or immediately following a transfer to or from the I2C module. TCF is cleared by reading the I2C data
	register in receive mode or by writing to the I2C data register in transmit mode.
	0 Transfer in progress
	1 Transfer complete
	 */
	return ((((I2C_Type*) I2C0_BASE)->S) >> 7) & 1U;
}

/*IAAS : for slave, not necessary!!!*/

bool i2c_dr_bus_is_busy(){
	//BUSY: 1 for bus busy. 0 for bus not busy
	return ((I2C_Type*) I2C0_BASE)->S |= 0x10;
}

// ARBL -> ARBITRATION LOST. NOT NECESSARY BECAUSE THE TRANSMISSION WILL ONLY BE FROM MASTER (MCU) TO SLAVE(MAGNETOMETER), READ AND WRITE.
//RAM -> RANGE ADDRESS MATCH.
// SRW -> FOR SLAVE ONLY!!

void i2c_dr_clear_iicif(){
	/* IICIF -- see i2c_dr_get_iicif also for more information
	 * This bit sets when an interrupt is pending. This bit must be cleared by software by writing 1 to it, such as in
	 * the interrupt routine.
	*/
	((I2C_Type*) I2C0_BASE)->S &= ~(1UL << 1);
}
bool i2c_dr_get_iicif(){
	/* IICIF -- see i2c_dr_clear_iicif also for more information
	 * This bit sets when an interrupt is pending. One of the following events can set this bit:
	 * One byte transfer, including ACK/NACK bit, completes if FACK is 0. An ACK or NACK is sent on the
	bus by writing 0 or 1 to TXAK after this bit is set in receive mode.
	 * One byte transfer, excluding ACK/NACK bit, completes if FACK is 1.
	 * Match of slave address to calling address including primary slave address, range slave address,
	alert response address, second slave address, or general call address.
	 * Arbitration lost
	 * In SMBus mode, any timeouts except SCL and SDA high timeouts
	 * I2C bus stop or start detection if the SSIE bit in the Input Glitch Filter register is 1
	*/
	return ((((I2C_Type*) I2C0_BASE)->S) >> 1) & 1U;
}

bool i2c_dr_get_rxak(){
	return (((I2C_Type*) I2C0_BASE)->S) & 1U;
}

/**************************************
************I2Cx_D field***************
***************************************
 * In master transmit mode, when data is written to this register, a data transfer is initiated. The most
significant bit is sent first. In master receive mode, reading this register initiates receiving of the next byte
of data.
NOTE: When making the transition out of master receive mode, switch the I2C mode before reading the
Data register to prevent an inadvertent initiation of a master receive data transfer.
*/
void i2c_dr_write_data(unsigned char data){
	((I2C_Type*) I2C0_BASE)->D = data;
}
unsigned char i2c_dr_read_data(){
	return ((I2C_Type*) I2C0_BASE)->D;
}


/**************************************
************I2Cx_FLT field*************
***************************************
*7		6		5		4	3,2,1,0
*SHEN, STOPF, SSIE, STARTF, FLT
*/

//STOPF ->FOR SLAVES ONLY!!!

void i2c_dr_set_start_stop_interrupt(bool enabled){

	/*SSIE -- I2C Bus Stop or Start Interrupt Enable
	* This bit enables the interrupt for I2C bus stop or start detection.
	* 		1 Stop or start detection interrupt is enabled
	* 		0 Stop or start detection interrupt is disabled
	 */
	unsigned char reg = ((I2C_Type*) I2C0_BASE)->FLT;
	(((I2C_Type*) I2C0_BASE)->FLT) ^= (-(unsigned char)enabled ^ reg) & (1UL << 5);
}

bool i2c_dr_get_startf(){
	/*STARTF -- see i2c_dr_get_startf for more information
	 * Hardware sets this bit when the I2C bus's start status is detected.
	 * 0 No start happens on I2C bus
	 * 1 Start detected on I2C bus
	 */
	return ((((I2C_Type*) I2C0_BASE)->FLT) >> 4) & 1U;
}

void i2c_dr_clear_startf(){
	/*STARTF -- see i2c_dr_get_startf for more information
	*The STARTF bit must be cleared by writing 1 to it.
	*/
	(((I2C_Type*) I2C0_BASE)->FLT) |= 1UL << 4;
}

