/*
************************************************************
* @file         gizwits_protocol.c
* @brief        Gizwits protocol related files (SDK API interface function definition)
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
#include "gizwits_protocol.h"
#include "gizwits_product.h"
#include "tx_api.h"
#include "task.h"

/** Protocol global variable **/
gizwitsProtocol_t gizwitsProtocol;

/**@name Gagent module related system task parameters
* @{
*/
/* Gizwits Thread related parameters */
#define GIZ_TASK_STACK_SIZE	(1024 * 16)
#define GIZ_TASK_PRIORITY	100
UCHAR gizTaskStack[GIZ_TASK_STACK_SIZE];
TX_THREAD *gizTaskHandle;

void ICACHE_FLASH_ATTR setConnectM2MStatus(uint8_t m2mFlag)
{
    gizwitsProtocol.connectM2MFlag = m2mFlag; 
    
    gizLog(LOG_INFO,"setConnectM2MStatus = %d ###$$$###\n", gizwitsProtocol.connectM2MFlag);
}

uint8_t ICACHE_FLASH_ATTR getConnectM2MStatus(void)
{
    gizLog(LOG_INFO,"getConnectM2MStatus = %d ###$$$###\n",gizwitsProtocol.connectM2MFlag);
    
    return gizwitsProtocol.connectM2MFlag; 
}

void ICACHE_FLASH_ATTR uploadDataCBFunc(int32 result, void *arg,uint8_t *pszDID)
{
    gizLog(LOG_INFO,"[uploadDataCBFunc]: result %d \n", result);
}

/**
* @brief Read system time millisecond count

* @param none
* @return System time milliseconds
*/
uint32_t ICACHE_FLASH_ATTR gizGetTimerCount(void)
{
    return gizwitsProtocol.timerMsCount;
}

/**
* @brief generates "controlled events" according to protocol

* @param [in] issuedData: Controlled data
* @param [out] info: event queue
* @param [out] dataPoints: data point data
* @return 0, the implementation of success, non-0, failed
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoint2Event(gizwitsIssued_t *issuedData, eventInfo_t *info, dataPoint_t *dataPoints)
{
    if((NULL == issuedData) || (NULL == info) ||(NULL == dataPoints))
    {
        gizLog(LOG_WARNING,"gizDataPoint2Event Error , Illegal Param\n");
        return -1;
    }
    
    /** Greater than 1 byte to do bit conversion **/
    if(sizeof(issuedData->attrFlags) > 1)
    {
        if(-1 == gizByteOrderExchange((uint8_t *)&issuedData->attrFlags,sizeof(attrFlags_t)))
        {
            gizLog(LOG_INFO,"gizByteOrderExchange Error\n");
            return -1;
        }
    }
    
    if(0x01 == issuedData->attrFlags.flagMotor)
    {
        info->event[info->num] = EVENT_Motor;
        info->num++;
        dataPoints->valueMotor = gizStandardDecompressionValue(Motor_BYTEOFFSET,Motor_BITOFFSET,Motor_LEN,(uint8_t *)&issuedData->attrVals.wBitBuf,sizeof(issuedData->attrVals.wBitBuf));
    }
        
        
    if(0x01 == issuedData->attrFlags.flagSensor2)
    {
        info->event[info->num] = EVENT_Sensor2;
        info->num++;
        dataPoints->valueSensor2 = gizX2Y(Sensor2_RATIO,  Sensor2_ADDITION, exchangeBytes(issuedData->attrVals.valueSensor2));
    }
    
    return 0;
}

