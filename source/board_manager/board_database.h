/***************************************************************************//**
  @file     board_database.h
  @brief    ...
  @author   Rocio Parra
 ******************************************************************************/


#ifndef SOURCE_BOARD_MANAGER_H
#define SOURCE_BOARD_MANAGER_H


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board_type.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define N_MAX_BOARDS        8

#ifndef ROCHI_DEBUG
#define ANGLE_TIMEOUT_MS    1000    // external boards are considered 'dead' after this time without new data
#define BA_UPDATE_MS        900     // resend angles from internal boards after this time has elapsed
#else
#define ANGLE_TIMEOUT_MS    10000    // external boards are considered 'dead' after this time without new data
#define BA_UPDATE_MS        5000     // resend angles from internal boards after this time has elapsed
#endif

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {NEW_PITCH, NEW_ROLL, NEW_ORIENTATION, NEW_TIMEOUT, N_EVS_DB} ev_db_t;

#if NEW_PITCH != PITCH || NEW_ROLL != ROLL || NEW_ORIENTATION != ORIENTATION
#error angles must be in the same order as in board_types.h!!
#endif
/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize board manager
*/
void bd_init(void);

/**
 * @brief Register new board in data base
 * @param id: Identification number for new board [0-7]
 * @param internal: Is this board internal or external?
 * */
void bd_add_board(uint8_t id, bool internal);

/**
 * @brief Register new angle values for existing board
 * @param id: board number
 * @param angle_type: which angle do you want to update?
 * @param value: new angle value */
void bd_update(uint8_t id, angle_type_t angle_type, int32_t value);

/**
 * @brief Get last angle data
 * @param id: Board ID
 * @param angle_type: Which angle do you want?
 * @return Last angle data. UINT32_MAX if error
 */
int32_t bd_get_angle(uint8_t id, angle_type_t angle_type);


/**
 * @brief Check whether a certain ID corresponds to a current valid board
 * @param id: Board ID
 * @return False: the board is timed out or was never initialized, or ID is not valid
 */
bool bd_is_ok(uint8_t id);

/**
 * @brief Check whether any new data is available for a given board
 * @param id Board ID
 * @return Last event, N_EV_TYPES if no new events available
 */
ev_db_t bd_newdata(uint8_t id);





/*******************************************************************************
 ******************************************************************************/


#endif //SOURCE_BOARD_MANAGER_H
