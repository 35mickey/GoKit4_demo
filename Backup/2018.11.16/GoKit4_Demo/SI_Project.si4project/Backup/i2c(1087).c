/******************************************************************************
*@file    i2c.c
*@brief   driver for i2c operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qapi_i2c_master.h"
#include "timer.h"
#include "i2c.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/

void *i2c_handle = NULL;
static qapi_I2CM_Config_t i2c_config;

// Transfer completed
static I2C_CB_Status_t CB_Parameter = I2C_CB_OK;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
void i2c_callback (const uint32 status, void *cb_para)  //  uint32
{
 	if(status == QAPI_OK)
		CB_Parameter = I2C_CB_OK;
	else
	{
		CB_Parameter = I2C_CB_ERROR;
	}
	gizLog(LOG_ERROR,"[I2C ERROR] i2c_callback: status=%d\r\n", status);
}

/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/******************************************** 
** <S>	- START bit
** <P>	- STOP	bit
** <Sr> - Repeat Start bit
** <A>	- Acknowledge bit
** <N>	- Not-Acknowledge bit
** <R>	- Read Transfer
** <W> - Write Transfer
********************************************/
qapi_Status_t i2c_read_reg (uint8 device_Address, uint8 reg, uint8 *buff)
{
	qapi_Status_t res = QAPI_OK;
	qapi_I2CM_Descriptor_t desc[2];
	unsigned int transferred1 = 0, transferred2 = 0;
	int cnt = 0;

//	qt_uart_dbg(uart_conf.hdlr,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);

	i2c_config.slave_Address = device_Address;

	// Read N bytes from a register 0x01 on a sensor device
	// <S><slave_address><W><A><0x01><A><S><slave_address><R><A>
	//					   <data1><A><data2><A>...<dataN><N><P>
	desc[0].buffer	   = &reg;
	desc[0].length	   = 1;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

	desc[1].buffer	   = buff;
	desc[1].length	   = 1;
	desc[1].transferred = (uint32)&transferred2;
	desc[1].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;

	/* Check the callback parameter, max 100ms */
	for(cnt=0;cnt<10;cnt++)
	{
		if(CB_Parameter != I2C_CB_BUSY)	break;
		qapi_Timer_Sleep(10, QAPI_TIMER_UNIT_MSEC, true);
	}
	
	if(cnt < 10)
	{
		CB_Parameter = I2C_CB_BUSY;
		res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 2, i2c_callback, &CB_Parameter, 0);
	}
	else
		return QAPI_I2CM_ERR_TRANSFER_TIMEOUT;

	return res;
}

qapi_Status_t i2c_write_reg (uint8 device_Address, uint8 reg, uint8 data)
{
	qapi_Status_t res = QAPI_OK;
	qapi_I2CM_Descriptor_t desc[2];
	unsigned int transferred1 = 0;
	unsigned char buff[2];
	int cnt = 0;

//	qt_uart_dbg(uart_conf.hdlr,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);

	i2c_config.slave_Address = device_Address;

	buff[0] = reg;
	buff[1] = data;
	
	// Write transfer for one reg, one data
	// <S><slave_address><W><A><data1><A><data2><A>...<dataN><A><P>
	desc[0].buffer		= buff;
	desc[0].length		= 2;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;

	gizLog(LOG_INFO,"[I2C] i2c_write_reg: start \n");

	/* Check the callback parameter, max 100ms */
	for(cnt=0;cnt<10;cnt++)
	{
		if(CB_Parameter != I2C_CB_BUSY)	break;
		qapi_Timer_Sleep(10, QAPI_TIMER_UNIT_MSEC, true);
	}

	if(cnt < 10)
	{
		CB_Parameter = I2C_CB_BUSY;
		gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: start \n");
		res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 1, i2c_callback, &CB_Parameter, 0);
		gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: in process status:%d \n", res);
	}
	else
		return QAPI_I2CM_ERR_TRANSFER_TIMEOUT;

	return res;
}

qapi_Status_t i2c_read_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff)
{
	qapi_Status_t res = QAPI_OK;
	qapi_I2CM_Descriptor_t desc[2];
	unsigned int transferred1 = 0, transferred2 = 0;

//	qt_uart_dbg(uart_conf.hdlr,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);

	i2c_config.slave_Address = device_Address;

	// Read N bytes from a register 0x01 on a sensor device
	// <S><slave_address><W><A><0x01><A><S><slave_address><R><A>
	//					   <data1><A><data2><A>...<dataN><N><P>
	desc[0].buffer	   = &reg;
	desc[0].length	   = 1;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;

	desc[1].buffer	   = data_buff;
	desc[1].length	   = len;
	desc[1].transferred = (uint32)&transferred2;
	desc[1].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
	res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 2, i2c_callback, &CB_Parameter, 0);

	return res;
}

