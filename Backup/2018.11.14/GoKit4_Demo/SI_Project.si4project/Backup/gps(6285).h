/******************************************************************************
*@file    gps.h
*@brief   driver of gps operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __GPS_H__
#define __GPS_H__

#include "qapi_location.h"
#include "../main/log.h"

extern qapi_Location_t location;

/*  User MACROs
 * @{
 */
#define IS_GPS_LOCKED	(location.flags & QAPI_LOCATION_HAS_LAT_LONG_BIT)
/**@} */
	
/*  User API interface
 * @{
 */
void gps_init(void);
void gps_deinit(void);
/**@} */

#endif /*__EXAMPLE_GPS_H__*/

