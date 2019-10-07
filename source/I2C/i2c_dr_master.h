/**
 * @file i2c_dr_master.h
 * @author Grupo 1 Labo de Micros
 * @date 28 Sep 2019
 * @brief I2C Master Driver
 * @details
 * I2C Driver
 * This driver should be used for the pin control of I2C master modules of the MK64F.
 * there are many unhandled cases (this is not a generic i2c master driver as it does not handle multimaster!!)
 */

#ifndef I2C_I2C_DR_MASTER_H_
#define I2C_I2C_DR_MASTER_H_
#include <stdbool.h>

/**
 * @typedef enum i2c_modules_dr_t
 * @brief I2C interface modules.
 */
typedef enum {I2C0_DR_MOD, I2C1_DR_MOD, I2C2_DR_MOD, AMOUNT_I2C_DR_MOD} i2c_modules_dr_t;

/**
 * @typedef void (*i2c_service_callback_t)(void)
 * @brief I2C callback to be called whenever a hardware interrupt related to
 * the I2C master driver is called. See also i2c_dr_master_init().
 */
typedef void (*i2c_service_callback_t)(i2c_modules_dr_t mod_id);

/**
 * @brief I2C initialize master mode register configuration.
 * @details Initializes master internal configuration on the i2c module.
 * Sets a callback for a specific I2C module
 * to be called whenever a hardware interrupt that is related to this master is called.
 * Check the MK64 user manual (Table 51-5) for information on what hardware interrupts would be called
 * and the status, flag and enable they signify.
 * Calling the init function twice has no effect (safe init).
 * @param mod : I2C module the callback is referring to.
 * @param callback : callback to be called when a hardware interrupt is called.
 */
void i2c_dr_master_init(i2c_modules_dr_t mod, i2c_service_callback_t callback);
/**
 * @brief I2C Driver get Tx or Rx mode.
 * @details Get the current data transfer mode for the specific I2C module:
 * Tx: Transferring data mode.
 * Rx: Reading data mode.
 * @param mod : I2C module from which to get the Tx or Rx mode.
 * @return *true* for tx mode, *false* for rx mode.
 */
bool i2c_dr_get_tx_rx_mode(i2c_modules_dr_t mod);

/**
 * @brief I2C Driver set Tx or Rx mode.
 * @details Sets the current data transfer mode for the master :
 * Tx: Transferring data mode.
 * Rx: Reading data mode.
 * @param mod : I2C module from which to set the Tx or Rx mode.
 * @param tx_mode : *true* for tx mode, *false* for rx mode.
 */
void i2c_dr_set_tx_rx_mode(i2c_modules_dr_t mod, bool tx_mode);
/**
 * @brief I2C Driver send Start or Stop signal.
 * @details send start or stop signal from a specific I2C module.
 * @param mod : I2C module from which to send the start or stop signal
 * @param start_stop : *true* for start signal, *false* for stop signal.
 */
void i2c_dr_send_start_stop(i2c_modules_dr_t mod, bool start_stop);

void i2c_dr_send_repeated_start(i2c_modules_dr_t mod);
/**
 * @brief I2C Get Data Transfer Complete
 * @details Gets the current status of the last data transfer.
 * This call is valid only during or immediately following a transfer to or from the I2C module.
 * The Transfer status is cleared by reading the I2C data register in receive mode
 * or by writing to the I2C data register in transmit mode.
 * @return *false* when the last data transfer is still in progress. *true* when completed.
 */
bool i2c_dr_get_transfer_complete(i2c_modules_dr_t mod);
/**
 * @brief I2C Get the bus status
 * @details I2C Gets the bus status: Busy or not.
 * The bus may be busy when this or some other master is taking over it
 * (a START sequence has been sent but no STOP sequence has arrived yet).
 * @return *true* when an interrupt is pending, *false* otherwise.
 */
bool i2c_dr_bus_is_busy(i2c_modules_dr_t mod);
/**
 * @brief I2C Get the interrupt flag status
 * @details I2C Gets the interrupt status: pending or already handled.
 * If the flag is on, then a pending interrupt should be handled.
 * @return *true* when an interrupt is pending, *false* otherwise.
 */
