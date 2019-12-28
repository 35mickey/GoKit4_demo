/******************************************************************************
*@file    uart.h
*@brief   driver of uart operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __UART_H__
#define __UART_H__

#include "../main/log.h"

typedef struct{
	qapi_UART_Handle_t  hdlr;
	qapi_UART_Port_Id_e port_id;
	char *tx_buff;
	uint32_t tx_len;
	char *rx_buff;
	uint32_t rx_len;
	uint32_t baudrate;
}QT_UART_CONF_PARA;

/**************************************************************************
* The uart port id which customer want to use.
* The GoKit 4.0 is In QUECTEL_QUECOPEN_V2, so DO NOT believe sch file!!!!!!!!!!!!!!!!!!
* 
* In QUECTEL_QUECOPEN_V1--->
*   QAPI_UART_PORT_001_E	:  Map to Chip's UART3, Module's Pin 34(RXD) & Pin 35(TXD)
*   QAPI_UART_PORT_002_E	:  Map to Chip's UART5, Module's Pin 22(RXD) & Pin 23(TXD)
*   QAPI_UART_PORT_003_E	:  Map to Chip's UART1, Module's Pin 28(RXD) & Pin 27(TXD)
* In QUECTEL_QUECOPEN_V2--->
*   QAPI_UART_PORT_001_E	:  Map to Chip's UART6, Module's Pin  6(RXD) & Pin	5(TXD)
*   QAPI_UART_PORT_002_E	:  Map to Chip's UART5, Module's Pin 22(RXD) & Pin 23(TXD)
*   QAPI_UART_PORT_003_E	:  Map to Chip's UART1, Module's Pin 28(RXD) & Pin 27(TXD)
***************************************************************************/

#define QT_UART_PORT_01		QAPI_UART_PORT_001_E
#define QT_UART_PORT_02		QAPI_UART_PORT_002_E
#define QT_UART_PORT_03		QAPI_UART_PORT_003_E

#define QT_UART_ENABLE_1ST		/* which has CTS..., close it if use SPI*/
//#define QT_UART_ENABLE_2ND	/* It's for dubug, don't use. */
#define QT_UART_ENABLE_3RD	/* datasheet says for..., only two pin */

#define UART_RX1_BUFF_SIZE	(1*1024)
#define UART_TX1_BUFF_SIZE	(1*1024)
#define UART_RX2_BUFF_SIZE	(1*1024)
#define UART_TX2_BUFF_SIZE	(1*1024)
#define UART_RX3_BUFF_SIZE	(1*1024)
#define UART_TX3_BUFF_SIZE	(1*1024)

/*  User global varients
 * @{
 */

/**@} */


/*  User API interface
 * @{
 */
void user_uart_init(void);
void uart1_send(char *buff, uint16_t len);
void uart1_printf(const char* fmt, ...);
void uart3_send(char *buff, uint16_t len);
void uart3_printf(const char* fmt, ...);


/**@} */

#endif /*__EXAMPLE_UART_H__*/

