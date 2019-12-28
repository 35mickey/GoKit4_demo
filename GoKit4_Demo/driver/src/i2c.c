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
unsigned char i2c_wbuffer[128] = {};

// Transfer completed
I2C_CB_Status_t CB_Parameter = I2C_CB_IDLE;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
void i2c_callback (const uint32 status, void *cb_para)  //  uint32
{
	CB_Parameter = I2C_CB_IDLE;
//	gizLog(LOG_INFO,"[I2C] i2c_callback: status=%d\r\n", status);
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
qapi_Status_t i2c_write_reg(uint8 device_Address, uint16 reg, uint8 data, I2C_REG_TYPE_t reg_type)
{
	qapi_Status_t res = QAPI_OK;
	static qapi_I2CM_Descriptor_t desc[2];	/* 必须静态 */
	qapi_I2CM_Config_t config;				/* 绝对不能静态 */
	static unsigned int transferred1 = 0;	/* 必须静态 */

	gizLog(LOG_INFO,"[I2C] i2c_write_reg: start. CB: %d\n", CB_Parameter);

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
	
	gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: start \n");
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

	gizLog(LOG_INFO,"[I2C] i2c_write_mem: start. CB: %d\n", CB_Parameter);

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

	gizLog(LOG_INFO,"i2c_read: slave_addr[0x%x] \n", device_Address);

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

	gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: start \n");
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


//
//qapi_Status_t i2c_read_reg (uint8 device_Address, uint8 reg, uint8 *buff)
//{
//	qapi_Status_t res = QAPI_OK;
//	qapi_I2CM_Descriptor_t desc[2];
//	qapi_I2CM_Config_t config;
//	unsigned int transferred1 = 0, transferred2 = 0;
//
////	gizLog(LOG_INFO,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);
//	
//	// Configure the bus speed and slave address
//	config.bus_Frequency_KHz = 50; 
//	config.slave_Address	 = device_Address;
//	config.SMBUS_Mode		 = 0;
//	config.slave_Max_Clock_Stretch_Us = 100000;
//	config.core_Configuration1 = 0;
//	config.core_Configuration2 = 0;
//
//	// Read N bytes from a register 0x01 on a sensor device
//	// <S><slave_address><W><A><0x01><A><S><slave_address><R><A>
//	//					   <data1><A><data2><A>...<dataN><N><P>
//	desc[0].buffer	   = &reg;
//	desc[0].length	   = 1;
//	desc[0].transferred = (uint32)&transferred1;
//	desc[0].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;
//
//	desc[1].buffer	   = buff;
//	desc[1].length	   = 1;
//	desc[1].transferred = (uint32)&transferred2;
//	desc[1].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
//
////	gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: start \n");
//	res = qapi_I2CM_Transfer (i2c_handle, &config, &desc[0], 2, i2c_callback, &CB_Parameter, 100);
//
//	if(res == QAPI_OK)
//	{
//		CB_Parameter = I2C_CB_BUSY;
//	}
//
//	return res;
//}
//
//qapi_Status_t i2c_read_reg_block(uint8 device_Address, uint8 reg, uint8 *buff)
//{
//	qapi_Status_t res = QAPI_ERROR;
//	int cnt;
//	res = i2c_read_reg(device_Address, reg, buff);
//	
//	if(res == QAPI_OK)
//	{
//		 /* Block to check the callback parameter, max 500ms */
//		for(cnt=0;cnt<500;cnt++)
//		{
//			if(CB_Parameter == I2C_CB_IDLE)
//			{
//				return QAPI_OK;
//			}
//			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
//		}
//	}
//	
//	return QAPI_ERROR;
//}
//
//qapi_Status_t i2c_write_reg(uint8 device_Address, uint8 reg, uint8 data)
//{
//	qapi_Status_t res = QAPI_OK;
//	qapi_I2CM_Descriptor_t desc[2];
//	qapi_I2CM_Config_t config;
//	unsigned int transferred1 = 2;
//
//	//gizLog(LOG_INFO,"[I2C] i2c_write_reg: start \n");
////	gizLog(LOG_INFO,"[I2C] i2c_write_reg: start. CB: %d\n", CB_Parameter);
//
//	i2c_wbuffer[0] = reg;
//	i2c_wbuffer[1] = data;
//
//	// Configure the bus speed and slave address
//	config.bus_Frequency_KHz = 50; 
//	config.slave_Address	 = device_Address;
//	config.SMBUS_Mode		 = 0;
//	config.slave_Max_Clock_Stretch_Us = 100000;
//	config.core_Configuration1 = 0;
//	config.core_Configuration2 = 0;
//	
//	desc[0].buffer		= i2c_wbuffer;
//	desc[0].length		= 2;
//	desc[0].transferred = (uint32)&transferred1;
//	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;
//	
////	gizLog(LOG_INFO,"[I2C] qapi_I2CM_Transfer: start \n");
//	res = qapi_I2CM_Transfer (i2c_handle, &config, &desc[0], 1, i2c_callback, &CB_Parameter, 100);
//
//	if(res == QAPI_OK)
//	{
//		CB_Parameter = I2C_CB_BUSY;
//	}
//
//	return res;
//}
//
//qapi_Status_t i2c_write_reg_block(uint8 device_Address, uint8 reg, uint8 data)
//{
//	qapi_Status_t res = QAPI_ERROR;
//	int cnt;
//	res = i2c_write_reg(device_Address, reg, data);
//	
//	if(res == QAPI_OK)
//	{
//		 /* Block to check the callback parameter, max 500ms */
//		for(cnt=0;cnt<500;cnt++)
//		{
//			if(CB_Parameter == I2C_CB_IDLE)
//			{
//				return QAPI_OK;
//			}
//			qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_MSEC, true);
//		}
//	}
//	
//	return QAPI_ERROR;
//}
//

//qapi_Status_t i2c_read_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff)
//{
//	qapi_Status_t res = QAPI_OK;
//	qapi_I2CM_Descriptor_t desc[2];
//	unsigned int transferred1 = 0, transferred2 = 0;
//
////	gizLog(LOG_INFO,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);
//
//	i2c_config.slave_Address = device_Address;
//
//	// Read N bytes from a register 0x01 on a sensor device
//	// <S><slave_address><W><A><0x01><A><S><slave_address><R><A>
//	//					   <data1><A><data2><A>...<dataN><N><P>
//	desc[0].buffer	   = &reg;
//	desc[0].length	   = 1;
//	desc[0].transferred = (uint32)&transferred1;
//	desc[0].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE;
//
//	desc[1].buffer	   = data_buff;
//	desc[1].length	   = len;
//	desc[1].transferred = (uint32)&transferred2;
//	desc[1].flags	   = QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_READ  | QAPI_I2C_FLAG_STOP;
//	res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 2, i2c_callback, &CB_Parameter, 0);
//
//	return res;
//}
//
//qapi_Status_t i2c_write_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff)
//{
//	qapi_Status_t res = QAPI_OK;
//	qapi_I2CM_Descriptor_t desc[2];
//	unsigned int transferred1 = 0;
//	int i = 0;
//	unsigned char buff[128];	/* May need to modify in real environments */
//
////	gizLog(LOG_INFO,"i2c_read: slave_addr[0x%x], reg[0x%x]\r\n", device_Address, reg);
//
//	i2c_config.slave_Address = device_Address;
//
//	buff[0] = reg;
//	if(len >= 127)
//	{
//		gizLog(LOG_ERROR,"[I2C] i2c_write_mem: data is too long");
//		return QAPI_ERROR;
//	}
//	else
//	{
//		for(i = 1;i <= len;i++)
//			buff[i] = data_buff[i-1];
//	}
//		
//	// Write transfer for one reg, one data
//	// <S><slave_address><W><A><data1><A><data2><A>...<dataN><A><P>
//	desc[0].buffer		= buff;
//	desc[0].length		= len + 1;
//	desc[0].transferred = (uint32)&transferred1;
//	desc[0].flags		= QAPI_I2C_FLAG_START | QAPI_I2C_FLAG_WRITE | QAPI_I2C_FLAG_STOP;
//	res = qapi_I2CM_Transfer (i2c_handle, &i2c_config, &desc[0], 1, i2c_callback, &CB_Parameter, 0);
//
//	return res;
//}


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

	status = i2c_write_reg_block(MPU6050_Addr,PWR_MGMT_1, 0x00, I2C_REG_8BIT);	//解除休眠状态
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	status = i2c_write_reg_block(MPU6050_Addr,SMPLRT_DIV, 0x07, I2C_REG_8BIT);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	status = i2c_write_reg_block(MPU6050_Addr,CONFIG, 0x06, I2C_REG_8BIT);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	status = i2c_write_reg_block(MPU6050_Addr,GYRO_CONFIG, 0x18, I2C_REG_8BIT);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	status = i2c_write_reg_block(MPU6050_Addr,ACCEL_CONFIG, 0x01, I2C_REG_8BIT);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_write_reg: status=%d\r\n", status);return;}

	gizLog(LOG_INFO,"[MPU6050] MPU6050 init success. \n");
}

//******读取MPU6050数据****************************************
int32 read_MPU6050_gx(void)
{
	qapi_Status_t status = QAPI_OK;
	uint16 H_byte = 0,L_byte = 0;
	int32 tmp;

	status = i2c_read_reg_block(MPU6050_Addr,GYRO_XOUT_L,(uint8 *)&L_byte, I2C_REG_8BIT); 
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_read_reg: status=%d\r\n", status);return 0;}
	status = i2c_read_reg_block(MPU6050_Addr,GYRO_XOUT_H,(uint8 *)&H_byte, I2C_REG_8BIT);
	if(status!=QAPI_OK)	{gizLog(LOG_ERROR,"[I2C ERROR] i2c_read_reg: status=%d\r\n", status);return 0;}
	tmp = (int32)((H_byte<<8)|L_byte);
	tmp /= 16.4; 						   //读取计算X轴数据
	return tmp;
}

#endif



