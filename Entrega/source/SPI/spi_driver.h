/**
 * @file SPI_driver.h
 * @author Grupo 1 Labo de Micros
 * @date 25 Sep 2019
 * @details
 * Todavia no tengo nada. Saludos
 * @see https://www.youtube.com/watch?v=dQw4w9WgXcQ
 */

#include <stdint.h>
#include <stdbool.h>

typedef enum{
	SPI_SCK_INACTIVE_LOW,
	SPI_SCK_INACTIVE_HIGH
}spi_cpol_t;

typedef enum{
	SPI_CPHA_CAP_IN_LEAD_CHANGE_FOLLOWING,
	SPI_CPHA_CHANGE_IN_LEAD_CAP_FOLLOWING
}spi_cpha_t;

typedef enum{
	SPI_LSB_FIRST,
	SPI_MSB_FIRST
}spi_transfer_order_t;

/**
 * @brief SPI initialization
 * @details configures the SPI module in master mode, using a default
 * set of configurations
 */
void spi_driver_init(void);
void spi_master_transfer_blocking(uint8_t * tx_data, uint8_t * rx_data, size_t length);
// CTAR CONFIG
void spi_set_double_baud_rate(bool double_br);
void spi_set_frame_size(uint8_t size);

//Clock configs
void spi_set_clock_polarity(spi_cpol_t csk_pol);
void spi_set_clock_phase(spi_cpha_t csk_phase);

//Transfer Order
void spi_set_transfer_order(spi_transfer_order_t order);

//Prescalers
void spi_set_pcs_to_sck_delay_prescaler(uint8_t delay_prescaler);
void spi_set_after_sck_delay_prescaler(uint8_t delay_prescaler);
void spi_set_after_transfer_prescaler(uint8_t delay_prescaler);
void spi_set_baud_rate_prescaler(uint8_t br_prescaler);
//Scalers
void spi_set_psc_to_sck_delay_scaler(uint8_t delay_scaler);
void spi_set_after_sck_delay_scaler(uint8_t delay_scaler);
void spi_set_after_transfer_delay_scaler(uint8_t delay_scaler);
void spi_set_baud_rate_scaler(uint8_t br_scaler);