/**
* @brief contrasts the current data with the last data
*
* @param [in] cur: current data point data
* @param [in] last: last data point data
*
* @return: 0, no change in data; 1, data changes
*/
static int8_t ICACHE_FLASH_ATTR gizCheckReport(dataPoint_t *cur, dataPoint_t *last)
{
    int8_t ret = 0;
    static uint32_t lastReportTime = 0;
    uint32_t currentTime = 0;

    if((NULL == cur) || (NULL == last))
    {
        gizLog(LOG_WARNING,"gizCheckReport Error , Illegal Param\n");
        return -1;
    }
    currentTime = gizGetTimerCount();
     if(last->valueMotor != cur->valueMotor)
    {
        gizLog(LOG_INFO,"valueMotor Changed\n");
        ret = 1;
    }
    if(last->valueSensor2 != cur->valueSensor2)
    {
        gizLog(LOG_INFO,"valueSensor2 Changed\n");
        ret = 1;
    }
    if(last->valueKey != cur->valueKey)
    {
        gizLog(LOG_INFO,"valueKey Changed\n");
        ret = 1;
    }

    if(last->valueSensor1 != cur->valueSensor1)
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            gizLog(LOG_INFO,"valueSensor1 Changed\n");
            ret = 1;
        }
    }
    if(0 != memcmp((uint8_t *)&last->valueStr1,(uint8_t *)&cur->valueStr1,sizeof(last->valueStr1)))
    {
        if(currentTime - lastReportTime >= REPORT_TIME_MAX)
        {
            gizLog(LOG_INFO,"valueStr1 Changed\n");
            ret = 1;
        }
    }

    if(1 == ret)
    {
        lastReportTime = gizGetTimerCount();
    }
    return ret;
}

/**
* @brief User data point data is converted to wit the cloud to report data point data
*
* @param [in] dataPoints: user data point data address
* @param [out] devStatusPtr: wit the cloud data point data address
*
* @return 0, the correct return; -1, the error returned
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoints2ReportData(dataPoint_t *dataPoints , devStatus_t *devStatusPtr)
{
    if((NULL == dataPoints) || (NULL == devStatusPtr))
    {
        gizLog(LOG_WARNING,"gizDataPoints2ReportData Error , Illegal Param\n");
        return -1;
    }

    gizMemset((uint8_t *)devStatusPtr->wBitBuf,0,sizeof(devStatusPtr->wBitBuf));
    gizMemset((uint8_t *)devStatusPtr->rBitBuf,0,sizeof(devStatusPtr->rBitBuf));

    gizStandardCompressValue(Motor_BYTEOFFSET,Motor_BITOFFSET,Motor_LEN,(uint8_t *)devStatusPtr,dataPoints->valueMotor);
    gizStandardCompressValue(Key_BYTEOFFSET,Key_BITOFFSET,Key_LEN,(uint8_t *)devStatusPtr,dataPoints->valueKey);
    gizByteOrderExchange((uint8_t *)devStatusPtr->wBitBuf,sizeof(devStatusPtr->wBitBuf));
    gizByteOrderExchange((uint8_t *)devStatusPtr->rBitBuf,sizeof(devStatusPtr->rBitBuf));


    devStatusPtr->valueSensor2 = exchangeBytes(gizY2X(Sensor2_RATIO,  Sensor2_ADDITION, dataPoints->valueSensor2)); 
    devStatusPtr->valueSensor1 = exchangeBytes(gizY2X(Sensor1_RATIO,  Sensor1_ADDITION, dataPoints->valueSensor1)); 



    gizMemcpy((uint8_t *)devStatusPtr->valueStr1,(uint8_t *)&dataPoints->valueStr1,sizeof(dataPoints->valueStr1));
    return 0;
}

/**
* @brief In this function is called by the Gagent module, when the connection status changes will be passed to the corresponding value
* @param [in] value WiFi connection status value
* @return none
*/
void ICACHE_FLASH_ATTR gizWiFiStatus(uint16_t value)
{
    uint8_t rssiValue = 0;
    wifi_status_t status;
    static wifi_status_t lastStatus;

    if(0 != value)
    {
        status.value = exchangeBytes(value); 

        gizLog(LOG_INFO,"@@@@ GAgentStatus[hex]:%02x | [Bin]:%d,%d,%d,%d,%d,%d \r\n", status.value, status.types.con_m2m, status.types.con_route, status.types.binding, status.types.onboarding, status.types.station, status.types.softap);

        //OnBoarding mode status
        if(1 == status.types.onboarding)
        {
            if(1 == status.types.softap)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"OnBoarding: SoftAP or Web mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_AIRLINK;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"OnBoarding: AirLink mode\r\n");
            }
        }
        else
        {
            if(1 == status.types.softap)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_SOFTAP;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"OnBoarding: SoftAP or Web mode\r\n");
            }

            if(1 == status.types.station)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_STATION;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"OnBoarding: Station mode\r\n");
            }
        }

        //binding mode status
        if(lastStatus.types.binding != status.types.binding)
        {
            lastStatus.types.binding = status.types.binding;
            if(1 == status.types.binding)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_OPEN_BINDING;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: in binding mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CLOSE_BINDING;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: out binding mode\r\n");
            }
        }

        //router status
        if(lastStatus.types.con_route != status.types.con_route)
        {
            lastStatus.types.con_route = status.types.con_route;
            if(1 == status.types.con_route)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_ROUTER;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: connected router\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_ROUTER;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: disconnected router\r\n");
            }
        }

        //M2M server status
        if(lastStatus.types.con_m2m != status.types.con_m2m)
        {
            lastStatus.types.con_m2m = status.types.con_m2m;
            if(1 == status.types.con_m2m)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_M2M;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: connected m2m\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_M2M;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: disconnected m2m\r\n");
            }
        }

        //APP status
        if(lastStatus.types.app != status.types.app)
        {
            lastStatus.types.app = status.types.app;
            if(1 == status.types.app)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CON_APP;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: app connect\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_DISCON_APP;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: no app connect\r\n");
            }
        }

        //test mode status
        if(lastStatus.types.test != status.types.test)
        {
            lastStatus.types.test = status.types.test;
            if(1 == status.types.test)
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_OPEN_TESTMODE;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: in test mode\r\n");
            }
            else
            {
                gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_CLOSE_TESTMODE;
                gizwitsProtocol.issuedProcessEvent.num++;
                gizLog(LOG_INFO,"WiFi status: out test mode\r\n");
            }
        }

        rssiValue = status.types.rssi;
        gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = WIFI_RSSI;
        gizwitsProtocol.issuedProcessEvent.num++;

        gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&rssiValue, sizeof(rssiValue));
        gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));

        lastStatus = status;
    }
}

