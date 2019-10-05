#include "timers.h"
#include <stdlib.h>
#include <Interrupts/SysTick.h>

#define SYSTICKS_PER_MILLISECOND 6

static void timers_ISR();

typedef struct
{
	unsigned int time_count_ms;
	unsigned int period_ms;
	timer_mode_t mode;
	bool enabled;
	timer_callback_t callback;
}timer_t;

static timer_t timers_array[TIMER_TOTAL_TIMERS];

//Initialize Timers
void timers_init()
{
	systick_init();
	timer_t timer;
	timer.time_count_ms = 0;
	timer.period_ms = 0;
	timer.mode = TIMER_REPEAT;
	timer.enabled = false;
	timer.callback = NULL;
	//Initialize all timers
	for(int i = 1 ; i < TIMER_TOTAL_TIMERS ; i++)
		timers_array[i] = timer;
	//Set ISR
	systick_add_callback(timers_ISR, SYSTICKS_PER_MILLISECOND, PERIODIC);
}

//Set timer Period
void timers_set_timer_period(unsigned int t_id, int t_ms)
{
	if(t_id < TIMER_TOTAL_TIMERS)
		timers_array[t_id].period_ms = t_ms;
}

//Set timer mode
void timers_set_timer_mode(unsigned int t_id, timer_mode_t mode)
{
	if(t_id < TIMER_TOTAL_TIMERS)
		timers_array[t_id].mode = mode;
}

//Set timer enabled. Resets timer.
void timers_set_timer_enabled(unsigned int t_id , bool enabled)
{
	if(t_id < TIMER_TOTAL_TIMERS)
	{
			timers_array[t_id].enabled = enabled;
			timers_array[t_id].time_count_ms = 0;
	}
}

//Reset timer
void timers_reset_timer(unsigned int t_id)
{
	if(t_id < TIMER_TOTAL_TIMERS)
		timers_array[t_id].time_count_ms = 0;
}

//Set timer callback
void timers_set_timer_callback(unsigned int t_id, timer_callback_t callback)
{
	if(t_id < TIMER_TOTAL_TIMERS)
		timers_array[t_id].callback = callback;
}

void timers_ISR()
{
	for(int i = 0 ; i < TIMER_TOTAL_TIMERS ; i++)
		if(timers_array[i].enabled && timers_array[i].period_ms != 0)
			if(++timers_array[i].time_count_ms == timers_array[i].period_ms)
			{
				//Reset Time count
				timers_array[i].time_count_ms = 0;
				//Disable timer if TIMER_SINGLE
				timers_array[i].enabled = timers_array[i].mode == TIMER_REPEAT ? true : false;
				//Execute callback.
				if(timers_array[i].callback != NULL)
					timers_array[i].callback(i);
			}
}
