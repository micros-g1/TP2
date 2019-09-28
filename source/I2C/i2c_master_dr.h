/*
 * i2c_master.h
 *
 *  Created on: 27 sep. 2019
 *      Author: Tomas
 */
/*
 * This file is prepared to
 * handle the communication with the magnetometer of the freedom board
 * there are many unhandled cases (this is not a generic i2c master driver!!)
 */

#ifndef I2C_I2C_MASTER_DR_H_
#define I2C_I2C_MASTER_DR_H_
#include <stdbool.h>

void i2c_master_init();

bool get_transfer_complete();
bool bus_is_busy();
void clear_iicif();
bool get_iicif();
bool get_rxak();
void write_data(unsigned char data);
unsigned char read_data();
void set_start_stop_interrupt(bool enabled);
bool get_startf();
void clear_startf();


#endif /* I2C_I2C_MASTER_DR_H_ */
