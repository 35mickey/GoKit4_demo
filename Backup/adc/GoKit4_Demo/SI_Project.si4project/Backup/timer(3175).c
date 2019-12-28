/******************************************************************************
*@file    timer.c
*@brief   driver for timer operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qapi_tlmm.h"
#include "qapi_timer.h"
#include "qapi_diag.h"
#include "qapi_uart.h"
#include "timer.h"
#include "../Gizwits/gizwits_protocol.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/

/** Gizwits Timer related parameters */ 
static qapi_TIMER_handle_t gizwits_timer_handle;
static qapi_TIMER_define_attr_t gizwits_timer_def_attr;
static qapi_TIMER_set_attr_t gizwits_timer_set_attr;


/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/


/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/*
@func
  timer_init
@brief
  User API Function, init the Timers. 
*/
void timer_init(void)
{
	qapi_Status_t status = QAPI_OK;

	/* For gizwits timer,100ms */
    memset(&gizwits_timer_def_attr, 0, sizeof(gizwits_timer_def_attr));
    gizwits_timer_def_attr.cb_type  = QAPI_TIMER_FUNC1_CB_TYPE;
    gizwits_timer_def_attr.deferrable = false;
    gizwits_timer_def_attr.sigs_func_ptr = gizTimerFunc;
    gizwits_timer_def_attr.sigs_mask_data = 0x01;
    status = qapi_Timer_Def(&gizwits_timer_handle, &gizwits_timer_def_attr);
	
    memset(&gizwits_timer_set_attr, 0, sizeof(gizwits_timer_set_attr));
    gizwits_timer_set_attr.reload = 90;	/* 这样理解对吗? */
    gizwits_timer_set_attr.time = 10;
    gizwits_timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;
//    status = qapi_Timer_Set(gizwits_timer_handle, &gizwits_timer_set_attr);
//    gizLog(LOG_INFO,"[gizwitsInit] gizTimer Create [%d]\n",status);


}

/*
@func
  all_timer_start
@brief
  User API Function, start all the Timers. 
*/
void all_timer_start(void)
{
	qapi_Status_t status = QAPI_OK;

	status = qapi_Timer_Set(gizwits_timer_handle, &gizwits_timer_set_attr);
	gizLog(LOG_INFO,"[gizwitsInit] gizTimer Create [%d]\n",status);
}