/**
* @brief This function is called by the Gagent module to receive the relevant protocol data from the cloud or APP
* @param [in] inData The protocol data entered
* @param [in] inLen Enter the length of the data
* @param [out] outData The output of the protocol data
* @param [out] outLen The length of the output data
* @return 0, the implementation of success, non-0, failed
*/
int32_t ICACHE_FLASH_ATTR gizIssuedProcess(uint8_t *didPtr, uint8_t *inData, uint32_t inLen,uint8_t *outData,int32_t *outLen)
{
	uint8_t i = 0;


    if((NULL == inData) || (NULL == outData) || (NULL == outLen)) 
    {
        gizLog(LOG_WARNING,"!!! IssuedProcess Error \n"); 
        return -1;
    }

    if(NULL == didPtr)
    {
        gizLog(LOG_INFO,"~~~gizIssuedProcess: did is NULL .\n");
    }
    else
    {
        gizLog(LOG_INFO,"~~~gizIssuedProcess: did %s\n", didPtr);
    }

    gizLog(LOG_INFO,"%s: ", "~~~issued data"); 
    //printf_bufs((uint8_t *)inData,inLen);

    if(NULL == didPtr) 
    { 
        switch(inData[0]) 
        {
            case ACTION_CONTROL_DEVICE:
            	gizDataPoint2Event((gizwitsIssued_t *)&inData[1], &gizwitsProtocol.issuedProcessEvent,&gizwitsProtocol.gizCurrentDataPoint); 
                gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&gizwitsProtocol.gizCurrentDataPoint, sizeof(dataPoint_t));
                gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));          
         
                *outLen = 0; 
                break;
                
            case ACTION_READ_DEV_STATUS:
                gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint,&gizwitsProtocol.reportData.devStatus);
                gizwitsProtocol.reportData.action = ACTION_READ_DEV_STATUS_ACK;
                gizMemcpy(outData, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t));
                *outLen = sizeof(gizwitsReport_t);
                
                gizLog(LOG_INFO,"%s: ", "~~~ReadReport \n");
                //printf_bufs((uint8_t *)outData,*outLen);
                break;
                
            case ACTION_W2D_TRANSPARENT_DATA: 
                gizMemcpy(gizwitsProtocol.transparentBuff, &inData[1], inLen-1);
                gizwitsProtocol.transparentLen = inLen-1;
                
                gizwitsProtocol.issuedProcessEvent.event[0] = TRANSPARENT_DATA;
                gizwitsProtocol.issuedProcessEvent.num = 1;
                gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)gizwitsProtocol.transparentBuff, gizwitsProtocol.transparentLen);

                gizMemset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
                gizMemset((uint8_t *)gizwitsProtocol.transparentBuff, 0, BUFFER_LEN_MAX);
                gizwitsProtocol.transparentLen = 0;
                *outLen = 0;

                break;
                
            default:
 
                break;
        }
    }
    else
    { 
        gizLog(LOG_WARNING," Error : didPtr  \n");
    }

    return 0;
}

