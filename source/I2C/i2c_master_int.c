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
#include "util/queue.h"
/*-------------------------------------------
 ----------------DEFINES---------------------
 -------------------------------------------*/


/*-------------------------------------------
 ----------------GLOBAL_VARIABLES------------
 -------------------------------------------*/
i2c_modules_dr_t i2c_dr_modules[AMOUNT_I2C_DR_MOD] = {I2C1_DR_MOD, I2C1_DR_MOD, I2C2_DR_MOD};
/*-------------------------------------------
 ---------STATIC_FUNCTION_DECLARATION--------
 -------------------------------------------*/
static void hardware_interrupt_routine();
static void i2c_master_int_reset(i2c_module_int_t* mod);
static void handle_master_mode();
static void handle_tx_mode();
static void handle_rx_mode();
static bool end_of_address_cycle(i2c_module_int_t* mod);

/*-------------------------------------------
 ----------FUNCTION_IMPLEMENTATION-----------
 -------------------------------------------*/
void i2c_master_int_init(i2c_module_id_int_t mod_id, i2c_module_int_t* mod){
	static bool initialized[AMOUNT_I2C_INT_MOD] = { false, false, false };
	if(initialized[mod_id]) return;

	mod->id = mod_id;
	i2c_dr_master_init(mod_id, hardware_interrupt_routine);

	i2c_master_int_reset(mod);
	initialized[mod_id] = true;
}

static void i2c_master_int_reset(i2c_module_int_t* mod){
	mod->starf_log_count = 0;
	mod->last_byte_transmitted = false;
	mod->last_byte_read = false;
	mod->second_2_last_byte_2_be_read = false;
	mod->to_be_written_lengths = 0;
	mod->written_bytes = 0;
	mod->to_be_read_lenght = 0;
	mod->read_bytes = 0;
	mod->about_to_read = false;
	q_init(&(mod->buffer));
}

static void hardware_interrupt_routine(i2c_module_int_t* mod){
	//the order of each call is important!!!
	if(i2c_dr_get_stopf(mod->id)){
		i2c_dr_clear_stopf(mod->id);
		i2c_dr_clear_iicif(mod->id);
		mod->starf_log_count = 0;
	}
	else if(i2c_dr_get_startf(mod->id)){
		i2c_dr_clear_startf(mod->id);
		i2c_dr_clear_iicif(mod->id);
		if( ( ++(mod->starf_log_count) ) > 1)		// repeated start!
			handle_master_mode(mod);
	}
	else{
		i2c_dr_clear_iicif(mod->id);
		handle_master_mode(mod);
	}
}

static void handle_master_mode(i2c_module_int_t* mod){
	i2c_dr_get_tx_rx_mode(mod->id) ? handle_tx_mode(mod->id) : handle_rx_mode(mod->id);
}

static void handle_tx_mode(i2c_module_int_t* mod){

	if(mod->last_byte_transmitted || i2c_dr_get_rxak(mod->id))
		i2c_dr_send_start_stop(mod->id, false);
	else if(end_of_address_cycle(mod)){
		i2c_dr_set_tx_rx_mode(mod->id, false);
		q_pushback( &(mod->buffer), i2c_dr_read_data(mod->id) );
	}
	else{
		i2c_dr_write_data(mod->id, mod->to_be_written[mod->written_bytes]);
		if( (++mod->written_bytes) < mod->to_be_written_lengths )
			mod->last_byte_transmitted = true;
	}
}
static void handle_rx_mode(i2c_module_int_t* mod){
	if(mod->last_byte_read)
		i2c_dr_send_start_stop(mod->id, false);
	/*ignoring the second to last by to be read condition because i m only reading from the magnetometer!!!*/
	q_pushback(&(mod->buffer), i2c_dr_read_data(mod->id));
}

bool i2c_master_int_has_new_data(i2c_module_int_t* mod){
	return (q_length(&(mod->buffer)) > 0);
}

void i2c_master_int_read_data(i2c_module_int_t* mod, unsigned char* question, int len_question, int amount_of_bytes){

	if(len_question > 0){
		mod->about_to_read = true;
	}
	else{
		mod->about_to_read = false;
	}

}
int i2c_master_int_get_new_data_length(i2c_module_int_t* mod){
	return q_length(&(mod->buffer));
}


void i2c_master_int_get_new_data(i2c_module_int_t* mod, unsigned char* read_data, int amount_of_bytes){
	for (int i =0; i < amount_of_bytes; i++)
		read_data[i] = q_popfront(&(mod->buffer));
}

void i2c_master_int_write_data(i2c_module_int_t* mod, unsigned char* write_data, int amount_of_bytes){
	mod->last_byte_transmitted = false;
}

static bool end_of_address_cycle(i2c_module_int_t* mod){
	return false;
}


void i2c_master_int_set_slave_add(i2c_module_int_t* mod, unsigned char slave_add){
	mod->slave_address = slave_add;
}
