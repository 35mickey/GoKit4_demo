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

#define QT_UART_PORT_01		QAPI_UART_PORT_001_E
#define QT_UART_PORT_02		QAPI_UART_PORT_002_E
#define QT_UART_PORT_03		QAPI_UART_PORT_003_E

#define QT_UART_ENABLE_1ST		/* which has CTS... */
//#define QT_UART_ENABLE_2ND	/* It's for dubug, don't use. */
#define QT_UART_ENABLE_3RD		/* datasheet says for..., only two pin */

#define UART1_RX1_BUFF_SIZE	(4*1024)
#define UART1_TX1_BUFF_SIZE	(4*1024)
#define UART1_RX2_BUFF_SIZE	(4*1024)
#define UART1_TX2_BUFF_SIZE	(4*1024)
#define UART1_RX3_BUFF_SIZE	(4*1024)
#define UART1_TX3_BUFF_SIZE	(4*1024)

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

