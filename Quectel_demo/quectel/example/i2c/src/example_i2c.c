/******************************************************************************
*@file    example_i2c.c
*@brief   example of i2c
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#if defined(__EXAMPLE_I2C__)
#include "txm_module.h"
#include "qapi_diag.h"
#include "qapi_timer.h"
#include "qapi_uart.h"
#include "quectel_utils.h"
#include "quectel_uart_apis.h"
#include "qapi_fs_types.h"
#include "qapi_fs.h"
#include "example_i2c.h"
#include "qapi_i2c_master.h"

///#include "qapi_quectel.h"

/**************************************************************************
*                                 DEFINE
***************************************************************************/

TX_BYTE_POOL *byte_pool_gpio;
#define GPIO_BYTE_POOL_SIZE		16*8*1024
UCHAR free_memory_Uart[GPIO_BYTE_POOL_SIZE];

/* uart rx tx buffer */
static char *rx_buff = NULL; /*!!! should keep this buffer as 4K Bytes */
static char *tx_buff = NULL;

/* uart config para*/
static QT_UART_CONF_PARA uart_conf;



/**************************************************************************
*                                 GLOBAL
***************************************************************************/
typedef enum
{
	I2C_REG_8BIT = 0,
	I2C_REG_16BIT
}I2C_REG_TYPE_t;

typedef enum
{
	I2C_CB_IDLE = 0,
	I2C_CB_BUSY
}I2C_CB_Status_t;

void    *i2c_handle = NULL;

uint8 i2c_rbuffer[128] = {};
uint8 i2c_wbuffer[128] = {};


/*===========================================================================

                           Static & global Variable Declarations

===========================================================================*/
/*
@func
  quectel_dbg_uart_init
@brief
  Entry function for task. 
*/
/*=========================================================================*/
void quectel_dbg_uart_init(qapi_UART_Port_Id_e port_id)
{
	uart_conf.hdlr 	  = NULL;
	uart_conf.port_id = port_id;
	uart_conf.tx_buff = tx_buff;
	uart_conf.tx_len  = sizeof(tx_buff);
	uart_conf.rx_buff = rx_buff;
	uart_conf.rx_len  = sizeof(rx_buff);
	uart_conf.baudrate= 115200;

	/* uart init */
	uart_init(&uart_conf);

	/* start uart receive */
	uart_recv(&uart_conf);
}


// Transfer completed
I2C_CB_Status_t CB_Parameter = I2C_CB_IDLE;
void i2c_callback (const uint32 status, void *cb_para)  //  uint32
{
	CB_Parameter = I2C_CB_IDLE;
 	//qt_uart_dbg(uart_conf.hdlr,"i2c_callback: status[%d] \n", status);
}

/******************************************** 
** <S>  - START bit
** <P>  - STOP  bit
** <Sr> - Repeat Start bit
** <A>  - Acknowledge bit
** <N>  - Not-Acknowledge bit
** <R>  - Read Transfer
** <W> - Write Transfer
********************************************/
qapi_Status_t i2c_write_reg(uint8 device_Address, uint16 reg, uint8 data, I2C_REG_TYPE_t reg_type)
{
	qapi_Status_t res = QAPI_OK;
	static qapi_I2CM_Descriptor_t desc[2];	/* 必须静态 */
	qapi_I2CM_Config_t config;				/* 绝对不能静态 */
	static unsigned int transferred1 = 0;	/* 必须静态 */

	//gizLog(LOG_INFO,"[I2C] i2c_write_reg: start \n");
	qt_uart_dbg(uart_conf.hdlr,"[I2C] i2c_write_reg: start. CB: %d\n", CB_Parameter);

	// Configure the bus speed and slave address
	config.bus_Frequency_KHz = 100; 
	config.slave_Address	 = device_Address;
	config.SMBUS_Mode		 = 0;
	config.slave_Max_Clock_Stretch_Us = 100000;
	config.core_Configuration1 = 0;
	config.core_Configuration2 = 0;

	if(reg_type == I2C_REG_16BIT)
	{
		i2c_wbuffer[0] = (uint8)(reg>>8);
		i2c_wbuffer[1] = (uint8)reg;
		i2c_wbuffer[2] = data;
		desc[0].length	   = 3;
	}
	else
	{
		i2c_wbuffer[0] = (uint8)reg;
		i2c_wbuffer[1] = data;
		desc[0].length	   = 2;
	}
	
	desc[0].buffer		= i2c_wbuffer;
//	desc[0].length		= 2;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;
	
	qt_uart_dbg(uart_conf.hdlr,"[I2C] qapi_I2CM_Transfer: start \n");
	res = qapi_I2CM_Transfer (i2c_handle, &config, desc, 1, i2c_callback, &CB_Parameter, 100);

	if(res == QAPI_OK)
	{
		CB_Parameter = I2C_CB_BUSY;
	}

	return res;
}

