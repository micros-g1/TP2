/*
 * uart_test.c
 *
 *  Created on: Sep 25, 2019
 *      Author: Rocío Parra
 */
#include "uart.h"

#include "MK64F12.h"
#include "MK64F12_features.h"
#include "hardware.h"
#include "gpio.h"

#include "util/queue.h"
#include "util/SysTick.h"

UART_Type * uarts[UART_N_IDS] = {UART0, UART1, UART2, UART3, UART4};
bool uart_active[UART_N_IDS] = {false, false, false, false, false};

pin_t pins [UART_N_IDS][2] = {
				/* RX					TX 					*/
/* UART_0 */	{ PORTNUM2PIN(PB, 16),	PORTNUM2PIN(PB, 17)},
/* UART_1 */	{ PORTNUM2PIN(PC, 03),	PORTNUM2PIN(PC, 04)},
/* UART_2 */	{ PORTNUM2PIN(PD, 02),	PORTNUM2PIN(PD, 03)},
/* UART_3 */	{ PORTNUM2PIN(PC, 16),	PORTNUM2PIN(PC, 17)},
/* UART_4 */	{ PORTNUM2PIN(PC, 14),	PORTNUM2PIN(PC, 15)}
};

uint32_t clock_gating_masks[UART_N_IDS] = { SIM_SCGC4_UART0_MASK, SIM_SCGC4_UART1_MASK,
		SIM_SCGC4_UART2_MASK, SIM_SCGC4_UART3_MASK, SIM_SCGC1_UART4_MASK };



static volatile msg_queue_t tx_q[UART_N_IDS];
static volatile msg_queue_t rx_q[UART_N_IDS];


void uart_irq_handler(uint8_t id);

void uart_periodic(void);


void uartInit (uint8_t id, uart_cfg_t config){
	if (id >= UART_N_IDS || uart_active[id] == true)
		return;

	//////////////////////
	// initialize buffers
	//////////////////////
	q_init(&tx_q[id]);
	q_init(&rx_q[id]);

	UART_Type * uart = uarts[id];
	PORT_Type * addr_arrays[] = PORT_BASE_PTRS;
	pin_t rx_pin = pins[id][0];
	PORT_Type * port = addr_arrays[PIN2PORT(rx_pin)];
	pin_t tx_pin = pins[id][1];

	////////////////
	// CLOCK GATING
	////////////////
	if (id < 4) {
		SIM->SCGC4 |= clock_gating_masks[id];
	}
	else {
		SIM->SCGC1 |= clock_gating_masks[id];
	}

	//////////////
	// PCR CONFIG
	//////////////
	port->PCR[PIN2NUM(rx_pin)] = 0x0;
	port->PCR[PIN2NUM(tx_pin)] = 0x0;

	port->PCR[PIN2NUM(rx_pin)] |= PORT_PCR_MUX(0x3);
	port->PCR[PIN2NUM(tx_pin)] |= PORT_PCR_MUX(0x3);

	port->PCR[PIN2NUM(rx_pin)] |= PORT_PCR_IRQC(0x0); // disable interrupts
	port->PCR[PIN2NUM(tx_pin)] |= PORT_PCR_IRQC(0x0); // disable interrupts


	uart->C2 &= !(UART_C2_TE_MASK | UART_C2_RE_MASK); // disable uart for configuration
	uart->C1 = 0x0; // default config

	////////////////////
	// BAUDRATE CONFIG
	///////////////////
	uint16_t sbr, brfa;
	uint32_t clock, baudrate;
	clock = (id <= 1 ? __CORE_CLOCK__ : __CORE_CLOCK__ >> 1);
	baudrate = config.baudrate;
//	baudrate = config.baudrate == 0 ? UART_HAL_DEFAULT_BAUDRATE :
//			( config.baudrate > 0x1FFF ? UART_HAL_DEFAULT_BAUDRATE : config.baudrate );

	sbr = clock / (baudrate<<4);
	brfa = (clock << 1) / baudrate - (sbr << 5);

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);

	/////////////////
	// DATA CONFIG
	/////////////////
	uart->C1 |= UART_C1_M(config.eight_bit_word? 0 : 1);
	uart->C1 |= UART_C1_PE(config.parity);
	if (config.parity) {
		uart->C1 |= UART_C1_PT(config.odd_parity);
	}


	////////////////
	// ENABLE UART
	////////////////
	uart->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK;
	//UART0->S2 &= ~(0x06); // MSBF = 0, BRK13 = 0
	switch(id) { // enable interrupts!
	case 0: NVIC_EnableIRQ(UART0_RX_TX_IRQn); break;
	case 1: NVIC_EnableIRQ(UART1_RX_TX_IRQn); break;
	case 2: NVIC_EnableIRQ(UART2_RX_TX_IRQn); break;
	case 3: NVIC_EnableIRQ(UART3_RX_TX_IRQn); break;
	case 4: NVIC_EnableIRQ(UART4_RX_TX_IRQn); break;
	default: break;
	}

	systick_init();
	systick_add_callback(uart_periodic, SYSTICK_HZ_TO_RELOAD(80), PERIODIC);
	uart_active[id] = true;
}



