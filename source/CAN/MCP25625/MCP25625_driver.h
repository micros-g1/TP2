/**
 * @file MCP25625_driver.h
 * @author Grupo 1 Labo de Micros
 * @date 21 Sep 2019
 * @brief MCP25625 Driver.
 * @details
 * Driver for MCP25625 - CAN Controller with Integrated Transceiver
 * A typical CAN solution consists of a CAN controller that
 * implements the CAN protocol, and a CAN transceiver
 * that serves as the interface to the physical CAN bus.
 * The MCP25625 integrates both the CAN controller and
 * the CAN transceiver. Therefore, it is a complete CAN
 * solution that can be easily added to a microcontroller
 * with an SPI interface.
 * This file aims to implement functionality described by datasheet,
 * and to model the registers.
 * @see http://ww1.microchip.com/downloads/en/DeviceDoc/20005282B.pdf
 */

// Documented with Doxygen

//TODO: Doxygen struct detailed definition?
//TODO: Check compiler's bitfield implementation.

#ifndef CAN_MCP25625_MCP25625_DRIVER_H_
#define CAN_MCP25625_MCP25625_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @enum mcp25625_instruction_t
 * @brief MCP25625 SPI Instructions.
 * @details Instruction Bytes for all operations
 */
typedef enum {

    MCP_RESET				= 0xC0,	///< Resets the internal registers to
									///		the default state, sets Configuration mode.
	MCP_READ				= 0x03,	///< Reads data from the register beginning
									///		at the selected address
	MCP_READ_RX_BUFFER		= 0x90,	///< When reading a receive buffer,
									///		reduces the overhead of a normal READ
									///		command by placing the Address Pointer at one
									///		of four locations, as indicated by ‘nm’
									/// 	(1001 0nm0)
	MCP_WRITE				= 0x02,	///< Writes data to the register beginning at the
									///		selected address.
	MCP_LOAD_TX_BUFFER		= 0x40,	///< When loading a transmit buffer, reduces the
									///		overhead of a normal WRITE command by placing
									///		the Address Pointer at one of six locations, as
									///		indicated by ‘abc’. (0100 0abc)
	MCP_RTS 				= 0x80,	///< Inst	ructs the controller to begin the message
									///		transmission sequence for any of the transmit
									///		buffers. 1000 0nnn -> nnn=(TXB2,TXB1,TXB0)
	MCP_READ_STATUS			= 0xA0,	///< Quick polling command that reads several Status
									///		bits for transmit and receive functions.
	MCP_RX_STATUS			= 0xB0,	///< Quick polling command that indicates a filter
									///		match and message type 	(standard, extended
									///		and/or remote) of the received message.
	MCP_BIT_MODIFY			= 0x05,	///< Allows the user to set or clear individual bits
									/// 	in a particular register. Not for all
									///		registers.
}mcp25625_instruction_t;



/*******************************************************************************
 *******************************************************************************
                        Typedefs for Registers and driver
 *******************************************************************************
 ******************************************************************************/

/**
 * @enum mcp25625_opmode_t
 * @brief Different MCP25625 Operation Modes.
 */
typedef enum
{
	NORMAL_OPERATION	= 0x00,		///< Normal Operation mode
	SLEEP_MODE			= 0x01,		///< Sleep mode
	LOOPBACK_MODE		= 0x02,		///< Loopback mode
	LISTEN_ONLY			= 0x03,		///< Listen-Only mode
	CONFIG_MODE			= 0x04		///< Configuration mode
}mcp25625_opmode_t;

/**
 * @enum mcp25625_txb_id_t
 * @brief MCP25625 Transmit buffers id.
 * @details MCP25625 has three Transmit buffers. Enum for transfer buffer id.
 */
typedef enum {
    TXB0	=	0,					///< Transmit buffer TXB0
    TXB1	=	1,					///< Transmit buffer TXB1
    TXB2	=	2					///< Transmit buffer TXB2
}mcp25625_txb_id_t;

/**
 * @enum mcp25625_rxb_id_t
 * @brief MCP25625 Receive buffers id.
 * @details MCP25625 has two receive buffers. Enum for receive buffer id.
 */
typedef enum {
    RXB0	=	0,					///< Receive buffer RXB0
    RXB1	=	1,					///< Receive buffer RXB1
}mcp25625_rxb_id_t;

/**
 * @enum mcp25625_txb_rts_flag_t
 * @brief MCP25625 Transmit buffers Request to send Mask
 * @details MCP25625 Masks for request to send.
 * These flags are used internally by the MCP25625's request to send instruction.
 */
typedef enum {
	TXB0_RTS	=	0x01,			///< TXB0 Request to send
	TXB1_RTS	=	0x02,			///< TXB1 Request to send
    TXB2_RTS	=	0x04,			///< TXB2 Request to send
}mcp25625_txb_rts_flag_t;

/**
 * @enum mcp25625_icod_t
 * @brief MCP25625 Interrupt Flag Code.
 * @details Indicates reason of interrupt.
 */
typedef enum
{
	NO_INTERRUPT 		= 0x00,		///< No Interrupt
	ERROR_INTERRUPT		= 0x01,		///< Error interrupt
	WAKE_UP_INTERRUPT	= 0x02,		///< Wake-up interrupt
	TXB0_INTERRUPT		= 0x03,		///< TXB0 interrupt
	TXB1_INTERRUPT		= 0x04,		///< TXB1 interrupt
	TXB2_INTERRUPT		= 0x05,		///< TXB2 interrupt
	RXB0_INTERRUPT		= 0x06,		///< RXB0 interrupt
	RXB1_INTERRUPT		= 0x07		///< RXB1 interrupt
}mcp25625_icod_t;

/**
 * @enum mcp25625_prescaler_t
 * @brief MCP25625 CLKOUT Pin Prescaler.
 * @details Possible prescalers for CLKOUT pin frequency
 */
typedef enum
{
	SYSCLK_DIV_1	= 0x00,		///< System Clock/1
	SYSCLK_DIV_2	= 0x01,		///< System Clock/2
	SYSCLK_DIV_4	= 0x02,		///< System Clock/4
	SYSCLK_DIV_8	= 0x03,		///< System Clock/8
}mcp25625_prescaler_t;

/**
 * @enum mcp25625_priority_t
 * @brief MCP25625 Transmit Buffer Priority.
 * @details Priorities that can be assigned to a transmit buffer.
 */
