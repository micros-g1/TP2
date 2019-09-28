/**
 * @file CAN.h
 * @author Grupo 1 Labo de Micros
 * @date 24 Sep 2019
 * @brief CAN Interface
 * @details
 * Can Interface header.
 */

#ifndef CAN_MCP25625_CAN_H_
#define CAN_MCP25625_CAN_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @enum can_frame_t
 * @brief CAN Frame options
 */
typedef enum
{
	CAN_STANDARD_FRAME,	///< @brief CAN Standard frame.
	CAN_EXTENDED_FRAME	///< @brief CAN Extended frame
}can_frame_t;

/**
 * @struct can_fir_t
 * @brief CAN Frame Information Register
 * @details Information about CAN Frame
 */
typedef struct
{
	uint8_t	dlc;				///< @brief CAN Data Length Code
								///< @details Should be <= 8. Can be larger, but 8 will be used\n
	bool rtr;					///< @brief Remote transmission request.
	can_frame_t frame_type;		///< @brief CAN Frame type
}can_fir_t;

/**
 * @struct can_message_t
 * @brief CAN Message
 * @details Struct representing a CAN Message
 */
typedef struct
{
	can_fir_t fir;				///< @brief CAN Frame Register
	uint8_t message_id;			///< @brief CAN Message id
	uint8_t data[8];			///< @brief data. data length: fir.dlc . Not used if fir.rtr .
}can_message_t;

typedef void (*CAN_message_callback_t) (const can_message_t*);	///< @brief CAN Message Callback definition
typedef void (*CAN_rx_buffer_overflow_callback_t)(void);	///< @brief CAN RX Buffer overflow callback definition

/**
 * @brief CAN Initialization
 * @details Initializes CAN
 */
void CAN_init();

/**
 * @brief CAN Message available
 * @return *true* if message available
 */
bool CAN_message_available();

/**
 * @brief CAN Send Message
 * @param p_message Pointer to CAN Message to send
 * @return *true* if could send (else, transmit buffer is full)
 */
bool CAN_send(const can_message_t *p_message);

/**
 * @brief CAN Get Message
 * @details Returns next received CAN Message.
 * @param p_frame Pointer to CAN Message struct where received message will be saved.
 * @return *false* if no message available, without modifying contents of p_message.
 */
bool CAN_get(can_message_t *p_frame);

/**
 * @brief CAN Set RX Buffer Overflow callback
 * @details Sets a callback for buffer overflow.
 * @param callback Callback, *NULL* for no callback.
 */
void CAN_set_rx_buffer_overflow_callback(CAN_rx_buffer_overflow_callback_t callback);
/**
 * @brief CAN Set RX Buffer Overflow callback
 * @details Sets a callback for buffer overflow\n
 * If a callback is assigned, message will be removed from RX buffer as soon as callback is called.
 * If no callback, message will be kept in buffer until *CAN_get* is used.
 * @param callback Callback, *NULL* for no callback.
 */
void CAN_set_message_callback(CAN_message_callback_t callback);


#endif /* CAN_MCP25625_CAN_H_ */
