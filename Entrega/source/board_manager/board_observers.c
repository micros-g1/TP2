//
// Created by Rocío Parra on 10/5/2019.
//

#include "board_observers.h"

#include "../pc_interface/pc_interface.h"
#include "board_can_network.h"
#include <string.h>

#ifdef ROCHI_DEBUG
#include <stdio.h>
#endif


#define MAX_MSG_SIZE    PC_MSG_LEN // one byte for id, one for angle type, one for sign, three for number
#if MAX_MSG_SIZE != MAX_LEN_CAN_MSG + 2
#error "this code only works if msg for can is the same as for pc, minus the id and pckg type"
#endif



void angle_to_string(int angle, uint8_t * str);
int map_to_360(int angle);
void print_angle(uint32_t angle, uint8_t * msg);


void bo_init()
{
    static bool is_init = false;
    if (is_init)
        return;
    is_init = true;

    pc_init();
    bn_init();
}

void bo_notify_data(observer_t who, uint8_t board_id, angle_type_t angle_type, int32_t angle_value)
{
    if (who >= N_OBSERVERS || angle_type >= N_ANGLE_TYPES)
        return;

    uint8_t msg[MAX_MSG_SIZE + 2]; // leave one byte for '\0' and one for package type
    msg[0] = 'D';
    msg[1] = board_id + '0'; // convert to char so 0 is not interpreted as terminator

    switch (angle_type) {
        case PITCH: msg[2] = PITCH_CHAR; break;
        case ROLL: msg[2] = ROLL_CHAR; break;
        case ORIENTATION: msg[2] = OR_CHAR; break;
        default: ; // this will not happen, see first statement in function
    }

    angle_to_string(angle_value, &msg[3]); //sign and three digit number

    switch (who) {
        case O_PC: pc_send(msg); break;
        case O_CAN: bn_send((uint8_t)msg[1], &msg[2]); break; // id is not part of msg in can, only one pckg type
        default: break; // this will not happen
    }
}

void bo_notify_timeout(observer_t who, uint8_t board_id) {
    if (who == O_PC) {
        uint8_t msg[PC_MSG_LEN+1] = {'T', board_id + '0'};
        strcpy(&msg[2], "00000");
        pc_send(msg);
    }
}


void bo_periodic() {
    bn_periodic();
    pc_periodic();
}



void angle_to_string(int angle, uint8_t * str)
{
    angle = map_to_360(angle);
    if (angle >= 0) {
        str[0] = '+';
    }
    else {
        str[0] = '-';
        angle = (-angle);
    }

    print_angle(angle, &str[1]);
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


void print_angle(uint32_t angle, uint8_t * msg)
{
	uint32_t cent = angle/100;
	uint32_t dec = (angle - cent*100)/10;
	uint32_t units = angle - cent*100 - dec*10;

	msg[0] = (uint8_t)cent + '0';
	msg[1] = (uint8_t)dec + '0';
	msg[2] = (uint8_t)units + '0';
	msg[3] = '\0';
}
