//
// Created by Rocío Parra on 10/4/2019.
//

#include "board_can_network.h"

#include "CAN.h"
#include <time.h>
#include "msg_queue.h"
#include <string.h>


#define CAN_MIN_MS  (1000/CAN_MAX_FREQ)

static msg_queue_t can_q;
static clock_t last;

static bn_callback_t callback;

void bn_init() {
    static bool is_init = false;
    if (is_init)
        return;

    is_init = true;
    callback = NULL;
    CAN_init();
    CAN_set_message_callback(NULL);
    CAN_set_rx_buffer_overflow_callback(NULL);

    mq_init(&can_q);
}

void bn_register_callback(bn_callback_t cb)
{
    callback = cb;
}

void bn_periodic()
{
    // send
    if (can_q.len) {
        float ms_elapsed = 1000.0*(clock()-last)/CLOCKS_PER_SEC;
        if (ms_elapsed >= CAN_MIN_MS) {
            char data[MAX_LEN_CAN_MSG + 2]; // one byte for terminator, one for id
            mq_popfront(&can_q, &data);

            can_message_t msg;
            msg.message_id = data[0];
            msg.fir.frame_type = CAN_STANDARD_FRAME;
            msg.fir.rtr = 0;
            msg.fir.dlc = strlen(&data)-1; // id will not be sent

            strcpy(msg.data, &data[1]);
            // this will copy the terminator too, but because there is extra space this is not an issue

            CAN_send(&msg);

            last = clock();
        }
    }

    // receive
    if (callback != NULL) {
        while (CAN_message_available()) {
            can_message_t msg;
            CAN_get(&msg);
            if (msg.fir.dlc <= MAX_LEN_CAN_MSG) {
                msg.data[msg.fir.dlc] = 0; // add terminator to data
                callback(msg.message_id, msg.data);
            }
        }
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

