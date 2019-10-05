//
// Created by Rocío Parra on 10/4/2019.
//

#ifndef TP2_BOARD_TYPE_H
#define TP2_BOARD_TYPE_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>




typedef enum {PITCH, ROLL, YAW, N_ANGLE_TYPES} angle_type_t; // it is important that YAW is the last value!!

#define PITCH_CHAR  'c'
#define ROLL_CHAR   'r'
#define YAW_CHAR    'o'



typedef struct {
    int32_t angles[N_ANGLE_TYPES];

    bool newData[N_ANGLE_TYPES];
    bool ok;
    bool internal;
    bool yawData;

    uint8_t id;

    clock_t last_update[N_ANGLE_TYPES]; // if internal, last transmitted; if external, last received
} board_t;





#endif //TP2_BOARD_TYPE_H
