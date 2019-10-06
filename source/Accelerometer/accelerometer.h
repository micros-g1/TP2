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


typedef enum {ACCEL_ACCEL_DATA, ACCEL_MAGNET_DATA} accel_data_options_t;

typedef struct {
	int x;
	int y;
	int z;
} accel_raw_data_t;

/**
 * @brief Accelerometer init.
 * @details Initialize the accelerometer interface
 * Has no effect when called twice with the same module (safe init).
 */
void accel_init();

accel_raw_data_t accel_get_last_data(accel_data_options_t data_option);



#endif /* ACCELEROMETER_ACCELEROMETER_H_ */
