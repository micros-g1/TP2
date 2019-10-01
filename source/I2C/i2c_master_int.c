/*
 * i2c_master_interface.c
 *
 *  Created on: 28 sep. 2019
 *      Author: Tomas
 */
/*
 * This module implements the master branch of Figure 51-6 from the MK64 reference manual. Typical I2C interrupt routine from the MK64.
 */
#include <I2C/i2c_dr_master.h>
#include <I2C/i2c_master_int.h>

static int starf_log_count =0;
static bool last_byte_transmitted = false;
static bool last_byte_read = false;
static bool second_2_last_byte_2_be_read = false;
static i2c_modules_dr_t i2c_dr_modules [AMOUNT_I2C_DR_MOD]= { I2C0_DR_MOD, I2C1_DR_MOD, I2C2_DR_MOD };
static i2c_modules_dr_t curr_mod;

static bool buffer_has_new_data = false;

static void hardware_interrupt_routine();
void i2c_master_interface_reset();
static void handle_master_mode();
static void handle_tx_mode();
static void handle_rx_mode();

void i2c_master_int_init(i2c_modules_int_t mod){
	static bool initialized = false;
	if(initialized) return;
	curr_mod = i2c_dr_modules[mod];
	i2c_dr_master_init(curr_mod, hardware_interrupt_routine);
	i2c_master_interface_reset();
	initialized = true;
}

void i2c_master_int_reset(){
	starf_log_count = 0;
	last_byte_transmitted = false;
	last_byte_read = false;
	second_2_last_byte_2_be_read = false;
}

static void hardware_interrupt_routine(){
	//the order of each call is important!!!

	if(i2c_dr_get_startf(curr_mod)){
		i2c_dr_clear_startf(curr_mod);
		i2c_dr_clear_iicif(curr_mod);
		if((++starf_log_count)> 1)		// repeated start!
			handle_master_mode();
	}
	else
		i2c_dr_clear_iicif(curr_mod);
}

static void handle_master_mode(){
	i2c_dr_get_tx_rx_mode(curr_mod) ? handle_tx_mode(curr_mod) : handle_rx_mode(curr_mod);
}

static void handle_tx_mode(){

	if(last_byte_transmitted || i2c_dr_get_rxak(curr_mod))
		i2c_dr_send_start_stop(curr_mod, false);
	else if(end_of_address_cycle())
		//i2c_dr_write_data(data);
		;
	else{
		i2c_dr_set_tx_rx_mode(curr_mod, false);
		unsigned char new_data = i2c_dr_read_data(curr_mod);		//dummy read, should be inserted into the buffer!!!
		buffer_has_new_data = true;
	}
}
static void handle_rx_mode(){
	if(last_byte_read)
		i2c_dr_send_start_stop(curr_mod, false);
	/*ignoring the second to last by to be read condition because i m only reading from the magnetometer!!!*/

	unsigned char new_data = i2c_dr_read_data(curr_mod);			//should be inserted into the buffer!!!
	buffer_has_new_data = true;
}
bool i2c_master_int_has_new_data(i2c_modules_int_t mod){
	return buffer_has_new_data;
}

unsigned char i2c_master_int_read_data(i2c_modules_int_t mod){
	curr_mod = i2c_dr_modules[mod];
	return 0;
}


