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
#include <stdlib.h>
/*-------------------------------------------
 ----------------DEFINES---------------------
 -------------------------------------------*/


/*-------------------------------------------
 ----------------GLOBAL_VARIABLES------------
 -------------------------------------------*/
i2c_modules_dr_t i2c_dr_modules[AMOUNT_I2C_DR_MOD] = {I2C1_DR_MOD, I2C1_DR_MOD, I2C2_DR_MOD};
i2c_module_int_t* i2cm_mods[AMOUNT_I2C_DR_MOD];

/*-------------------------------------------
 ---------STATIC_FUNCTION_DECLARATION--------
 -------------------------------------------*/
static void hardware_interrupt_routine(i2c_modules_dr_t mod_id);
static void i2c_master_int_reset(i2c_modules_dr_t mod_id);
static void handle_master_mode(i2c_modules_dr_t mod_id);
static void handle_tx_mode(i2c_modules_dr_t mod_id);
static void handle_rx_mode(i2c_modules_dr_t mod_id);

static void read_byte(i2c_module_int_t* mod);
static void write_byte(i2c_module_int_t* mod);



/*-------------------------------------------
 ----------FUNCTION_IMPLEMENTATION-----------
 -------------------------------------------*/
void i2c_master_int_init(i2c_module_id_int_t mod_id, i2c_module_int_t* mod){
	static bool initialized[AMOUNT_I2C_INT_MOD] = { false, false, false };
	if(initialized[mod_id]) return;

	mod->id = mod_id;
	i2c_dr_master_init(mod_id, hardware_interrupt_routine);
	i2cm_mods[mod_id] = mod;

	i2c_master_int_reset(mod_id);
	initialized[mod_id] = true;
}

static void i2c_master_int_reset(i2c_modules_dr_t mod_id){
	i2c_module_int_t* mod = i2cm_mods[mod_id];

	mod->starf_log_count = 0;
	mod->last_byte_transmitted = false;
	mod->last_byte_read = false;
	mod->second_2_last_byte_2_be_read = false;
	mod->to_be_written_length = 0;
	mod->written_bytes = 0;
	mod->to_be_read_length = 0;

	q_init(&(mod->buffer));
}

static void hardware_interrupt_routine(i2c_modules_dr_t mod_id){
	//the order of each call is important!!!
	i2c_module_int_t* mod = i2cm_mods[mod_id];
	if(i2c_dr_get_stopf(mod_id)){
		i2c_dr_clear_stopf(mod_id);
		i2c_dr_clear_iicif(mod_id);
		mod->starf_log_count = 0;
	}
	else if(i2c_dr_get_startf(mod_id)){
		i2c_dr_clear_startf(mod_id);
		i2c_dr_clear_iicif(mod_id);
		if( ( ++(mod->starf_log_count) ) > 1)		// repeated start!
			handle_master_mode(mod_id);
	}
	else{
		i2c_dr_clear_iicif(mod_id);
		handle_master_mode(mod_id);
	}
}

static void handle_master_mode(i2c_modules_dr_t mod_id){
	i2c_dr_get_tx_rx_mode(mod_id) ? handle_tx_mode(mod_id) : handle_rx_mode(mod_id);
}

static void handle_tx_mode(i2c_modules_dr_t mod_id){
	i2c_module_int_t* mod = i2cm_mods[mod_id];

	if(mod->last_byte_transmitted || i2c_dr_get_rxak(mod_id))
		i2c_dr_send_start_stop(mod_id, false);
	else if(mod->last_byte_transmitted && (mod->to_be_read_length > 0) ){
		i2c_dr_set_tx_rx_mode(mod_id, false);
		read_byte(mod);
	}
	else
		write_byte(mod);

}
static void handle_rx_mode(i2c_modules_dr_t mod_id){
	i2c_module_int_t* mod = i2cm_mods[mod_id];

	read_byte(mod);
	if(mod->last_byte_read)
		i2c_dr_send_start_stop(mod_id, false);
	/*ignoring the second to last by to be read condition because i m only reading from the magnetometer!!!*/

}

bool i2c_master_int_has_new_data(i2c_module_int_t* mod){
	return (q_length(&(mod->buffer)) > 0);
}

void i2c_master_int_read_data(i2c_module_int_t* mod, unsigned char* question, int len_question, int amount_of_bytes){

	if( len_question > 0 ){
		i2c_master_int_write_data(mod, question, len_question);
		mod->to_be_written[mod->to_be_written_length] = (mod->slave_address << 1) | 1u;
		(mod->to_be_written_length)++;
	}
	else
		i2c_master_int_write_data(mod, NULL, 0);

	mod->to_be_read_length = amount_of_bytes;
	mod->last_byte_read = false;
	mod->second_2_last_byte_2_be_read = (mod->to_be_read_length > 1);

}

int i2c_master_int_get_new_data_length(i2c_module_int_t* mod){
	return q_length(&(mod->buffer));
}

void i2c_master_int_get_new_data(i2c_module_int_t* mod, unsigned char* read_data, int amount_of_bytes){
	for (int i =0; i < amount_of_bytes; i++)
		read_data[i] = q_popfront(&(mod->buffer));
}

void i2c_master_int_write_data(i2c_module_int_t* mod, unsigned char* write_data, int amount_of_bytes){

	mod->to_be_written[0] = (mod->slave_address << 1) | 0u;
	for(int i = 0; i < amount_of_bytes; i++)
		mod->to_be_written[i+1] = write_data[i];

	mod->to_be_written_length =  amount_of_bytes + 1;
	mod->last_byte_transmitted = false;
	mod->written_bytes = 0;

	i2c_dr_set_tx_rx_mode(mod->id, true);
	i2c_dr_send_start_stop(mod->id, true);
	write_byte(mod);
}


void i2c_master_int_set_slave_add(i2c_module_int_t* mod, unsigned char slave_add){
	mod->slave_address = slave_add;
}

static void read_byte(i2c_module_int_t* mod){

	q_pushback(&(mod->buffer), i2c_dr_read_data(mod->id));

	mod->last_byte_read = (--(mod->to_be_read_length));
	mod->second_2_last_byte_2_be_read = mod->to_be_read_length > 1;
}

static void write_byte(i2c_module_int_t* mod){
	i2c_dr_write_data(mod->id, mod->to_be_written[mod->written_bytes]);
	mod->last_byte_transmitted = ( (++mod->written_bytes) >= mod->to_be_written_length );
}
