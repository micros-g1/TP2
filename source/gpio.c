/*
 * gpio.c
 *
 *  Created on: 24 ago. 2019
 *      Author: Tomas, Lisandro, Rocio, Gonzalo
 */
#include "gpio.h"
#include "MK64F12.h"
#include <stdint.h>

static void initial_conf_pcr(int port_num, int pin_num);
static void initial_conf_gpio(int port_num, int pin_num);
static void set_input_mode(int port_num, int pin_num);
static void set_output_mode(int port_num, int pin_num);
static void set_input_pulldown_mode(int port_num, int pin_num);
static void set_input_pullup_mode(int port_num, int pin_num);

static void enable_interrupt_pins(int port_num, int pin_num);
static void disable_interrupt_pins(int port_num, int pin_num);
bool gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun);


pinIrqFun_t interrupt_matrix[5][32];
int interrupt_pins[5][32];

void interrupts_init(){
	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTB_IRQn);
	NVIC_EnableIRQ(PORTC_IRQn);
	NVIC_EnableIRQ(PORTD_IRQn);
	NVIC_EnableIRQ(PORTE_IRQn);
	int i, j;
	for(i = 0; i < 5; i++)
		for(j = 0; j< 32; j++)
			interrupt_pins[i][j] = -1;
}

//queda todo como en reset
void gpioMode (pin_t pin, uint8_t mode){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);

	// Poner el registo PCR en un estado casi de reset
	initial_conf_pcr(port_num, pin_num);
	// poner los pines de los regstros de GPIO en estado de reset
	initial_conf_gpio(port_num, pin_num);

	switch(mode){
	case INPUT:
		set_input_mode(port_num, pin_num);
		break;
	case OUTPUT:
		set_output_mode(port_num, pin_num);
		break;
	case INPUT_PULLDOWN:
		set_input_pulldown_mode(port_num, pin_num);
		break;
	case INPUT_PULLUP:
		set_input_pullup_mode(port_num, pin_num);
		break;
	default:
		//Excepcion!!!
		break;
	}
}

/***********************************
*********initial_conf_pcr**********
************************************
* initial_conf_pcr deja configurada a una estructura
* PORT_Type como quedaria configurada luego de un reset
* de maquina para el pin N en especfico, a excepcion
* del MUX, que es seteado a GPIO:
*	MUX = 001
*	DSE = 1 (PTA0 a PTA5), 0 (c.c)
*	PFE = 0
*	SRE = 0
*	PE = 0
*	PS = 0
*	ISF = 1
*	LK = 0
*	INPUT:
*		- port_conf : Estructura que sera modificada
*		- port_num : numero de puerto cuyos valores de port_conf seran
*		- pin_num : numero de pin cuyos valores de port_conf seran actualizados.
*	OUTPUT:
*		void. Todo se cambia por referencia.
*/
static void initial_conf_pcr (int port_num, int pin_num){

	PORT_Type * addr_array[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_array[port_num];

	port->PCR[pin_num] = 0;
	port->PCR[pin_num] |= 1 << PORT_PCR_MUX_SHIFT;
	port->PCR[pin_num] |= 1 << PORT_PCR_ISF_SHIFT;
	if(port_num == PA && pin_num <= 5){
		port->PCR[pin_num] |= 1 << PORT_PCR_DSE_SHIFT;
	}

}

/***********************************
*********initial_conf_gpio**********
************************************
* initial_conf_gpio deja configurado a una estructura
* GPIO_Type como quedaria configurada luego de un reset
* de maquina para el pin N en especfico. Esto quiere decir:
*	pdor todo a cero
*	psor todo a cero
*	pcor todo a cero
*	ptor todo a cero
*	pddr todo a cero
*	pdir no se toca porque es de input y no se puede modificar
*
*	INPUT:
*		- gpio_conf : Estructura que sera modificada con los valores de reset por referencia
*		- pin_num : numero de pin cuyos valores de gpio_conf seran actualizados a reset.
*	OUTPUT:
*		void.
*/
static void initial_conf_gpio(int port_num, int pin_num){

	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];

	gpio->PCOR &= ~(1 << pin_num);
	gpio->PDDR &= ~(1 << pin_num);
	gpio->PDOR &= ~(1 << pin_num);
	gpio->PSOR &= ~(1 << pin_num);
	gpio->PTOR &= ~(1 << pin_num);

}
/***********************************
*********set_input_mode**********
************************************
* set_input_mode configures the specified pin in input mode
* and sets the pull enable (PE) to 0 (disabled).
* The pull select (PS) value is untouched, so the user should check in the reference manual
* what value it will be on once the PE is enabled again.
*	INPUT:
*		- port_num : port that will be set to input mode (A,B,C,...), as given by define PIN2PORT
*		- pin_num : pin number that will be set to input mode.
*	OUTPUT:
*		void.
*/
static void set_input_mode(int port_num, int pin_num){
	/*from the MK64 reference manual, section 11.5.1, PORTx_PCRn field descriptions
	 * 	PE:
			0 Internal pullup or pulldown resistor is not enabled on the corresponding pin.*/
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];
	port->PCR[pin_num] &=  ~((uint32_t)2);		//clears bit1 (PE)

	/*from the MK64 reference manual, section 55.2.6:
	0 : Pin is configured as general-purpose input, for the GPIO function.*/
	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];
	gpio->PDDR &= ~(1 << pin_num);
}

