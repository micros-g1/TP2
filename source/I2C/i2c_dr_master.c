/*
 * i2c_master.c
 *
 *  Created on: 27 sep. 2019
 *      Author: Tomas
 */

#include <I2C/i2c_dr_master.h>
#include "MK64F12.h"
#include "stdlib.h"
#include "general.h"

#define I2C_CLK_FREQ	50000000

i2c_service_callback_t interruption_callback[AMOUNT_I2C_DR_MOD] = {NULL, NULL, NULL};
static I2C_Type* const i2c_dr_modules [AMOUNT_I2C_DR_MOD]= { I2C0, I2C1, I2C2 };
static void clock_gating_mod(i2c_modules_dr_t mod);

void i2c_dr_master_init(i2c_modules_dr_t mod, i2c_service_callback_t callback){
	static bool initialized[AMOUNT_I2C_DR_MOD] = {false, false, false};
	if(initialized[mod]) return;

	clock_gating_mod(mod);

	I2C_Type* i2c_pos = i2c_dr_modules[mod];

	//clock module is set to 50Mhz!!
//	i2c_pos->F = 0x2D;		//set baud rate to 78125Hz. mult == 0x0-> mult=1; scl div ==2D-> scl div=640; !!!
//	i2c_pos->F = 0x28;		//set baud rate to 156250Hz. mult == 0x0-> mult=1; scl div ==28-> scl div=320; !!!
	i2c_pos->F = 0x2E;		//set baud rate to Hz. mult == 0x0-> mult=1; scl div ==2E-> scl div=768; !!!

//	i2c_pos->F = I2C_F_ICR(0x20) | I2C_F_MULT(0x2);
	i2c_pos->C1 = 0xC0;	//IICEN = 1; IICIE = 1; MST = 0; TX = 0; TXAK = 1; RSTA = 0; WUEN = 0; DMAEN = 0.
	(i2c_pos->C2) &= ~(1UL << 3);	//RMEN = 0

	interruption_callback[mod] = callback;
//	i2c_pos->SMB |= 1UL << 7;		//FACK to 1.

	i2c_pos->FLT |= I2C_FLT_SSIE_MASK;
	uint32_t irq_interrupts[] = I2C_IRQS;//get the module interrupt
	NVIC_EnableIRQ(irq_interrupts[mod]);	//enable the module interrupt.

	i2c_dr_clear_startf(mod);
	initialized[mod] = true;
}
static void clock_gating_mod(i2c_modules_dr_t mod){

	SIM->SCGC4 |= SIM_SCGC4_I2C0(1);		//clock gating, feed clock to the module
	if(mod == I2C0_DR_MOD)
		SIM->SCGC4 |= SIM_SCGC4_I2C0(1);		//clock gating, feed clock to the module
	else if(mod == I2C1_DR_MOD)
		SIM->SCGC4 |= SIM_SCGC4_I2C1(1);		//clock gating, feed clock to the module

	else if(mod == I2C2_DR_MOD)
		SIM->SCGC1 |= SIM_SCGC1_I2C2(1);		//clock gating, feed clock to the module

}
void I2C0_IRQHandler(){
	interruption_callback[I2C0_DR_MOD](I2C0_DR_MOD);
}
void I2C1_IRQHandler(){
	interruption_callback[I2C1_DR_MOD](I2C1_DR_MOD);
}
void I2C2_IRQHandler(){
	interruption_callback[I2C2_DR_MOD](I2C2_DR_MOD);
}
/**************************************
****************I2Cx_C1 field***********
***************************************
*7		6		5	4	3		2	1		0
*IICEN, IICIE, MST, TX, TXAK, RSTA, WUEN, DMAEN
*/
/*
 * Master Mode Select
 * When MST is changed from 0 to 1, a START signal is generated on the bus and master mode is selected.
 * When this bit changes from 1 to 0, a STOP signal is generated and the mode of operation changes from
 * master to slave.
 * 0 Slave mode
 * 1 Master mode
 */

bool i2c_dr_get_tx_rx_mode(i2c_modules_dr_t mod){
	return ((i2c_dr_modules[mod]->C1) >> 4) & 1U;
}
bool  i2c_dr_get_mst(i2c_modules_dr_t mod){
	return ((i2c_dr_modules[mod]->C1) >> 5) & 1U;
}
void i2c_dr_set_tx_rx_mode(i2c_modules_dr_t mod, bool tx_mode){
	unsigned char word = i2c_dr_modules[mod]->C1;
	(i2c_dr_modules[mod]->C1) ^= (-(unsigned char)tx_mode ^ word) & (1U << 4);
}

