/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Grupo 1 - Labo de Micros 2019
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <gpio.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
// On Board User LEDs
#define PIN_LED_RED 	PORTNUM2PIN (PB,22)
#define PIN_LED_GREEN	PORTNUM2PIN (PE,26)
#define PIN_LED_BLUE    PORTNUM2PIN (PB,21) // PTB21

#define LED_ACTIVE      LOW

// Accelerometer pins
#define ACCEL_SCL_PIN	PORTNUM2PIN(PE, 24u)
#define ACCEL_SDA_PIN	PORTNUM2PIN(PE, 25u)

#define MCP25625_INTREQ_PIN	PORTNUM2PIN(PD, 0)

/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
