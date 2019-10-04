/******************************************************************************
  @file     App.c
  @brief    Application functions
  @author	Grupo 1 - Labo de Micros 2019
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board.h"
#include "general.h"
#include "I2C/i2c_master_int.h"
#include "Accelerometer/accelerometer.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
//static void pin_config(int pin);
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama una vez, al comienzo del programa */

bool init = false;
void App_Init (void)
{
	init = true;
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
	if(init){
		accel_init();
		init = false;
	}
	while(1);
	//DO LOOP
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
//static void pin_config(int pin){
//	int port_num = PIN2PORT(pin);
//	int pin_num = PIN2NUM(pin);
//
//	PORT_Type * addr_array[] = PORT_BASE_PTRS;
//	PORT_Type * port = addr_array[port_num];
//
//	port->PCR[pin_num] = 0;
//	port->PCR[pin_num] |= PORT_PCR_MUX(5);
//	port->PCR[pin_num] |= 1 << PORT_PCR_ISF_SHIFT;
//}

/*******************************************************************************
 ******************************************************************************/