typedef enum
{
	HIGHEST_PRIORITY	=	0x03,	///< Highest message priority
	HIGH_PRIORITY		= 	0x02,	///< High intermediate message priority
	LOW_PRIORITY		=	0x01,	///< Low intermediate message priority
	LOWEST_PRIOIRTY		=	0x00	///< Lowest message priority
}mcp25625_priority_t;

/**
 * @enum mcp25625_accfltr_id_t
 * @brief Acceptance Filter id
 * @details Id of different Acceptance filters.
 */
typedef enum
{
	RXF0 = 0x00,		///< Acceptance Filter 0
	RXF1 = 0x01,		///< Acceptance Filter 1
	RXF2 = 0x02,		///< Acceptance Filter 2
	RXF3 = 0x03,		///< Acceptance Filter 3
	RXF4 = 0x04,		///< Acceptance Filter 4
	RXF5 = 0x05,		///< Acceptance Filter 5
	RXF0_RO = 0x06,		///< Acceptance Filter 0 (Used when rollover to RXB1 occurs)
	RXF1_RO = 0x07		///< Acceptance Filter 1 (Used when rollover to RXB1 occurs)
}mcp25625_accfltr_id_t;

/**
 * @enum mcp25625_accfltr_rxb0_id_t
 * @brief Acceptance Filter id for RXB0
 * @details Id of different Acceptance filters\n
 * Equivalent to mcp25625_accfltr_id_t, but defined to prevent compiler warning
 * when creating bitfield with 1 bit (see mcp25625_rxb0ctrl_t)
 */
typedef enum
{
	RXF0_RXB0 = 0x00,		///< Acceptance Filter 0
	RXF1_RXB0 = 0x01,		///< Acceptance Filter 1
}mcp25625_accfltr_rxb0_id_t;

/**
 * @enum mcp25625_accmsk_id_t
 * @brief Acceptance Mask id
 * @details Id of different Acceptance masks\n
 */
typedef enum
{
	RXM0 = 0x00,		///< Acceptance Filter 0
	RXM1 = 0x01,		///< Acceptance Filter 1
}mcp25625_accmsk_id_t;

/**
 * @enum mcp25625_rbopmod_t
 * @brief Receive Buffer Operating mode
 */
typedef enum
{
	RECEIVE_ANY_MESSAGE		=	0x03,			///< See mcp25625_rxb0ctrl_t or mcp25625_rxb1ctrl_t documentation
	RECEIVE_VALID_MESSAGES	=	0x00						///See mcp25625_rxb0ctrl_t or mcp25625_rxb1ctrl_t documentation
}mcp25625_rbopmod_t;

/**
 * @enum mcp25625_msg_type_t
 * @brief MCP25625 Message type.
 * @details Indicates received message type.
 */
typedef enum {
    STD_DATA_FRAME		=	0x00,				///< Standard Data Frame
    STD_REMOTE_FRAME	=	0x01,				///< Standard Remote Frame
    EXT_DATA_FRAME		=	0x02,				///< Extended Data Frame
	EXT_REMOTE_FRAME 	=	0x03				///< Extended Remote Frame
}mcp25625_msg_type_t;

/**
 * @enum mcp25625_rec_msg_info_t
 * @brief MCP25625 Received message info
 * @details Indicates if rx message received, and its buffer
 */
typedef enum {
    NO_RX_MSG		=	0x00,				///< No RX Message
    RXB0_MSG		=	0x01,				///< Message in RXB0
    RXB1_MSG		=	0x02,				///< Message in RXB1
	BOTH_RX_MSG 	=	0x03				///< Messages in Both Buffers
}mcp25625_rec_msg_info_t;


/*******************************************************************************
 *******************************************************************************
                             Memory Map (Addresses)
 *******************************************************************************
 ******************************************************************************/

/**
 * @enum mcp25625_addr_t
 * @brief MCP25625 Registers' Addresses
 * @details Enum indicating the address of the different registers
 * and offsets for accessing specific registers within certain regions
 */
typedef enum {
	CTRL_OFFST		=	-1,		///< Offset to add to TXBx_ADDR or RXBx_ADDR for accessing CTRL Register
    SIDH_OFFST		=	0,		///< Offset to add to RXFx_ADDR, RXMx_ADDR, TXBx_ADDR or RXBx_ADDR for pointing to SIDH
	SIDL_OFFST		=	1,		///< Offset to add to RXFx_ADDR, RXMx_ADDR, TXBx_ADDR or RXBx_ADDR for pointing to SIDL
	EIF8_OFFST		=	2,		///< Offset to add to RXFx_ADDR, RXMx_ADDR, TXBx_ADDR or RXBx_ADDR for pointing to EID8
	EIF0_OFFST		=	3,		///< Offset to add to RXFx_ADDR, RXMx_ADDR, TXBx_ADDR or RXBx_ADDR for pointing to EID0
	DLC_OFFST		=	4,		///< Offset to add to TXBx_ADDR or RXBx_ADDR for pointing to DLC
	DATA_OFFST		=	5,		///< Offset to add to TXBx_ADDR or RXBx_ADDR for pointing to start of DATA Buffer
	RXF0_ADDR		=	0x00,	///< RXF0 Region Address
	RXF1_ADDR		=	0x04,	///< RXF1 Region Address
	RXF2_ADDR		=	0x08,	///< RXF2 Region Address
	RXF3_ADDR		=	0x10,	///< RXF3 Region Address
	RXF4_ADDR		=	0x14,	///< RXF4 Region Address
	RXF5_ADDR		=	0x18,	///< RXF5 Region Address
	RXM0_ADDR		=	0x20,	///< RXM0 Region Address
	RXM1_ADDR		=	0x24,	///< RXM1 Region Address
	TXB0_ADDR		=	0x31,	///< TXB0 Region Address
	TXB1_ADDR		=	0x41,	///< TXB1 Region Address
	TXB2_ADDR		=	0x51,	///< TXB2 Region Address
	RXB0_ADDR		=	0x61,	///< RXB0 Region Address
	RXB1_ADDR		=	0x71,	///< RXB1 Region Address
	BFPCTRL_ADDR	=	0x0C,	///< BFPCTRL Address
	TXRTSCTRL_ADDR	= 	0x0D,	///< TXRTSCTRL Address
	CANSTAT_ADDR	=	0x0E,	///< CANSTAT Address
	CANCTRL_ADDR	=	0x0F,	///< CANCTRL Address
	TEC_ADDR		=	0x1C,	///< TEC Address
	REC_ADDR		=	0x1D,	///< REC Address
	CNF3_ADDR		=	0x28,	///< CNF3 Address
	CNF2_ADDR		=	0x29,	///< CNF2 Address
	CNF1_ADDR		=	0x2A,	///< CNF1 Address
	CANINTE_ADDR	=	0x2B,	///< CANINTE Address
	CANINTF_ADDR	=	0x2C,	///< CANINTF Address
	EFLG_ADDR		=	0x2D,	///< EFLG Address
}mcp25625_addr_t;