qapi_Status_t i2c_write_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff)
{
	qapi_Status_t res = QAPI_OK;
	qapi_I2CM_Descriptor_t desc[2];
	unsigned int transferred1 = 0;
	int i = 0;
	unsigned char buff[128];	/* May need to modify in real environments */

//	qt_uart_dbg(uart_conf.hdlr,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);

	i2c_config.slave_Address = device_Address;

	buff[0] = reg;
	if(len >= 127)
	{
		gizLog(LOG_ERROR,"[I2C] i2c_write_mem: data is too long");
		return QAPI_ERROR;
	}
	else
	{
		for(i = 1;i <= len;i++)
			buff[i] = data_buff[i-1];
	}
		
	// Write transfer for one reg, one data
	// <S><slave_address><W><A><data1><A><data2><A>...<dataN><A><P>
	desc[0].buffer		= buff;
	desc[0].length		= len + 1;
	desc[0].transferred = (uint32)&transferred1;
	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;
	res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 1, i2c_callback, &CB_Parameter, 0);

	return res;
}


/*
@func
  i2c_init
@brief
  User API Function, init the i2c peripheral. 
*/
void i2c_init(void)
{
	qapi_Status_t status = QAPI_OK;

	// Obtain a client specific connection handle to the i2c bus instance 4, I2C
    status = qapi_I2CM_Open(QAPI_I2CM_INSTANCE_004_E, &i2c_handle);    
    gizLog(LOG_INFO,"[I2C] qapi_i2cm_open: status=%d, hdl=0x%x\r\n", status, i2c_handle);

    status = qapi_I2CM_Power_On(i2c_handle);
  	gizLog(LOG_INFO,"[I2C] qapi_I2CM_Power_On: status=%d\r\n", status);
	
	// Configure the bus speed and slave address
	i2c_config.bus_Frequency_KHz = 10; 
	//config.slave_Address	= device_Address;
	i2c_config.SMBUS_Mode		= 0;
	i2c_config.slave_Max_Clock_Stretch_Us = 100000;
	i2c_config.core_Configuration1 = 0;
	i2c_config.core_Configuration2 = 0;
}

#ifdef MPU6050_TEST_ON

/* 以下为测试MPU6050时使用的函数和定义 */
//初始化MPU6050，根据需要请参考pdf进行修改************************
void init_MPU6050(void)
{
	qapi_Status_t status = QAPI_OK;
/*
   i2c_write_reg(MPU6050_Addr,PWR_M, 0x80);   //
   i2c_write_reg(MPU6050_Addr,SMPL, 0x07);    //
   i2c_write_reg(MPU6050_Addr,DLPF, 0x1E);    //±2000°
   i2c_write_reg(MPU6050_Addr,INT_C, 0x00 );  //
   i2c_write_reg(MPU6050_Addr,PWR_M, 0x00);   //
*/
	gizLog(LOG_INFO,"[MPU6050] MPU6050 init start. \n");

	status = i2c_write_reg(MPU6050_Addr,PWR_MGMT_1, 0x00);	//解除休眠状态
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}
	status = i2c_write_reg(MPU6050_Addr,SMPLRT_DIV, 0x07);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}
	status = i2c_write_reg(MPU6050_Addr,CONFIG, 0x06);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}
	status = i2c_write_reg(MPU6050_Addr,GYRO_CONFIG, 0x18);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}
	status = i2c_write_reg(MPU6050_Addr,ACCEL_CONFIG, 0x01);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	gizLog(LOG_INFO,"[MPU6050] MPU6050 init success. \n");
}

//******读取MPU6050数据****************************************
int32 read_MPU6050_gx(void)
{
	qapi_Status_t status = QAPI_OK;
	uint16 H_byte = 0,L_byte = 0;
	int32 tmp;

	status = i2c_read_reg(MPU6050_Addr,GYRO_XOUT_L,(uint8 *)&L_byte); 
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_read_reg: status=%d\r\n", status);return 0;}
	status = i2c_read_reg(MPU6050_Addr,GYRO_XOUT_H,(uint8 *)&H_byte);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_read_reg: status=%d\r\n", status);return 0;}
	tmp = (int32)((H_byte<<8)|L_byte);
	tmp /= 16.4; 						   //读取计算X轴数据
	return tmp;
}

#endif



