/***************************************************************************//**
 * @file board_can_network.h
 * @brief General definitions for board inclination app
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/

#ifndef TP2_BOARD_CAN_NETWORK_H
#define TP2_BOARD_CAN_NETWORK_H


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board_type.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_LEN_CAN_MSG 5 // in bytes, not counting terminator

#ifndef ROCHI_DEBUG
#define CAN_MAX_FREQ    20	// max N msgs per second to send
#else
#define CAN_MAX_FREQ    1
#endif

//	callback to be called with new messages
typedef void (*bn_callback_t )(uint8_t msg_id, uint8_t * can_data);

/**
 * @brief Initialize CAN network
 */
void bn_init();

/**
 * @brief Register callback for new messages
 * @param cb Function to be called when a new message is received, with the ID and data of the message
 */
void bn_register_callback(bn_callback_t cb);

/**
 * @brief Call periodically so messages are transmitted
 */
void bn_periodic();


/**
 * @brief Send message via to CAN network
 * @param msg_id Priority of the message
 * @param data chars to send, 0 terminated
 */
void bn_send(uint8_t msg_id, uint8_t * data);


#endif //TP2_BOARD_CAN_NETWORK_H