/***********************************
*********set_output_mode**********
************************************
* set_output_mode configures the specified pin in output mode
*	INPUT:
*		- port_num : port that will be set to output mode (A,B,C,...), as given by define PIN2PORT
*		- pin_num : pin number that will be set to input mode as given by define PIN2NUM
*	OUTPUT:
*		void.
*/
static void set_output_mode(int port_num, int pin_num){

	/*from the MK64 reference manual, section 55.2.6:
	1 : Pin is configured as general-purpose output, for the GPIO function.*/
	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];
	gpio->PDDR |= (1 << pin_num);
}

/***********************************
*********set_input_pulldown_mode****
************************************
* set_input_pulldown_mode configures the specified pin in input mode,
* with its pull down resistor enabled.
*	INPUT:
*		- port_num : port that will be set to input pulldown mode (A,B,C,...), as given by define PIN2PORT
*		- pin_num : pin number that will be set to input pulldown mode.
*	OUTPUT:
*		void.
*/
static void set_input_pulldown_mode(int port_num, int pin_num){
	set_input_mode(port_num, pin_num);
	/*from the MK64 reference manual, section 11.5.1, PORTx_PCRn field descriptions
	 * 	PE:
	 		1 : Internal pullup or pulldown resistor is enabled on the corresponding pin, if the pin is configured as a
digital input.
	 *	PS:
			0 : Internal pulldown resistor is enabled on the corresponding pin, if the corresponding PE field is set.*/
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];
	port->PCR[pin_num] |= (1 & ~(1 << 1));	//sets b0 to 1, enabling PE, and sets bit1 to 0 for pulldown.

}

/***********************************
*********set_input_pullup_mode**********
************************************
* set_input_pullup_mode configures the specified pin in input mode,
* with its pull up resistor enabled.
*	INPUT:
*		- port_num : port that will be set to input pullup mode (A,B,C,...), as given by define PIN2PORT
*		- pin_num : pin number that will be set to input pullup mode.
*	OUTPUT:
*		void.
*/
static void set_input_pullup_mode(int port_num, int pin_num){
	set_input_mode(port_num, pin_num);
	/*from the MK64 reference manual, section 11.5.1, PORTx_PCRn field descriptions
	 * 	PE:
	 		1 : Internal pullup or pulldown resistor is enabled on the corresponding pin, if the pin is configured as a
digital input.
	 *	PS:
			1 : Internal pullup resistor is enabled on the corresponding pin, if the corresponding PE field is set.*/
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];
	port->PCR[pin_num] |= (uint32_t) 0x2;			//sets both bit0 and bit1, setting PS in pullup and enabling PE respectively.
}

void gpioWrite (pin_t pin, bool value){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);

	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];
	if(value)
		gpio->PSOR |= (1 << pin_num) ;
	else
		gpio->PCOR |= (1 << pin_num);

}

void gpioToggle (pin_t pin){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);
	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];
	gpio->PTOR |= (1 << pin_num);
}

bool gpioRead (pin_t pin){
	int port_num = PIN2PORT(pin);
	int pin_num = PIN2NUM(pin);
	GPIO_Type * addr_array[] = GPIO_BASE_PTRS;
	GPIO_Type * gpio = addr_array[port_num];

	return gpio->PDIR & (1 << pin_num);
}

bool gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun){
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
	}
	interrupt_matrix[PA][pin_num]();
}

void PORTB_IRQHandler(void){
	int pin_num = get_interrupt_pin(PB);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PB, pin_num);
	}
	interrupt_matrix[PB][pin_num]();
}

void PORTC_IRQHandler(void){
	int pin_num = get_interrupt_pin(PC);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PC, pin_num);
	}
	interrupt_matrix[PC][pin_num]();
}

void PORTD_IRQHandler(void){
	int pin_num = get_interrupt_pin(PD);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PD, pin_num);
	}
	interrupt_matrix[PD][pin_num]();
}

void PORTE_IRQHandler(void){
	int pin_num = get_interrupt_pin(PE);
	if (pin_num != -1){
		PORT_ClearInterruptFlag(PE, pin_num);
	}
	interrupt_matrix[PE][pin_num]();
}

void PORT_ClearInterruptFlag (int port_num, int pin_num){

	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	PORT_Type * port = addr_arrays[port_num];
	port->PCR[pin_num] |= PORT_PCR_ISF_MASK;
}

static void enable_interrupt_pins(int port_num, int pin_num){
	for (int i = 0; i < 32; i++)
		if(interrupt_pins[port_num][i] == pin_num)
			break;
		else if(interrupt_pins[port_num][i] == -1){
			interrupt_pins[port_num][i] = pin_num;
			break;
		}
}

static void disable_interrupt_pins(int port_num, int pin_num){
	for (int i = 0; i < 32; i++)
		if(interrupt_pins[port_num][i] == pin_num){
			for(int j = i + 1; j < 32; j++)
				interrupt_pins[port_num][j-1] = interrupt_pins[port_num][j];
			interrupt_pins[port_num][31] = -1;
			break;
		}
}