bool i2c_dr_get_iicif(i2c_modules_dr_t mod);
/**
 * @brief I2C Clear the interrupt flag status
 * @details Should be called when handling an I2C master interrupt. See also i2c_dr_get_iicif().
 */
void i2c_dr_clear_iicif(i2c_modules_dr_t mod);
/**
 * @brief I2C Get the received ACK signal.
 * @details I2C Gets the received ACK signal from the slave.
 * If the ACK signal shows if there was a problem in the communication and
 * whether the slave is able to receive more information or not.
 * @return *false* if the ACK showed no problem, *true* otherwise.
 */
bool i2c_dr_get_rxak(i2c_modules_dr_t mod);

/**
 * @brief I2C Write data to the bus.
 * @details writes an entire byte to the bus buffer.
 */
void i2c_dr_write_data(i2c_modules_dr_t mod, unsigned char data);
/**
 * @brief I2C Read data from the bus.
 * @details Reads an entire byte from the bus buffer (sent from slave to this master).
 * Clears the Transfer status when called in receive mode. See also i2c_dr_get_transfer_complete()
 * @return the byte that was read.
 */
unsigned char i2c_dr_read_data(i2c_modules_dr_t mod);
/**
 * @brief I2C Enable or Disable Start and Stop Interrupts
 * @details Disables or enables the start or stop detection interrupts.
 * NOTE: To clear the I2C bus stop or start detection interrupt: In the interrupt service routine, first call
 * i2c_dr_clear_startf, and then call i2c_dr_clear_iicif.
 * If this sequence is reversed, the IICIF flag is asserted again.
 * @param enabled : true when the start stop interrupts should be enabled. False otherwise.
 */
void i2c_dr_set_start_stop_interrupt(i2c_modules_dr_t mod, bool enabled);
/**
 * @brief I2C Get the current status of the start interrupt flag
 * @details I2C Gets the current status of the start interrupt flag.
 * @return *true* if the flag is set, *false* otherwise.
 */
bool i2c_dr_get_startf(i2c_modules_dr_t mod);
/**
 * @brief I2C clear start interrupt flag
 * @details Clears the startf interrupt flag, see also i2c_dr_get_startf() .
 * @param mod : I2C module that should clear its starf flag.
 */
void i2c_dr_clear_startf(i2c_modules_dr_t mod);

/**
 * @brief I2C Get the current status of the stop interrupt flag
 * @details I2C Gets the current status of the stop interrupt flag.
 * @param mod : I2C module to get the flag status from.
 * @return *true* if the flag is set, *false* otherwise.
 */
bool i2c_dr_get_stopf(i2c_modules_dr_t mod);
/**
 * @brief I2C clear stop interrupt flag
 * @details Clears the stopf interrupt flag, see also i2c_dr_get_stopf() .
 * @param mod : I2C module that should clear its stopf flag.
 */
void i2c_dr_clear_stopf(i2c_modules_dr_t mod);

/**
 * @brief I2C send an ACK or NACK signal
 * @details Sends an ACK or NACK signal if the timing of that ACK/NACK is correct.
 * @param mod : I2C module that should send the ACK/NACK signal.
 * @param ack_value : *true* to send NACK, *false* to send ACK.
 *
 */
void i2c_dr_send_ack(i2c_modules_dr_t mod, bool ack_value);

/**
 * @brief I2C get Master/Slave status.
 * @details Gets the current status of the I2C module. Slave (no START signal has been send by the module)
 * or Master.
 * @param mod : I2C module to get the current status from
 * @return *true* for MASTER mode. *false* for Slave mode.
 */
bool  i2c_dr_get_mst(i2c_modules_dr_t mod);

/**
 * @brief I2C BUS is busy
 * @details Check the status of the bus BUSY flag.
 * IMPORTANT : This flag does not update immediately after a start signal has been been detected on the bus,
 * so, in case the user wants to keep updated in real on the state of the bus,
 * he/she/it/Apache Helicopter should implement a software layer that either sets a small delay after sending a start signal
 * or solves this problem in any other way.
 * @param mod : I2C module to get the bus status from.
 * @return *true* if the bus is busy. *false* otherwise.
 */
bool i2c_dr_bus_busy(i2c_modules_dr_t mod);



#endif /* I2C_I2C_DR_MASTER_H_ */
