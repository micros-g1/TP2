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
#define ACCEL_DATA_PACK_LEN	13


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

static unsigned char reading_buffer[ACCEL_DATA_PACK_LEN];
static accel_raw_data_t last_read_data_mag;
static accel_raw_data_t last_read_data_acc;

static void pin_config();
static void handling_reading();
static void handling_reading_calls();
static void write_reg(unsigned char reg, unsigned char data);


static accel_errors_t start();

void accel_init(){
	bool initialized = false;
	if(initialized) return;

	pin_config(ACCEL_SCL_PIN);
	pin_config(ACCEL_SDA_PIN);
	i2c_master_int_init(I2C0_INT_MOD);

	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	systick_init();

	while(start() == I2C_ERROR);
	systick_add_callback(handling_reading, 10, PERIODIC);			//more than 800 Hz!!!
	systick_add_callback(handling_reading_calls, 20, PERIODIC);		//800 Hz!!!

	initialized = true;

}

// FROM  THE FXOS8700CQ REFERENCE MANUAL, SECTION 13.4
// function configures FXOS8700CQ combination accelerometer and magnetometer sensor
static accel_errors_t start(){

	unsigned char question = ACCEL_WHOAMI;

	i2c_master_int_set_slave_addr(I2C0_INT_MOD, ACCEL_SLAVE_ADDR);
	i2c_master_int_read_data(I2C0_INT_MOD, &question, 1 , 1);

	unsigned char data[2] = {0, 0};

	while(!i2c_master_int_has_new_data(I2C0_INT_MOD));

	if (i2c_master_int_get_new_data_length(I2C0_INT_MOD) != 1)
		return (I2C_ERROR); // read and check the FXOS8700CQ WHOAMI register
	else {
		i2c_master_int_get_new_data(I2C0_INT_MOD, data, 1);
		if(data[0] != ACCEL_WHOAMI_VAL)
			return (I2C_ERROR);
	}

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));
	write_reg(ACCEL_M_CTRL_REG1, 0x00);

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));
	write_reg(ACCEL_M_CTRL_REG1, 0x1F);

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));
	write_reg(ACCEL_M_CTRL_REG2, 0x20);

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));
	write_reg(ACCEL_XYZ_DATA_CFG, 0x01);

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));
	write_reg(ACCEL_CTRL_REG1, 0x0D);

	while(i2c_master_int_bus_busy(I2C0_INT_MOD));

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


static void handling_reading(){
	systick_disable_callback(handling_reading_calls);		//cant try to read now because that will empty the buffer!!!

	//reads the last ACC_DATA_PACK_LEN (exactly!!) amount of bytes from the i2c master buffer.
	while(i2c_master_int_has_new_data(I2C0_INT_MOD) && (i2c_master_int_get_new_data_length(I2C0_INT_MOD) >= ACCEL_DATA_PACK_LEN)){
		i2c_master_int_get_new_data(I2C0_INT_MOD, reading_buffer, ACCEL_DATA_PACK_LEN);

		//the first byte of the reading operation should be ignored!!
		//accelerometer data : serial... 14 bits
		last_read_data_acc.x = (int16_t)((reading_buffer[1] << 8) | reading_buffer[2])>> 2;
		last_read_data_acc.y = (int16_t)((reading_buffer[3] << 8) | reading_buffer[4])>> 2;
		last_read_data_acc.z = (int16_t)((reading_buffer[5] << 8) | reading_buffer[6])>> 2;

		//magnetometer data : serial... 16 bits
		last_read_data_acc.x = (reading_buffer[7] << 8) | reading_buffer[8];
		last_read_data_acc.y = (reading_buffer[9] << 8) | reading_buffer[10];
		last_read_data_acc.z = (reading_buffer[11] << 8) | reading_buffer[12];
	}
	systick_enable_callback(handling_reading_calls);		//can now try to read
}

static void handling_reading_calls(){

	unsigned char read_addr = ACCEL_STATUS;
	if(!i2c_master_int_bus_busy(I2C0_INT_MOD)){
		systick_disable_callback(handling_reading);		//cant update data while reading it!
		i2c_master_int_read_data(I2C0_INT_MOD, &read_addr, 1, ACCEL_DATA_PACK_LEN);
		systick_enable_callback(handling_reading);		//can now update data if necessary
	}
}

static void write_reg(unsigned char reg, unsigned char data){
	unsigned char reg_data[2] = {reg, data};
	i2c_master_int_write_data(I2C0_INT_MOD, reg_data, 2);
}

accel_raw_data_t accel_get_last_data(accel_data_options_t data_option){
	accel_raw_data_t returnable;

	systick_disable_callback(handling_reading);		//cant update data while reading it!

	if(data_option == ACCEL_ACCEL_DATA)
		returnable = last_read_data_acc;
	else if(data_option == ACCEL_MAGNET_DATA)
		returnable = last_read_data_mag;

	systick_enable_callback(handling_reading);		//can now update data if necessary

	return returnable;
}
