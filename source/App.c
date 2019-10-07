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
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	ba_periodic();
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
