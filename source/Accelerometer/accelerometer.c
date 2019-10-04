/*
 * accelerometer.c
 *
 *  Created on: 1 oct. 2019
 *      Author: Tomas
 */


#include "accelerometer.h"
#include "board.h"
#include "I2C/i2c_master_int.h"
#include "MK64F12.h"
#include "util/SysTick.h"

#define ACCEL_ADDRESS	0x1D
#define ACCEL_DATA_PACK_LEN	7


// ACCEL I2C address
#define ACCEL_SLAVE_ADDR 0x1D


// ACCEL internal register addresses
#define ACCEL_STATUS 0x00
#define ACCEL_WHOAMI 0x0D
#define ACCEL_XYZ_DATA_CFG 0x0E
#define ACCEL_CTRL_REG1 0x2A
#define ACCEL_M_CTRL_REG1 0x5B
#define ACCEL_M_CTRL_REG2 0x5C
#define ACCEL_WHOAMI_VAL 0xC7

// number of bytes to be read from the ACCEL
#define ACCEL_READ_LEN 13 // status plus 6 channels =13 bytes
typedef enum { I2C_ERROR, I2C_OK} accel_errors_t;
/*
 * From the Freedom MK64F user manual:
 * An NXP FXOS8700CQ low-power, six-axis Xtrinsic sensor is interfaced through an I2C bus and two GPIO signals,
 * as shown in Table 5. By default, the I2C address is 0x1D.
*/

static accel_raw_data_t last_read_data;
static i2c_module_int_t i2c_module;
static void pin_config();
static void handling_reading();
static void handling_reading_calls();
static void handle_delay();
static void write_reg_and_wait(unsigned char reg, unsigned char data, int reload);

static void config_delay(int reload);
static void wait_for(int reload);

static accel_errors_t _mqx_ints_FXOS8700CQ_start();
static bool delaying = false;
static accel_errors_t config_ok;

void accel_init(){
	bool initialized = false;
	if(initialized) return;

	pin_config(ACCEL_SCL_PIN);
	pin_config(ACCEL_SDA_PIN);
	i2c_master_int_init(I2C0_INT_MOD, &i2c_module);

	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	systick_init();
	config_ok = I2C_ERROR;

	while(_mqx_ints_FXOS8700CQ_start() != I2C_ERROR);
//	systick_add_callback(handling_reading, 10, PERIODIC);			//more than 800 Hz!!!
//	systick_add_callback(handling_reading_calls, 20, PERIODIC);		//800 Hz!!!

	initialized = true;

}

// FROM  THE FXOS8700CQ REFERENCE MANUAL, SECTION 13.4
// function configures FXOS8700CQ combination accelerometer and magnetometer sensor
static accel_errors_t _mqx_ints_FXOS8700CQ_start(){

	unsigned char question = ACCEL_WHOAMI;

	i2c_master_int_set_slave_add(&i2c_module, ACCEL_SLAVE_ADDR);
	i2c_master_int_read_data(&i2c_module, &question, 1 , 1);


	unsigned char data[2] = {0, 0};
//	while(1);
	while(!i2c_master_int_has_new_data(&i2c_module)){
		wait_for(10000);
		if (i2c_master_int_get_new_data_length(&i2c_module) != 1)
			return (I2C_ERROR); // read and check the FXOS8700CQ WHOAMI register
		else {
			i2c_master_int_get_new_data(&i2c_module, data, 1);
			if(data[0] != 1)
				return (I2C_ERROR);
		}
	}


	wait_for(1000);

	write_reg_and_wait(ACCEL_M_CTRL_REG1, 0x00, 1000);

	write_reg_and_wait(ACCEL_M_CTRL_REG1, 0x1F, 1000);

	write_reg_and_wait(ACCEL_M_CTRL_REG2, 0x20, 1000);

	write_reg_and_wait(ACCEL_XYZ_DATA_CFG, 0x01, 1000);

	write_reg_and_wait(ACCEL_CTRL_REG1, 0x0D, 1000);


	systick_delete_callback(handle_delay);
	delaying = false;

	return I2C_OK;
}

static void pin_config(int pin){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);

	PORT_Type * addr_array[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_array[port_num];

	port->PCR[pin_num] = 0;
	port->PCR[pin_num] |= PORT_PCR_MUX(5);
	port->PCR[pin_num] |= 1 << PORT_PCR_ISF_SHIFT;
	port->PCR[pin_num] |= PORT_PCR_ODE_MASK;
	port->PCR[pin_num] |= PORT_PCR_PE_MASK;
	port->PCR[pin_num] |= PORT_PCR_PS(1);
}

accel_raw_data_t accel_get_last_data(){
	return last_read_data;
}

static void config_delay(int reload){
	static bool first_config = true;
	if(first_config){
		systick_add_callback(handle_delay, reload, SINGLE_SHOT);			//more than 800 Hz!!!
		first_config = false;
	}
	else
		systick_enable_callback(handle_delay);
	delaying = true;
}

static void handle_delay(){
	delaying = false;
}
static void handling_reading(){
	//reads the last ACC_DATA_PACK_LEN (exactly!!) amount of bytes from the i2c master buffer.
	while(i2c_master_int_has_new_data(I2C0_INT_MOD) && (i2c_master_int_get_new_data_length(I2C0_INT_MOD) >= ACCEL_DATA_PACK_LEN))
			i2c_master_int_get_new_data(I2C0_INT_MOD, (unsigned char*) &last_read_data, ACCEL_DATA_PACK_LEN);
}

static void handling_reading_calls(){
	i2c_master_int_read_data(I2C0_INT_MOD, NULL, 0, ACCEL_DATA_PACK_LEN);
}

static void write_reg_and_wait(unsigned char reg, unsigned char data, int reload){
	unsigned char reg_data[2] = {reg, data};
	i2c_master_int_write_data(&i2c_module, reg_data, 2);
	wait_for(reload);
}

static void wait_for(int reload){
	config_delay(reload);
	while(!delaying);
}