/********************************************************************************
 ********************************************************************************
                      Configuration Registers Description
 ********************************************************************************
 ********************************************************************************
 * For configuration registers, masks for configuration are provided			*
 * Also, for each configuration register, an union with the whole byte			*
 * and access to each specific element is provided.								*
 ********************************************************************************/

/******************************************************************************
 * BFPCTRL: !RxnBF PIN CONTROL AND STATUS REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_bfpctrl_mask_t
 * @brief !RxnBF PIN CONTROL AND STATUS REGISTER Masks
 */
typedef enum
{
	B0BFM	=	0x01,			///< !Rx0BF Pin Operation mode bit Mask
	B1BFM	=	0x02,			///< !Rx1BF Pin Operation mode bit Mask
	B0BFE	=	0x04,			///< !Rx0BF Pin function Enable bit Mask
	B1BFE	=	0x08, 			///< !Rx1BF Pin function Enable bit Mask
	B0BFS	=	0x10,			///< !Rx0BF Pin State bit Mask
	B1BFS	=	0x20			///< !Rx1BF Pin State bit Mask
}mcp25625_bfpctrl_mask_t;

/**
 * @union mcp25625_bfpctrl_t
 * @brief !RxnBF PIN CONTROL AND STATUS REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t b0bfm		:1;		///< !Rx0BF Pin Operation mode\n
		//!< 1 = Pin is used as an interrupt when a valid message is loaded into RXB0\n
		//!< 0 = Digital Output mode.
		uint8_t b1bfm		:1;		///< !Rx1BF Pin Operation mode\n
		//!< 1 = Pin is used as an interrupt when a valid message is loaded into RXB1\n
		//!< 0 = Digital Output mode.
		uint8_t b0bfe		:1;		///< !Rx0BF Pin function Enable\n
		//!< 1 = Pin function is enabled, operation mode is determined by the B0BFM bit\n
		//!< 0 = Pin function is disabled, pin goes to the high-impedance state.
		uint8_t b1bfe		:1;		///< !Rx1BF Pin function Enable\n
		//!< 1 = Pin function is enabled, operation mode is determined by the B1BFM bit\n
		//!< 0 = Pin function is disabled, pin goes to the high-impedance state.
		uint8_t b0bfs		:1;	///< !Rx0BF Pin State bit\n
		//!< Reads as ‘0’ when Rx0BF is configured as an interrupt.
		//!<
		uint8_t b1bfs		:1;		///< Rx1BF Pin State bit\n
		//!< Reads as ‘0’ when Rx1BF is configured as an interrupt.
		//!<
		uint8_t				:2;		///< Unimplemented
		//!< Read as ‘0’
		//!<
	};
}mcp25625_bfpctrl_t;

/******************************************************************************
 * TXRTSCTRL: !TxnRTS PIN CONTROL AND STATUS REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_txrtsctrl_mask_t
 * @brief !TxnRTS PIN CONTROL AND STATUS REGISTER Masks
 */
typedef enum
{
	B0RTSM	=	0x01,			///< !Tx0RTS Pin mode bit Mask
	B1RTSM	=	0x02,			///< !Tx1RTS Pin mode bit Mask
	B2RTSM	=	0x04,			///< !Tx2RTS Pin mode bit Mask
	B0RTS	=	0x08,			///< !Tx0RTS Pin State bit Mask
	B1RTS	=	0x10,			///< !Tx1RTS Pin State bit Mask
	B2RTS	=	0x20			///< !Tx2RTS Pin State bit Mask
}mcp25625_txrtsctrl_mask_t;

/**
 * @union mcp25625_txrtsctrl_t
 * @brief !TxnRTS PIN CONTROL AND STATUS REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t b0rtsm		:1;			///< !Tx0RTS Pin mode\n
		//!< 1 if Pin is used to request message transmission of TXB0 buffer (on falling edge)\n
		//!< 0 if Digital input.
		uint8_t b1rtsm		:1;			///< !Tx1RTS Pin mode\n
		//!< 1 if Pin is used to request message transmission of TXB1 buffer (on falling edge)\n
		//!< 0 if Digital input.
		uint8_t b2rtsm		:1;			///< !Tx2RTS Pin mode\n
		//!< 1 if Pin is used to request message transmission of TXB2 buffer (on falling edge)\n
		//!< 0 if Digital input.
		uint8_t b0rts		:1;			///< !Tx0RTS Pin State\n
		//!< - Reads state of Tx0RTS pin when in Digital Input mode\n
		//!< - Reads as ‘0’ when pin is in ‘Request-to-Send’ mode.
		uint8_t b1rts		:1;			///< !Tx1RTS Pin State\n
		//!< - Reads state of Tx1RTS pin when in Digital Input mode\n
		//!< - Reads as ‘0’ when pin is in ‘Request-to-Send’ mode.
		uint8_t b2rts		:1;			///< !Tx2RTS Pin State\n
		//!< - Reads state of Tx2RTS pin when in Digital Input mode\n
		//!< - Reads as ‘0’ when pin is in ‘Request-to-Send’ mode.
		uint8_t				:2;			///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
	};
}mcp25625_txrtsctrl_t;

/******************************************************************************
 * CANSTAT: CAN STATUS REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_canstat_mask_t
 * @brief CAN STATUS REGISTER Masks
 */
typedef enum
{
	ICOD	=	0x0E,			///< Operation Mode bits Mask
	ICOD_0	=	0x02,			///< Operation Mode bit 0 Mask
	ICOD_1	=	0x04,			///< Operation Mode bit 1 Mask
	ICOD_2	=	0x08,			///< Operation Mode bit 2 Mask
	OPMOD	=	0xE0,			///< Interrupt Flag Code bits Mask
	OPMOD_0	=	0x20,			///< Interrupt Flag Code bit 0 Mask
	OPMOD_1	=	0x40,			///< Interrupt Flag Code bit 1 Mask
	OPMOD_2	=	0x80,			///< Interrupt Flag Code bit 2 Mask
}mcp25625_canstat_mask_t;

/**
 * @union mcp25625_canstat_t
 * @brief CAN STATUS REGISTER
 */
