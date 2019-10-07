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
#include <CAN/CAN.h>
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
	CAN_init();
	can_filter_t filter;
	filter.id = 0x00;
	filter.mask = 0x00;
	filter.frame_type = CAN_STANDARD_FRAME;
	CAN_set_filter_config(filter);
	CAN_start();
	//DO INIT
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	can_message_t message,rec_message;
	message.header.dlc = 1;
	message.header.frame_type = CAN_STANDARD_FRAME;
	message.header.rtr = false;
	message.header.message_id = 0x5AA;
	message.data[0] = 0x00;
	bool message_available = false;
	while(true)
	{
		CAN_send(&message);
		message_available = CAN_message_available();
		CAN_get(&rec_message);
		message_available = CAN_message_available();
		message.data[0]++;
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


/*******************************************************************************
 ******************************************************************************/
