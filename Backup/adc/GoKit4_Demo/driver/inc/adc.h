/******************************************************************************
*@file    adc.h
*@brief   driver of adc operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/
#ifndef __ADC_H__
#define __ADC_H__

#include "../main/log.h"

/*  Only for MACROs, don't use following varients!!
 * @{
 */

/**@} */

/*  User MACROs
 * @{
 */

/**@} */

/*  User API interface
 * @{
 */
void adc_init(void);
uint32_t adc0_update(void);
uint32_t adc1_update(void);
/**@} */

#endif /*__EXAMPLE_ADC_H__*/