typedef union
{
	uint8_t register_byte;							///< Whole register
	struct
	{
		uint8_t						:1;		///< Unimplemented
		//!< Read as ‘0’
		//!<
		mcp25625_icod_t icod		:3;		///< Interrupt Flag Code
		//!<
		uint8_t						:1;		///< Unimplemented
		//!< Read as ‘0’
		//!<
		mcp25625_opmode_t opmode	:3;		///< Operation Mode
		//!<
	};
}mcp25625_canstat_t;

/******************************************************************************
 * CANCTRL: CAN CONTROL REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_canctrl_mask_t
 * @brief CAN CONTROL REGISTER Masks
 */
typedef enum
{
	CLKPRE		=	0x03,		///< CLKOUT Pin Prescaler bits Mask
	CLKPRE0		=	0x01,		///< CLKOUT Pin Prescaler bit 0 Mask
	CLKPRE1		=	0x02,		///< CLKOUT Pin Prescaler bit 1 Mask
	CLKEN		=	0x04,		///< CLKOUT Pin Enable bit Mask
	OSM			=	0x08,		///< One-Shot Mode bit Mask
	ABAT		=	0x10,		///< Abort All Pending Transmissions bit Mask
	REQOP		=	0xE0,		///< Request Operation mode bits Mask
	REQOP0		=	0x02,		///< Request Operation mode bit 0 Mask
	REQOP1		=	0x04,		///< Request Operation mode bit 1 Mask
	REQOP2		=	0x08		///< Request Operation mode bit 2 Mask
}mcp25625_canctrl_mask_t;

/**
 * @union mcp25625_canctrl_t
 * @brief CAN CONTROL REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		mcp25625_prescaler_t clkpre		:3;		///< CLKOUT Pin Prescaler.
		//!<
		//!<
		uint8_t clken					:1;		///< CLKOUT Pin Enable\n
		//!< 1 = CLKOUT pin is enabled\n
		//!< 0 = CLKOUT pin is disabled (pin is in a high-impedance state).
		uint8_t osm						:1;		///< One-Shot Mode\n
		//!< 1 = Enabled: Message will only attempt to transmit one time\n
		//!< 0 = Disabled: Messages will reattempt transmission if required.
		uint8_t abat					:1;		///< Abort All Pending Transmissions\n
		//!< 1 = Requests abort of all pending transmit buffers\n
		//!< 0 =Terminates request to abort all transmission.
		mcp25625_opmode_t	reqop		:3;		///< Request Operation mode.
		//!<
	};
}mcp25625_canctrl_t;

/******************************************************************************
 * TEC: Transmit Error Counter	/	REC: Receive Error Counter
 ******************************************************************************/

//Just counters, no need to define them.

/******************************************************************************
 * CNF3: CONFIGURATION 3 REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_cnf3_mask_t
 * @brief CONFIGURATION 3 REGISTER Masks
 */
typedef enum
{
	PHSEG2		=	0x03,		///< PS2 Length bits Mask
	PHSEG2_0	=	0x01,		///< PS2 Length bit 0 Mask
	PHSEG2_1	=	0x02,		///< PS2 Length bit 1 Mask
	PHSEG2_2	=	0x04,		///< PS2 Length bit 2 Mask
	WAKFIL		=	0x08,		///< Wake-up Filter bit Mask
	SOF			=	0x10,		///< Start-of-Frame Signal bit Mask
}mcp25625_cnf3_mask_t;

/**
 * @union mcp25625_cnf3_t
 * @brief CONFIGURATION 3 REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t	phseg2				:3;		///< PS2 Length\n
		//!< (PHSEG2 + 1) x TQ\n\n
		//!< Minimum valid setting for PS2 is 2 TQ.
		uint8_t						:3;		///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
		uint8_t wakfil				:1;		///< Wake-up Filter\n
		//!< 1 = Wake-up filter is enabled\n
		//!< 0 = Wake-up filter is disabled.
		uint8_t sof					:1;		///< Start-of-Frame Signal\n
		//!< If CLKEN (CANCTRL<2>) = 1:\n
		//!< 1 = CLKOUT pin is enabled for SOF signal\n
		//!< 0 = CLKOUT pin is enabled for clock out function\n
		//!< If CLKEN (CANCTRL<2>) = 0:\n
		//!< don’t care.
	};
}mcp25625_cnf3_t;

/******************************************************************************
 * CNF2: CONFIGURATION 2 REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_cnf2_mask_t
 * @brief CONFIGURATION 2 REGISTER Masks
 */
typedef enum
{
	PRSEG		=	0x03,		///< Propagation Segment Length bits Mask
	PRSEG_0		=	0x01,		///< Propagation Segment Length bit 0 Mask
	PRSEG_1		=	0x02,		///< Propagation Segment Length bit 1 Mask
	PHSEG_2		=	0x04,		///< Propagation Segment Length bit 2 Mask
	PHSEG1		=	0x38,		///< PS1 Length bits Mask
	PHSEG1_0	=	0x08,		///< PS1 Length bit 0 Mask
	PHSEG1_1	=	0x10,		///< PS1 Length bit 1 Mask
	PHSEG1_2	=	0x20,		///< PS1 Length bit 2 Mask
	SAM			=	0x08,		///< Wake-up Filter bit Mask
	BLTMODE		=	0x10		///< Start-of-Frame Signal bit Mask
}mcp25625_cnf2_mask_t;

/**
 * @union mcp25625_cnf2_t
 * @brief CONFIGURATION 2 REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t prseg		:3;		///< Propagation Segment Length\n
		//!< (PRSEG<2:0> + 1) x TQ.
		//!<
		uint8_t phseg1		:3;		///< PS1 Length\n
		//!< (PHSEG1<2:0> + 1) x TQ.
		//!<
		uint8_t sam			:1;		///< Sample Point Configuration\n
		//!< 1 = Bus line is sampled three times at the sample point\n
		//!< 0 = Bus line is sampled once at the sample point.
		uint8_t btlmode		:1;		///< PS2 Bit Time Length\n
		//!< 1 = Length of PS2 is determined by the PHSEG2<2:0> bits of CNF3\n
		//!< 0 = Length of PS2 is the greater of PS1 and IPT (2 TQ).
	};
}mcp25625_cnf2_t;

/******************************************************************************
 * CNF1: CONFIGURATION 1 REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_cnf1_mask_t
 * @brief CONFIGURATION 1 REGISTER Masks
 */
