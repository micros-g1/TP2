/*
 * @file SysTick.c
 * @brief SysTick driver implementation
 * @author 22.99 Group 1: Alvarez, Gonzalez, Parra, Reina
 */
#include "SysTick.h"
#include "MK64F12.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "core_cm4.h"
#include "board.h"

/*-------------------------------------------
 ----------------DEFINES---------------------
 -------------------------------------------*/
#define FCLK	100000000U // clock frequency, Hz

#if FCLK % SYSTICK_ISR_FREQUENCY_HZ != 0
#warning SYSTICK cannot implement this exact frequency. Using floor(FCLK/SYSTICK_ISR_FREQUENCY_HZ) instead.
#elif SYSTICK_ISR_FREQUENCY_HZ <= 0
#error SYSTICK frequency must be positive
#endif /* FCLK % SYSTICK_ISR_FREQUENCY_HZ != 0 */

/*should be left in this value.
If COUNTER_INIT should change its value, one should change SysTick_Handler's implementation before doing so
(for more information on this, check SysTick_Handler's implementation)
*/
#define COUNTER_INIT	-1
typedef struct {
    systick_callback_t func;
    /*has to be a SIGNED int so that COUNTER_INIT may be negative!!
    *see COUNTER_INIT's definition for a more detailed explanation*/
    int counter;
    unsigned int reload;
    bool enabled;
    callback_conf_t conf;
} st_cb_data_t;


/*-------------------------------------------
 ----------GLOBAL_VARIABLES------------------
 -------------------------------------------*/

static st_cb_data_t st_callbacks[MAX_N_ST_CALLBACKS];

/*-------------------------------------------
 ----------STATIC_FUNCTION_DECLARATION-------
 -------------------------------------------*/
void SysTick_Handler(void);
void reset_callback_data(st_cb_data_t* data);

/*-------------------------------------------
 ------------FUNCTION_IMPLEMENTATION---------
 -------------------------------------------*/

void systick_init ()
{
	static bool initialized = false;

	if(initialized) return;

	NVIC_EnableIRQ(SysTick_IRQn);
	SysTick->CTRL = 0x00; 								// enable systick interrupts
	SysTick->LOAD = FCLK/SYSTICK_ISR_FREQUENCY_HZ - 1; 	// load value = pulses per period - 1
	SysTick->VAL=0x00;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk| SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	for (int i = 0; i < MAX_N_ST_CALLBACKS; i++)
		reset_callback_data(&st_callbacks[i]);
	//gpioMode(IT_PERIODIC_PIN, OUTPUT);
	//gpioWrite(IT_PERIODIC_PIN, false);
	initialized = true;
}

// DO NOT CHANGE THE NAME, overrides core_cm4.h weak definition
void SysTick_Handler(void){
	//gpioWrite(IT_PERIODIC_PIN, true);
	/* for SysTick, clearing the interrupt flag is not necessary
	* it is not an omission!*/
    for (int i = 0; i < MAX_N_ST_CALLBACKS; i++) {
        if (st_callbacks[i].func != NULL && st_callbacks[i].enabled) {
        	/*counter has to be incremented first so that if .func() disables itself from inside the function call,
        	*the function will still function correctly when reload value is 0.
        	this is taken into account when defining COUNTER_INIT!!*/
        	st_callbacks[i].counter++;
            if (st_callbacks[i].counter == st_callbacks[i].reload) {
            	st_callbacks[i].func();
                st_callbacks[i].counter = COUNTER_INIT;
                if(st_callbacks[i].conf == SINGLE_SHOT)
                	st_callbacks[i].enabled = false;
            }
        }
    }
	//gpioWrite(IT_PERIODIC_PIN, false);
}



void systick_add_callback(systick_callback_t cb, unsigned int reload, callback_conf_t conf)
{
	if(cb != NULL) {
        for (int i = 0; i < MAX_N_ST_CALLBACKS; i++) {
            if (st_callbacks[i].func == NULL) {
                st_callbacks[i].func = cb;
                st_callbacks[i].enabled = true;
                st_callbacks[i].counter = COUNTER_INIT;
                st_callbacks[i].reload = reload;
                st_callbacks[i].conf = conf;
                break;                        //this break instruction is important here
            }
        }
    }
}

void systick_enable_callback(systick_callback_t callback) {
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++) {
        if (st_callbacks[i].func == callback)
            st_callbacks[i].enabled = true;
    }
}

void systick_disable_callback(systick_callback_t callback) {
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++) {
        if (st_callbacks[i].func == callback) {
            st_callbacks[i].enabled = false;
            st_callbacks[i].counter = COUNTER_INIT;
            break;
        }
    }
}

void systick_delete_callback(systick_callback_t callback){
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++) {
        if (st_callbacks[i].func == callback) {
            reset_callback_data(&st_callbacks[i]);
            break;
        }
    }
}

void reset_callback_data(st_cb_data_t* data){
	data->func = NULL;
	data->enabled = false;
	data->counter = COUNTER_INIT;
}

bool systick_has_callback(systick_callback_t callback){
	bool has = false;
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++)
		if(( has = (st_callbacks[i].func == callback) ))
			break;

	return has;
}

callback_conf_t systick_get_callback_conf(systick_callback_t callback){
	callback_conf_t conf = PERIODIC;
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++)
		if(st_callbacks[i].func == callback) {
            conf = st_callbacks[i].conf;
            break;
        }
	return conf;
}

void systick_set_calback_conf(systick_callback_t callback, callback_conf_t conf){
	for(int i = 0; i < MAX_N_ST_CALLBACKS; i++)
		if(st_callbacks[i].func == callback)
			st_callbacks[i].conf = conf;
}
