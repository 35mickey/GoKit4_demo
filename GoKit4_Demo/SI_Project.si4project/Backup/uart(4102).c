/******************************************************************************
*@file    uart.c
*@brief   driver for uart operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#include "qapi_tlmm.h"
#include "qapi_timer.h"
#include "qapi_diag.h"
#include "qapi_uart.h"
#include "uart.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/
TX_BYTE_POOL *byte_pool_uart;
//#define UART_BYTE_POOL_SIZE		10*8*1024
//UCHAR free_memory_uart[UART_BYTE_POOL_SIZE];
	
	
#ifdef QT_UART_ENABLE_1ST
static qapi_UART_Handle_t uart1_handle;
/* uart1 rx tx buffer */
static char rx1_buff[UART1_RX1_BUFF_SIZE]; /*!!! should keep this buffer as 4K Bytes */
static char tx1_buff[UART1_TX1_BUFF_SIZE];
#endif
	
#ifdef QT_UART_ENABLE_2ND
static qapi_UART_Handle_t uart2_handle;
/* uart2 rx tx buffer */
static char *rx2_buff = NULL; /*!!! should keep this buffer as 4K Bytes */
static char *tx2_buff = NULL;
#endif
	
#ifdef QT_UART_ENABLE_3RD
static qapi_UART_Handle_t uart3_handle;
/* uart3 rx tx buffer */
static char *rx3_buff = NULL; /*!!! should keep this buffer as 4K Bytes */
static char *tx3_buff = NULL;
#endif

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
#ifdef QT_UART_ENABLE_1ST
static void uart1_rx_cb(uint32_t num_bytes, void *cb_data);
static void uart1_tx_cb(uint32_t num_bytes, void *cb_data);
#endif
#ifdef QT_UART_ENABLE_2ND
static void uart2_rx_cb(uint32_t num_bytes, void *cb_data);
static void uart2_tx_cb(uint32_t num_bytes, void *cb_data);
#endif
#ifdef QT_UART_ENABLE_3RD
static void uart3_rx_cb(uint32_t num_bytes, void *cb_data);
static void uart3_tx_cb(uint32_t num_bytes, void *cb_data);
#endif



/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/*
@func
  user_uart_init
@brief
  User API Function, init the UARTs. 
*/
void user_uart_init(void)
{
	UINT status;
	qapi_UART_Open_Config_t uart1_cfg;
	qapi_UART_Open_Config_t uart2_cfg;
	qapi_UART_Open_Config_t uart3_cfg;

#ifdef QT_UART_ENABLE_1ST
	uart1_cfg.baud_Rate			= 115200;
	uart1_cfg.enable_Flow_Ctrl	= QAPI_FCTL_OFF_E;
	uart1_cfg.bits_Per_Char		= QAPI_UART_8_BITS_PER_CHAR_E;
	uart1_cfg.enable_Loopback 	= 0;
	uart1_cfg.num_Stop_Bits		= QAPI_UART_1_0_STOP_BITS_E;
	uart1_cfg.parity_Mode 		= QAPI_UART_NO_PARITY_E;
	uart1_cfg.rx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart1_rx_cb;
	uart1_cfg.tx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart1_tx_cb;;

	status = qapi_UART_Open(&uart1_handle, QT_UART_PORT_01, &uart1_cfg);
	gizLog(LOG_INFO,"QT# qapi_UART_Open [%d] status %d \n", QT_UART_PORT_01, status);

	status = qapi_UART_Power_On(uart1_handle);
	gizLog(LOG_INFO,"QT# qapi_UART_Power_On [%d] status %d \n", QT_UART_PORT_01, status);

	
	status = qapi_UART_Receive(uart1_handle, (char *)rx1_buff, UART1_RX1_BUFF_SIZE, (void*)NULL);
	gizLog(LOG_INFO,"QT# qapi_UART_Receive [%d] status %d \n", QT_UART_PORT_01, status);

	gizLog(LOG_INFO,"[uart 1] UART1 init complete %d \n");
#endif

#ifdef QT_UART_ENABLE_2ND
#endif

#ifdef QT_UART_ENABLE_3RD
	uart3_cfg.baud_Rate			= 115200;
	uart3_cfg.enable_Flow_Ctrl	= QAPI_FCTL_OFF_E;
	uart3_cfg.bits_Per_Char		= QAPI_UART_8_BITS_PER_CHAR_E;
	uart3_cfg.enable_Loopback 	= false;
	uart3_cfg.num_Stop_Bits		= QAPI_UART_1_0_STOP_BITS_E;
	uart3_cfg.parity_Mode 		= QAPI_UART_NO_PARITY_E;
	uart3_cfg.rx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart3_rx_cb;
	uart3_cfg.tx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart3_tx_cb;;

	status = qapi_UART_Open(&uart3_handle, QT_UART_PORT_03, &uart3_cfg);
	gizLog(LOG_INFO,"QT# qapi_UART_Open [%d] status %d \n", QT_UART_PORT_03, status);

	status = qapi_UART_Power_On(uart3_handle);
	gizLog(LOG_INFO,"QT# qapi_UART_Power_On [%d] status %d \n", QT_UART_PORT_03, status);

	
	status = qapi_UART_Receive(uart3_handle, (char *)rx3_buff, UART1_RX3_BUFF_SIZE, (void*)NULL);
	gizLog(LOG_INFO,"QT# qapi_UART_Receive [%d] status %d \n", QT_UART_PORT_03, status);

	gizLog(LOG_INFO,"[uart 3] UART3 init complete \n");
#endif

}

