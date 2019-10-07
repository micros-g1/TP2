/***************************************************************************//**
  @file     UART.c
  @brief    UART Driver for K64F. Non-Blocking.
  @author   Grupo 1 Laboratorio de Microprocesadores
 ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_N_IDS   5

//#define UART_HAL_DEFAULT_BAUDRATE 4800


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	uint8_t parity : 1; 		// true for using parity bit
	uint8_t odd_parity : 1;		// true for odd parity (if parity is true)
	uint8_t eight_bit_word : 1;	// true for eight bit words, false for nine
	uint32_t baudrate;
} uart_cfg_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, word size)
*/
void uartInit (uint8_t id, uart_cfg_t config);

/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has being received
*/
bool uartIsRxMsg(uint8_t id);

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
*/
uint8_t uartGetRxMsgLength(uint8_t id);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t uartReadMsg(uint8_t id, uint8_t * msg, uint8_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transferred
 * @param cant Desired quantity of bytes to be transferred
 * @return Real quantity of bytes to be transferred
*/
uint8_t uartWriteMsg(uint8_t id, const uint8_t* msg, uint8_t cant);

/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
*/
bool uartIsTxMsgComplete(uint8_t id);



/*******************************************************************************
 * BLOCKING FUNCTIONS
 ******************************************************************************/

/**
 * @brief transmit one character, blocking
 * @param id UART's number
 * @param ch character to send
 */
void uart_putchar (uint8_t id, uint8_t ch);

/**
 * @brief receive one character, blocking
 * @param id UART's number
 * @return character received
 */
uint8_t uart_getchar (uint8_t id);

/**
 * @brief checks whether any chars have been received. to be used before uart_getchar, and not with uartReadMsg!
 * @param id UART's number
 * @return true if char present
 */
int uart_getchar_present (uint8_t id);




/*******************************************************************************
 ******************************************************************************/

#endif // _UART_H_
