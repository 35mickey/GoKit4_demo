/******************************************************************************
*@file    gps.c
*@brief   driver for gps operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/

#include "txm_module.h"
#include "gps.h"
#include "uart.h"
#include "timer.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/

/*==========================================================================
  Signals used for waiting in test app for callbacks
===========================================================================*/
static TX_EVENT_FLAGS_GROUP* gps_signal_handle; /* Not must for gps, but for the demo */
static uint32 gps_tracking_id;

#define LOCATION_TRACKING_FIXED_BIT (0x1 << 0)
#define LOCATION_TRACKING_RSP_OK_BIT (0x1 << 1)
#define LOCATION_TRACKING_RSP_FAIL_BIT (0x1 << 2)
qapi_Location_t location;

#define GPS_USER_BUFFER_SIZE 4096
static uint8_t GPSuserBuffer[GPS_USER_BUFFER_SIZE];
qapi_loc_client_id loc_clnt;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/

/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/*==========================================================================
LOCATION API REGISTERED CALLBACKS
===========================================================================*/
static void location_capabilities_callback(qapi_Location_Capabilities_Mask_t capabilities)
{

    gizLog(LOG_INFO,"[GPS] Location mod has tracking capability:%d \n",capabilities);
}

static void location_response_callback(qapi_Location_Error_t err, uint32_t id)
{
    gizLog(LOG_INFO,"[GPS] LOCATION RESPONSE CALLBACK err=%u id=%u \n", err, id); 
	if(err == QAPI_LOCATION_ERROR_SUCCESS)
	{
		tx_event_flags_set(gps_signal_handle, LOCATION_TRACKING_RSP_OK_BIT, TX_OR);	
	}
	else
	{
		tx_event_flags_set(gps_signal_handle, LOCATION_TRACKING_RSP_FAIL_BIT, TX_OR);	
	}
}
static void location_collective_response_cb(size_t count, qapi_Location_Error_t* err, uint32_t* ids)
{
    
}

static void location_tracking_callback(qapi_Location_t loc)
{
	/* Every tracking update, the location data are transformed to location */
	memcpy(&location,&loc,sizeof(qapi_Location_t));

	uart1_printf("UTC: %llu LAT: %d.%d LON: %d.%d ALT: %d.%d ACC: %d.%d \n",
	            location.timestamp,
	            (int)location.latitude, ((int)(location.latitude*10000000))%10000000,
	            (int)location.longitude, ((int)(location.longitude*10000000))%10000000,
	            (int)location.altitude, ((int)(location.altitude*100))%100,
	            (int)location.accuracy, ((int)(location.accuracy*100))%100);
	
    if(loc.flags & QAPI_LOCATION_HAS_LAT_LONG_BIT) 
    {
        tx_event_flags_set(gps_signal_handle, LOCATION_TRACKING_FIXED_BIT, TX_OR);
    }
}

qapi_Location_Callbacks_t location_callbacks= {
    sizeof(qapi_Location_Callbacks_t),
    location_capabilities_callback,
    location_response_callback,
    location_collective_response_cb,
    location_tracking_callback,
    NULL,
    NULL,
    NULL
};


/*
@func
  gps_init
@brief
  User API Function, init the location function. 
*/
void gps_init(void)
{
	qapi_Location_Error_t ret;
	uint32 signal = 0;
	
	qapi_Location_Options_t Location_Options = {sizeof(qapi_Location_Options_t),
												1000,//update lat/lon frequency, in milliseconds.
												0};
	txm_module_object_allocate(&gps_signal_handle, sizeof(TX_EVENT_FLAGS_GROUP));
	qapi_Timer_Sleep(1000, QAPI_TIMER_UNIT_MSEC, true);//need delay 1s, otherwise maybe there is no callback.

	/* This flags are not must needed */
    tx_event_flags_create(gps_signal_handle, "ts_gps_app_events");

    ret = qapi_Loc_Init(&loc_clnt, &location_callbacks);
    if (ret == QAPI_LOCATION_ERROR_SUCCESS)
    {
        gizLog(LOG_INFO,"[GPS] LOCATION_INIT SUCCESS \n");
    }
    else
    {
        gizLog(LOG_INFO,"[GPS] LOCATION_INIT FAILED \n");
        
    }
    ret = (qapi_Location_Error_t)qapi_Loc_Set_User_Buffer(loc_clnt, (uint8_t*)GPSuserBuffer, GPS_USER_BUFFER_SIZE);
    if (ret != QAPI_LOCATION_ERROR_SUCCESS) {
        gizLog(LOG_INFO,"[GPS] Set user buffer failed ! (ret %d) \n", ret);
    }

	qapi_Loc_Start_Tracking(loc_clnt, &Location_Options, &gps_tracking_id);
	tx_event_flags_get(gps_signal_handle, LOCATION_TRACKING_RSP_OK_BIT|LOCATION_TRACKING_RSP_FAIL_BIT, TX_OR_CLEAR, &signal, TX_WAIT_FOREVER);
	if(signal&LOCATION_TRACKING_RSP_OK_BIT)
	{
		gizLog(LOG_INFO,"[GPS] wating for tracking... \n");
	}
	else if(signal&LOCATION_TRACKING_RSP_FAIL_BIT)
	{
		gizLog(LOG_INFO,"[GPS] start tracking failed \n");
	}
}

/*
@func
  gps_deinit
@brief
  User API Function, deinit the location function. 
*/
void gps_deinit(void)
{
    qapi_Loc_Stop_Tracking(loc_clnt, gps_tracking_id);
    qapi_Loc_Deinit(loc_clnt);
    tx_event_flags_delete(gps_signal_handle);
}


