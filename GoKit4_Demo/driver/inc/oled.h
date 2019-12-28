/* 
 * File:		oled.h
 * Author:		Mickey
 * Function:	OLED屏幕驱动库
 * Date:		2018-5-10
 */

#ifndef __OLED_H
#define	__OLED_H

#include "gpio.h"

/* OLED模式设置	 */
/* 0:4线串行模式 */
/* 1:并行8080模式*/
#define OLED_MODE 0
	    						  
/* -----------------OLED端口定义，详见bsp.h---------------- */  					   
//#define OLED_CS_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_4)	/* CS */
//#define OLED_CS_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_4)

#define OLED_RST_Clr() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_02], gpio_map_tbl[PIN_E_GPIO_02].gpio_id, QAPI_GPIO_LOW_VALUE_E) /* RES */
#define OLED_RST_Set() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_02], gpio_map_tbl[PIN_E_GPIO_02].gpio_id, QAPI_GPIO_HIGH_VALUE_E)

#define OLED_DC_Clr() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_04], gpio_map_tbl[PIN_E_GPIO_04].gpio_id, QAPI_GPIO_LOW_VALUE_E)	/* DC */
#define OLED_DC_Set() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_04], gpio_map_tbl[PIN_E_GPIO_04].gpio_id, QAPI_GPIO_HIGH_VALUE_E)

#if OLED_MODE==1
//	#define OLED_WR_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_14)
//	#define OLED_WR_Set() GPIO_SetBits(GPIOG,GPIO_Pin_14)
//
//	#define OLED_RD_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_13)
//	#define OLED_RD_Set() GPIO_SetBits(GPIOG,GPIO_Pin_13)
//
//	//PC0~7,作为数据线
//	#define DATAOUT(x) GPIO_Write(GPIOC,x);//输出  
#else
	#define OLED_SCLK_Clr() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_01], gpio_map_tbl[PIN_E_GPIO_01].gpio_id, QAPI_GPIO_LOW_VALUE_E)	/* CLK */
	#define OLED_SCLK_Set() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_01], gpio_map_tbl[PIN_E_GPIO_01].gpio_id, QAPI_GPIO_HIGH_VALUE_E)

	#define OLED_SDIN_Clr() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_03], gpio_map_tbl[PIN_E_GPIO_03].gpio_id, QAPI_GPIO_LOW_VALUE_E)	/* DIN */
	#define OLED_SDIN_Set() qapi_TLMM_Drive_Gpio(gpio_id_tbl[PIN_E_GPIO_03], gpio_map_tbl[PIN_E_GPIO_03].gpio_id, QAPI_GPIO_HIGH_VALUE_E)
#endif

#define OLED_CMD  0	/* 写命令 */
#define OLED_DATA 1	/* 写数据 */

/* OLED控制用函数 */
void OLED_WR_Byte(uint8 dat, uint8 cmd);
void oled_display_on(void);
void oled_display_off(void);
void oled_init(void);
void oled_clear(void);
//void OLED_DrawPoint(uint8 x, uint8 y, uint8 t);
//void OLED_Fill(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 dot);
//void OLED_ShowChar(uint8 x, uint8 y, uint8 chr);
//void OLED_ShowNum(uint8 x, uint8 y, u32 num, uint8 len, uint8 size);
//void OLED_ShowString(uint8 x, uint8 y, uint8 *p);
//void OLED_Set_Pos(unsigned char x, unsigned char y);
//void OLED_ShowCHinese(uint8 x, uint8 y, uint8 no);
//void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);

void oled_display_128x64(uint8 *dp);
void oled_display_ASCII_8x16(uint8 page,uint8 column,uint8 *s);
void oled_inplay_ASCII_8x16(uint8 page,uint8 column,uint8 *s);
void oled_display_GB2312_16x16(uint8 row,uint8 col,uint8 *s);
void oled_inplay_GB2312_16x16(uint8 row,uint8 col,uint8 *s);
int lcd_printf_display(uint8 row,uint8 col,const char *format ,...);
int lcd_printf_inplay(uint8 row,uint8 col,const char *format ,...);

#endif	/* __OLED_H */

