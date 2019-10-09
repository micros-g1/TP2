/*
 * timers.h
 *
 *  Created on: 11 Sep 2019
 *      Author: grein
 */

#ifndef TIMER_TIMERS_H_
#define TIMER_TIMERS_H_

#include <stdbool.h>

//Total timers to implement
#define TIMER_TOTAL_TIMERS 2

//Timer Callback
typedef void (*timer_callback_t)(unsigned int t_id);

//TIMER_SINGLE: Deactivate timer after one period
//TIMER_REPEAT: Keep timer running for several periods.
//At the end of each period, callback will be called (as long as callback is not NULL)

typedef enum {TIMER_REPEAT,TIMER_SINGLE} timer_mode_t;

//Initialize Timers
void timers_init();
//Set timer Period
void timers_set_timer_period(unsigned int t_id, int t_ms);
//Set timer mode
void timers_set_timer_mode(unsigned int t_id, timer_mode_t mode);
//Set timer running. Resets timer.
void timers_set_timer_enabled(unsigned int t_id, bool enabled);
//Reset timer
void timers_reset_timer(unsigned int t_id);
//Set timer callback.
void timers_set_timer_callback(unsigned int t_id, timer_callback_t callback);

#endif /* TIMER_TIMERS_H_ */
