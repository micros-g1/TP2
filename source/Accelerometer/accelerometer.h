/**
 * @file accelerometer.h
 * @author Grupo 1 Labo de Micros
 * @date 1 Octubre 2019
 * @brief Accelerometer interface
 * @details This interface is prepared to handle the communication with the magnetometer of the freedom board
 */

#ifndef ACCELEROMETER_ACCELEROMETER_H_
#define ACCELEROMETER_ACCELEROMETER_H_
#include "general.h"

typedef enum {ACCEL_X, ACCEL_Y, ACCEL_Z, ACCEL_ALL} accel_data_coords_t;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t z;
} accel_raw_data_t;

/**
 * @brief Accelerometer init.
 * @details Initialize the accelerometer interface
 * Has no effect when called twice with the same module (safe init).
 */
void accel_init();
/**
 * @brief I2C Master, get new data
 * @details I2C Master, get data from buffer.
 * Should know that there is new data to be read before calling this function!!!
 * ( See also i2c_master_int_has_new_data() )
 * @return new data from buffer.
 */
accel_raw_data_t accel_get_last_data();

#endif /* ACCELEROMETER_ACCELEROMETER_H_ */
