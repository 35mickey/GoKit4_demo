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
#include "timer.h"
#include "task.h"
#include "uart.h"

void gagentMain( void )
{
    userInit();

	/* 任务的信号队列初始化 */
	queue_init();

	/* 机智云相关的初始化，包含一个任务 */
    gizwitsInit();
	
	/* 此处初始化了一个心跳任务 */
	alive_task_init();

	/* 先开任务，最后开定时器 */
	all_timer_start();
}
