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
	
#ifdef QT_UART_ENABLE_1ST
/* uart1 rx tx buffer */
static char rx1_buff[UART_RX1_BUFF_SIZE]; /*!!! should keep this buffer as 4K Bytes */
static char tx1_buff[UART_TX1_BUFF_SIZE];
static QT_UART_CONF_PARA uart1_conf =
{
	.hdlr 		= NULL,
	.port_id 	= QT_UART_PORT_01,
	.tx_buff 	= tx1_buff,
	.tx_len 	= UART_TX1_BUFF_SIZE,
	.rx_buff 	= rx1_buff,
	.rx_len 	= UART_RX1_BUFF_SIZE,
	.baudrate 	= 115200
};
#endif
	
#ifdef QT_UART_ENABLE_2ND
/* uart2 rx tx buffer */
static char rx2_buff[UART_RX2_BUFF_SIZE]; /*!!! should keep this buffer as 4K Bytes */
static char tx2_buff[UART_TX2_BUFF_SIZE];
static QT_UART_CONF_PARA uart2_conf =
{
	.hdlr 		= NULL,
	.port_id 	= QT_UART_PORT_02,
	.tx_buff 	= tx2_buff,
	.tx_len 	= UART_TX2_BUFF_SIZE,
	.rx_buff 	= rx2_buff,
	.rx_len 	= UART_RX2_BUFF_SIZE,
	.baudrate 	= 115200
};
#endif
	
#ifdef QT_UART_ENABLE_3RD
/* uart3 rx tx buffer */
static char rx3_buff[UART_RX3_BUFF_SIZE]; /*!!! should keep this buffer as 4K Bytes */
static char tx3_buff[UART_TX3_BUFF_SIZE];
static QT_UART_CONF_PARA uart3_conf =
{
	.hdlr 		= NULL,
	.port_id 	= QT_UART_PORT_03,
	.tx_buff 	= tx3_buff,
	.tx_len 	= UART_TX3_BUFF_SIZE,
	.rx_buff 	= rx3_buff,
	.rx_len 	= UART_RX3_BUFF_SIZE,
	.baudrate 	= 115200
};
#endif

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
static void uart_rx_cb(uint32_t num_bytes, void *cb_data);
static void uart_tx_cb(uint32_t num_bytes, void *cb_data);

/**************************************************************************
*                                 FUNCTION
***************************************************************************/
/*
@func
  static uartX_init
@brief
  User API Function, init the UARTs. 
*/
static void uartX_init(QT_UART_CONF_PARA *uart_conf)
{
	qapi_Status_t status;
	qapi_UART_Open_Config_t uart_cfg;

	uart_cfg.baud_Rate			= uart_conf->baudrate;
	uart_cfg.enable_Flow_Ctrl	= QAPI_FCTL_OFF_E;
	uart_cfg.bits_Per_Char		= QAPI_UART_8_BITS_PER_CHAR_E;
	uart_cfg.enable_Loopback	= 0;
	uart_cfg.num_Stop_Bits		= QAPI_UART_1_0_STOP_BITS_E;
	uart_cfg.parity_Mode		= QAPI_UART_NO_PARITY_E;
	uart_cfg.rx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart_rx_cb;
	uart_cfg.tx_CB_ISR			= (qapi_UART_Callback_Fn_t)&uart_tx_cb;;

	status = qapi_UART_Open(&uart_conf->hdlr, uart_conf->port_id, &uart_cfg);
	gizLog(LOG_INFO,"QT# qapi_UART_Open [%d] status %d", uart_conf->port_id, status);

	status = qapi_UART_Power_On(uart_conf->hdlr);
	gizLog(LOG_INFO,"QT# qapi_UART_Power_On [%d] status %d", uart_conf->port_id, status);
	
}

/*
@func
  static uartX_recv
@brief
  Start a uart receive action.
*/
static void uartX_recv(QT_UART_CONF_PARA *uart_conf)
{
	qapi_Status_t status;
	status = qapi_UART_Receive(uart_conf->hdlr, uart_conf->rx_buff, uart_conf->rx_len, (void*)uart_conf);
	gizLog(LOG_INFO,"QT# qapi_UART_Receive [%d] status %d", (qapi_UART_Port_Id_e)uart_conf->port_id, status);
}

/*
@func
  user_uart_init
@brief
  User API Function, init the all UARTs. 
*/
void user_uart_init(void)
{
	UINT status;

#ifdef QT_UART_ENABLE_1ST
	/* uart 1 init			*/
	uartX_init(&uart1_conf);
	/* start uart 1 receive */
	uartX_recv(&uart1_conf);
#endif
	
#ifdef QT_UART_ENABLE_2ND
	/* uart 2 init			*/
	uartX_init(&uart2_conf);
	/* start uart 2 receive */
	uartX_recv(&uart2_conf);
#endif
	
#ifdef QT_UART_ENABLE_3RD
	/* uart 3 init			*/
	uartX_init(&uart3_conf);
	/* start uart 3 receive */
	uartX_recv(&uart3_conf);
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
	qapi_UART_Transmit(uart1_conf.hdlr, buff, len, NULL);
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

    status = qapi_UART_Transmit(uart1_conf.hdlr, log_buf, strlen(log_buf), NULL);
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
	qapi_UART_Transmit(uart3_conf.hdlr, buff, len, NULL);
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

    status = qapi_UART_Transmit(uart3_conf.hdlr, log_buf, strlen(log_buf), NULL);
}

/*
@func
  uart_rx_cb
@brief
  uart rx callback handler.
*/
static void uart_rx_cb(uint32_t num_bytes, void *cb_data)
{
	QT_UART_CONF_PARA *uart_conf = (QT_UART_CONF_PARA*)cb_data;

	if(num_bytes == 0)
	{
		uartX_recv(uart_conf);
		return;
	}
	
	if(num_bytes >= uart_conf->rx_len)
	{
		num_bytes = uart_conf->rx_len;
	}
	/* User code start */

	gizLog(LOG_INFO,"[uart_rx_cb] uart_rx_cb data [%d][%s]", num_bytes, uart_conf->rx_buff);

	/* User code end */
	uartX_recv(uart_conf);
}

/*
@func
  uart_tx_cb
@brief
  uart tx callback handler.
*/
static void uart_tx_cb(uint32_t num_bytes, void *cb_data)
{
	qapi_Status_t status;
	gizLog(LOG_INFO,"[uart_tx_cb] uart_tx_cb [%d] num %d \n", QT_UART_PORT_01, num_bytes);
}