qapi_Status_t i2c_write_reg_block(uint8 device_Address, uint16 reg, uint8 data, I2C_REG_TYPE_t reg_type)
{
	qapi_Status_t res = QAPI_ERROR;
	int cnt;
	res = i2c_write_reg(device_Address, reg, data,reg_type);
	
	if(res == QAPI_OK)
	{
		 /* Block to check the callback parameter, max 500ms */
		for(cnt=0;cnt<500;cnt++)
		{
			if(CB_Parameter == I2C_CB_IDLE)
			{
				return QAPI_OK;
			}
			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
		}
	}
	
	return QAPI_ERROR;
}

qapi_Status_t i2c_write_mem(uint8 device_Address, uint8 len, uint8 *data_buff)
{
	qapi_Status_t res = QAPI_OK;
	static qapi_I2CM_Descriptor_t desc[2];	/* 必须静态 */
	qapi_I2CM_Config_t config;				/* 绝对不能静态 */
	static unsigned int transferred1 = 0;	/* 必须静态 */
	int i=0;

	//gizLog(LOG_INFO,"[I2C] i2c_write_reg: start \n");
	qt_uart_dbg(uart_conf.hdlr,"[I2C] i2c_write_mem: start. CB: %d\n", CB_Parameter);

	if(len >= 127)
	{
		return QAPI_ERROR;
	}
	else
	{
		for(i = 0;i < len;i++)
			i2c_wbuffer[i] = data_buff[i];
	}

	// Configure the bus speed and slave address
	config.bus_Frequency_KHz = 100; 
	config.slave_Address	 = device_Address;
	config.SMBUS_Mode		 = FALSE;
	config.slave_Max_Clock_Stretch_Us = 100000;
	config.core_Configuration1 = 0;
	config.core_Configuration2 = 0;
		
	// Write transfer for one reg, one data
	// <S><slave_address><W><A><data1><A><data2><A>...<dataN><A><P>
	desc[0].buffer		= i2c_wbuffer;
	desc[0].length		= len;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;

	res = qapi_I2CM_Transfer (i2c_handle, &config, desc, 1, i2c_callback, &CB_Parameter, 100);

	if(res == QAPI_OK)
	{
		CB_Parameter = I2C_CB_BUSY;
	}

	return res;
}

qapi_Status_t i2c_write_mem_block(uint8 device_Address,uint8 len, uint8 *data_buff)
{
	qapi_Status_t res = QAPI_ERROR;
	int cnt;
	res = i2c_write_mem(device_Address, len, data_buff);
	
	if(res == QAPI_OK)
	{
		 /* Block to check the callback parameter, max 500ms */
		for(cnt=0;cnt<500;cnt++)
		{
			if(CB_Parameter == I2C_CB_IDLE)
			{
				return QAPI_OK;
			}
			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
		}
	}
	
	return QAPI_ERROR;
}

qapi_Status_t i2c_read_mem (uint8 device_Address, uint8 len, uint8 *buff)
{
	qapi_Status_t res = QAPI_OK;
	static qapi_I2CM_Descriptor_t desc[2];	/* 必须静态 */
	qapi_I2CM_Config_t config;				/* 绝对不能静态 */
	static unsigned int transferred1 = 0;	/* 必须静态 */

	qt_uart_dbg(uart_conf.hdlr,"i2c_read: slave_addr[0x%x] \n", device_Address);

	// Configure the bus speed and slave address
	config.bus_Frequency_KHz = 100; 
	config.slave_Address	 = device_Address;
	config.SMBUS_Mode		 = 0;
	config.slave_Max_Clock_Stretch_Us = 100000;
	config.core_Configuration1 = 0;
	config.core_Configuration2 = 0;

	if(len >= 127)
	{
		return QAPI_ERROR;
	}

	// Read N bytes from a register 0x01 on a sensor device
	// <S><slave_address><W><A><0x01><A><S><slave_address><R><A>
	//					   <data1><A><data2><A>...<dataN><N><P>
	desc[0].buffer	   = buff;
	desc[0].length	   = len;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ | QAPI_I2C_FLAG_STOP;

	qt_uart_dbg(uart_conf.hdlr,"[I2C] qapi_I2CM_Transfer: start \n");
	res = qapi_I2CM_Transfer (i2c_handle, &config, desc, 1, i2c_callback, &CB_Parameter, 100);

	if(res == QAPI_OK)
	{
		CB_Parameter = I2C_CB_BUSY;
	}

	return res;
}

