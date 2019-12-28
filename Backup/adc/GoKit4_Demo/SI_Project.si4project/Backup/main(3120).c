/**
************************************************************
* @file         main.c
* @brief        main
* @author       Gizwits
* @date         2018-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/
#include "gagent_soc.h"
#include "../Gizwits/gizwits_product.h"
#include "log.h"


#define QUERY_INTERVAL 5 //S


TX_TIMER *userTimer;

void ICACHE_FLASH_ATTR userTimerCB(void)
{
    static uint8_t ctime = 0;
    static uint8_t ccount = 0;

    ctime++;  
}

void sensorInit(void)
{
    int32 ret = -1;
    
    gizLog(LOG_INFO,"Sensor initialization ...\n"); 

    txm_module_object_allocate(&userTimer, sizeof(TX_TIMER));
    ret = tx_timer_create(userTimer, "userTimer", userTimerCB, NULL, 1,
                          100, TX_AUTO_ACTIVATE);
    if(ret != TX_SUCCESS)
    {
        gizLog(LOG_WARNING,"Failed to create UserTimer.\n");
    }
}


void gagentMain( void )
{
    getFreeHeap();
    sensorInit();
    gizwitsInit();
}