/*int8_t ICACHE_FLASH_ATTR gizTaskSend(int32_t tID, uint8_t * dPtr)
{
    int32_t iRet; 

    iRet = tx_event_flags_set(&gizTaskEvent, tID, TX_OR); 
    if(TX_SUCCESS == iRet)
    {
        return 0;
    }
    
    return -1; 
}*/

/**
* @brief Gizwits protocol related system event callback function
* @param [in] events Event data pointer
* @return none
* @note in the function to complete the Gizwits protocol related to the handling of system events
*/
void ICACHE_FLASH_ATTR gizwitsTask(ULONG para)
{
    uint32_t status = 0;
    TASK_COMM gizTaskComm;

    gizLog(LOG_INFO, "[gizwitsTask] Gizwits Task Start ...\n");

    while(1)
    {
        status = tx_queue_receive(tx_queue_handle, &gizTaskComm, TX_WAIT_FOREVER);
        if(TX_SUCCESS == status)
        {
            gizLog(LOG_INFO,"[gizwitsTask] Receive Message %d \n", gizTaskComm.message);
            switch (gizTaskComm.message)
            {
                case SIG_IMM_REPORT:
                    gizLog(LOG_INFO,"[SIG] :SIG_IMM_REPORT \n");
                    gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint, (devStatus_t *)&gizwitsProtocol.reportData.devStatus);
                    gizwitsProtocol.reportData.action = ACTION_REPORT_DEV_STATUS;
                    gagentUploadData(NULL, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t),getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
                    //printf_bufs((uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t)); 
                 break;
                 case SIG_UPGRADE_DATA:
                    gizLog(LOG_INFO,"[SIG] :SIG_UPGRADE_DATA\n"); 
                    gizwitsHandle((dataPoint_t *)&currentDataPoint);
                 break;
                 deafult:
                    gizLog(LOG_WARNING,"[SIG] NULL \n"); 
                 break;
            }
        }
    }
}

 

/**
* @brief gizwits data point update report processing

* The user calls the interface to complete the device data changes reported

* @param [in] dataPoint User device data points
* @return none
*/
int8_t ICACHE_FLASH_ATTR gizwitsHandle(dataPoint_t *dataPoint)
{
    if(NULL == dataPoint)
    {
        gizLog(LOG_WARNING,"!!! gizReportData Error \n");

        return (-1);
    }

    //Regularly report conditional
    if((1 == gizCheckReport(dataPoint, (dataPoint_t *)&gizwitsProtocol.gizLastDataPoint)))
    {
        gizDataPoints2ReportData(dataPoint, &gizwitsProtocol.reportData.devStatus);

        gizwitsProtocol.reportData.action = ACTION_REPORT_DEV_STATUS;

        gagentUploadData(NULL, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t),getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
 
        gizLog(LOG_INFO,"~~~reportData \n"); 
        //printf_bufs((uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t));

        gizMemcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)dataPoint, sizeof(dataPoint_t));
    }

    return 0;
}
/**@} */