bool uartIsRxMsg(uint8_t id)
{
	return uartGetRxMsgLength(id) > 0;
}

uint8_t uartGetRxMsgLength(uint8_t id)
{
	return rx_q[id].len;
}


uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant)
{
	uint8_t i = 0;
	while (i < cant && rx_q[id].len) {
		msg[i] = q_popfront(&rx_q[id]);
		i++;
	}
	return i;
}

uint8_t uartWriteMsg(uint8_t id, const char* msg, uint8_t cant)
{
	uint8_t i = 0;
	while (i < cant && !q_isfull(&rx_q[id])) {
		q_pushback(&tx_q[id], msg[i]);
		i++;
	}
	return i;
}


bool uartIsTxMsgComplete(uint8_t id)
{
	return tx_q[id].len == 0;
}


void uart_periodic(void)
{
	unsigned int i;
	for (i = 0; i < UART_N_IDS; i++) {
		if (uart_active[i]) {
			// get data
			while ((uarts[i]->S1 & UART_S1_RDRF_MASK) && !q_isfull(&rx_q[i])) {
				q_pushback(&rx_q[i], uarts[i]->D);
			}

			// send data
			while ((uarts[i]->S1 & UART_S1_TDRE_MASK) && tx_q[i].len) {
				uarts[i]->D = q_popfront(&tx_q[i]);
			}
		}
	}
}



void uart_putchar (UART_Type * channel, char ch)
{
	/* Wait until space is available in the FIFO */
	while(!(channel->S1 & UART_S1_TDRE_MASK));
	/* Send the character */
	channel->D = (uint8_t)ch;
}

char uart_getchar (UART_Type * channel)
{
	/* Wait until character has been received */
	while (!(channel->S1 & UART_S1_RDRF_MASK));
	/* Return the 8-bit data from the receiver */
	return channel->D;
}

int uart_getchar_present (UART_Type * channel)
{
	return (channel->S1 & UART_S1_RDRF_MASK);
}


void uart_irq_handler(uint8_t id)
{
	uint8_t tmp=uarts[id]->S1; // Read Status (necessary to clear interrupt request)
	q_pushback(&rx_q[id], uarts[id]->D);
}

__ISR__ UART0_RX_TX_IRQHandler (void)
{
	uart_irq_handler(0);
}

__ISR__ UART1_RX_TX_IRQHandler (void)
{
	uart_irq_handler(1);
}

__ISR__ UART2_RX_TX_IRQHandler (void)
{
	uart_irq_handler(2);
}

__ISR__ UART3_RX_TX_IRQHandler (void)
{
	uart_irq_handler(3);
}

__ISR__ UART4_RX_TX_IRQHandler (void)
{
	uart_irq_handler(4);
}
