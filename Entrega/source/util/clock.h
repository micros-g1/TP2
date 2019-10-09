/*
 * clock.h
 *
 *  Created on: Oct 6, 2019
 *      Author: Rocío Parra
 */

#ifndef UTIL_CLOCK_H_
#define UTIL_CLOCK_H_

#include <stdint.h>

typedef uint32_t clock_t;

#define CLOCKS_PER_SECOND	8000U

void clock_init(void);
clock_t get_clock(void);


#endif /* UTIL_CLOCK_H_ */
