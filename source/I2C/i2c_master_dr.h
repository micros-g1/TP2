/**
 * @file i2c_master_dr.h
 * @author Grupo 1 Labo de Micros
 * @date 24 Sep 2019
 * @brief I2C Master Driver
 * @details
 * I2C Driver
 * This driver is prepared to
 * handle the communication with the magnetometer of the freedom board
 * there are many unhandled cases (this is not a generic i2c master driver as it does not handle multimaster!!)
 * Although not all multimaster cases are handled, some of them may be implemented for future upgrades/implementations.
 */

#ifndef I2C_I2C_MASTER_DR_H_
#define I2C_I2C_MASTER_DR_H_
#include <stdbool.h>

/**
 * @typedef void (*i2c_service_callback_t)(void)
 * @brief I2C callback to be called whenever a hardware interrupt related to
 * the I2C master driver is called. See also i2c_dr_master_init().
 */
typedef void (*i2c_service_callback_t)(void);

/**
 * @brief I2C initialize master mode register configuration.
 * @details Initializes master internal configuration on the i2c module.
 * Sets a callback to be called whenever a hardware interrupt that is related
 * to this master is called.
 * Check the MK64 user manual (Table 51-5) for information on what hardware interrupts would be called
 * and the status, flag and enable they signify.
 * Uses the I2c0 module.
 * @param callback : callback to be called when a hardware interrupt is called.
 */
void i2c_dr_master_init(i2c_service_callback_t callback);
/**
 * @brief I2C Get Data Transfer Complete
 * @details Gets the current status of the last data transfer.
 * This call is valid only during or immediately following a transfer to or from the I2C module.
 * The Transfer status is cleared by reading the I2C data register in receive mode
 * or by writing to the I2C data register in transmit mode.
 * @return *false* when the last data transfer is still in progress. *true* when completed.
 */
bool i2c_dr_get_transfer_complete();
/**
 * @brief I2C Get the bus status
 * @details I2C Gets the bus status: Busy or not.
 * The bus may be busy when this or some other master is taking over it
 * (a START sequence has been sent but no STOP sequence has arrived yet).
 * @return *true* when an interrupt is pending, *false* otherwise.
 */
bool i2c_dr_bus_is_busy();
/**
 * @brief I2C Get the interrupt flag status
 * @details I2C Gets the interrupt status: pending or already handled.
 * If the flag is on, then a pending interrupt should be handled.
 * @return *true* when an interrupt is pending, *false* otherwise.
 */
bool i2c_dr_get_iicif();
/**
 * @brief I2C Clear the interrupt flag status
 * @details Should be called when handling an I2C master interrupt. See also i2c_dr_get_iicif().
 */
void i2c_dr_clear_iicif();
/**
 * @brief I2C Get the received ACK signal.
 * @details I2C Gets the received ACK signal from the slave.
 * If the ACK signal shows if there was a problem in the communication and
 * whether the slave is able to receive more information or not.
 * @return *false* if the ACK showed no problem, *true* otherwise.
 */
bool i2c_dr_get_rxak();

/**
 * @brief I2C Write data to the bus.
 * @details writes an entire byte to the bus buffer.
 */
void i2c_dr_write_data(unsigned char data);
/**
 * @brief I2C Read data from the bus.
 * @details Reads an entire byte from the bus buffer (sent from slave to this master).
 * Clears the Transfer status when called in receive mode. See also i2c_dr_get_transfer_complete()
 * @return the byte that was read.
 */
unsigned char i2c_dr_read_data();
/**
 * @brief I2C Enable or Disable Start and Stop Interrupts
 * @details Disables or enables the start or stop detection interrupts.
 * NOTE: To clear the I2C bus stop or start detection interrupt: In the interrupt service routine, first call
 * i2c_dr_clear_startf, and then call i2c_dr_clear_iicif.
 * If this sequence is reversed, the IICIF flag is asserted again.
 * @param enabled : true when the start stop interrupts should be enabled. False otherwise.
 */
void i2c_dr_set_start_stop_interrupt(bool enabled);
/**
 * @brief I2C Get the current status of the start interrupt flag
 * @details I2C Gets the current status of the start interrupt flag.
 * @return *true* if the flag is set, *false* otherwise.
 */
bool i2c_dr_get_startf();
/**
 * @brief I2C clear start interrupt flag
 * @details Clears the startf interrupt flag, see also i2c_dr_get_startf() .
 */
void i2c_dr_clear_startf();


#endif /* I2C_I2C_MASTER_DR_H_ */
