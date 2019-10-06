/*
 * gpio.c
 *
 *  Created on: 24 ago. 2019
 *      Author: Tomas, Lisandro, Rocio, Gonzalo
 */
//#include "gpio.h"
#include <Interrupts/interrupts.h>
#include "MK64F12.h"
#include <stdint.h>


static void enable_interrupt_pins(int port_num, int pin_num);
static void disable_interrupt_pins(int port_num, int pin_num);
void PORT_ClearInterruptFlag (int port_num, int pin_num);
#define NUM_PORTS			5
#define NUM_PINS_PER_PORT	32
pinIrqFun_t interrupt_matrix[NUM_PORTS][NUM_PINS_PER_PORT];
int interrupt_pins[NUM_PORTS][NUM_PINS_PER_PORT];

void interrupts_init(){
	static bool is_init = false;
	if (is_init)
		return;
	is_init = true;

	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTB_IRQn);
	NVIC_EnableIRQ(PORTC_IRQn);
	NVIC_EnableIRQ(PORTD_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);

	for(int i = 0; i < NUM_PORTS; i++)
		for(int j = 0; j< NUM_PINS_PER_PORT; j++)
			interrupt_pins[i][j] = -1;
}

void gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);
	interrupt_matrix[port_num][pin_num] = irqFun;

	if(irqMode == GPIO_IRQ_MODE_DISABLE)
		disable_interrupt_pins(port_num, pin_num);
	else
		enable_interrupt_pins(port_num, pin_num);

	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];

	switch(irqMode){
		case GPIO_IRQ_MODE_DISABLE:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
		case GPIO_IRQ_MODE_RISING_EDGE:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			port->PCR[pin_num] |= 0b1001 << PORT_PCR_IRQC_SHIFT;
			break;
		case GPIO_IRQ_MODE_FALLING_EDGE:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			port->PCR[pin_num] |= 0b1010 << PORT_PCR_IRQC_SHIFT;
			break;
		case GPIO_IRQ_MODE_BOTH_EDGES:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			port->PCR[pin_num] |= 0b1011 << PORT_PCR_IRQC_SHIFT;
			break;
		case GPIO_IRQ_CANT_MODES:
			break;
		default:
			break;
	}
}


/***********************************
*********get_interrupt_pin**********
************************************
* get_interrupt_pin returns the first pin in interrupt_pins which has
* isf pin asserted in the specified port
*		- port_num : port number that genereted the interrupt
*	OUTPUT:
*		- pin_num : pin number with isf = 1
*/
int get_interrupt_pin(int port_num){

	PORT_Type * addr_array[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_array[port_num];
	uint32_t isf;
	int pin_num = -1;
	int i = 0;
	while(interrupt_pins[port_num][i] != -1){
		pin_num = interrupt_pins[port_num][i];
		isf = port->PCR[pin_num] & PORT_PCR_ISF_MASK;
		isf >>= PORT_PCR_ISF_SHIFT;
		if(isf == 1)
			return pin_num;
		i++;
	}
	return -1;
}

void PORTA_IRQHandler(void){
	int pin_num = get_interrupt_pin(PA);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PA, pin_num);
		interrupt_matrix[PA][pin_num]();
	}

}

void PORTB_IRQHandler(void){
	int pin_num = get_interrupt_pin(PB);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PB, pin_num);
		interrupt_matrix[PB][pin_num]();
	}

}

void PORTC_IRQHandler(void){
	int pin_num = get_interrupt_pin(PC);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PC, pin_num);
		interrupt_matrix[PC][pin_num]();
	}
}

void PORTD_IRQHandler(void){
	int pin_num = get_interrupt_pin(PD);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PD, pin_num);
		interrupt_matrix[PD][pin_num]();
	}

}

void PORTE_IRQHandler(void){
	int pin_num = get_interrupt_pin(PE);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PE, pin_num);
		interrupt_matrix[PE][pin_num]();
	}
}

void PORT_ClearInterruptFlag (int port_num, int pin_num){

	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];
	port->PCR[pin_num] |= PORT_PCR_ISF_MASK;
}

static void enable_interrupt_pins(int port_num, int pin_num){
	for (int i = 0; i < NUM_PINS_PER_PORT; i++)
		if(interrupt_pins[port_num][i] == pin_num)
			break;
		else if(interrupt_pins[port_num][i] == -1){
			interrupt_pins[port_num][i] = pin_num;
			break;
		}
}

static void disable_interrupt_pins(int port_num, int pin_num){
	for (int i = 0; i < NUM_PINS_PER_PORT; i++)
		if(interrupt_pins[port_num][i] == pin_num){
			for(int j = i + 1; j < NUM_PINS_PER_PORT; j++)
				interrupt_pins[port_num][j-1] = interrupt_pins[port_num][j];
			interrupt_pins[port_num][NUM_PINS_PER_PORT-1] = -1;
			break;
		}
}
