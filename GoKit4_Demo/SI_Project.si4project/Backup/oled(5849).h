/* 
 * File:		oled.h
 * Author:		Administrator
 * Function:	OLED��Ļ������
 * Date:		2018-5-10
 */

#ifndef __OLED_H
#define	__OLED_H

#include "bsp.h"

/* OLEDģʽ����	 */
/* 0:4�ߴ���ģʽ */
/* 1:����8080ģʽ*/
#define OLED_MODE 0
	    						  
/* -----------------OLED�˿ڶ��壬���bsp.h---------------- */  					   
//#define OLED_CS_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_4)	/* CS */
//#define OLED_CS_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_4)

//#define OLED_RST_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_0) /* RES */
//#define OLED_RST_Set() GPIO_SetBits(GPIOB,GPIO_Pin_0)

//#define OLED_DC_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_1)	/* DC */
//#define OLED_DC_Set() GPIO_SetBits(GPIOB,GPIO_Pin_1)

//#if OLED_MODE==1
//	#define OLED_WR_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_14)
//	#define OLED_WR_Set() GPIO_SetBits(GPIOG,GPIO_Pin_14)

//	#define OLED_RD_Clr() GPIO_ResetBits(GPIOG,GPIO_Pin_13)
//	#define OLED_RD_Set() GPIO_SetBits(GPIOG,GPIO_Pin_13)

//	//PC0~7,��Ϊ������
//	#define DATAOUT(x) GPIO_Write(GPIOC,x);//���  
//#else
//	#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_5)	/* CLK */
//	#define OLED_SCLK_Set() GPIO_SetBits(GPIOA,GPIO_Pin_5)

//	#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_7)	/* DIN */
//	#define OLED_SDIN_Set() GPIO_SetBits(GPIOA,GPIO_Pin_7)
//#endif

#define OLED_CMD  0	/* д���� */
#define OLED_DATA 1	/* д���� */

/* OLED�����ú��� */
void OLED_WR_Byte(u8 dat, u8 cmd);
void oled_display_on(void);
void oled_display_off(void);
void oled_init(void);
void oled_clear(void);
//void OLED_DrawPoint(u8 x, u8 y, u8 t);
//void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot);
//void OLED_ShowChar(u8 x, u8 y, u8 chr);
//void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size);
//void OLED_ShowString(u8 x, u8 y, u8 *p);
//void OLED_Set_Pos(unsigned char x, unsigned char y);
//void OLED_ShowCHinese(u8 x, u8 y, u8 no);
//void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);

void oled_display_128x64(u8 *dp);
void oled_display_ASCII_8x16(u8 page,u8 column,u8 *s);
void oled_inplay_ASCII_8x16(u8 page,u8 column,u8 *s);
void oled_display_GB2312_16x16(u8 row,u8 col,u8 *s);
void oled_inplay_GB2312_16x16(u8 row,u8 col,u8 *s);

#endif	/* __OLED_H */

