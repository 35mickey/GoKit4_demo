/******************************************************************************
*@file    adc.c
*@brief   driver for adc operation
*
*  ---------------------------------------------------------------------------
*
*  Copyright (c) 2018 Quectel Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Quectel Technologies, Inc.
*  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qapi_tlmm.h"
#include "qapi_diag.h"
#include "qapi_uart.h"
#include "qapi_adc.h"
#include "timer.h"
#include "gps.h"
#include "adc.h"
#include "../Gizwits/gizwits_protocol.h"

/**************************************************************************
*                                 GLOBAL
***************************************************************************/

/*ADC handle*/
qapi_ADC_Handle_t adc_handle;
qapi_Adc_Input_Properties_Type_t Properties_ADC0;
qapi_Adc_Input_Properties_Type_t Properties_ADC1;

/**************************************************************************
*                           FUNCTION DECLARATION
***************************************************************************/


/**************************************************************************
*                                 FUNCTION
***************************************************************************/
static qapi_Status_t adc_open_handle(void)
{
	qapi_Status_t status = QAPI_ERROR;
	status = qapi_ADC_Open(&adc_handle, 0);
	if (QAPI_OK != status) 
	{
		gizLog(LOG_INFO,"[ADC] Get ADC Handle ERROR!\r\n");
	}
	return status;
}


/*
@func
	adc_get_properties
@brief
	Get properties of the ADC 
*/
static qapi_Status_t adc_get_properties(const char *Channel_Name_Ptr,
				        qapi_Adc_Input_Properties_Type_t *Properties_Ptr
				        )
{
    qapi_Status_t status = QAPI_ERROR;
    uint32_t Channel_Name_Size = strlen(Channel_Name_Ptr) + 1;

    status = qapi_ADC_Get_Input_Properties(adc_handle, Channel_Name_Ptr, Channel_Name_Size, Properties_Ptr);
    if (QAPI_OK != status)
    {   
        gizLog(LOG_INFO,"[ADC] ADC Get Channel-%s Properties ERROR!\r\n", Channel_Name_Ptr);
    }
    return status;
}

/*
@func
  adc_init
@brief
  User API Function, init the adc. 
*/
void adc_init(void)
{
	qapi_Status_t status = QAPI_ERROR;

    const char *Channel_Name_ADC0 = ADC_INPUT_ADC0;
    const char *Channel_Name_ADC1 = ADC_INPUT_ADC1;

	/*get an adc handle*/
    status = adc_open_handle();
    if(status != QAPI_OK)
    {
        gizLog(LOG_INFO,"[ADC] Get ADC Handle ERROR!");
    }

    /*get the adc channel configuration*/
    status = adc_get_properties(Channel_Name_ADC0, &Properties_ADC0);
    if(status != QAPI_OK)
    {
        gizLog(LOG_INFO,"[ADC] Get ADC channel-%s Configuration ERROR!", Channel_Name_ADC0);
    }

    status = adc_get_properties(Channel_Name_ADC1, &Properties_ADC1);
    if(status != QAPI_OK)
    {
        gizLog(LOG_INFO,"[ADC] Get ADC channel-%s Configuration ERROR!", Channel_Name_ADC1);
    }
}

/*
@func
  adc0_update
@brief
  User API Function, do adc0 value get for once. Return the value.
*/
uint32_t adc0_update(void)
{
	qapi_ADC_Read_Result_t result;
	qapi_Status_t status = QAPI_ERROR;
	const char *Channel_Name_ADC0 = ADC_INPUT_ADC0;
	
	memset(&result, 0, sizeof(result));
    status = qapi_ADC_Read_Channel(adc_handle,  &Properties_ADC0, &result);
    if(QAPI_OK == status)
    {
        if(ADC_RESULT_VALID == result.eStatus)
        {
            return result.nMicrovolts/1000*3;	/* Compensate reducing voltage resistor. */
        }
		else
		{
			return 0;	/* Depend on the real enverionment */
		}
    }
    else
    {
    	return 0;		/* Depend on the real enverionment */
        gizLog(LOG_INFO,"[ADC] Read ADC %s ERROR!", Channel_Name_ADC0);
    }
}

/*
@func
  adc1_update
@brief
  User API Function, do adc1 value get for once. Return the value.
*/
uint32_t adc1_update(void)
{
	qapi_ADC_Read_Result_t result;
	qapi_Status_t status = QAPI_ERROR;
	const char *Channel_Name_ADC1 = ADC_INPUT_ADC1;
	
	memset(&result, 0, sizeof(result));
    status = qapi_ADC_Read_Channel(adc_handle,  &Properties_ADC1, &result);
    if(QAPI_OK == status)
    {
        if(ADC_RESULT_VALID == result.eStatus)
        {
            return result.nMicrovolts/1000*3;	/* Compensate reducing voltage resistor. */
        }
		else
		{
			return 0;	/* Depend on the real enverionment */
		}
    }
    else
    {
    	return 0;		/* Depend on the real enverionment */
        gizLog(LOG_INFO,"[ADC] Read ADC %s ERROR!", Channel_Name_ADC1);
    }
}


