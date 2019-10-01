/**
 * @file i2c_master_interface.h
 * @author Grupo 1 Labo de Micros
 * @date 28 Sep 2019
 * @brief I2C Master Interface
 * @details
 * This interface is prepared to handle communications using any I2C module as a master.
 * there are many unhandled cases (this is not a generic i2c master interface as it does not handle multimaster!!)
 * Although not all multimaster cases are handled, some of them may be implemented for future upgrades/implementations.
 */
#ifndef I2C_I2C_MASTER_INT_H_
#define I2C_I2C_MASTER_INT_H_

#include <stdbool.h>
#include <stdint.h>
/**
 * @typedef enum i2c_modules_int_t
 * @brief I2C interface modules
 */
typedef enum {I2C0_INT_MOD, I2C1_INT_MOD, I2C2_INT_MOD, AMOUNT_I2C_INT_MOD} i2c_modules_int_t;

/**
 * @brief I2C MASTER INIT
 * @details Initialize I2C master interface.
 * Has no effect when called twice with the same module (safe init)
 * @param mod : module to be initialized.
 */
void i2c_master_int_init(i2c_modules_int_t mod);


/**
 * @brief I2C Master has new data
 * @details ask if a specific module has new data in the buffer that has not been read yet.
 * @param mod : I2C module to be asked if new data has been received.
 * @return *true* if new data has arrived to the buffer. *false* otherwise
 */
bool i2c_master_int_has_new_data(i2c_modules_int_t mod);

/**
 * @brief I2C Master Read Data
 * @details read data from a specific I2C module.
 * @param mod : I2C module from which data will be read.
 */
unsigned char i2c_master_int_read_data(i2c_modules_int_t mod);

#endif /* I2C_I2C_MASTER_INT_H_ */
