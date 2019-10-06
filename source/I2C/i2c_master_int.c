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
#include "gpio.h"
#include "MK64F12.h"

/*-------------------------------------------
 ----------------DEFINES---------------------
 -------------------------------------------*/
#define DEBUG_PIN 	PORTNUM2PIN (PC, 4)
#define DEBUG_READ	PORTNUM2PIN	(PD, 0)

/**
 * @typedef struct i2c_module_t
 * @brief I2C interface module information
 * @details To be used internally by the interface.
 * The user should not change its values and should only use the struct as an argument for an interface function!!!
 */
typedef struct{
	i2c_module_id_int_t id;

	int starf_log_count;

	//tx mode
	bool last_byte_transmitted;
	unsigned char to_be_written[MAX_WRITE_CHARS];
	unsigned char slave_address;		//bytes that will be written in the current write request

	int to_be_written_length;			//amount of bytes to be written in the current write request
	int written_bytes;

	bool rs_sent;
	//rx mode
	bool last_byte_read;
	bool second_2_last_byte_2_be_read;
	queue_t buffer;

	bool new_data;

	int to_be_read_length;
	bool bus_busy;

} i2c_module_int_t;

/*-------------------------------------------
 ----------------GLOBAL_VARIABLES------------
 -------------------------------------------*/
i2c_modules_dr_t i2c_dr_modules[AMOUNT_I2C_INT_MOD] = {I2C1_DR_MOD, I2C1_DR_MOD, I2C2_DR_MOD};
i2c_module_int_t i2cm_mods[AMOUNT_I2C_INT_MOD];

/*-------------------------------------------
 ---------STATIC_FUNCTION_DECLARATION--------
 -------------------------------------------*/
static void hardware_interrupt_routine(i2c_modules_dr_t mod_id);
static void i2c_master_int_reset(i2c_modules_dr_t mod_id);
static void handle_master_mode(i2c_modules_dr_t mod_id);
static void handle_tx_mode(i2c_module_id_int_t mod_id);
static void handle_rx_mode(i2c_module_id_int_t mod_id);

static void read_byte(i2c_module_id_int_t mod_id);
static void write_byte(i2c_module_id_int_t mod_id);
static void send_start_stop(i2c_module_id_int_t mod_id, bool start_stop);
/*-------------------------------------------
 ----------FUNCTION_IMPLEMENTATION-----------
 -------------------------------------------*/
void i2c_master_int_init(i2c_module_id_int_t mod_id){
	static bool initialized[AMOUNT_I2C_INT_MOD] = { false, false, false };
	if(initialized[mod_id]) return;

	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	mod->id = mod_id;
	i2c_dr_master_init(mod_id, hardware_interrupt_routine);

	i2c_master_int_reset(mod_id);
	gpioMode(DEBUG_PIN, OUTPUT);
	gpioMode(DEBUG_READ, OUTPUT);

	initialized[mod_id] = true;
}

static void i2c_master_int_reset(i2c_modules_dr_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	mod->new_data = false;
	mod->starf_log_count = 0;
	mod->last_byte_transmitted = false;
	mod->last_byte_read = false;
	mod->second_2_last_byte_2_be_read = false;
	mod->to_be_written_length = 0;
	mod->written_bytes = 0;
	mod->to_be_read_length = 0;
	mod->rs_sent = false;
	q_init(&(mod->buffer));
	mod->bus_busy = false;
}

static void hardware_interrupt_routine(i2c_modules_dr_t mod_id){
	//the order of each call is important!!!
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	if(i2c_dr_get_stopf(mod_id)){			//bus detected stop
		i2c_dr_clear_stopf(mod_id);
		i2c_dr_clear_iicif(mod_id);
		mod->starf_log_count = 0;
	}
	else if(i2c_dr_get_startf(mod_id)){		//bus detected start
		i2c_dr_clear_startf(mod_id);
		i2c_dr_clear_iicif(mod_id);

		if( ( ++(mod->starf_log_count) ) == 1)
			handle_master_mode(mod_id);
		else if( (mod->starf_log_count)  >= 2){	// repeated start!
			mod->rs_sent = true;				//lets the module know that a repeated start has been correctly sent.
			handle_master_mode(mod_id);
		}
	}
	else{										//base case, no start or stop detection.
		i2c_dr_clear_iicif(mod_id);
		handle_master_mode(mod_id);
	}
}

static void handle_master_mode(i2c_modules_dr_t mod_id){
	i2c_dr_get_tx_rx_mode(mod_id) ? handle_tx_mode(mod_id) : handle_rx_mode(mod_id);
}