typedef enum
{
	BRP			=	0x3F,		///< Baud Rate Prescaler bits Mask
	BRP_0		=	0x01,		///< Baud Rate Prescaler bit 0 Mask
	BRP_1		=	0x02,		///< Baud Rate Prescaler bit 1 Mask
	BRP_2		=	0x04,		///< Baud Rate Prescaler bit 2 Mask
	BRP_3		=	0x08,		///< Baud Rate Prescaler bit 3 Mask
	BRP_4		=	0x10,		///< Baud Rate Prescaler bit 4 Mask
	BRP_5		=	0x20,		///< Baud Rate Prescaler bit 5 Mask
	SJW			=	0xC0,		///< Synchronization Jump Width Length bits Mask
	SJW_0		=	0xC0,		///< Synchronization Jump Width Length bit 0 Mask
	SJW_1		=	0xC0		///< Synchronization Jump Width Length bit 1 Mask
}mcp25625_cnf1_mask_t;

/**
 * @union mcp25625_cnf1_t
 * @brief CONFIGURATION 1 REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t	brp			:6;		///< Baud Rate Prescaler\n
		//!< TQ = 2 x (BRP<5:0> + 1)/FOSC.
		//!<
		uint8_t	sjw			:2;		///< Synchronization Jump Width Length\n
		//!< nm = Length = ((nm+1)*TQ) (See datasheet).
		//!<
	};
}mcp25625_cnf1_t;

/******************************************************************************
 * CANINTE: CAN INTERRUPT ENABLE REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_caninte_mask_t
 * @brief CAN INTERRUPT ENABLE REGISTER Masks
 */
typedef enum
{
	RX0IE		=	0x01,		///< Receive Buffer 0 Full Interrupt Enable bit Mask
	RX1IE		=	0x02,		///< Receive Buffer 1 Full Interrupt Enable bit Mask
	TX0IE		=	0x04,		///< Transmit Buffer 0 Empty Interrupt Enable bit Mask
	TX1IE		=	0x08,		///< Transmit Buffer 1 Empty Interrupt Enable bit Mask
	TX2IE		=	0x10,		///< Transmit Buffer 2 Empty Interrupt Enable bit Mask
	ERRIE		=	0x20,		///< Error Interrupt Enable bit Mask (multiple sources in the EFLG register)
	WAKIE		=	0x40,		///< Wake-up Interrupt Enable bit Mask
	MERRE		=	0x80		///< Message Error Interrupt Enable bit Mask
}mcp25625_caninte_mask_t;

/**
 * @union mcp25625_caninte_t
 * @brief CAN INTERRUPT ENABLE REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t rx0ie		:1;			///< Receive Buffer 0 Full Interrupt Enable\n
		//!< 1 = Interrupt when message is received in RXB0\n
		//!< 0 = Disabled.
		uint8_t rx1ie		:1;			///< Receive Buffer 1 Full Interrupt Enable\n
		//!< 1 = Interrupt when message is received in RXB1\n
		//!< 0 = Disabled.
		uint8_t tx0ie		:1;			///< Transmit Buffer 0 Empty Interrupt Enable\n
		//!< 1 = Interrupt on TXB0 becoming empty\n
		//!< 0 = Disabled.
		uint8_t tx1ie		:1;			///< Transmit Buffer 1 Empty Interrupt Enable\n
		//!< 1 = Interrupt on TXB1 becoming empty\n
		//!< 0 = Disabled.
		uint8_t tx2ie		:1;			///< Transmit Buffer 2 Empty Interrupt Enable\n
		//!< 1 = Interrupt on TXB2 becoming empty\n
		//!< 0 = Disabled.
		uint8_t errie		:1;			///< Error Interrupt Enable (multiple sources in the EFLG register)\n
		//!< 1 = Interrupt on EFLG error condition change\n
		//!< 0 = Disabled.
		uint8_t wakie		:1;			///< Wake-up Interrupt Enable\n
		//!< 1 = Interrupt on CAN bus activity\n
		//!< 0 = Disabled.
		uint8_t merre		:1;			///< Message Error Interrupt Enable\n
		//!< true:	Interrupt on error during message reception or transmission\n
		//!< 0 = Disabled.
	};
}mcp25625_caninte_t;

/******************************************************************************
 * CANINTF: CAN INTERRUPT FLAG REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_canintf_mask_t
 * @brief CAN INTERRUPT FLAG REGISTER Masks
 */
typedef enum
{
	RX0IF		=	0x01,		///< Receive Buffer 0 Full Interrupt Enable bit Mask
	RX1IF		=	0x02,		///< Receive Buffer 1 Full Interrupt Enable bit Mask
	TX0IF		=	0x04,		///< Transmit Buffer 0 Empty Interrupt Enable bit Mask
	TX1IF		=	0x08,		///< Transmit Buffer 1 Empty Interrupt Enable bit Mask
	TX2IF		=	0x10,		///< Transmit Buffer 2 Empty Interrupt Enable bit Mask
	ERRIF		=	0x20,		///< Error Interrupt Flag bit Mask (multiple sources in the EFLG register)
	WAKIF		=	0x40,		///< Wake-up Interrupt Flag bit Mask
	MERRF		=	0x80		///< Message Error Interrupt Flag bit Mask
}mcp25625_canintf_mask_t;

/**
 * @union mcp25625_canintf_t
 * @brief CAN INTERRUPT FLAG REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t rx0if		:1;			///< Receive Buffer 0 Full Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t rx1if		:1;			///< Receive Buffer 1 Full Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t tx0if		:1;			///< Transmit Buffer 0 Empty Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t tx1if		:1;			///< Transmit Buffer 1 Empty Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t tx2if		:1;			///< Transmit Buffer 2 Empty Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t errif		:1;			///< Error Interrupt Flag (multiple sources in the EFLG register)\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition\n
		//!< 0 = No interrupt pending.
		uint8_t wakif		:1;			///< Wake-up Interrupt Flag\n
		//!< 1 = Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
		uint8_t merrf		:1;			///< Message Error Interrupt Flag\n
		//!< true:	Interrupt pending (must be cleared by MCU to reset interrupt condition)\n
		//!< 0 = No interrupt pending.
	};
}mcp25625_canintf_t;

/******************************************************************************
 * EFLG: ERROR FLAG REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_eflg_mask_t
 * @brief ERROR FLAG REGISTER Masks
 */
typedef enum
{
	EWARN		=	0x01,			///< Error Warning Flag bit
	RXWAR		=	0x02,			///< Receive Error Warning Flag bit
	TXWAR		=	0x04,			///< Transmit Error Warning Flag bit
	RXEP		=	0x08,			///< Receive Error-Passive Flag bit
	TXEP		=	0x10,			///< Transmit Error-Passive Flag bit
	TXBO		=	0x20,			///< Bus-Off Error Flag bit
	TX0OVR		=	0x40,			///< Receive Buffer 0 Overflow Flag bit
	RX1OVR		=	0x80			///< Receive Buffer 1 Overflow Flag bit
}mcp25625_eflg_mask_t;

