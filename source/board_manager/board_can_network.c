//
// Created by Rocío Parra on 10/4/2019.
//

#include "board_can_network.h"

//#define ROCHI_DEBUG

#ifndef ROCHI_DEBUG
#include "../CAN/CAN.h"
#else
#include <stdio.h>
#endif

#include "../util/clock.h"
#include "../util/msg_queue.h"
#include <string.h>


#define CAN_MIN_MS  (1000.0/CAN_MAX_FREQ)

static msg_queue_t can_q;
static clock_t last;

static bn_callback_t callback;

void bn_init() {
    static bool is_init = false;
    if (is_init)
        return;

    is_init = true;
    callback = NULL;
#ifndef ROCHI_DEBUG
    CAN_init();

    can_filter_t filter;
    filter.mask = 0x7F8;
    filter.id = 0x100;
    filter.frame_type = CAN_STANDARD_FRAME;
    CAN_set_filter_config(filter);

    CAN_start();
#endif

    mq_init(&can_q);

    clock_init();
    last = get_clock();
}

void bn_register_callback(bn_callback_t cb)
{
    callback = cb;
}

void bn_periodic()
{
    // send
    if (can_q.len) {
        float ms_elapsed = 1000.0*(get_clock()-last)/CLOCKS_PER_SECOND;
        if (ms_elapsed >= CAN_MIN_MS) {
            char data[MAX_LEN_CAN_MSG + 2]; // one byte for terminator, one for id
            mq_popfront(&can_q, data);

#ifndef ROCHI_DEBUG
            can_message_t msg;
            msg.header.message_id = data[0] - '0' + 0x100; // need actual ID number and not char
            msg.header.frame_type = CAN_STANDARD_FRAME;
            msg.header.rtr = false;
            msg.header.dlc = strlen(&data)-1; // id will not be sent

            strcpy(msg.data, &data[1]);
            // this will copy the terminator too, but because there is extra space this is not an issue

            CAN_send(&msg);
#else
            printf("CAN: %s \n", data);
#endif
            last = get_clock();
        }
    }

    // receive
    if (callback != NULL) {
#ifndef ROCHI_DEBUG
        while (CAN_message_available()) {
            can_message_t msg;
            CAN_get(&msg);
            if (msg.header.dlc <= MAX_LEN_CAN_MSG && !msg.header.rtr) {
                msg.data[msg.header.dlc] = 0; // add terminator to data
                callback(msg.header.message_id, msg.data);
            }
        }
#else

#endif
    }
}

void bn_send(uint8_t msg_id, char * data)
{
    char buffer[MAX_LEN_CAN_MSG + 2];
    buffer[0] = msg_id;
    strcpy(&buffer[1], data);
    mq_pushback(&can_q, buffer);
    bn_periodic(); // see if i can send it right now
}

