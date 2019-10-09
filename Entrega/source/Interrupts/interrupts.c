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
#include <stdlib.h>

#define NUM_PORTS			5
#define NUM_PINS_PER_PORT	32
pinIrqFun_t interrupt_matrix[NUM_PORTS][NUM_PINS_PER_PORT];

static void PORT_IRQHandler(uint8_t port_id);

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
	for(int i = 0 ; i < NUM_PORTS ; i++)
		for(int j = 0 ; j < NUM_PINS_PER_PORT ; j++)
			interrupt_matrix[i][j] = NULL;
}

void gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);

	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];

	if(irqMode != GPIO_IRQ_MODE_DISABLE)
		interrupt_matrix[port_num][pin_num] = irqFun;
	switch(irqMode)
	{
		case GPIO_IRQ_MODE_DISABLE:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			interrupt_matrix[port_num][pin_num] = NULL;
			break;
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
		case GPIO_IRQ_MODE_LOGIC_0:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			port->PCR[pin_num] |= 0b1000 << PORT_PCR_IRQC_SHIFT;
			break;
		case GPIO_IRQ_MODE_LOGIC_1:
			port->PCR[pin_num] &= ~(0b1111 << PORT_PCR_IRQC_SHIFT);
			port->PCR[pin_num] |= 0b1100 << PORT_PCR_IRQC_SHIFT;
			break;
		case GPIO_IRQ_CANT_MODES:
			break;
		default:
			break;
	}
}

void PORTA_IRQHandler(void){
	PORT_IRQHandler(PA);
}

void PORTB_IRQHandler(void){
	PORT_IRQHandler(PB);
}

void PORTC_IRQHandler(void){
	PORT_IRQHandler(PC);
}

void PORTD_IRQHandler(void){
	PORT_IRQHandler(PD);
}

void PORTE_IRQHandler(void){
	PORT_IRQHandler(PE);
}

void PORT_IRQHandler(uint8_t port_id)
{
	if(port_id >= NUM_PORTS)
		return;
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_id];
	uint32_t port_flags = port->ISFR;
	for(int i = 1, j = 0 ; port_flags != 0; i <<= 1 , j++ )
	{
		if(port_flags & i)
		{
			//Clear flag of port flags and port
			port_flags &= ~i;
			port->ISFR |= i;
			if(interrupt_matrix[port_id][j] != NULL)
				interrupt_matrix[port_id][j]();
			//Clean again (for logic level interrupts!)
			port->ISFR |= i;
		}
	}
}