/**
 * @union mcp25625_eflg_t
 * @brief ERROR FLAG REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		uint8_t ewarn		:1;			///< Error Warning Flag\n
		//!< Sets when TEC or REC is equal to or greater than 96 (TXWAR or RXWAR = 1)\n
		//!< Resets when both REC and TEC are less than 96.
		uint8_t rxwar		:1; 		///< Receive Error Warning Flag\n
		//!< Sets when REC is equal to or greater than 96\n
		//!< Resets when REC is less than 96.
		uint8_t txwar		:1; 		///< Transmit Error Warning Flag\n
		//!< Sets when TEC is equal to or greater than 96\n
		//!< Resets when TEC is less than 96.
		uint8_t rxep		:1; 		///< Receive Error-Passive Flag\n
		//!< Sets when REC is equal to or greater than 128\n
		//!< Resets when REC is less than 128.
		uint8_t txep		:1; 		///< Transmit Error-Passive Flag\n
		//!< Sets when TEC is equal to or greater than 128\n
		//!< Resets when TEC is less than 128.
		uint8_t txbo		:1; 		///< Bus-Off Error Flag\n
		//!< Bit sets when TEC reaches 255\n
		//!< Resets after a successful bus recovery sequence.
		uint8_t rx0ovr		:1; 		///< Receive Buffer 0 Overflow Flag\n
		//!< Sets when a valid message is received for RXB0 and the RX0IF bit in the CANINTF register is ‘1’\n
		//!< Must be reset by MCU.
		uint8_t rx1ovr		:1;			///< Receive Buffer 1 Overflow Flag\n
		//!< Sets when a valid message is received for RXB1 and the RX1IF bit in the CANINTF register is ‘1’\n
		//!< Must be reset by MCU.
	};
}mcp25625_eflg_t;

/******************************************************************************
 *  TXBxCTRL: TRANSMIT BUFFER x CONTROL REGISTER
 ******************************************************************************/

/**
 * @enum mcp25625_txbxctrl_mask_t
 * @brief MCP25625 TRANSMIT BUFFER x CONTROL REGISTER Masks
 */
typedef enum
{
	TXP  		=	0x03,			///< Transmit Buffer Priority bits Mask
	TXP_0		=	0x01,			///< Transmit Buffer Priority bit 0 Mask
	TXP_1		=	0x02,			///< Transmit Buffer Priority bit 1 Mask
	TXREQ		=	0x08,			///< Message Transmit Request bit Mask
	TXERR		=	0x10,			///< Transmission Error Detected bit Mask
	MLOA		=	0x20,			///< Message Lost Arbitration bit Mask
	ABFT		=	0x40			///< Message Aborted Flag bit Mask
}mcp25625_txbxctrl_mask_t;

/**
 * @union mcp25625_txbxctrl_t
 * @brief MCP25625 TRANSMIT BUFFER x CONTROL REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		mcp25625_priority_t txp			:2;			///< Transmit Buffer Priority\n
		//!< Determines buffer priority.
		//!<
		uint8_t 						:1;			///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
		uint8_t txreq					:1;			///< Message Transmit Request\n
		//!< 1 if buffer is currently pending transmission (MCU sets this bit to
		//!< request message be transmitted – bit is automatically cleared when the
		//!< message is sent).
		uint8_t txerr					:1;			///< Transmission Error Detected\n
		//!< 1 if a bus error occurred while the message was being transmitted.
		//!<
		uint8_t mloa					:1;			///< Message Lost Arbitration\n
		//!< 1 if message lost arbitration while being sent.
		//!<
		uint8_t abft					:1;			///< Message Aborted Flag\n
		//!< 1 if message was aborted.
		//!<
		uint8_t							:1;			///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
	};
}mcp25625_txbxctrl_t;

/******************************************************************************
 *  RXBxCTRL: RECEIVE BUFFER x CONTROL REGISTERS: RXB0CTRL / RXB1CTRL
 ******************************************************************************/

/**
 * @enum mcp25625_rxbxctrl_mask_t
 * @brief RXBxCTRL: RECEIVE BUFFER x CONTROL REGISTER Masks
 */
typedef enum
{
	FILHIT__0	=		0x01,		///< Filter Hit bits Mask (RXB0CTRL)
	FILHIT__1	=		0x03,		///< Filter Hit bits Mask (RXB1CTRL)
	FILHIT_0	=		0x01,		///< Filter Hit bit 0 Mask
	BUKT1		=		0x02,		///< Read-Only Copy of BUKT bit (used internally by the MCP25625) Mask (RXB0CTRL)
	FILHIT_1	=		0x02,		///< Filter Hit bit 1 Mask (RXB1CTRL)
	BUKT		=		0x04,		///< Rollover Enable bit Mask (RXB0CTRL)
	FILHIT_2	=		0x04,		///< Filter Hit bit 2 Mask (RXB1CTRL)
	RXRTR		=		0x08,		///< Received Remote Transfer Request bit Mask
	RXM			=		0x60,		///< Receive Buffer Operating mode bits Mask
	RXM_0 		= 		0x20,		///< Receive Buffer Operating mode bit 0 Mask
	RXM_1 		= 		0x40		///< Receive Buffer Operating mode bit 1 Mask
}mcp25625_rxbxctrl_mask_t;

/**
 * @union mcp25625_rxb0ctrl_t
 * @brief RXB0CTRL: RECEIVE BUFFER 0 CONTROL REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		//TODO: Ignore warning?
		mcp25625_accfltr_rxb0_id_t filhit	:1;	///< Filter Hit
		//!< Indicates which acceptance filter enabled the reception of a message (RXF0 or RXF1)\n
		//!< If a rollover from RXB0 to RXB1 occurs, the FILHIT0 bit will reflect the filter that accepted the message
		//!< that rolled over.
		uint8_t bukt1						:1;	///< Read-Only Copy of BUKT\n
		//!< Used internally by the MCP25625.
		//!<
		uint8_t bukt						:1;	///<Rollover Enable\n
		//!< 1 = RXB0 message will roll over and be written to RXB1 if RXB0 is full\n
		//!< 0 = Rollover is disabled.
		uint8_t rxrtr						:1;	///< Received Remote Transfer Request\n
		//!< 1 = Remote Transfer Request received\n
		//!< 0 = No Remote Transfer Request received.
		uint8_t								:1;	///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
		mcp25625_rbopmod_t rxm				:2;	///< Receive Buffer Operating mode\n
		//!< MOD_A: Turns mask/filters off; receives any message.
		//!< MOD_B: Receives all valid messages using either Standard or Extended Identifiers that meet filter criteria;
		//!< Extended ID Filter registers, RXFxEID8 and RXFxEID0, are applied to the first two bytes of data in
		//!< the messages with standard IDs.
		uint8_t								:1;			///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
	};
}mcp25625_rxb0ctrl_t;

/**
 * @union mcp25625_rxb1ctrl_t
 * @brief RXB1CTRL: RECEIVE BUFFER 1 CONTROL REGISTER
 */
