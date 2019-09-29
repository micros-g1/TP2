/**
 * @file i2c_master_interface.h
 * @author Grupo 1 Labo de Micros
 * @date 28 Sep 2019
 * @brief I2C Master Interface
 * @details
 * I2C Driver
 * This interface is prepared to
 * handle the communication with the magnetometer of the freedom board
 * there are many unhandled cases (this is not a generic i2c master interface as it does not handle multimaster!!)
 * Although not all multimaster cases are handled, some of them may be implemented for future upgrades/implementations.
 */
#ifndef I2C_I2C_MASTER_INTERFACE_H_
#define I2C_I2C_MASTER_INTERFACE_H_

void i2c_master_interface_init();
#endif /* I2C_I2C_MASTER_INTERFACE_H_ */
