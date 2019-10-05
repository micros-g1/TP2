//
// Created by Rocío Parra on 10/4/2019.
//

#include "msg_queue.h"
#include "hardware.h"
#include <string.h>


void mq_init(msg_queue_t * q)
{
    q->len = q->in = q->out = 0;
}


//Wait for data
void mq_read_blocking(msg_queue_t * q, char * data)
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
    hw_DisableInterrupts();
    //Must set these three variables to zero before continuing...
    mq_init(q);
    hw_EnableInterrupts();
}

//Add data to queue
bool mq_pushback(msg_queue_t * q, char * data)
{
    bool ret_val = false;
    hw_DisableInterrupts();

    if(q->len != Q_MAX_LENGTH)
    {
        data[Q_MSG_LEN]=0; // assure there is a terminator
        strcpy(q->buffer[q->in++], data);
        if(q->in == Q_MAX_LENGTH)
            q->in = 0;
        q->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Add data to queue
bool mq_pushfront(msg_queue_t * q, char * data)
{
    bool ret_val = false;
    hw_DisableInterrupts();

    if(q->len != Q_MAX_LENGTH) {
        if(q->out == 0) {
            q->out = Q_MAX_LENGTH;
        }
        data[Q_MSG_LEN] = 0;
        q->out--;
        strcpy(q->buffer[q->out], data);
        q->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
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



void mq_popfront(msg_queue_t * q, char * data)
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