typedef union
{
	uint8_t register_byte;					///< Whole register
	struct
	{
		mcp25625_accfltr_id_t filhit		:3;	///< Filter Hit\n
		//!< Indicates which acceptance filter enabled the reception of a message\n
		//!< RXF1, RXF0 only if the BUKT bit is set in RXB0CTRL.
		uint8_t rxrtr					:1;	///< Received Remote Transfer Request\n
		//!< 1 = Remote Transfer Request received\n
		//!< 0 = No Remote Transfer Request received.
		uint8_t							:1;	///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
		mcp25625_rbopmod_t rxm			:2;	///< Receive Buffer Operating mode\n
		//!< MOD_A: Turns mask/filters off; receives any message\n
		//!< MOD_B: Receives all valid messages using either Standard or Extended Identifiers that meet filter criteria.
		uint8_t							:1;			///< Unimplemented\n
		//!< Read as ‘0’.
		//!<
	};
}mcp25625_rxb1ctrl_t;

/*******************************************************************************
 *******************************************************************************
        Acceptance Filters, masks and receive / transmit buffers registers
 *******************************************************************************
 * For these register, a struct with the contents of the registers is provided
 * Some registers share the same struct
 *******************************************************************************/

/******************************************************************************
 *  SIDH: STANDARD IDENTIFIER HIGH REGISTER
 ******************************************************************************/

/**
 * @struct mcp25625_sidh_t
 * @brief SIDH: STANDARD IDENTIFIER HIGH REGISTER.
 * @details Models RXFxSIDH/RXMxSIDH/TXBxSIDH/RXBxSIDH registers.
 */
typedef struct
{
	uint8_t	sid_h					:8;		///< Standard Identifier high part\n
	//!< Refer to datasheet.
	//!<
}mcp25625_sidh_t;

/******************************************************************************
 *  SIDL: STANDARD IDENTIFIER LOW REGISTER
 ******************************************************************************/

/**
 * @struct mcp25625_sidl_t
 * @brief SIDL: STANDARD IDENTIFIER LOW REGISTER.\n
 * @details Models RXFxSIDL/RXMxSIDL/TXBxSIDL/RXBxSIDL registers\n
 * Refer to datasheet for register-specific documentation.
 */
typedef struct
{
	uint8_t	eid_hh				:2;		///< Extended Identifier higher part\n
	//!< Refer to datasheet.
	//!<
	uint8_t						:1;		///< Unimplemented
	//!< Read as ‘0’.
	//!<
	uint8_t ide					:1;		///< Extended Identifier Flag / Enable\n
	//!< Refer to datasheet.
	//!<
	uint8_t ssr					:1;		///< Standard Frame Remote Transmit Request\n
	//!< Refer to datasheet
	//!<
	uint8_t	sid_l		:3;		///< Standard Identifier low part\n
	//!< Refer to datasheet
	//!<
}mcp25625_sidl_t;

/******************************************************************************
 *  EID8: EXTENDED IDENTIFIER HIGH REGISTER
 ******************************************************************************/

/**
 * @struct mcp25625_eid8_t
 * @brief EID8: EXTENDED IDENTIFIER HIGH REGISTER\n
 * @details Models RXFxEID8/RXMxEID8/TXBxEID8/RXBxEID8 registers\n
 * Refer to datasheet for register-specific documentation.
 */
typedef struct	//Using union for uniform code
{
	uint8_t	eid_h					:8;		///< Extended Identifier high part\n
	//!< Refer to datasheet.
	//!<
}mcp25625_eid8_t;

/******************************************************************************
 *  EID0: EXTENDED IDENTIFIER LOW REGISTER
 ******************************************************************************/

/**
 * @struct mcp25625_eid0_t
 * @brief EID0: EXTENDED IDENTIFIER LOW REGISTER\n
 * @details Models RXFxEID0/RXMxEID0/TXBxEID0/RXBxEID0 registers\n
 * Refer to datasheet for register-specific documentation.
 */
typedef struct	//Using union for uniform code
{
	uint8_t	eid_l					:8;		///< Extended Identifier low part\n
	//!< Refer to datasheet.
	//!<
}mcp25625_eid0_t;

/******************************************************************************
 *  DLC: DATA LENGTH CODE REGISTER
 ******************************************************************************/

/**
 * @struct mcp25625_dlc_t
 * @brief DLC: DATA LENGTH CODE REGISTER.
 * @details Models TXBxDLC/RXBxDLC registers.\n
 * Refer to datasheet for register-specific documentation.
 */
typedef struct	//Using union for uniform code
{
	uint8_t	dlc					:4;		///< Data Length Code\n
	//!< Refer to datasheet.
	//!<
	uint8_t 					:2;		///< Unimplemented / Reserved
	//!< Refer to datasheet.
	//!<
	uint8_t rtr					:1;		///< Remote Transmission Request
	//!< Refer to datasheet.
	//!<
	uint8_t						:1;		///< Unimplemented
	//!< Read as ‘0’
	//!<
}mcp25625_dlc_t;

/*******************************************************************************
 *******************************************************************************
        					Useful Combination of registers
 *******************************************************************************
 * Structs modeling consecutive registers that represents a single block
 *******************************************************************************/

/******************************************************************************
 * ID: ID Struct
 ******************************************************************************/

/**
 * @struct mcp25625_id_t
 * @brief ID: Collection of SIDH/SIDL/EID8/EID0 Registers
 * @details Contains information about message id.
 */
typedef struct	//Using union for uniform code
{
	mcp25625_sidh_t sidh;		///< STANDARD IDENTIFIER HIGH REGISTER
	mcp25625_sidl_t sidl;		///< STANDARD IDENTIFIER LOW REGISTER
	mcp25625_eid8_t eid8;		///< EXTENDED IDENTIFIER HIGH REGISTER.
	mcp25625_eid0_t eid0;		///< EXTENDED IDENTIFIER LOW REGISTER.
}mcp25625_id_t;

