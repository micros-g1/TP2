/*
 * Interrupts.h
 *
 *  Created on: 30 ago. 2019
 *      Author: Tomas
 */

#ifndef INTERRUPTS_INTERRUPTS_H_
#define INTERRUPTS_INTERRUPTS_H_
#include <gpio.h>

// Digital values
#ifndef LOW
#define LOW     0
#define HIGH    1
#endif // LOW


// IRQ modes
enum {
	GPIO_IRQ_MODE_LOGIC_0,
	GPIO_IRQ_MODE_LOGIC_1,
    GPIO_IRQ_MODE_DISABLE,
    GPIO_IRQ_MODE_RISING_EDGE,
    GPIO_IRQ_MODE_FALLING_EDGE,
    GPIO_IRQ_MODE_BOTH_EDGES,
    GPIO_IRQ_CANT_MODES
};


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef void (*pinIrqFun_t)(void);
/**
 * @brief Configures how the pin reacts when an IRQ event ocurrs
 * @param pin the pin whose IRQ mode you wish to set (according PORTNUM2PIN)
 * @param irqMode disable, risingEdge, fallingEdge or bothEdges
 * @param irqFun function to call on pin event
 * @return Registration succeed
 */
void gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun);
void interrupts_init();
#endif /* INTERRUPTS_INTERRUPTS_H_ */
