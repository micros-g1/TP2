 /***************************************************************************//**
  @file     board_manager.c
  @brief    ...
  @author   Rocio Parra
 ******************************************************************************/



/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "board_database.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/**
 * @brief brief check_timeouts: updates board.ok
 * @param board board to check
 */
void check_timeouts(board_t * board);


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/




/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

board_t boards[N_MAX_BOARDS];


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void bd_init(void)
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    unsigned int i;
    for (i = 0; i < N_MAX_BOARDS; i++) {
        boards[i].ok = false;
    }
}

void bd_add_board(uint8_t id, bool internal) {
    if (id < N_MAX_BOARDS) {
        boards[id].internal = internal;
        boards[id].ok = true;
        boards[id].yawData = internal; // for external boards, we assume we wont get yaw til we get one measurement

        unsigned int i;
        time_t now;
        time(&now);
        for (i = 0; i < N_ANGLE_TYPES; i++){
            boards[id].newData[i] = true;
            boards[id].angles[i] = 0;
            boards[id].last_update[i] = now;
            boards[id].id = id;  // this will not be used, set for consistency
        }
    }
}

void bd_update(uint8_t id, angle_type_t angle_type, int32_t value) {
    if (id >= N_MAX_BOARDS) {
        return; // error
    }
    clock_t now = clock();
    boards[id].angles[angle_type] = value;
    if (angle_type == YAW) {
        boards[id].yawData = true;
    }
    boards[id].last_update[angle_type] = now;

    check_timeouts(&boards[id]);
    if (boards[id].ok) {
        boards[id].newData[angle_type] = true;
    }
}


int32_t bd_get_angle(uint8_t id, angle_type_t angle_type)
{
    if (bd_is_ok(id)) {
        return UINT32_MAX; // error
    }

    return boards[id].angles[angle_type];
}


bool bd_is_ok(uint8_t id)
{
    return id >= N_MAX_BOARDS || !check_timeouts(&boards[id]);
}


void check_timeouts(board_t * board)
{
    clock_t now = clock();

    board->ok = true;
    unsigned int i, n = board->yawData ? N_ANGLE_TYPES : N_ANGLE_TYPES - 1;
    for (i = 0; i < n; i++) {
        float ms_elapsed = (now - board->last_update[i])*1000.0/CLOCKS_PER_SEC;
        if (board->internal) {
            if (ms_elapsed >= BA_UPDATE_MS) {
                board->newData[i] = true; // must resend data so other boards dont think im dead
            }
        }
        else if (ms_elapsed >= ANGLE_TIMEOUT_MS) {
            board->ok = false;   // this board is dead
            board->newData[i] = true;
            break;              // no point in checking other angles
        }
    }
}


bool bd_newdata_any(uint8_t id)
{
    if (id >= N_MAX_BOARDS)
        return false;

    unsigned int i;
    for (i = 0; i < N_ANGLE_TYPES; i++) {
        if (boards[id].newData[i]) {
            return true;
        }
    }
    return false;
}


bool bd_newdata(uint8_t id, angle_type_t angle_type)
{
    return id < N_MAX_BOARDS && boards[id].newData[angle_type];
}

/*******************************************************************************
 ******************************************************************************/

