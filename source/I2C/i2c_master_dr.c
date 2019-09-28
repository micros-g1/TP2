/*
 * i2c_master.c
 *
 *  Created on: 27 sep. 2019
 *      Author: Tomas
 */

#include <I2C/i2c_master_dr.h>
#include "MK64F12.h"


#define I2C_CLK_FREQ	50000000


void i2c_master_init(){
	SIM->SCGC4 |= SIM_SCGC4_I2C0(1);		//clock gating, feed clock to the module
	I2C_Type* i2c0_pos = (I2C_Type*) I2C0_BASE;
	//clock module is set to 50Mhz!!
	i2c0_pos->F = 0x2D;		//set baud rate to 78125Hz. mult == 0x0-> mult=1; scl div ==2D-> scl div=640; !!!
	i2c0_pos->C1 = 0xF8;	//IICEN = 1; IICIE = 1; MST = 1; TX = 1; TXAK = 1; RSTA = 0; WUEN = 0; DMAEN = 0.
}

/**************************************
****************I2Cx_S field***********
***************************************
*7		6		5	4	3		2	1		0
*TCF, IAAS, BUSY, ARBL, RAM, SRW, IICIF, RXAK
*/
/*TCF
 * Acknowledges a byte transfer; TCF is set on the completion of a byte transfer. This bit is valid only during
or immediately following a transfer to or from the I2C module. TCF is cleared by reading the I2C data
register in receive mode or by writing to the I2C data register in transmit mode.
0 Transfer in progress
1 Transfer complete
 */
bool get_transfer_complete(){
	return ((((I2C_Type*) I2C0_BASE)->S) >> 7) & 1U;
}

/*IAAS : for slave, not necessary!!!*/

//BUSY: 1 for bus busy.
bool bus_is_busy(){
	return ((I2C_Type*) I2C0_BASE)->S |= 0x10;
}
// ARBL -> ARBITRATION LOST. NOT NECESSARY BECAUSE THE TRANSMISSION WILL ONLY BE FROM MASTER (MCU) TO SLAVE(MAGNETOMETER), READ AND WRITE.
//RAM -> RANGE ADDRESS MATCH.
// SRW -> FOR SLAVE ONLY!!
/* IICIF
 * This bit sets when an interrupt is pending. This bit must be cleared by software by writing 1 to it, such as in
the interrupt routine. One of the following events can set this bit:

• One byte transfer, including ACK/NACK bit, completes if FACK is 0. An ACK or NACK is sent on the
bus by writing 0 or 1 to TXAK after this bit is set in receive mode.
• One byte transfer, excluding ACK/NACK bit, completes if FACK is 1.
• Match of slave address to calling address including primary slave address, range slave address,
alert response address, second slave address, or general call address.
• Arbitration lost
• In SMBus mode, any timeouts except SCL and SDA high timeouts
• I2C bus stop or start detection if the SSIE bit in the Input Glitch Filter register is 1
*/
void clear_iicif(){
	((I2C_Type*) I2C0_BASE)->S &= ~(1UL << 1);
}
bool get_iicif(){
	return ((((I2C_Type*) I2C0_BASE)->S) >> 1) & 1U;
}

//RXAK -> Receive Acknowledge. 0: Acknowledge signal detected after the completion of one byte of data transmission on the bus.
bool get_rxak(){
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
void write_data(unsigned char data){
	((I2C_Type*) I2C0_BASE)->D = data;
}
unsigned char read_data(){
	return ((I2C_Type*) I2C0_BASE)->D;
}


/**************************************
************I2Cx_FLT field*************
***************************************
*7		6		5		4	3,2,1,0
*SHEN, STOPF, SSIE, STARTF, FLT
*/

//STOPF ->FOR SLAVES ONLY!!!

/*SSIE
* I2C Bus Stop or Start Interrupt Enable
* This bit enables the interrupt for I2C bus stop or start detection.
* NOTE: To clear the I2C bus stop or start detection interrupt: In the interrupt service routine, first clear the
* 		STOPF or STARTF bit by writing 1 to it, and then clear the IICIF bit in the status register.
* 		If this sequence is reversed, the IICIF bit is asserted again.
* 		1 Stop or start detection interrupt is enabled
* 		0 Stop or start detection interrupt is disabled
 */
void set_start_stop_interrupt(bool enabled){
	unsigned char reg = ((I2C_Type*) I2C0_BASE)->FLT;
	(((I2C_Type*) I2C0_BASE)->FLT) ^= (-(unsigned char)enabled ^ reg) & (1UL << 5);
}
/*STARTF
 * Hardware sets this bit when the I2C bus's start status is detected. The STARTF bit must be cleared by
writing 1 to it.
0 No start happens on I2C bus
1 Start detected on I2C bus
 */
bool get_startf(){
	return ((((I2C_Type*) I2C0_BASE)->FLT) >> 4) & 1U;
}

void clear_startf(){
	(((I2C_Type*) I2C0_BASE)->FLT) |= 1UL << 4;
}

