//
// Created by Rocío Parra on 10/5/2019.
//

#include "board_observers.h"

#include <stdio.h>
#include "pc_interface.h"
#include "board_can_network.h"



#define MAX_MSG_SIZE    PC_MSG_LEN // one byte for id, one for angle type, one for sign, three for number
#if MAX_MSG_SIZE != MAX_LEN_CAN_MSG + 1
#error "this code only works if msg for can is the same as for pc, minus the id"
#endif



void angle_to_string(int angle, char * str);
int map_to_360(int angle);


void bo_init()
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    pc_init();
    bn_init();
}

void bo_notify(observer_t who, uint8_t board_id, angle_type_t angle_type, int32_t angle_value)
{
    if (who >= N_OBSERVERS || angle_type >= N_ANGLE_TYPES)
        return;

    char msg[MAX_MSG_SIZE + 1]; // leave one byte for '\0'
    msg[0] = board_id;

    switch (angle_type) {
        case PITCH: msg[1] = PITCH_CHAR; break;
        case ROLL: msg[1] = ROLL_CHAR; break;
        case YAW: msg[1] = YAW_CHAR; break;
        default: ; // this will not happen, see first statement in function
    }

    angle_to_string(angle_value, &msg[2]); //sign and three digit number

    switch (who) {
        case O_PC: pc_send(msg); break;
        case O_CAN: bn_send((uint8_t)msg[0], &msg[1]); break; // id is not part of msg in can
        default: break; // this will not happen
    }
}

void bo_periodic() {
    bn_periodic();
    pc_periodic();
}


void angle_to_string(int angle, char * str)
{
    angle = map_to_360(angle);
    if (angle >= 0) {
        str[0] = '+';
    }
    else {
        str[0] = '-';
        angle = (-angle);
    }

    snprintf(&str[1], 3, "%03d", angle);
}

int map_to_360(int angle)
{
    bool sg = angle >= 0;
    angle = sg ? angle : (-angle);
    angle %= 360;
    angle = angle <= 180 ? angle : angle-360;
    angle = sg ? angle : (-angle);
    return angle;
}
