/***************************************************************************//**
 * @file pc_interface.h
 * @brief Sends strings to PC
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/


#ifndef TP2_PC_INTERFACE_H
#define TP2_PC_INTERFACE_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define ROCHI_DEBUG

#ifdef ROCHI_DEBUG
#define PC_MAX_FREQ 10
#else
#define PC_MAX_FREQ 100 // 60 frames per second is the fastest ppl will notice so no point in going any faster
#endif

#define PC_MSG_LEN  7  // one byte for pckg type, one for id, one for angle type, one for sign, three for number


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief initializes pc interface
 */
void pc_init();

/**
 * @brief queues message to send to pc
 * @param data message to send, 0 terminated, with a max length of PC_MSG_LEN
 */
void pc_send(uint8_t * data);

/**
 * @brief call periodically so messages can be sent
 */
void pc_periodic(); // so it can send any messages it has on queue


#endif //TP2_PC_INTERFACE_H
