//
// Created by Rocío Parra on 10/5/2019.
//

#include "board_ev_sources.h"
#include "board_can_network.h"
#include "board_database.h"
#include <string.h>
#include <stdlib.h>

void can_callback(uint8_t msg_id, char * can_data);

bool isint(char * s);
bool isdigit(char c);


void be_init()
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    bd_init();

    bn_init();
    bn_register_callback(can_callback);
    // initialize magnetometer & accelerometer
}

void be_periodic()
{
    bn_periodic();
    // mag & acc periodic
}

void can_callback(uint8_t msg_id, char * can_data)
{
    if (can_data != NULL)
    {
        uint32_t len = strlen(can_data);
        if (len <= MAX_LEN_CAN_MSG && len >= 2) { // at least angle type and one number
            angle_type_t type;

            switch (can_data[0]) {
                case PITCH_CHAR:    type = PITCH;           break;
                case ROLL_CHAR:     type = ROLL;            break;
                case YAW_CHAR:      type = YAW;             break;
                default:            type = N_ANGLE_TYPES;   break;// error in msg
            }

            if (type <= N_ANGLE_TYPES) {
                char * end;
                int32_t value = (int32_t)strtol(&can_data[1], &end, 10);
                if (end[0] == 0) {
                    bd_update(msg_id, type, value);
                }
            }
        }
    }
}

