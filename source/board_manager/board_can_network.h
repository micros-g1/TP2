//
// Created by Rocío Parra on 10/4/2019.
//

#ifndef TP2_BOARD_CAN_NETWORK_H
#define TP2_BOARD_CAN_NETWORK_H

#include "board_type.h"


#define MAX_LEN_CAN_MSG 5

#ifndef ROCHI_DEBUG
#define CAN_MAX_FREQ    20
#else
#define CAN_MAX_FREQ    1
#endif
typedef void (*bn_callback_t )(uint8_t msg_id, uint8_t * can_data);


void bn_init();

void bn_register_callback(bn_callback_t cb);

void bn_periodic();

void bn_send(uint8_t msg_id, uint8_t * data);


#endif //TP2_BOARD_CAN_NETWORK_H
