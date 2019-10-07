/******************************************************************************
  @file     App.c
  @brief    Application functions
  @author	Grupo 1 - Labo de Micros 2019
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <CAN/CAN.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SPI/spi_driver.h>
#include "board_app.h"

#include "Accelerometer/accelerometer.h"
#include "pc_interface/UART/uart.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama una vez, al comienzo del programa */
int i;
void App_Init (void)
{
	ba_init();
	//DO INIT
//	uart_cfg_t conf;
//	conf.parity = false;		// true for using parity bit
//	conf.odd_parity = false;		// true for odd parity (if parity is true)
//	conf.eight_bit_word = true;	// true for eight bit words, false for nine
//	conf.interrupts = true;		// true to enable non blocking functionality
//	conf.baudrate = 9600;
//	uartInit(0, conf);
}

/* Función que se llama constantemente en un ciclo infinito */
bool init = true;
void App_Run (void)
{
	ba_periodic();
//	if(init){
//		accel_init();
//		init = !init;
//	}
//	accel_raw_data_t data_mag = accel_get_last_data(ACCEL_MAGNET_DATA);
//
//
//	uartWriteMsg(0, &data_mag, 6);
//	can_message_t message,rec_message;
//	message.fir.dlc = 1;
//	message.fir.frame_type = CAN_STANDARD_FRAME;
//	message.fir.rtr = false;
//	message.message_id = 0;
//	message.data[0] = 0xFF;
//	while(true)
//	{
//		CAN_send(&message);
//		CAN_send(&message);
//		CAN_get(&rec_message);
//		CAN_get(&rec_message);
//	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
