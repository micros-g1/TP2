//
// Created by Rocío Parra on 10/5/2019.
//

#ifndef TP2_PC_INTERFACE_H
#define TP2_PC_INTERFACE_H

#include <stdint.h>

//#define ROCHI_DEBUG

#ifdef ROCHI_DEBUG
#define PC_MAX_FREQ 10
#else
#define PC_MAX_FREQ 60 // 60 frames per second is the fastest ppl will notice so no point in going any faster
#endif

#define PC_MSG_LEN  7  // one byte for pckg type, one for id, one for angle type, one for sign, three for number


void pc_init();
void pc_send(char * data);
void pc_periodic(); // so it can send any messages it has on queue


#endif //TP2_PC_INTERFACE_H
