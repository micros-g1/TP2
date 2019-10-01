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
#include "Interrupts/SysTick.h"

#define ACC_ADDRESS	0x1D
/*
 * From the Freedom MK64F user manual:
 * An NXP FXOS8700CQ low-power, six-axis Xtrinsic sensor is interfaced through an I2C bus and two GPIO signals,
 * as shown in Table 5. By default, the I2C address is 0x1D.
*/

static void pin_config();

void accel_init(){
	bool initialized = false;
	if(initialized) return;

	i2c_master_int_init(I2C0_INT_MOD);

	pin_config(ACCEL_SCL_PIN);
	pin_config(ACCEL_SDA_PIN);

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

void accel_ask_for_data(accel_data_coords_t aked_data){

}

bool accel_has_new_data(accel_data_coords_t aked_data){
	return false;
}
unsigned char accel_get_new_data(accel_data_coords_t aked_data){
	return 0;
}
