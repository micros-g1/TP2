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

#define ACC_ADDRESS	0x1D
#define ACC_DATA_PACK_LEN	7
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

void accel_init(){
	bool initialized = false;
	if(initialized) return;


	i2c_master_int_init(I2C0_INT_MOD, &i2c_module);
	pin_config(ACCEL_SCL_PIN);
	pin_config(ACCEL_SDA_PIN);

	systick_init();
	systick_add_callback(handling_reading, 10, PERIODIC);			//more than 800 Hz!!!
	systick_add_callback(handling_reading_calls, 20, PERIODIC);		//800 Hz!!!
	initialized = true;
}


static void pin_config(int pin){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);

	PORT_Type * addr_array[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_array[port_num];

	port->PCR[pin_num] = 0;
	port->PCR[pin_num] |= PORT_PCR_MUX(5);
	port->PCR[pin_num] |= 1 << PORT_PCR_ISF_SHIFT;
}

accel_raw_data_t accel_get_last_data(){
	return last_read_data;
}

static void handling_reading(){
	//reads the last ACC_DATA_PACK_LEN (exactly!!) amount of bytes from the i2c master buffer.
	while(i2c_master_int_has_new_data(I2C0_INT_MOD) && (i2c_master_int_get_new_data_length(I2C0_INT_MOD) >= ACC_DATA_PACK_LEN))
			i2c_master_int_get_new_data(I2C0_INT_MOD, (unsigned char*) &last_read_data, ACC_DATA_PACK_LEN);
}

static void handling_reading_calls(){
	i2c_master_int_read_data(I2C0_INT_MOD, NULL, 0, ACC_DATA_PACK_LEN);
}
