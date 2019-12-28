/******************************************************************************
*@file    task.c
*@brief   driver for task create operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/

#include "txm_module.h"
#include "qapi_timer.h"
#include "qapi_diag.h"
#include "qapi_uart.h"
#include "task.h"
#include "uart.h"
#include "gps.h"
#include "adc.h"
#include "i2c.h"
#include "oled.h"
#include "../Gizwits/gizwits_protocol.h"
#include "../Gizwits/gizwits_product.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/
/* Queue related parameters */
TX_QUEUE *tx_queue_handle = NULL;
void *task_comm = NULL;
TX_BYTE_POOL *byte_pool_task;
#define TASK_BYTE_POOL_SIZE     16*8*1024
char free_memory_task[TASK_BYTE_POOL_SIZE];

#define QT_Q_MAX_INFO_NUM       16

/* Alive task related parameters */
//#define ALIVE_THREAD_PRIORITY   	180
#define ALIVE_THREAD_PRIORITY   	99
#define ALIVE_THREAD_STACK_SIZE 	(1024 * 16)

TX_THREAD* alive_thread_handle; 
char *alive_thread_stack = NULL;


/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
void alive_task_entry(void);


/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/*
@func
  queue_init
@brief
  User API Function, init the Queue.Must before task create. 
*/
void queue_init(void)
{
    int ret_txm = -1;
    UINT status = TX_SUCCESS;
//    uint32 message_size;

	/* Queue Create */
    ret_txm = txm_module_object_allocate(&byte_pool_task, sizeof(TX_BYTE_POOL));
    if(ret_txm != TX_SUCCESS)
    {
        gizLog(LOG_WARNING,"[gizwitsInit] txm_module_object_allocate [byte_pool_task] failed, %d \n", ret_txm);
        return;
    }

    ret_txm = tx_byte_pool_create(byte_pool_task, "task application pool", free_memory_task, TASK_BYTE_POOL_SIZE);
    if(ret_txm != TX_SUCCESS)
    {
        gizLog(LOG_WARNING,"[gizwitsInit] tx_byte_pool_create [byte_pool_task] failed, %d \n", ret_txm);
        return;
    }

    ret_txm = txm_module_object_allocate(&tx_queue_handle, sizeof(TX_QUEUE));
    if(ret_txm != TX_SUCCESS)
    {
        gizLog(LOG_WARNING,"[gizwitsInit] txm_module_object_allocate gizQueue failed, %d \n", ret_txm);
        return;
    }

    ret_txm = tx_byte_allocate(byte_pool_task, (void **)&task_comm, QT_Q_MAX_INFO_NUM * sizeof(TASK_COMM), TX_NO_WAIT);
    if(ret_txm != TX_SUCCESS)
    {
        gizLog(LOG_WARNING,"[gizwitsInit] tx_byte_allocate task_comm failed");
        return;
    }

    status = tx_queue_create(tx_queue_handle, "q_task_comm", TX_16_ULONG, task_comm, QT_Q_MAX_INFO_NUM * sizeof(TASK_COMM));
    if (TX_SUCCESS != status)
    {
        gizLog(LOG_WARNING,"[gizwitsInit] gizQueue Create Failed [%d] \n",status);
        return;
    }
    else
    {
        gizLog(LOG_WARNING,"[gizwitsInit] gizQueue Create Success [%d] \n",status);
    }

}

