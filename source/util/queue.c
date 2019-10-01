//
// Created by Rocío Parra on 9/9/2019.
//

#include "queue.h"
#include "hardware.h"

// queue Variables
static volatile uint8_t queue[Q_MAX_LENGTH];	//Circular queue
static volatile unsigned int queue_length = 0;				//Keeps track of queue length
static volatile unsigned int in_offset = 0;					//Offset for adding next data
static volatile unsigned int out_offset = 0;				//Offset for reading next data

void q_init(queue_t * q)
{
	q->len = q->in = q->out = 0;
}


//Wait for data
uint8_t q_read_blocking(queue_t * q)
{
	uint8_t ans;
    //Atomic operation (assembly)
    while(q->len == 0) {;} // wait for data

    ans = q->buffer[q->out++];
    if(q->out == Q_MAX_LENGTH) {
        q-> out = 0;
    }
    //Atomic operation (assembly)
    q->len--;

    return ans;
}

//Flush queue
void q_flush(queue_t * q)
{
    hw_DisableInterrupts();
    //Must set these three variables to zero before continuing...
    q_init(q);
    hw_EnableInterrupts();
}

//Add data to queue
bool q_pushback(queue_t * q, uint8_t data)
{
    bool ret_val = false;
    hw_DisableInterrupts();

    if(q->len != Q_MAX_LENGTH)
    {
        q->buffer[q->in++] = data;
        if(q->in == Q_MAX_LENGTH)
            q->in = 0;
        q->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Add data to queue
bool q_pushfront(queue_t * q, uint8_t data)
{
    bool ret_val = false;
    hw_DisableInterrupts();

    if(q->len != Q_MAX_LENGTH) {
    	if(q->out == 0) {
    		q->out = Q_MAX_LENGTH;
    	}
    	q->out--;
        q->buffer[q->out] = data;
        q->len++;
        ret_val = true;
    }
    hw_EnableInterrupts();
    return ret_val;
}


//Get current queue length
unsigned int q_length(queue_t * q)
{
    //atomic read operation (assembly)
    return q->len;
}

bool q_isfull(queue_t * q)
{
	return q->len == Q_MAX_LENGTH;
}



uint8_t q_popfront(queue_t * q)
{
	uint8_t data = 0;

    if (q->len) {
        q->len--;

        data = q->buffer[q->out++];
        if(q->out == Q_MAX_LENGTH) {
        	q->out = 0;
        }
        //DEBUG
//        if(q.len == Q_MAX_LENGTH)
//        	q.len = Q_MAX_LENGTH+1;
    }

    return data;
}
