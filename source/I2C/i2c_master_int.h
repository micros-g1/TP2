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
#include "util/queue.h"

/**
 * @define MAX_WRITE_CHARS
 * @brief maximum amount of bytes that can be written in a single write message.
 */
#define MAX_WRITE_CHARS	8
/**
 * @typedef enum i2c_modules_int_t
 * @brief I2C interface modules
 */
typedef enum {I2C0_INT_MOD, I2C1_INT_MOD, I2C2_INT_MOD, AMOUNT_I2C_INT_MOD} i2c_module_id_int_t;

/**
 * @brief I2C MASTER INIT
 * @details Initialize I2C master interface.
 * Has no effect when called twice with the same module (safe init)
 * @param mod_id : module to be initialized.
 */
void i2c_master_int_init(i2c_module_id_int_t mod_id);

/**
 * @brief I2C Master Read Data
 * @details sends a read data package from a specific I2C module.
 * The read package will be preceded by a write package with information (for the slave)
 * about the type of reading that will be performed. If this package is not needed, then it will not be sent.
 * @param mod : I2C module from which data will be read.
 * @param question : reading information
 * @param len_question : amount of bytes question has. If 0, then the writing package will not be sent.
 * @param amount_of_bytes : amount of bytes to be received when the reading is performed.
 */
void i2c_master_int_read_data(i2c_module_id_int_t mod_id, unsigned char* question, int len_question, int amount_of_bytes);

/**
 * @brief I2C Master has new data
 * @details ask if a specific module has new data in the buffer that has not been read yet.
 * @param mod : I2C module to be asked if new data has been received.
 * @return *true* if new data has arrived to the buffer. *false* otherwise
 */
bool i2c_master_int_has_new_data(i2c_module_id_int_t mod_id);

/**
 * @brief I2C Master get new data length
 * @details ask if a specific module has new data in the buffer that has not been read yet.
 * The user has to know that new data has arrived. See also i2c_master_int_has_new_data() .
 * @param mod : I2C module to be asked the new data's length.
 * @return amount of bytes the new data has.
 */
int i2c_master_int_get_new_data_length(i2c_module_id_int_t mod_id);


/**
 * @brief I2C Master get new data
 * @details gets new data from the I2C buffer, following FIFO convention.
 * data obtained by the use of this function is deleted from the buffer.
 * @param mod : I2C module to be asked for the new data.
 * @param read_data : buffer in which new data should be loaded
 * @param amount_of_bytes : amount of bytes to read from buffer.
 * @return amount of bytes the new data has.
 */
void i2c_master_int_get_new_data(i2c_module_id_int_t mod_id, unsigned char* read_data, int amount_of_bytes);

/**
 * @brief I2C Master write data
 * @details
 * @param mod : I2C module that will write the data to the slave.
 * @param write_data : data that will be sent to the slave.
 * @param amount_of_bytes : amount of bytes that will be sent to the slave.
 */
void i2c_master_int_write_data(i2c_module_id_int_t mod_id, unsigned char* write_data, int amount_of_bytes);

/**
 * @brief I2C Master set slave address
 * @details
 * @param mod_id : I2C module which slave will be set.
 */
void i2c_master_int_set_slave_addr(i2c_module_id_int_t mod_id, unsigned char slave_add);

bool i2c_master_int_bus_busy(i2c_module_id_int_t mod_id);

#endif /* I2C_I2C_MASTER_INT_H_ */
