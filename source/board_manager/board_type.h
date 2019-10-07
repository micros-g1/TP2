/***************************************************************************//**
 * @file board_type.h
 * @brief General definitions for board inclination app
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/
#ifndef TP2_BOARD_TYPE_H
#define TP2_BOARD_TYPE_H

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "../util/clock.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//#define ROCHI_DEBUG // debugging

// standard angle type coding for external messages
#define PITCH_CHAR  'c'
#define ROLL_CHAR   'r'
#define OR_CHAR     'o'


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/**
 * @typedef angle_type_t
 * @brief orientation and inclination information for a board
 */
typedef enum {PITCH, ROLL, ORIENTATION, N_ANGLE_TYPES} angle_type_t;
// it is important that orientation is the last value!


/**
 * @typedef board_t
 * @brief inclination and timing information for a board
 */
typedef struct {
    int32_t angles[N_ANGLE_TYPES];  // angle values

    bool newData[N_ANGLE_TYPES];    // new data flag for each angle
    bool timed_out[N_ANGLE_TYPES];  // true if corresponding angle data is timed out
    bool new_timeout;               // new timeout flag
    bool valid;                     // to check that this slot of the database is not actually empty
    bool internal;                  // is this board internal or external?
    bool orientationData;           // does this board provide orientation data)

    uint8_t id;                     // board id

    clock_t last_update[N_ANGLE_TYPES]; // time of last update for each angle
} board_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/








#endif //TP2_BOARD_TYPE_H
