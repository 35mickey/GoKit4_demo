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

/** User area The current device state structure */
dataPoint_t currentDataPoint;

/**@name Gizwits User Interface
* @{
*/

/* 这里不用static会导致初始化失败 */
static qapi_GPIO_ID_t	   	gpio_id;
static qapi_TLMM_Config_t 	tlmm_config;
static qapi_Status_t	   	status = QAPI_OK;


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
                qapi_TLMM_Drive_Gpio(gpio_id,15,QAPI_GPIO_LOW_VALUE_E);
            }
            else
            {
                qapi_TLMM_Drive_Gpio(gpio_id,15,QAPI_GPIO_HIGH_VALUE_E);
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

	_100ms_cnt++;
	
	if(0 == _100ms_cnt%50)	/* 5s一次 */	
	{
		currentDataPoint.valueSensor1 = currentDataPoint.valueSensor1 + 1;
		gizLog(LOG_INFO,"Sensor1 update every 5s!!!!!!!!!!\n");
		sensorUpdateHandle(&currentDataPoint);
		//gizSendQueue(SIG_IMM_REPORT); /* 同步信息到云端 */
	}
	 
    //system_os_post(USER_TASK_PRIO_2, SIG_UPGRADE_DATA, 0);
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
	currentDataPoint.valueSensor2 = 511;
	currentDataPoint.valueKey = 0;
	currentDataPoint.valueSensor1 = 101;

	currentDataPoint.valueStr1[0]=0x99;
	currentDataPoint.valueStr1[1]=0x88;
	currentDataPoint.valueStr1[2]=0x77;
	currentDataPoint.valueStr1[3]=0x66;
	currentDataPoint.valueStr1[4]=0x11;

    tlmm_config.pin = 15;				 //GPIO 9 的pin num
    tlmm_config.func = 0;                // Using the functionality tied to
                                         // pin mux value 1
    tlmm_config.dir = QAPI_GPIO_OUTPUT_E;
    tlmm_config.pull = QAPI_GPIO_PULL_UP_E;
    tlmm_config.drive = QAPI_GPIO_10MA_E; // drive is for output pins, specify
                                         // the default here

    status = qapi_TLMM_Get_Gpio_ID( &tlmm_config, &gpio_id);

	gizLog(LOG_INFO,"Init MOTOR!\n");
	
	if (status == QAPI_OK)
	{
		status = qapi_TLMM_Config_Gpio(gpio_id, &tlmm_config);

		if (status != QAPI_OK)
		{
			// Handle failed case here
			gizLog(LOG_INFO,"GPIO MOTOR Failed!\n");
		}
		else
		{
			gizLog(LOG_INFO,"GPIO MOTOR Pass!\n");
		}
	} 

	qapi_TLMM_Drive_Gpio(gpio_id,15,QAPI_GPIO_HIGH_VALUE_E);	/* 小电机不要转了 */
    
}


