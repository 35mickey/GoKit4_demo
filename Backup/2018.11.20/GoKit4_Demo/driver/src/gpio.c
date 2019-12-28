/******************************************************************************
*@file    gpio.c
*@brief   driver for gpio operation
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
#include "qapi_gpioint.h"
#include "gpio.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/

/*  !!! This Pin Enumeration Only Applicable BG96-OPEN Project !!!
 */
GPIO_MAP_TBL gpio_map_tbl[PIN_E_GPIO_MAX] = {
/* PIN NUM,     PIN NAME,    GPIO ID  GPIO FUNC */
	{  4, 		"GPIO01",  		23, 	 0},
	{  5, 		"GPIO02",  		20, 	 0},
	{  6, 		"GPIO03",  		21, 	 0},
	{  7, 		"GPIO04",  		22, 	 0},
	{ 18, 		"GPIO05",  		11, 	 0},
	{ 19, 		"GPIO06",  		10, 	 0},
	{ 22, 		"GPIO07",  		 9, 	 0},
	{ 23, 		"GPIO08",  	 	 8, 	 0},
	{ 26, 		"GPIO09",  		15, 	 0},
	{ 27, 		"GPIO10",  		12, 	 0},
	{ 28, 		"GPIO11",  		13, 	 0},
	{ 40, 		"GPIO19",  		19, 	 0},
	{ 41, 		"GPIO20",  		18, 	 0},
	{ 64, 		"GPIO21",  		07, 	 0},
};

/* gpio id table */
qapi_GPIO_ID_t gpio_id_tbl[PIN_E_GPIO_MAX];

/* gpio tlmm config table */
qapi_TLMM_Config_t tlmm_config[PIN_E_GPIO_MAX];

/* User handles for interrupt */
static qapi_Instance_Handle_t GPIO05_handle;

/* KEY1 state */
boolean key1_state = KEY_OFF;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/
void GPIO05_falling(qapi_GPIOINT_Callback_Data_t data);

/**************************************************************************
*                                 FUNCTION
***************************************************************************/

/* What the FUCK!!!That it was put in qapi_gpioint.h */
void qapi_custom_cb_gpioint_dispatcher(UINT cb_id, void *app_cb, 
                                       UINT cb_param1)
{
  qapi_GPIOINT_CB_t pfn_app_cb1;
  if(cb_id == TXM_QAPI_GPIOINT_REGISTER_INTERRUPT_CB) // custom cb-type1
  {
    pfn_app_cb1 = (qapi_GPIOINT_CB_t)app_cb;
    (pfn_app_cb1)((qapi_GPIOINT_Callback_Data_t)(cb_param1));
  }
}

/*
@func
  gpio_config
@brief
  [in]  m_pin
  		MODULE_PIN_ENUM type; the GPIO pin which customer want used for operation;
  [in]  gpio_dir
  		qapi_GPIO_Direction_t type; GPIO pin direction.
  [in]  gpio_pull
  		qapi_GPIO_Pull_t type; GPIO pin pull type.
  [in]  gpio_drive
  		qapi_GPIO_Drive_t type; GPIO pin drive strength. 
*/
static void gpio_config(MODULE_PIN_ENUM m_pin,
				 qapi_GPIO_Direction_t gpio_dir,
				 qapi_GPIO_Pull_t gpio_pull,
				 qapi_GPIO_Drive_t gpio_drive
				 )
{
	qapi_Status_t status = QAPI_OK;

	tlmm_config[m_pin].pin   = gpio_map_tbl[m_pin].gpio_id;
	tlmm_config[m_pin].func  = gpio_map_tbl[m_pin].gpio_func;
	tlmm_config[m_pin].dir   = gpio_dir;
	tlmm_config[m_pin].pull  = gpio_pull;
	tlmm_config[m_pin].drive = gpio_drive;

	// the default here
	status = qapi_TLMM_Get_Gpio_ID(&tlmm_config[m_pin], &gpio_id_tbl[m_pin]);
	gizLog(LOG_INFO,"[GPIO] gpio_id[%d] status = %d \n", gpio_map_tbl[m_pin].gpio_id, status);
	if (status == QAPI_OK)
	{
		status = qapi_TLMM_Config_Gpio(gpio_id_tbl[m_pin], &tlmm_config[m_pin]);
		gizLog(LOG_INFO,"[GPIO] gpio_id[%d] status = %d \n", gpio_map_tbl[m_pin].gpio_id, status);
		if (status != QAPI_OK)
		{
			gizLog(LOG_INFO,"[GPIO] gpio_config failed \n");
		}
	}
}

 /*
 @func
   gpio_int_config
 @brief
   [in]  m_pin
		 MODULE_PIN_ENUM type; the GPIO pin which customer want used for operation;
   [in]  GPIO_handle
		 qapi_Instance_Handle_t* type; GPIO interrupt handle.
   [in]  pfnCallback
		 qapi_GPIOINT_CB_t type; GPIO interrupt callback function.
   [in]  eTrigger
		 qapi_GPIOINT_Trigger_e type; GPIO trigger type falling/rising/alledge. 
 */
 static void gpio_int_config(MODULE_PIN_ENUM m_pin,
 				  qapi_Instance_Handle_t *GPIO_handle,
				  qapi_GPIOINT_CB_t pfnCallback,
				  qapi_GPIOINT_Trigger_e eTrigger
				  )
{
	qapi_Status_t status = QAPI_OK;
	qbool_t is_pending = 0;

	status = qapi_GPIOINT_Register_Interrupt(GPIO_handle,
											 gpio_map_tbl[m_pin].gpio_id,
											 pfnCallback,
											 NULL,
											 eTrigger,
											 QAPI_GPIOINT_PRIO_LOWEST_E,
											 false);
	gizLog(LOG_INFO,"[GPIO INT] status[%x] \n", status);

	status = qapi_GPIOINT_Is_Interrupt_Pending(GPIO_handle, gpio_map_tbl[m_pin].gpio_id, &is_pending);
	gizLog(LOG_INFO,"[GPIO INT] status[%x] is_pending[%x] \n", status, is_pending);
}


/*
@func
  gpio_init
@brief
  User API Function, init the GPIOs. 
*/
void gpio_init(void)
{
	qapi_Status_t status = QAPI_OK;

	/* For Motor */
	gpio_config(PIN_E_GPIO_09, QAPI_GPIO_OUTPUT_E, QAPI_GPIO_PULL_UP_E, QAPI_GPIO_10MA_E);
	GPIO_MOTOR_OFF;
	
	/* For KEY1 */
//	gpio_config(PIN_E_GPIO_05, QAPI_GPIO_INPUT_E, QAPI_GPIO_PULL_UP_E, QAPI_GPIO_2MA_E);
	gpio_int_config(PIN_E_GPIO_05,&GPIO05_handle,GPIO05_falling,QAPI_GPIOINT_TRIGGER_EDGE_FALLING_E);

	/* For PWM1 */
	gpio_config(PIN_E_GPIO_06, QAPI_GPIO_OUTPUT_E, QAPI_GPIO_PULL_DOWN_E, QAPI_GPIO_10MA_E);
}


void GPIO05_falling(qapi_GPIOINT_Callback_Data_t data)
{
	//could not print log in this callback
	key1_state = KEY_ON;
}


