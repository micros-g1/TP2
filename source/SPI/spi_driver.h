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

typedef struct{
	bool cont_pcs;
	uint8_t ctar_n;
	bool eoq;
	bool cont_clear;
	uint8_t pcs_assert;
	uint8_t data;
}spi_push_data_t;

typedef struct{
	uint8_t * tx_data;
	uint8_t * rx_data;
	uint16_t frames_to_transfer;
	uint16_t frames_completed;
}spi_transfer_data_t;

/**
 * @brief SPI initialization
 * @details configures the SPI module in master mode, using a default
 * set of configurations
 */
void spi_init(s);
void spi_push_frame(spi_push_data_t push_data);

void spi_master_transfer_blocking(spi_transfer_data_t * transfer_data);

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