/******************************************************************************
 * DATA: Data structure
 ******************************************************************************/
/**
 * @struct mcp25625_data_t
 * @brief Data: Data Length Code Register with Data buffer
 * @details Contains data length and transfered / received data
 */
typedef struct	//Using union for uniform code
{
	mcp25625_dlc_t dlc;			///< DATA LENGTH CODE
	uint8_t	buffer[8];			///< Data Buffer
}mcp25625_data_t;

/******************************************************************************
 * BUFFER_DATA: ID + DATA
 ******************************************************************************/
/**
 * @struct mcp25625_id_data_t
 * @brief ID+Data:  SIDH/SIDL/EID8/EID0/DLC and Buffer.
 * @details Represents the whole buffer (without configuration register)\n
 * Buffer Id + DLC + Buffer data.
 */
typedef struct	//Using union for uniform code
{
	mcp25625_id_t id;			///< ID
	mcp25625_data_t	data;		///< DLC + buffer
}mcp25625_id_data_t;

/*******************************************************************************
 *******************************************************************************
        					Status Registers
 *******************************************************************************
 *******************************************************************************/

/******************************************************************************
 * STATUS: several Status bits for transmit and receive functions.
 ******************************************************************************/

/**
 * @struct mcp25625_status_t
 * @brief Status: often used Status bits for message reception and transmission.
 */
typedef struct	//Using union for uniform code
{
	uint8_t	rx0if		:1;			///< RX0IF (CANINTF Register)
	uint8_t rx1if		:1;			///< RX1IF (CANINTF Register)
	uint8_t txreq0		:1;			///< TXREQ (TXB0CTRL Register)
	uint8_t tx0if		:1;			///< TX0IF (CANINTF Register)
	uint8_t txreq1		:1;			///< TXREQ (TXB1CTRL Register)
	uint8_t tx1if		:1;			///< TX1IF (CANINTF Register)
	uint8_t txreq2		:1;			///< TXREQ (TXB2CTRL Register)
	uint8_t tx2if		:1;			///< TX2IF (CANINTF Register)
}mcp25625_status_t;

/******************************************************************************
 * RX_STATUS: Information about which filter matched the message
 * and message type (standard, extended, remote).
 ******************************************************************************/

/**
 * @struct mcp25625_rx_status_t
 * @brief RX_STATUS: Information about which filter matched the message
 * and message type (standard, extended, remote).
 */
typedef struct	//Using union for uniform code
{
	mcp25625_accfltr_id_t filter_match		:3;		///< Filter Match
	mcp25625_msg_type_t msg_type			:3;		///< Msg Type Received
	mcp25625_rec_msg_info_t msg_info		:3;		///< Received Message Buffer
}mcp25625_rx_status_t;


/*******************************************************************************
 *******************************************************************************
                        	FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//Low Level functions

/**
 * @brief Sends reset command
 */
void mcp25625_reset(void);

/**
 * @brief Write data to mcp25625.
 * @param addr Address where to start writing data.
 * @param length Total bytes to write.
 * @param p_data pointer to data to write.
 */
void mcp25625_write(mcp25625_addr_t addr, size_t length, const uint8_t *p_data);

/**
 * @brief read data from mcp25625 single register.
 * @param addr Register's Address..
 * @return Register content.
 */
uint8_t mcp25625_read_register(mcp25625_addr_t addr);

/**
 * @brief read data from mcp25625.
 * @param addr Address where to start reading data.
 * @param length Total bytes to read.
 * @param p_data pointer to data where to write read data.
 */
void mcp25625_read(mcp25625_addr_t addr, size_t length, uint8_t *p_data);

/**
 * @brief Modify bits in register using mask.
 * @param addr Address of register to modify.
 * @param mask Mask with ones in bits that must be modified.
 * @param data Data with bits to modify.
 */
void mcp25625_bit_modify(mcp25625_addr_t addr, uint8_t mask, uint8_t data);

/**
 * @brief Read RX Buffer id.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_id pointer to id struct where to write read data.
 */
void mcp25625_read_rx_buffer_id(mcp25625_rxb_id_t buffer_id, mcp25625_id_t *p_id);

/**
 * @brief Load RX Buffer id.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_id pointer to id struct with data to write.
 */
void mcp25625_load_tx_buffer_id(mcp25625_txb_id_t buffer_id, const mcp25625_id_t *p_id);

/**
 * @brief Read RX Buffer data.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_data pointer to data struct where to write read data.
 */
void mcp25625_read_rx_buffer_data(mcp25625_rxb_id_t buffer_id, mcp25625_data_t *p_data);

/**
 * @brief Load RX Buffer data.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_data pointer to id struct where to write read data.
 */
void mcp25625_load_tx_buffer_data(mcp25625_txb_id_t buffer_id, const mcp25625_data_t *p_data);

/**
 * @brief Read RX Buffer id+data.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_id_data pointer to id+data struct where to write read data.
 */
void mcp25625_read_rx_buffer_id_data(mcp25625_rxb_id_t buffer_id, mcp25625_id_data_t *p_id_data);

/**
 * @brief Load RX Buffer id+data.
 * @param buffer_id Id of RX Buffer to use.
 * @param p_id_data pointer to id+data struct where to write read data.
 */
void mcp25625_load_tx_buffer_id_data(mcp25625_txb_id_t buffer_id, const mcp25625_id_data_t *p_id_data);

/**
 * @brief TX Buffer Request to send.
 * @param tx_rts_flags flags indicating which buffers should be sent.
 */
void mcp25625_tx_request_to_send(mcp25625_txb_rts_flag_t tx_rts_flags);

/**
 * @brief Read Status.
 * @return Struct with Status.
 */
mcp25625_status_t mcp25625_read_status(void);

/**
 * @brief Read RX Status.
 * @return Struct with RX Status.
 */
mcp25625_rx_status_t mcp25625_read_rx_status(void);

//Higher level functions
void mcp25625_request_mode(mcp25625_opmode_t opmode);
mcp25625_opmode_t mcp25625_get_mode(void);
void mcp25625_configure_acceptance_filter(mcp25625_accfltr_id_t filter_id, const mcp25625_accfltr_id_t *p_filter_id);
void mcp25625_configure_acceptance_mask(mcp25625_accmsk_id_t mask_id, const mcp25625_accfltr_id_t *p_mask_id);


#endif /* CAN_MCP25625_MCP25625_DRIVER_H_ */