static void handle_tx_mode(i2c_module_id_int_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	if( ((I2C0->S) >> 4) & 1U){					//FOR DEBUGGING ONLY!!!
		send_start_stop(mod_id, false);
	}

	//last byte transmitted or a NACK received when only sending information (no reading action will be performed before sending stop)
	if(( mod->last_byte_transmitted || i2c_dr_get_rxak(mod_id) ) && (mod->to_be_read_length == 0))
		send_start_stop(mod_id, false);

	/*last byte to be transmitted when a reading action will be performed is the address of the slave followed by a R bit.
	  Before this address, a repeated start should be sent!		*/
	else if( ( (mod->to_be_written_length - mod->written_bytes) == 1 ) && (mod->to_be_read_length > 0) ){
		if(!mod->rs_sent)
			i2c_dr_send_repeated_start(mod_id);			//sends the repeated start before sending the ADDR | R byte
		else
			write_byte(mod->id);						//sends the ADDR | R byte after the repeated start has been sent.
	}
	else if(mod->last_byte_transmitted){				//the ADDR | R byte has already been sent, so should change to RX mode.
		i2c_dr_set_tx_rx_mode(mod->id, false);
		read_byte(mod->id);								//dummy read : triggers the necessary clock cycles for the slave to transfer the first data byte.
	}
	else
		write_byte(mod->id);							//general case for TX mode : sending a byte of data.

}
static void handle_rx_mode(i2c_module_id_int_t mod_id){

	if(i2cm_mods[mod_id].last_byte_read){
		send_start_stop(mod_id, false);
		/*if we want to read N bytes from the slave, N+1 reading calls should be performed to get those bytes,
		 * the first reading call is a dummy read an is made when changing to RX mode from TX mode.
		 * the last reading call is performed AFTER the stop signal has been sent so as not to trigger any more clock cycles in the bus*/
		read_byte(mod_id);
		i2cm_mods[mod_id].new_data = true;
	}
	else{
		read_byte(mod_id);
	}
}

bool i2c_master_int_has_new_data(i2c_module_id_int_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];
	return mod->new_data;
}

void i2c_master_int_read_data(i2c_module_id_int_t mod_id, unsigned char* question, int len_question, int amount_of_bytes){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	if( len_question > 0 ){
		i2c_master_int_write_data(mod->id, question, len_question);
		mod->to_be_written[mod->to_be_written_length] = (mod->slave_address << 1) | 1u;
		(mod->to_be_written_length)++;
	}
	else{
		i2c_master_int_write_data(mod->id, NULL, 0);
		mod->to_be_written[0] = (mod->slave_address << 1) | 1u;
	}
	mod->to_be_read_length = amount_of_bytes;
	mod->last_byte_read = false;
	mod->second_2_last_byte_2_be_read = (mod->to_be_read_length > 1);

}

int i2c_master_int_get_new_data_length(i2c_module_id_int_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];
	return q_length(&(mod->buffer))-1;
}

void i2c_master_int_get_new_data(i2c_module_id_int_t mod_id, unsigned char* read_data, int amount_of_bytes){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];
	read_data[0] = q_popfront(&(mod->buffer));		//filtering dummy read.
	for (int i =0; i < amount_of_bytes; i++)
		read_data[i] = q_popfront(&(mod->buffer));
}

void i2c_master_int_write_data(i2c_module_id_int_t mod_id, unsigned char* write_data, int amount_of_bytes){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	i2c_master_int_reset(mod->id);
	mod->to_be_written[0] = (mod->slave_address << 1) | 0u;
	for(int i = 0; i < amount_of_bytes; i++)
		mod->to_be_written[i+1] = write_data[i];

	mod->to_be_written_length =  amount_of_bytes + 1;
	mod->last_byte_transmitted = false;
	mod->written_bytes = 0;

	i2c_dr_set_tx_rx_mode(mod->id, true);
	i2c_dr_set_start_stop_interrupt(mod->id, true);

	send_start_stop(mod->id, true);
}


void i2c_master_int_set_slave_addr(i2c_module_id_int_t mod_id, unsigned char slave_add){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];
	mod->slave_address = slave_add;
}

static void read_byte(i2c_module_id_int_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	//reading call number N from the N+1 calls that should be performed to read N bytes.
	if(mod->to_be_read_length == 1)
		i2c_dr_send_ack(mod_id, true);			//sends NACK for the N+1 byte
	else
		i2c_dr_send_ack(mod_id, false);
//	gpioToggle(DEBUG_PIN);
	unsigned char data = i2c_dr_read_data(mod->id);		//performs the reading action, dummy or not.
//	gpioToggle(DEBUG_PIN);
	q_pushback(&(mod->buffer), data);					//inserts the new data into buffer, dummy or not.

	mod->last_byte_read = !(--(mod->to_be_read_length));
	mod->second_2_last_byte_2_be_read = mod->to_be_read_length > 1;
}

static void write_byte(i2c_module_id_int_t mod_id){
	i2c_module_int_t* mod = &i2cm_mods[mod_id];

	i2c_dr_write_data(mod->id, mod->to_be_written[mod->written_bytes]);
	mod->last_byte_transmitted = ( (++mod->written_bytes) >= mod->to_be_written_length );
}

bool i2c_master_int_bus_busy(i2c_module_id_int_t mod_id){
	//both software and hardware should be free for the bus to be free!
	return i2cm_mods[mod_id].bus_busy || i2c_dr_bus_busy(mod_id);
}
static void send_start_stop(i2c_module_id_int_t mod_id, bool start_stop){
	i2cm_mods[mod_id].bus_busy = start_stop;				//informs the bus is busy (software)

	i2c_dr_send_start_stop(mod_id, start_stop);
}
