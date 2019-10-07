//
// Created by Rocío Parra on 10/5/2019.
//

#include "board_app.h"
#include "board_manager/board_database.h"
#include "board_manager/board_observers.h"
#include "board_manager/board_ev_sources.h"


void ba_init()
{
    be_init();
    bd_init();
    unsigned int i;
    for (i = 0; i < N_MAX_BOARDS; i++) {
        if (i == BA_MY_ID) {
            bd_add_board(i, true); // add internal board
        }
        else {
            bd_add_board(i, false); // add external boards
        }
    }

    bo_init();

    // initialize board network and pc network
    // tell board network which function to call when it has new data, which should update the data base

    // initialize internal board, tell it which function to call when it has new data

    // both callbacks will parse the data so it fits the same format, and then both will call a function which
    // actually updates the data base with this new 'normalized' set of data

    // callbacks must be called from within the main app, so there must be an intermediate layer that polls the
    // peripherals, and then calls the callbacks, within a periodic update function
}

void ba_periodic()
{
    be_periodic();  // register new events

    // check if there is any new information and if so, notify observers
    unsigned int i;
    for (i = 0; i < N_MAX_BOARDS; i++) {
        ev_db_t ev = bd_newdata(i);

        while (ev != N_EVS_DB) {
            if (ev == NEW_TIMEOUT) {
                bo_notify_timeout(O_PC, i);
                ev = N_EVS_DB;
            }
            else {
                int32_t angle_value = bd_get_angle(i, (angle_type_t)ev);
                if (i == BA_MY_ID) {
                    bo_notify_data(O_CAN, i, (angle_type_t) ev, angle_value);   // notify board network
                }

                bo_notify_data(O_PC, i, (angle_type_t) ev, angle_value);        // notify pc network
                ev = bd_newdata(i);

            }
        }
    }

    bo_periodic();
}
