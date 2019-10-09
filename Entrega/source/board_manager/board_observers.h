/***************************************************************************//**
 * @file board_observers.h
 * @brief Manages observers of board app
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/

#ifndef TP2_BOARD_OBSERVERS_H
#define TP2_BOARD_OBSERVERS_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board_type.h"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum {O_PC, O_CAN, N_OBSERVERS} observer_t; // list of available observers

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize board observers
 */
void bo_init();


/**
 * @brief Send data to observer. Non blocking
 * @param who The observer who will be notified
 * @param board_id Which board generated this event
 * @param angle_type Which angle was updated
 * @param angle_value New angle value
 */
void bo_notify_data(observer_t who, uint8_t board_id, angle_type_t angle_type, int32_t angle_value);

/**
 * @brief Send timeout notification to observer. Non blocking
 * @param who The observer who will be notified
 * @param board_id Which board generated this event
 */
void bo_notify_timeout(observer_t who, uint8_t board_id);

/**
 * @brief Call periodically so all pending messages can be sent
 */
void bo_periodic();



#endif //TP2_BOARD_OBSERVERS_H
