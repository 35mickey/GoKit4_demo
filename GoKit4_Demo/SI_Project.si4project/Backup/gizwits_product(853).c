/**
************************************************************
* @file         gizwits_product.c
* @brief        Control protocol processing, and platform-related hardware initialization
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "gps.h"
#include "adc.h"
#include "i2c.h"

/** User area The current device state structure */
dataPoint_t currentDataPoint;

/**@name Gizwits User Interface
* @{
*/

/**
* @brief Event handling interface

* Description:

* 1. Users can customize the changes in WiFi module status

* 2. Users can add data points in the function of event processing logic, such as calling the relevant hardware peripherals operating interface

* @param [in] info: event queue
* @param [in] data: protocol data
* @param [in] len: protocol data length
* @return NULL
* @ref gizwits_protocol.h
*/
int8_t ICACHE_FLASH_ATTR gizwitsEventProcess(eventInfo_t *info, uint8_t *data, uint32_t len)
{
    uint8_t i = 0;
    dataPoint_t * dataPointPtr = (dataPoint_t *)data;
    moduleStatusInfo_t * wifiData = (moduleStatusInfo_t *)data;

    if((NULL == info) || (NULL == data))
    {
        gizLog(LOG_WARNING,"!!! gizwitsEventProcess Error \n");
        return -1;
    }

    for(i = 0; i < info->num; i++)
    {
        switch(info->event[i])
        {
		case EVENT_Motor :
            currentDataPoint.valueMotor = dataPointPtr->valueMotor;
            gizLog(LOG_INFO,"Evt: EVENT_Motor %d \n", currentDataPoint.valueMotor);
            if(0x01 == currentDataPoint.valueMotor)
             {
                GPIO_MOTOR_ON;
            }
            else
            {
                GPIO_MOTOR_OFF;
            }
            break;


         case EVENT_Sensor2:
            currentDataPoint.valueSensor2= dataPointPtr->valueSensor2;
            gizLog(LOG_INFO,"Evt:EVENT_Sensor2 %d\n",currentDataPoint.valueSensor2);
            //user handle
            if(currentDataPoint.valueSensor2 > 0)
            {
            	currentDataPoint.valueStr1[4] = currentDataPoint.valueSensor2 / 10;
            }
            break;
			

        case WIFI_SOFTAP:
            break;
        case WIFI_AIRLINK:
            break;
        case WIFI_STATION:
            break;
        case WIFI_CON_ROUTER:
            gizLog(LOG_INFO,"@@@@ connected router\n");
 
            break;
        case WIFI_DISCON_ROUTER:
            gizLog(LOG_INFO,"@@@@ disconnected router\n");
 
            break;
        case WIFI_CON_M2M:
            gizLog(LOG_INFO,"@@@@ connected m2m\n");
			setConnectM2MStatus(0x01);
 
            break;
        case WIFI_DISCON_M2M:
            gizLog(LOG_INFO,"@@@@ disconnected m2m\n");
			setConnectM2MStatus(0x00);
 
            break;
        case WIFI_RSSI:
            gizLog(LOG_INFO,"@@@@ RSSI %d\n", wifiData->rssi);
            break;
        case TRANSPARENT_DATA:
            gizLog(LOG_INFO,"TRANSPARENT_DATA \n");
            //user handle , Fetch data from [data] , size is [len]
            break;
        case MODULE_INFO:
            gizLog(LOG_INFO,"MODULE INFO ...\n");
            break;
            
        default:
            break;
        }
    }
    gizSendQueue(SIG_UPGRADE_DATA);
    
    return 0; 
}


/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void ICACHE_FLASH_ATTR userHandle(void)
{
    /*
    currentDataPoint.valuehumidity = ;//Add Sensor Data Collection

    */
	static uint32_t _100ms_cnt = 0;/* 为了防止反复发送，必须自己定义计数器 */
//	qapi_GPIO_Value_t key1_read_val = QAPI_GPIO_HIGH_VALUE_E;/* 读取KEY1状态 */
//	static uint8 key1_status_val = 0;/* 保存KEY1状态 */

	/* 6s一次同步数据 */
	_100ms_cnt++;
	if(0 == _100ms_cnt%60)	/* 6s一次 */	
	{
		currentDataPoint.valueSensor1 = adc0_update() + 1;
		gizLog(LOG_INFO,"*****************Sensor1 update every 6s*****************\n");
		sensorUpdateHandle(&currentDataPoint);
		//gizSendQueue(SIG_IMM_REPORT); /* 同步信息到云端 */
	}

	/* 按键动作检测 */
//	GPIO_KEY1_TO(&key1_read_val);
//	if(QAPI_GPIO_LOW_VALUE_E == key1_read_val)
//	{
//		key1_status_val = 1;
//	}
//	if(QAPI_GPIO_HIGH_VALUE_E == key1_read_val && 1 == key1_status_val)
//	{
//		key1_status_val = 2;
//	}
//	if(1 == key1_status_val)
//	{
//		key1_status_val = 0;
//		gizLog(LOG_INFO,"*****************KEY1 Clicked*****************\n");
//		currentDataPoint.valueKey = currentDataPoint.valueKey==true?false:true;
//		sensorUpdateHandle(&currentDataPoint);
//	}
	if(key1_state == KEY_ON)
	{
		key1_state = KEY_OFF;
		gizLog(LOG_INFO,"*****************KEY1 Clicked*****************\n");
		uart3_printf("*****************KEY1 Clicked*****************\n");
		currentDataPoint.valueKey = currentDataPoint.valueKey==true?false:true;
		sensorUpdateHandle(&currentDataPoint);
	}
	
}


/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void ICACHE_FLASH_ATTR userInit(void)
{
    gizMemset((uint8_t *)&currentDataPoint, 0, sizeof(dataPoint_t));

 	/** Warning !!! DataPoint Variables Init , Must Within The Data Range **/ 
    
	currentDataPoint.valueMotor = 0;
	currentDataPoint.valueSensor2 = -300;
	currentDataPoint.valueKey = 0;
	currentDataPoint.valueSensor1 = 888;

	currentDataPoint.valueStr1[0]=0x00;
	currentDataPoint.valueStr1[1]=0x99;
	currentDataPoint.valueStr1[2]=0x88;
	currentDataPoint.valueStr1[3]=0x77;
	currentDataPoint.valueStr1[4]=0x66;

	gizLog(LOG_INFO,"[userInit] userInit Begin \n");

	/* All driver init. */
    gpio_init();

	timer_init();

	user_uart_init();

//	gps_init();

	i2c_init();
	qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);	
	init_MPU6050();

//	adc_init();

	/* 电机转 1 秒 */
	GPIO_MOTOR_ON;
	qapi_Timer_Sleep(1, QAPI_TIMER_UNIT_SEC, true);	
	GPIO_MOTOR_OFF;
    
}


