//
// Created by Rocío Parra on 10/4/2019.
//

#include "msg_queue.h"
#include <string.h>

#define ROCHI_DEBUG

#ifndef ROCHI_DEBUG
#include "hardware.h"
#endif

void mq_init(msg_queue_t * q)
{
    q->len = q->in = q->out = 0;
}


//Wait for data
void mq_read_blocking(msg_queue_t * q, uint8_t * data)
{
    //Atomic operation (assembly)
    while(q->len == 0) {;} // wait for data

    strcpy(data, q->buffer[q->out++]);
    if(q->out == Q_MAX_LENGTH) {
        q-> out = 0;
    }
    //Atomic operation (assembly)
    q->len--;
}

//Flush queue
void mq_flush(msg_queue_t * q)
{
#ifndef ROCHI_DEBUG
    hw_DisableInterrupts();
#endif
    //Must set these three variables to zero before continuing...
    mq_init(q);

#ifndef ROCHI_DEBUG
    hw_EnableInterrupts();
#endif
}

//Add data to queue
bool mq_pushback(msg_queue_t * q, uint8_t * data)
{
    bool ret_val = false;
#ifndef ROCHI_DEBUG
    hw_DisableInterrupts();
#endif

    data[Q_MSG_LEN]=0; // assure there is a terminator
    strcpy(q->buffer[q->in++], data);
    if(q->in == Q_MAX_LENGTH) {
        q->in = 0;
    }
    q->len = q->len <= Q_MAX_LENGTH ? q->len+1 : Q_MAX_LENGTH;
    ret_val = true;
#ifndef ROCHI_DEBUG
    hw_EnableInterrupts();
#endif
    return ret_val;
}


//Add data to queue
bool mq_pushfront(msg_queue_t * q, uint8_t * data)
{
    bool ret_val = false;

#ifndef ROCHI_DEBUG
    hw_DisableInterrupts();
#endif
    if(q->out == 0) {
        q->out = Q_MAX_LENGTH;
    }
    data[Q_MSG_LEN] = 0;
    q->out--;
    strcpy(q->buffer[q->out], data);
    q->len++;
    ret_val = true;
#ifndef ROCHI_DEBUG

    hw_EnableInterrupts();
#endif
    return ret_val;
}


//Get current queue length
unsigned int mq_length(msg_queue_t * q)
{
    //atomic read operation (assembly)
    return q->len;
}

bool mq_isfull(msg_queue_t * q)
{
    return q->len == Q_MAX_LENGTH;
}



void mq_popfront(msg_queue_t * q, uint8_t * data)
{
   if (q->len) {
        q->len--;

        strcpy(data, q->buffer[q->out++]);
        if(q->out == Q_MAX_LENGTH) {
            q->out = 0;
        }
        //DEBUG
//        if(q.len == Q_MAX_LENGTH)
//        	q.len = Q_MAX_LENGTH+1;
    }
}