qapi_Status_t i2c_read_reg_block(uint8 device_Address, uint16 reg, uint8 *buff, I2C_REG_TYPE_t reg_type)
{
	qapi_Status_t res = QAPI_ERROR;
	static uint8 r_reg[2];
	int cnt;

	if(reg_type == I2C_REG_16BIT)
	{
		r_reg[0] = (uint8)(reg>>8);
		r_reg[1] = (uint8)reg;

		res = i2c_write_mem(device_Address, 2, r_reg);
	}
	else
	{
		r_reg[0] = (uint8)reg;
		
		res = i2c_write_mem(device_Address, 1, r_reg);
	}

	if(res == QAPI_OK)
	{
		 /* Block to check the callback parameter, max 500ms */
		for(cnt=0;cnt<500;cnt++)
		{
			if(CB_Parameter == I2C_CB_IDLE)
			{
				break;
			}
			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
		}
		if(CB_Parameter != I2C_CB_IDLE)
		{
			return QAPI_ERROR;
		}
	}
	else
	{
		return QAPI_ERROR;
	}

	res = i2c_read_mem(device_Address, 1, buff);
	if(res == QAPI_OK)
	{
		 /* Block to check the callback parameter, max 500ms */
		for(cnt=0;cnt<500;cnt++)
		{
			if(CB_Parameter == I2C_CB_IDLE)
			{
				return QAPI_OK;
			}
			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
		}
	}
	
	return QAPI_ERROR;
}


int quectel_task_entry(void)
{
	
    qapi_Status_t res = QAPI_OK;
//    uint8   slave_Address = 0x1B;  // address of CODEC ALC5616
	uint8   slave_Address = 0x50;
    uint16   reg =0x0125;
    uint8   wbuf[5] = {0x55, 0x88, 0xA9};    
	int ret = -1;
	uint8 cnt = 0;
  
	qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);
	

	ret = txm_module_object_allocate(&byte_pool_gpio, sizeof(TX_BYTE_POOL));
	if(ret != TX_SUCCESS)
	{
		return ret;
	}

	ret = tx_byte_pool_create(byte_pool_gpio, "Sensor1 application pool",free_memory_Uart, GPIO_BYTE_POOL_SIZE);
	if(ret != TX_SUCCESS)
	{
		return ret;
	}
	
	ret = tx_byte_allocate(byte_pool_gpio, (VOID *)&rx_buff, 4*1024, TX_NO_WAIT);
	if(ret != TX_SUCCESS)
	{
		return ret;
	}

	ret = tx_byte_allocate(byte_pool_gpio, (VOID *)&tx_buff, 4*1024, TX_NO_WAIT);
	if(ret != TX_SUCCESS)
	{
		return ret;
	}

	quectel_dbg_uart_init(QAPI_UART_PORT_002_E);

	/* prompt task running */
    qt_uart_dbg(uart_conf.hdlr,"[task_create] start task ~");

    // Obtain a client specific connection handle to the i2c bus instance 4, I2C
    res = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_004_E, &i2c_handle);    
    qt_uart_dbg(uart_conf.hdlr,"qapi_i2cm_open: res=%d, hdl=%x\r\n", res, i2c_handle);

    res = qapi_I2CM_Power_On(i2c_handle);
  	qt_uart_dbg(uart_conf.hdlr,"qapi_I2CM_Power_On: res=%d\r\n", res);
    
   // Reads the value of the register
   //res = i2c_read(slave_Address, reg);     
   //qt_uart_dbg(uart_conf.hdlr,"i2c_read:byte1[0x%x], byte2[0x%x]\r\n", rbuf[0], rbuf[1]);

	while(1)
	{
   
   // write the register
//   res = i2c_write(slave_Address, reg, i2c_wbuffer, 2); 
//	res = i2c_write_reg_block(slave_Address, reg, 0x03, I2C_REG_16BIT);
	res = i2c_write_reg_block(slave_Address, reg, cnt, I2C_REG_16BIT);
//	res = i2c_write_mem_block(slave_Address, 3, wbuf);
    
//   qapi_Timer_Sleep(500, QAPI_TIMER_UNIT_MSEC, true);
   qt_uart_dbg(uart_conf.hdlr,"i2c_write_mem:res=%d\r\n", res);
  
    // read the register again
   //res = i2c_read(slave_Address, reg);  
   res = i2c_read_reg_block(slave_Address, reg, i2c_rbuffer, I2C_REG_16BIT);
    
//   qapi_Timer_Sleep(500, QAPI_TIMER_UNIT_MSEC, true);
   qt_uart_dbg(uart_conf.hdlr,"i2c_read:byte1[0x%x], byte2[0x%x]\r\n", i2c_rbuffer[0], i2c_rbuffer[1]);

	cnt++;
	}
	
    // Close the connection handle to the i2c bus instance
    res = qapi_I2CM_Close (i2c_handle);    
 	qt_uart_dbg(uart_conf.hdlr,"qapi_i2cm_close: res=%d\r\n", res);

	qt_uart_dbg(uart_conf.hdlr,"\r\n===quectel i2c task entry Exit!!!===\r\n");
  
    return 0;
}

#endif /*__EXAMPLE_I2C__*/