/*
@func
  alive_task_entry
@brief
  User API Function.It's a task alive all time, and debug print per second.
*/
void alive_task_entry(void)
{
	uint32 _50ms_cnt = 0;
	uint16 i = 0;
	uint8 pwm_send_buffer[7];
	uint8 xor_result = 0;

	gizLog(LOG_WARNING,"[Alive task] $$$$$$ Alive task entry now $$$$$$$$\n");
	qapi_Timer_Sleep(2, QAPI_TIMER_UNIT_SEC, true);
	while(1)
	{
		_50ms_cnt++;
		qapi_Timer_Sleep(50, QAPI_TIMER_UNIT_MSEC, true);

		if(0 == _50ms_cnt%20)
		{
			gizLog(LOG_WARNING,"[Alive task] $$$$$$$ I am alive $$$$$$$$ \n");
			currentDataPoint.valueAlive_seconds++;

			if(IS_GPS_LOCK)
			{
				uart3_printf("UTC: %llu LAT: %d.%d LON: %d.%d ALT: %d.%d ACC: %d.%d \n",
		            location.timestamp,
		            (int)location.latitude, ((int)(location.latitude*10000000))%10000000,
		            (int)location.longitude, ((int)(location.longitude*10000000))%10000000,
		            (int)location.altitude, ((int)(location.altitude*100))%100,
		            (int)location.accuracy, ((int)(location.accuracy*100))%100);

				currentDataPoint.valueGPS_Locked = TRUE;
				for(i=0;i<8;i++)
				{
					currentDataPoint.valueGPS_long[i] = (uint8)(((int)(location.longitude*10000000))>>(i*8));
					currentDataPoint.valueGPS_lat[i]  = (uint8)(((int)(location.latitude*10000000))>>(i*8));
				}
				for(i=0;i<4;i++)
				{
					currentDataPoint.valueGPS_Speed[i] = (uint8)(((int)(location.speed*100))>>(i*8));
					currentDataPoint.valueGPS_acc[i]  = (uint8)(((int)(location.accuracy*100))>>(i*8));
				}
			}
			else
			{
//				uart3_printf("GPS Unlock... \n");
				currentDataPoint.valueGPS_Locked = FALSE;
			}
//
//			uart1_printf("MPU6050 gx: %d \n",read_MPU6050_gx());
		}

		/* OLED refresh, every 50 ms. */
		if(currentDataPoint.valueKey1_value == TRUE)
		{
			lcd_printf_inplay(0, 0, "GPS:%s", currentDataPoint.valueGPS_Locked==TRUE?"LOCK":"DOWN");
			lcd_printf_display(0, 64, "ACC:%2d m", (int)(location.accuracy));
			lcd_printf_display(2, 0, "speed: %2d.%02d m/s",(int)location.speed, ((int)(location.speed*100))%100);
			lcd_printf_display(4, 0, " long: %4d.%02d  ", (int)location.longitude, ((int)(location.longitude*10000))%10000);
			lcd_printf_display(6, 0, "  lat: %4d.%02d  ", (int)location.latitude, ((int)(location.latitude*10000))%10000);
		}
		else
		{
			lcd_printf_display(0,0,"Alive:%2d",currentDataPoint.valueAlive_seconds%100);
			lcd_printf_display(0,64," Stop:%s",currentDataPoint.valueStop==TRUE?" Y":" N");
			lcd_printf_display(2,0,"pwm1:%3d        ",currentDataPoint.valueSpeed_value);
			lcd_printf_display(4,0,"pwm2:%3d        ",currentDataPoint.valueDirection_value);
			lcd_printf_display(6,0,"                ");
		}

		/* PWM values send to STM32, every 50ms*/
		pwm_send_buffer[0] = 0x55;
		pwm_send_buffer[1] = (uint8)currentDataPoint.valueSpeed_value;
		pwm_send_buffer[2] = (uint8)currentDataPoint.valueDirection_value;
		pwm_send_buffer[3] = currentDataPoint.valueStop==true?0x00:0x01;
		pwm_send_buffer[4] = 0x00;
		pwm_send_buffer[5] = 0x00;
		for(i=0,xor_result=0;i<6;i++)
		{
			xor_result ^= pwm_send_buffer[i];
		}
		pwm_send_buffer[6] = xor_result;
		uart3_send((char *)pwm_send_buffer, 7);
	}
}

/*
@func
  alive_task_init
@brief
  User API Function.
*/
int alive_task_init(void)
{
	int ret = -1;

	/* create a alive task */
	if(TX_SUCCESS != txm_module_object_allocate((VOID *)&alive_thread_handle, sizeof(TX_THREAD))) 
	{
		gizLog(LOG_WARNING,"[Alive task] txm_module_object_allocate alive_thread_handle failed");
		return - 1;
	}

	ret = tx_byte_allocate(byte_pool_task, (VOID **) &alive_thread_stack, ALIVE_THREAD_STACK_SIZE, TX_NO_WAIT);
	if(ret != TX_SUCCESS)
	{
		gizLog(LOG_WARNING,"[Alive task] txm_module_object_allocate alive_thread_stack failed");
		return ret;
	}

	/* create a new task : alive task */
	ret = tx_thread_create(alive_thread_handle,
						   "Mickey's alive Task Thread",
						   alive_task_entry,
						   NULL,
						   alive_thread_stack,
						   ALIVE_THREAD_STACK_SIZE,
						   ALIVE_THREAD_PRIORITY,
						   ALIVE_THREAD_PRIORITY,
						   TX_NO_TIME_SLICE,
						   TX_AUTO_START
						   );
	      
	if(ret != TX_SUCCESS)
	{
		gizLog(LOG_WARNING,"[Alive task] Thread creation failed");
	}

	return 0;
}





