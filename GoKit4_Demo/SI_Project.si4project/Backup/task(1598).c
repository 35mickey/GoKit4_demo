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
#include "../Gizwits/gizwits_protocol.h"

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
	gizLog(LOG_WARNING,"[Alive task] $$$$$$ Alive task entry now $$$$$$$$\n");
	qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);
	while(1)
	{
		gizLog(LOG_WARNING,"[Alive task] $$$$$$$ I am alive $$$$$$$$ \n");
		qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);

		if(IS_GPS_LOCK)
		{
			uart1_printf("UTC: %llu LAT: %d.%d LON: %d.%d ALT: %d.%d ACC: %d.%d \n",
	            location.timestamp,
	            (int)location.latitude, ((int)(location.latitude*10000000))%10000000,
	            (int)location.longitude, ((int)(location.longitude*10000000))%10000000,
	            (int)location.altitude, ((int)(location.altitude*100))%100,
	            (int)location.accuracy, ((int)(location.accuracy*100))%100);
		}
		else
		{
			uart1_printf("GPS Unlock... \n");
		}
		uart3_printf("Uart3 nice job!\n");
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





