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
 * @brief Accelerometer, ask for data
 * @details Accelerometer, ask for data.
 * Loads new data into the accel buffer when this data is received.
 * see also accel_has_new_data() for asking for the data that was loaded to the buffer.
 * @param aked_data : data to be read.
 */
void accel_ask_for_data(accel_data_coords_t aked_data);
/**
 * @brief Accelerometer, ask for data
 * @details Accelerometer, ask if buffer has new data
 * ask if the buffer has new data to be read.
 * Should call accel_has_new_data before a call to this function.
 * See also accel_ask_for_data()
 * @param aked_data : data to be read.
 * @return *true* when the buffer has new data to be read, *false* otherwise.
 */
bool accel_has_new_data(accel_data_coords_t aked_data);

/**
 * @brief Accelerometer, ask for data
 * @details Accelerometer, get data from buffer.
 * Should know that there is new data to be read before calling this function!!!
 * ( See also accel_ask_for_data() )
 * @param aked_data : data to be read.
 * @return new data from buffer.
 */
unsigned char accel_get_new_data(accel_data_coords_t aked_data);

#endif /* ACCELEROMETER_ACCELEROMETER_H_ */
