/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */    
#include "gpio.h"
#include "usart.h"
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
signed char speed_value = 0;
signed char dir_value = 0;
unsigned char pwm_en = 0;
/* USER CODE END Variables */
osThreadId SYS_Normal_TaskHandle;
osThreadId UART_Parse_TaskHandle;
osThreadId Display_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void LED_Task(void const * argument);
void PWM_Task(void const * argument);
void LCD_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of SYS_Normal_Task */
  osThreadDef(SYS_Normal_Task, LED_Task, osPriorityHigh, 0, 128);
  SYS_Normal_TaskHandle = osThreadCreate(osThread(SYS_Normal_Task), NULL);

  /* definition and creation of UART_Parse_Task */
  osThreadDef(UART_Parse_Task, PWM_Task, osPriorityAboveNormal, 0, 1024);
  UART_Parse_TaskHandle = osThreadCreate(osThread(UART_Parse_Task), NULL);

  /* definition and creation of Display_Task */
  osThreadDef(Display_Task, LCD_Task, osPriorityBelowNormal, 0, 512);
  Display_TaskHandle = osThreadCreate(osThread(Display_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_LED_Task */
/**
  * @brief  Function implementing the SYS_Normal_Task thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_LED_Task */
void LED_Task(void const * argument)
{

  /* USER CODE BEGIN LED_Task */
  /* Infinite loop */
  for(;;)
  {
	  SYS_OK;
    osDelay(500);
  }
  /* USER CODE END LED_Task */
}

/* USER CODE BEGIN Header_PWM_Task */
/**
* @brief Function implementing the UART_Parse_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_PWM_Task */
void PWM_Task(void const * argument)
{
  /* USER CODE BEGIN PWM_Task */
	uint16_t pwm1_value=0;
	uint16_t pwm2_value=0;
  /* Infinite loop */
  for(;;)
  {
	  if(usart1_rx_flag == 1)
	  {
		  //usart1_printf("I got speed_value:%d dir_value:%d \n",speed_value,dir_value);
		  /* 900 - 2100us */
		  if(pwm_en == 1)
		  {
			  pwm1_value = (speed_value+speed_value*dir_value/100)*6+899;	/* (0~200)*600/100 */
			  pwm2_value = (speed_value-speed_value*dir_value/100)*6+899;
			  
//			  pwm1_value = (speed_value>0?speed_value:(-1*speed_value))*1200/100 + 900;
			  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,pwm1_value);
			  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,pwm2_value);
		  }
		  else
		  {
			  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,900);
			  __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2,900);
		  }
		  usart1_rx_flag = 0;
	  }
    osDelay(5);
  }
  /* USER CODE END PWM_Task */
}

/* USER CODE BEGIN Header_LCD_Task */
/**
* @brief Function implementing the Display_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LCD_Task */
void LCD_Task(void const * argument)
{
  /* USER CODE BEGIN LCD_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END LCD_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
