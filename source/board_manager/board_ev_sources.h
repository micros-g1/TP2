/***************************************************************************//**
 * @file board_ev_sources.h
 * @brief Manages event generation for board app
 * @author Grupo 1 Laboratorio de Microprocesadores
******************************************************************************/

#ifndef TP2_BOARD_EV_SOURCES_H
#define TP2_BOARD_EV_SOURCES_H

/**
 * @brief Initialize event sources
 */
void be_init();

/**
 * @brief Call periodically to update data base with latest data
 */
void be_periodic();


#endif //TP2_BOARD_EV_SOURCES_H
