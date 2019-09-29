/*
 * i2c_master_interface.c
 *
 *  Created on: 28 sep. 2019
 *      Author: Tomas
 */
/*
 * This module implements the master branch of Figure 51-6. Typical I2C interrupt routine from the MK64.
 */
#include <I2C/i2c_dr_master.h>
#include "i2c_master_interface.h"

static int starf_log_count =0;
static bool last_byte_transmitted = false;
static bool last_byte_read = false;
static bool second_2_last_byte_2_be_read = false;

static void hardware_interrupt_routine();
void i2c_master_interface_reset();
static void handle_master_mode();
static void handle_tx_mode();
static void handle_rx_mode();

void i2c_master_interface_init(){
	static bool initialized = false;
	if(initialized) return;
	i2c_dr_master_init(hardware_interrupt_routine);
	i2c_master_interface_reset();
}

void i2c_master_interface_reset(){
	starf_log_count = 0;
	last_byte_transmitted = false;
	last_byte_read = false;
	second_2_last_byte_2_be_read = false;
}

static void hardware_interrupt_routine(){

	if(i2c_dr_get_startf()){
		//the order of each call is important!!!
		i2c_dr_clear_startf();
		i2c_dr_clear_iicif();
		if((++starf_log_count)> 1)		// repeated start!
			handle_master_mode();
	}
	else
		i2c_dr_clear_iicif();
}

static void handle_master_mode(){
	i2c_dr_get_tx_rx_mode() ? handle_tx_mode() : handle_rx_mode();
}

static void handle_tx_mode(){

	if(last_byte_transmitted || i2c_dr_get_rxak())
		i2c_dr_send_start_stop(false);
	else if(end_of_address_cycle())
		i2c_dr_write_data(data);
	else{
		i2c_dr_set_tx_rx_mode(false);
		unsigned char new_data = i2c_dr_read_data();		//dummy read, should be inserted into the buffer!!!
	}
}
static void handle_rx_mode(){
	if(last_byte_read)
		i2c_dr_send_start_stop(false);
	/*ignoring the second to last by to be read condition because i m only reading from the magnetometer!!!*/

	unsigned char new_data = i2c_dr_read_data();			//should be inserted into the buffer!!!
}
