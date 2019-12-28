/******************************************************************************
*@file    timer.h
*@brief   driver of timer operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "qapi_timer.h"
#include "../main/log.h"

/*  Only for MACROs, don't use following varients!!
 * @{
 */
extern qapi_TIMER_handle_t gps_timer_handle;
extern qapi_TIMER_set_attr_t gps_timer_set_attr;
/**@} */

/*  User MACROs
 * @{
 */
#define GPS_TIMER_RESET	qapi_Timer_Set(gps_timer_handle, &gps_timer_set_attr)
/**@} */

/*  User API interface
 * @{
 */
void timer_init(void);
void all_timer_start(void);
/**@} */

#endif /*__EXAMPLE_TIMER_H__*/

