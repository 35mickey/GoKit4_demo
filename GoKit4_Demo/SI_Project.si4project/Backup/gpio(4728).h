/******************************************************************************
*@file    gpio.h
*@brief   driver of gpio operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "../main/log.h"

/*  !!! This Pin Enumeration Only Applicable BG96-OPEN Project !!!
 */
typedef enum{
	PIN_E_GPIO_01=0,
	PIN_E_GPIO_02,
	PIN_E_GPIO_03,
	PIN_E_GPIO_04,
	PIN_E_GPIO_05,
	PIN_E_GPIO_06,
	PIN_E_GPIO_07,
	PIN_E_GPIO_08,
	PIN_E_GPIO_09,
	PIN_E_GPIO_10,
	PIN_E_GPIO_11,
	PIN_E_GPIO_19,
	PIN_E_GPIO_20,
	PIN_E_GPIO_21,
	PIN_E_GPIO_MAX
}MODULE_PIN_ENUM;

typedef struct{
	uint32_t pin_num;
	char     *pin_name;
	uint32_t gpio_id;
	uint32_t gpio_func;
}GPIO_MAP_TBL;

#define KEY_ON 	TRUE
#define KEY_OFF	FALSE

/*  Only for MACROs, don't use following varients!!
 * @{
 */
extern GPIO_MAP_TBL gpio_map_tbl[PIN_E_GPIO_MAX];
extern qapi_GPIO_ID_t gpio_id_tbl[PIN_E_GPIO_MAX];
extern qapi_TLMM_Config_t tlmm_config[PIN_E_GPIO_MAX];
/**@} */

extern boolean key1_state;

/*  User MACROs
 * @{
 */
#define GPIO_MOTOR_ON	qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_09], gpio_map_tbl[PIN_E_GPIO_09].gpio_id, QAPI_GPIO_LOW_VALUE_E)
#define GPIO_MOTOR_OFF	qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_09], gpio_map_tbl[PIN_E_GPIO_09].gpio_id, QAPI_GPIO_HIGH_VALUE_E)
#define GPIO_PWM1_HIGH	qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_06], gpio_map_tbl[PIN_E_GPIO_06].gpio_id, QAPI_GPIO_HIGH_VALUE_E)
#define GPIO_PWM1_LOW	qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_06], gpio_map_tbl[PIN_E_GPIO_06].gpio_id, QAPI_GPIO_LOW_VALUE_E)
#define GPIO_KEY1_TO(a)	qapi_TLMM_Read_Gpio(gpio_id_tbl[PIN_E_GPIO_05], gpio_map_tbl[PIN_E_GPIO_05].gpio_id, a)
/**@} */
	

/*  User API interface
 * @{
 */
void gpio_init(void);
/**@} */

#endif /*__EXAMPLE_GPIO_H__*/