/**
* @brief 放在UserHandle中，用来同步数据

* @param [in] dataPoint User device data points
* @return none
*/
int8_t ICACHE_FLASH_ATTR sensorUpdateHandle(dataPoint_t *dataPoint)
{
	if(NULL == dataPoint)
    {
        gizLog(LOG_WARNING,"!!! sensorUpdateHandle Error \n");

        return (-1);
    }

//    gizDataPoints2ReportData(dataPoint, &gizwitsProtocol.reportData.devStatus);
//
//    gizwitsProtocol.reportData.action = ACTION_REPORT_DEV_STATUS;
//
//    gagentUploadData(NULL, (uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t),getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
//
//    gizLog(LOG_INFO,"~~~reportData \n"); 
//    //printf_bufs((uint8_t *)&gizwitsProtocol.reportData, sizeof(gizwitsReport_t));
//
    gizMemcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)dataPoint, sizeof(dataPoint_t));

	gizSendQueue(SIG_IMM_REPORT); 
	
    return 0;
}


void gizSendQueue(uint8_t message)
{
    TASK_COMM taskComm;
    UINT status = 0;
    taskComm.message = message;
    status = tx_queue_send(tx_queue_handle, &taskComm, TX_WAIT_FOREVER);
    if (TX_SUCCESS != status)
    {
        gizLog(LOG_WARNING,"[gizSendQueue] Send Message Failed [%d]\n",status);
    }   
}
/**
* @brief Timer callback function, in the function to complete 10 minutes of regular reporting
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR gizTimerFunc(void)
{
    //100+6ms 
    static uint32_t repCtime = 0;
    gizLog(LOG_DUMP,"**--**\n");

    repCtime++;
    if(0 == repCtime%10)
    {
        gizwitsProtocol.timerMsCount++;
        gizLog(LOG_INFO,"[gizTimerFunc] One Second \n", gizwitsProtocol.timerMsCount);
        if(0 == gizwitsProtocol.timerMsCount%600)
        {
            gizLog(LOG_INFO,"[gizTimerFunc] 10 Minute Report ...\n");
            gizSendQueue(SIG_IMM_REPORT); 
        }
    }

	userHandle();	/* 更新传感器数据，看来是100ms执行一次 */
}

/**@name User API interface
* @{
*/

/**
* @brief WiFi configuration interface

*  The user can call the interface to bring the WiFi module into the corresponding configuration mode or reset the module

* @param[in] mode: 0x0, module reset; 0x01, SoftAp mode; 0x02, AirLink mode; 0x03, module into production test; 0x04: allow users to bind equipment
* @return Error command code
*/
void ICACHE_FLASH_ATTR gizwitsSetMode(uint8_t mode)
{
    switch(mode)
    {
        case WIFI_RESET_MODE:
            gagentReset();
            break;
        case WIFI_SOFTAP_MODE:
        case WIFI_AIRLINK_MODE:
        case WIFI_PRODUCTION_TEST:
            gagentConfig(mode);
            break;
        case WIFI_NINABLE_MODE:
            GAgentEnableBind();
            break;
        case WIFI_REBOOT_MODE:
            //system_restart();
            break;
        default :
            break;
    }
}

/**
* @brief Obtain a network timestamp interface
* @param none
* @return _tm
*/
protocolTime_t ICACHE_FLASH_ATTR gizwitsGetNTPTime(void)
{
    protocolTime_t tmValue;

    gagentGetNTP((_tm *)&tmValue);

    return tmValue;
}

/**
* @brief Obtain a network timestamp interface
* @param none
* @return ntp
*/
uint32_t ICACHE_FLASH_ATTR gizwitsGetTimeStamp(void)
{
    _tm tmValue;

    gagentGetNTP(&tmValue);

    return tmValue.ntp;
}

