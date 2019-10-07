/***************************************************************************//**
 * @file board_app.h
 * @brief Receives and sends information regarding board inclination
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/
#ifndef TP2_BOARD_APP_H
#define TP2_BOARD_APP_H

#define BA_MY_ID        0x01    // internal board id (only one internal board is presently supported)


/***************************************************************************//**
 * @brief Initialize app
******************************************************************************/
void ba_init();

/**************************************************************************//**
 * @brief Call periodically to update app status
 *****************************************************************************/
void ba_periodic();

#endif //TP2_BOARD_APP_H
