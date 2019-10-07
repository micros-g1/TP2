/**
 * @file accelerometer.h
 * @author Grupo 1 Labo de Micros
 * @date 1 Octubre 2019
 * @brief Accelerometer interface
 * @details This interface is prepared to handle the communication with the magnetometer and accelerometer of the freedom board.
 * The interface will recollect new data from the sensors in a periodic fashion, so that if the user asks for data
 * faster than the refresh rate of the sensors, the data he/she/Apache Helicopter will be the same in at least two consecutive calls
 */

#ifndef ACCELEROMETER_ACCELEROMETER_H_
#define ACCELEROMETER_ACCELEROMETER_H_
#include "general.h"

/**
 * @typedef enum accel_data_options_t
 * @brief Data Types: Accelerometer or Magnetometer
 */
typedef enum {ACCEL_ACCEL_DATA, ACCEL_MAGNET_DATA} accel_data_options_t;
/**
 * @typedef enum accel_data_options_t
 * @brief Data Types: Accelerometer or Magnetometer
 */
typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} accel_raw_data_t;

/**
 * @brief Accelerometer and Magnetometer init.
 * @details Initialize the accelerometer and magnetometer interface
 * Has no effect when called twice with the same module (safe init).
 */
void accel_init();
/**
 * @brief Accelerometer and Magnetometer get last updated data.
 * @details Get the last update of the accelerometer and magnetometer sensors
 * Has no effect when called twice with the same module (safe init).
 */
accel_raw_data_t accel_get_last_data(accel_data_options_t data_option);



#endif /* ACCELEROMETER_ACCELEROMETER_H_ */
