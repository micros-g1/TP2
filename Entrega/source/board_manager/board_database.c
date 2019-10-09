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

    clock_init();
    unsigned int i;
    for (i = 0; i < N_MAX_BOARDS; i++) {
        boards[i].valid = false;
    }

    clock_init();
}

void bd_add_board(uint8_t id, bool internal) {
    if (id < N_MAX_BOARDS) {
        boards[id].internal = internal;
        boards[id].valid = true;
        boards[id].orientationData = internal; // for external boards, we assume we wont get yaw til we get one measurement

        unsigned int i;
        clock_t now = get_clock();
        for (i = 0; i < N_ANGLE_TYPES; i++){
            boards[id].newData[i] = true;
            boards[id].angles[i] = 0;
            boards[id].timed_out[i] = !internal; // these will all be true til data is updated
            boards[id].last_update[i] = now;
        }
        boards[id].id = id;  // this will not be used, set for consistency
        boards[id].new_timeout = false; // default state
    }
}

void bd_update(uint8_t id, angle_type_t angle_type, int32_t value) {
    if (id >= N_MAX_BOARDS) {
        return; // error
    }

    clock_t now = get_clock();
    boards[id].angles[angle_type] = value;
    if (angle_type == ORIENTATION) {
        boards[id].orientationData = true;
    }
    boards[id].last_update[angle_type] = now;
    boards[id].timed_out[angle_type] = false;
    boards[id].newData[angle_type] = true;
}


int32_t bd_get_angle(uint8_t id, angle_type_t angle_type)
{
    if (id >= N_MAX_BOARDS) {
        return UINT32_MAX; // error
    }
    boards[id].newData[angle_type] = false;
    return boards[id].angles[angle_type];
}


bool bd_is_ok(uint8_t id)
{
	if (id >= N_MAX_BOARDS)
		return false;

	check_timeouts(&boards[id]);
    return !(boards[id].timed_out[0] || boards[id].timed_out[1] || boards[id].timed_out[2]);
}


void check_timeouts(board_t * board)
{
    clock_t now = get_clock();

    unsigned int i, n = board->orientationData ? N_ANGLE_TYPES : N_ANGLE_TYPES - 1;
    for (i = 0; i < n; i++) {
        if (!board->timed_out[i]) { // no point in checking if it was already timed out
            float ms_elapsed = ((float)(now - board->last_update[i])) * 1000.0 / (float)CLOCKS_PER_SECOND;

            if (board->internal) {
                if (ms_elapsed >= BA_UPDATE_MS) {
                    board->newData[i] = true; // must resend data so other boards dont think im dead
                    board->last_update[i] = now;
                }
            } else if (ms_elapsed >= ANGLE_TIMEOUT_MS) {
                board->timed_out[i] = true;

                unsigned int j;
                board->new_timeout = true;
                for (j = 0; j < n; j++) {
                    if (j != i && board->timed_out[j]) {
                        board->new_timeout = false;
                        break;
                    }
                }
            }
        }
    }
}


ev_db_t bd_newdata(uint8_t id)
{
    ev_db_t ev = N_EVS_DB;
    if (id <= N_MAX_BOARDS) {
        if (bd_is_ok(id)) {
            unsigned int i;
            unsigned int n = boards[id].orientationData ? N_ANGLE_TYPES : N_ANGLE_TYPES - 1;
            for (i = 0; i < n; i++) {
                if (boards[id].newData[i]) {
                    ev = i;
                    boards[id].newData[i] = false;
                    break;
                }
            }
        }
        else if (boards[id].new_timeout) {
            ev = NEW_TIMEOUT;
            boards[id].new_timeout = false;
        }
    }
    return ev;
}

/*******************************************************************************
 ******************************************************************************/

