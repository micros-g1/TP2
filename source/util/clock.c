/*
 * clock.c
 *
 *  Created on: Oct 6, 2019
 *      Author: Rocío Parra
 */


#include "clock.h"
#include "Systick.h"

#if CLOCKS_PER_SECOND != SYSTICK_ISR_FREQUENCY_HZ
#error "use same frequency as systick!"
#endif


static volatile clock_t clock;
void callback(void);


void clock_init(void)
{
	static bool isinit = false;
	if (isinit)
		return;

	isinit = true;
	clock = 0;
	systick_init();
	systick_add_callback(callback, 0, PERIODIC);
}

void callback(void)
{
	clock++;
}

clock_t get_clock(void)
{
	return clock;
}