/**
* @brief gizwits protocol initialization interface

* The user can call the interface to complete the Gizwits protocol-related initialization (including protocol-related timer, serial port initial)

* The user can complete the initialization status of the data points in this interface

* @param none
* @return none
*/
void ICACHE_FLASH_ATTR gizwitsInit(void)
{
    int32_t ret = 0;
//gizwits init
    gizMemset((uint8_t *)&gizwitsProtocol, 0, sizeof(gizwitsProtocol_t)); 

    gizLog(LOG_INFO,"[gizwitsInit] Init Begin \n");

//Task Creat
    if(TX_SUCCESS != txm_module_object_allocate((VOID *)&gizTaskHandle, sizeof(TX_THREAD)))
    {
        gizLog(LOG_WARNING,"[gizwitsInit] gizTaskHandle malloc failed \n");
        return;
    }
    
    ret = tx_thread_create(gizTaskHandle,
                            "gizTaskThread",
                            gizwitsTask,
                            100,
                            gizTaskStack,
                            GIZ_TASK_STACK_SIZE,
                            GIZ_TASK_PRIORITY,
                            GIZ_TASK_PRIORITY,
                            TX_NO_TIME_SLICE,
                            TX_AUTO_START
                            );
    if(ret != TX_SUCCESS)
    {
        gizLog(LOG_INFO,"[gizwitsInit] Thread gizwitsTask Create Failed [%d]\n",ret);
        return;
    }
    else
    {
        gizLog(LOG_INFO,"[gizwitsInit] Thread gizwitsTask Create Success [%d]\n",ret);        
    }

//gagent init
    struct devAttrs attrs;
    unsigned short time=0;
    gizMemset(&attrs,0,sizeof(attrs));

    attrs.mDevAttr[7] |= DEV_IS_GATEWAY<<0;
    attrs.mDevAttr[7] |= (0x01<<1);
    gizMemcpy(attrs.mstrDevHV, HARDWARE_VERSION, gizStrlen(HARDWARE_VERSION));
    gizMemcpy(attrs.mstrDevSV, SOFTWARE_VERSION, gizStrlen(SOFTWARE_VERSION));
    gizMemcpy(attrs.mstrP0Ver, P0_VERSION, gizStrlen(P0_VERSION));
    gizMemcpy(attrs.mstrProductKey, PRODUCT_KEY, gizStrlen(PRODUCT_KEY));
    gizMemcpy(attrs.mstrPKSecret, PRODUCT_SECRET, gizStrlen(PRODUCT_SECRET));
    gizMemcpy(attrs.mstrProtocolVer, PROTOCOL_VERSION, gizStrlen(PROTOCOL_VERSION));
    gizMemcpy(attrs.mstrSdkVerLow, SDK_VERSION, gizStrlen(SDK_VERSION));   

    /********************************* GAgent deafult val *********************************/
    attrs.szWechatDeviceType = (uint8 *)"gh_35dd1e10ab57";
    attrs.szGAgentSever      = (uint8 *)"api.gizwits.com";
    attrs.gagentSeverPort    = (uint8 *)"80";
    attrs.szGAgentSoftApName = (uint8 *)"XPG-GAgent-";
    attrs.szGAgentSoftApPwd  = (uint8 *)"123456789";
    attrs.m2mKeepAliveS      = (uint8 *)"150";
    attrs.localHeartbeatIntervalS = (uint8 *)"40";
    attrs.localTransferIntervalMS = (uint8 *)"450";
    attrs.networkCardName = (uint8 *)"ens33";
    attrs.configMode = (uint8 *)"2";
    /***************************************************************************************/
    gagentInit(attrs);

    gizLog(LOG_INFO,"gizwitsInit OK \r\n");
}

/**
* @brief gizwits report transparent transmission data interface

* The user can call the interface to complete the reporting of private protocol data

* @param [in] data entered private protocol data
* @param [in] len Enter the length of the private protocol data
* @return -1, error returned; 0, returned correctly
*/
int32_t ICACHE_FLASH_ATTR gizwitsPassthroughData(uint8_t * data, uint32_t len)
{
    uint8_t *passReportBuf = NULL;
    uint32_t passReportBufLen = len + sizeof(uint8_t);

    if(NULL == data)
    {
        gizLog(LOG_INFO,"!!! gizReportData Error \n");

        return (-1);
    }

    passReportBuf = (uint8 * )gizMalloc(passReportBufLen);
    if(NULL == passReportBuf)
    {
        gizLog(LOG_INFO,"Failed to malloc buffer\n");
        return (-1);
    }

    passReportBuf[0] = ACTION_D2W_TRANSPARENT_DATA;
    gizMemcpy((uint8_t *)&passReportBuf[1], data, len);

    gagentUploadData(NULL,passReportBuf, passReportBufLen,getConnectM2MStatus(),gizwitsProtocol.mac, uploadDataCBFunc);
 

    gizFree(passReportBuf);
    passReportBuf = NULL;

    return 0;
}