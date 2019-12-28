/******************************************************************************
*@file    task.h
*@brief   driver of task create operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __TASK_H__
#define __TASK_H__

#include "../main/log.h"

typedef struct TASK_COMM_S{
	int msg_id;
	int message;
	CHAR name[16];
	CHAR buffer[32];
}TASK_COMM;


/*  User global varients
 * @{
 */

/**@} */
extern TX_QUEUE *tx_queue_handle;


/*  User API interface
 * @{
 */
void queue_init(void);
int alive_task_init(void);

/**@} */

#endif /*__EXAMPLE_TASK_H__*/