void i2c_dr_send_start_stop(i2c_modules_dr_t mod, bool start_stop){
	 /* The internal register change from:
	 * Master to slave when sending a stop signal.
	 * Slave to Master when sending a start signal. */
	unsigned char word = i2c_dr_modules[mod]->C1;
	(i2c_dr_modules[mod]->C1) ^= (-(unsigned char)start_stop ^ word) & (1U << 5);

//	(i2c_dr_modules[mod]->C1) ^= (-(unsigned char)word ^ start_stop) & (1U << 5);
}

void i2c_dr_send_repeated_start(i2c_modules_dr_t mod){
	(i2c_dr_modules[mod]->C1) |= 1UL << 2;
}
void i2c_dr_send_ack(i2c_modules_dr_t mod, bool ack_value){
	unsigned char word = i2c_dr_modules[mod]->C1;
	(i2c_dr_modules[mod]->C1) ^= (-(unsigned char)ack_value ^ word) & (1U << 3);
}


/**************************************
****************I2Cx_S field***********
***************************************
*7		6		5	4	3		2	1		0
*TCF, IAAS, BUSY, ARBL, RAM, SRW, IICIF, RXAK
*/

bool i2c_dr_get_transfer_complete(i2c_modules_dr_t mod){
	/*TCF
	 * Acknowledges a byte transfer; TCF is set on the completion of a byte transfer. This bit is valid only during
	or immediately following a transfer to or from the I2C module. TCF is cleared by reading the I2C data
	register in receive mode or by writing to the I2C data register in transmit mode.
	0 Transfer in progress
	1 Transfer complete
	 */
	return ((i2c_dr_modules[mod]->S) >> 7) & 1U;
}
bool i2c_dr_bus_busy(i2c_modules_dr_t mod){
	return ((i2c_dr_modules[mod]->S) >> 5) & 1U;
}

/*IAAS : for slave, not necessary!!!*/

// ARBL -> ARBITRATION LOST. NOT NECESSARY BECAUSE THE TRANSMISSION WILL ONLY BE FROM MASTER (MCU) TO SLAVE(MAGNETOMETER), READ AND WRITE.
//RAM -> RANGE ADDRESS MATCH.
// SRW -> FOR SLAVE ONLY!!

void i2c_dr_clear_iicif(i2c_modules_dr_t mod){
	/* IICIF -- see i2c_dr_get_iicif also for more information
	 * This bit sets when an interrupt is pending. This bit must be cleared by software by writing 1 to it, such as in
	 * the interrupt routine.
	*/
	(i2c_dr_modules[mod]->S) |= I2C_S_IICIF_MASK;

}
bool i2c_dr_get_iicif(i2c_modules_dr_t mod){
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
	return ((i2c_dr_modules[mod]->S) >> 1) & 1U;
}

bool i2c_dr_get_rxak(i2c_modules_dr_t mod){
	return (i2c_dr_modules[mod]->S) & 1U;
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
void i2c_dr_write_data(i2c_modules_dr_t mod, unsigned char data){
	i2c_dr_modules[mod]->D = data;
}
unsigned char i2c_dr_read_data(i2c_modules_dr_t mod){
	unsigned char data = i2c_dr_modules[mod]->D;
	data = i2c_dr_modules[mod]->D;
	return data;
}

/**************************************
************I2Cx_FLT field*************
***************************************
*7		6		5		4	3,2,1,0
*SHEN, STOPF, SSIE, STARTF, FLT
*/

//STOPF ->FOR SLAVES ONLY!!!

void i2c_dr_set_start_stop_interrupt(i2c_modules_dr_t mod, bool enabled){

	/*SSIE -- I2C Bus Stop or Start Interrupt Enable
	* This bit enables the interrupt for I2C bus stop or start detection.
	* 		1 Stop or start detection interrupt is enabled
	* 		0 Stop or start detection interrupt is disabled
	 */
	unsigned char reg = ((I2C_Type*) I2C0_BASE)->FLT;
	(i2c_dr_modules[mod]->FLT) ^= (-(unsigned char)enabled ^ reg) & (1UL << 5);
}

bool i2c_dr_get_startf(i2c_modules_dr_t mod){
	/*STARTF -- see i2c_dr_get_startf for more information
	 * Hardware sets this bit when the I2C bus's start status is detected.
	 * 0 No start happens on I2C bus
	 * 1 Start detected on I2C bus
	 */
	return ((i2c_dr_modules[mod]->FLT) >> 4) & 1U;
}

void i2c_dr_clear_startf(i2c_modules_dr_t mod){
	/*STARTF -- see i2c_dr_get_startf for more information
	*The STARTF bit must be cleared by writing 1 to it.
	*/
	i2c_dr_modules[mod]->FLT |= I2C_FLT_STARTF(1);
}

bool i2c_dr_get_stopf(i2c_modules_dr_t mod){
	return ((i2c_dr_modules[mod]->FLT) >> 6) & 1U;

}

void i2c_dr_clear_stopf(i2c_modules_dr_t mod){
	(i2c_dr_modules[mod]->FLT) |= 1UL << 6;
}






