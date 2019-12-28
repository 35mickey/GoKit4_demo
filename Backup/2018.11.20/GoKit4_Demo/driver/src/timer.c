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
#include "qapi_diag.h"
#include "qapi_uart.h"
#include "timer.h"
#include "gps.h"
#include "gpio.h"
#include "../Gizwits/gizwits_protocol.h"
#include "../Gizwits/gizwits_product.h"


/**************************************************************************
*                                 GLOBAL
***************************************************************************/

/** Gizwits Timer related parameters */ 
static qapi_TIMER_handle_t gizwits_timer_handle;
static qapi_TIMER_define_attr_t gizwits_timer_def_attr;
static qapi_TIMER_set_attr_t gizwits_timer_set_attr;

/** gps flags reset Timer related parameters */ 
qapi_TIMER_handle_t gps_timer_handle;
static qapi_TIMER_define_attr_t gps_timer_def_attr;
qapi_TIMER_set_attr_t gps_timer_set_attr;

/** pwm Timer related parameters */ 
qapi_TIMER_handle_t pwm_timer_handle;
static qapi_TIMER_define_attr_t pwm_timer_def_attr;
qapi_TIMER_set_attr_t pwm_timer_set_attr;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
void pwmTimerFunc(void);

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
    gizwits_timer_def_attr.sigs_mask_data = 0x10;
    status = qapi_Timer_Def(&gizwits_timer_handle, &gizwits_timer_def_attr);
	
    memset(&gizwits_timer_set_attr, 0, sizeof(gizwits_timer_set_attr));
    gizwits_timer_set_attr.reload = 100;	/* 这个是定时间隔 */
    gizwits_timer_set_attr.time = 10;		/* 这个是从定时器开始运行到触发回调的时间 */
    gizwits_timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;

	/* For gps reset timer,500ms */
    memset(&gps_timer_def_attr, 0, sizeof(gps_timer_def_attr));
    gps_timer_def_attr.cb_type  = QAPI_TIMER_FUNC1_CB_TYPE;
    gps_timer_def_attr.deferrable = false;
    gps_timer_def_attr.sigs_func_ptr = gpsTimerFunc;
    gps_timer_def_attr.sigs_mask_data = 0x05;
    status = qapi_Timer_Def(&gps_timer_handle, &gps_timer_def_attr);
	
    memset(&gps_timer_set_attr, 0, sizeof(gps_timer_set_attr));
    gps_timer_set_attr.reload = 500;	/* 这个是定时间隔 */
    gps_timer_set_attr.time = 10;		/* 这个是从定时器开始运行到触发回调的时间 */
    gps_timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;

	/* For pwm timer,10ms */
    memset(&pwm_timer_def_attr, 0, sizeof(pwm_timer_def_attr));
    pwm_timer_def_attr.cb_type  = QAPI_TIMER_FUNC1_CB_TYPE;
    pwm_timer_def_attr.deferrable = false;
    pwm_timer_def_attr.sigs_func_ptr = pwmTimerFunc;
    pwm_timer_def_attr.sigs_mask_data = 0x01;
    status = qapi_Timer_Def(&pwm_timer_handle, &pwm_timer_def_attr);
	
    memset(&pwm_timer_set_attr, 0, sizeof(pwm_timer_set_attr));
    pwm_timer_set_attr.reload = 2;	/* 这个是定时间隔 */
    pwm_timer_set_attr.time = 1;	/* 这个是从定时器开始运行到触发回调的时间 */
    pwm_timer_set_attr.unit = QAPI_TIMER_UNIT_MSEC;
	
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

	/* gizwits timer start */
	status = qapi_Timer_Set(gizwits_timer_handle, &gizwits_timer_set_attr);
	gizLog(LOG_INFO,"[gizwitsInit] gizTimer Create [%d]\n",status);

	/* gps timer start */
	status = qapi_Timer_Set(gps_timer_handle, &gps_timer_set_attr);
	gizLog(LOG_INFO,"[GPS Init] GPS Timer Create [%d]\n",status);

	/* pwm timer start */
	status = qapi_Timer_Set(pwm_timer_handle, &pwm_timer_set_attr);
	gizLog(LOG_INFO,"[PWM Init] PWM Timer Create [%d]\n",status);
}

/**
* @brief Timer callback function, in the function to generate pwm on GPIOs.
* @param none
* @return none
*/
void pwmTimerFunc(void)
{
	static uint16 cnt = 0;

	if(cnt >= (50 - 1))
	{
		cnt = 0;
		if(cnt == (currentDataPoint.valueSpeed_value/2))	GPIO_MOTOR_OFF;
		else 												GPIO_MOTOR_ON;
	}
	else
	{
		if(cnt == (currentDataPoint.valueSpeed_value/2))	GPIO_MOTOR_OFF;
		cnt++;
	}
}


