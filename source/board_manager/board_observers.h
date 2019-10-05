//
// Created by Rocío Parra on 10/5/2019.
//

#ifndef TP2_BOARD_OBSERVERS_H
#define TP2_BOARD_OBSERVERS_H

#include "board_type.h"

typedef enum {O_PC, O_CAN, N_OBSERVERS} observer_t;

void bo_init();
void bo_notify(observer_t who, uint8_t board_id, angle_type_t angle_type, int32_t angle_value);
void bo_periodic();



#endif //TP2_BOARD_OBSERVERS_H