/*
@func
  uart1_send
@brief
  Start a uart transmit action.
  [in]  buff
  		char* type; The data buffer ptr needs to be transmit;
  [in]  len
  		uint16_t type; The data buffer length.
*/
void uart1_send(char *buff, uint16_t len)
{
	qapi_UART_Transmit(uart1_handle, buff, len, NULL);
}

/*
@func
  uart1_printf
@brief
  Start a uart1 transmit action.
*/
void uart1_printf(const char* fmt, ...)
{
	qapi_Status_t status;
	static char log_buf[256] = {0};

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(log_buf, sizeof(log_buf), fmt, ap);
	va_end(ap);

    status = qapi_UART_Transmit(uart1_handle, log_buf, strlen(log_buf), NULL);
	gizLog(LOG_INFO,"QT# qapi_UART_Receive [%d] status %d", QT_UART_PORT_01, status);
}

/*
@func
  uart1_rx_cb
@brief
  uart1 rx callback handler.
*/
static void uart1_rx_cb(uint32_t num_bytes, void *cb_data)
{
	gizLog(LOG_INFO,"QT# qapi_UART_tx_cb [%d] num %d", QT_UART_PORT_01, num_bytes);
}

/*
@func
  uart1_tx_cb
@brief
  uart1 tx callback handler.
*/
static void uart1_tx_cb(uint32_t num_bytes, void *cb_data)
{
	;
}

/*
@func
  uart3_send
@brief
  Start a uart transmit action.
  [in]  buff
  		char* type; The data buffer ptr needs to be transmit;
  [in]  len
  		uint16_t type; The data buffer length.
*/
void uart3_send(char *buff, uint16_t len)
{
	qapi_UART_Transmit(uart3_handle, buff, len, NULL);
}

/*
@func
  uart3_printf
@brief
  Start a uart3 transmit action.
*/
void uart3_printf(const char* fmt, ...)
{
	qapi_Status_t status;
	static char log_buf[256] = {0};

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(log_buf, sizeof(log_buf), fmt, ap);
	va_end(ap);

    status = qapi_UART_Transmit(uart3_handle, log_buf, strlen(log_buf), NULL);
	gizLog(LOG_INFO,"QT# qapi_UART_Receive [%d] status %d", QT_UART_PORT_03, status);
}

/*
@func
  uart3_rx_cb
@brief
  uart3 rx callback handler.
*/
static void uart3_rx_cb(uint32_t num_bytes, void *cb_data)
{
	gizLog(LOG_INFO,"QT# qapi_UART_tx_cb [%d] num %d", QT_UART_PORT_01, num_bytes);
}

/*
@func
  uart3_tx_cb
@brief
  uart3 tx callback handler.
*/
static void uart3_tx_cb(uint32_t num_bytes, void *cb_data)
{
	;
}





