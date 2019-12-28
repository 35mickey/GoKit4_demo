/******************************************************************************
*@file    i2c.h
*@brief   driver of i2c operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __I2C_H__
#define __I2C_H__

#include "qapi_fs_types.h"
#include "../main/log.h"

typedef enum
{
	I2C_CB_IDLE = 0,
	I2C_CB_BUSY,
}I2C_CB_Status_t;

/*  Only for MACROs, don't use following varients!!
 * @{
 */
#define MPU6050_TEST_ON		/* Only for test */
#ifdef MPU6050_TEST_ON
// 定义MPU6050内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42

#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48

#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		0x75	//IIC地址寄存器(默认数值0x68，只读)


//****************************

#define	MPU6050_Addr   0x68	  //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改

#endif
/**@} */

/*  User MACROs
 * @{
 */

/**@} */

/*  User API interface
 * @{
 */
void i2c_init(void);
qapi_Status_t i2c_read_reg (uint8 device_Address, uint8 reg, uint8 *buff);
qapi_Status_t i2c_write_reg (uint8 device_Address, uint8 reg, uint8 data);
//qapi_Status_t i2c_read_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff);
//qapi_Status_t i2c_write_mem (uint8 device_Address, uint8 reg, uint8 len, uint8 *data_buff);

#ifdef MPU6050_TEST_ON
void init_MPU6050(void);
//int32 read_MPU6050_gx(void);
#endif

/**@} */

#endif /*__EXAMPLE_I2C_H__*/

