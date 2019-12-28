#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#include "math.h"
#include "oled.h"
#include "timer.h"
#include "Asc8X16E.h"

/*******************************
OLED的显存存放格式如下.
[0]0 1 2 3 ... 127	
[1]0 1 2 3 ... 127	
[2]0 1 2 3 ... 127	
[3]0 1 2 3 ... 127	
[4]0 1 2 3 ... 127	
[5]0 1 2 3 ... 127	
[6]0 1 2 3 ... 127	
[7]0 1 2 3 ... 127 
********************************/		

//#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF
#define X_WIDTH 	128
#define Y_WIDTH 	64

#if OLED_MODE==1
	/*
	 * 名称: OLED_WR_Byte
	 * 功能: 向SSD1106写入一个字节。
	 * 输入: dat:要写入的数据/命令
	 *		 cmd:数据/命令标志 0,表示命令;1,表示数据;
	 * 输出: 无
	 */
	void OLED_WR_Byte(uint8 dat, uint8 cmd)
	{
		DATAOUT(dat);
		if (cmd)
			OLED_DC_Set();
		else
			OLED_DC_Clr();
		OLED_CS_Clr();
		OLED_WR_Clr();
		OLED_WR_Set();
		OLED_CS_Set();
		OLED_DC_Set();
	}
#else
	/*
	 * 名称: OLED_WR_Byte
	 * 功能: 向SSD1106写入一个字节。
	 * 输入: dat:要写入的数据/命令
	 *		 cmd:数据/命令标志 0,表示命令;1,表示数据;
	 * 输出: 无
	 */
	void OLED_WR_Byte(uint8 dat, uint8 cmd)
	{
		uint8 i;
		if (cmd)
			OLED_DC_Set();
		else
			OLED_DC_Clr();
//		OLED_CS_Clr();
		for (i = 0; i<8; i++)
		{
			OLED_SCLK_Clr();
			if (dat & 0x80)
				OLED_SDIN_Set();
			else
				OLED_SDIN_Clr();
			OLED_SCLK_Set();
			dat <<= 1;
		}
//		OLED_CS_Set();
		OLED_DC_Set();
	}
#endif

/*
 * 名称: OLED_Set_Pos
 * 功能: 设置显示起始地址
 * 输入: x:水平点，0~127
 *		 y:垂直点，0~63
 * 输出: 无
 */
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}

/*
 * 名称: oled_display_On
 * 功能: 开启OLED显示 
 * 输入: 无
 * 输出: 无
 */
void oled_display_on(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

/*
* 名称: oled_display_Off
* 功能: 关闭OLED显示
* 输入: 无
* 输出: 无
*/    
void oled_display_off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}

/*
* 名称: oled_Clear
* 功能: 清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	 
* 输入: 无
* 输出: 无
*/     
void oled_clear(void)
{
	uint8 i, n;
	for (i = 0; i<8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD);  //设置页地址（0~7）
		OLED_WR_Byte(0x02, OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte(0x10, OLED_CMD);      //设置显示位置—列高地址   
		for (n = 0; n<128; n++)
			OLED_WR_Byte(0, OLED_DATA);
	} //更新显示
}

/*
* 名称: oled_init
* 功能: 初始化OLED
* 输入: 无
* 输出: 无
*/     
void oled_init(void)
{ 	
	OLED_RST_Set();
	qapi_Timer_Sleep(100, QAPI_TIMER_UNIT_MSEC, true);
	OLED_RST_Clr();
	qapi_Timer_Sleep(100, QAPI_TIMER_UNIT_MSEC, true);
	OLED_RST_Set(); 
					  
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel

	OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/ 
	oled_clear();
	OLED_Set_Pos(0,0); 	
} 

/*
 * 名称: oled_display_128x64
 * 功能: 显示一张128x64的图片
 * 输入: 无
 * 输出: 无
 */
void oled_display_128x64(uint8 *dp)
{
	uint16 i,j;
	for(j=0;j<8;j++)
	{
		OLED_Set_Pos(0,j);
		for (i=0;i<128;i++)
		{
			OLED_WR_Byte(*dp,OLED_DATA); //写数据到LCD,每写完一个8 位的数据后列地址自动加1
			dp++;
		}
	}
}

/*
 * 名称: oled_display_ASCII_8x16
 * 功能: 显示8x16 ASCII
 * 输入: page:	垂直页，0~7
 *		 colemn:水平点，0~127
 * 输出: 无
 */
void oled_display_ASCII_8x16(uint8 page,uint8 column,uint8 *s)
{
	uint8 i,k;
	for(k = 0;k<(strlen((char *)s)>16?16:strlen((char *)s));k++)
	{
		OLED_Set_Pos(column+k*8,page);
		for (i=0;i<8;i++)
		{
			OLED_WR_Byte(Zk_ASCII8X16[(*(s+k))*16+i],OLED_DATA);
		}
		OLED_Set_Pos(column+k*8,page+1);
		for (i=0;i<8;i++)
		{
			OLED_WR_Byte(Zk_ASCII8X16[(*(s+k))*16+i+8],OLED_DATA); 
		}
	}
}

/*
 * 名称: oled_inplay_ASCII_8x16
 * 功能: 反色显示8x16 ASCII
 * 输入: page:	垂直页，0~7
 *		 colemn:水平点，0~127
 * 输出: 无
 */
void oled_inplay_ASCII_8x16(uint8 page,uint8 column,uint8 *s)
{
	uint8 i,k;
	for(k = 0;k<(strlen((char *)s)>16?16:strlen((char *)s));k++)
	{
		OLED_Set_Pos(column+k*8,page);
		for (i=0;i<8;i++)
		{
			OLED_WR_Byte(~Zk_ASCII8X16[(*(s+k))*16+i],OLED_DATA);
		}
		OLED_Set_Pos(column+k*8,page+1);
		for (i=0;i<8;i++)
		{
			OLED_WR_Byte(~Zk_ASCII8X16[(*(s+k))*16+i+8],OLED_DATA); 
		}
	}
}
//
///*
// * 名称: oled_display_GB2312_16x16
// * 功能: 显示16x16 GB2312汉字
// * 输入: row:	垂直页，0~3
// *		 col:	水平点，0~127
// * 输出: 无
// */
//void oled_display_GB2312_16x16(uint8 row,uint8 col,uint8 *s)
//{
//	static uint16 i,j,k;
//	row = row * 2;
//	j = 0;
//	/* 只处理汉字和空格 */
//	for(;(*s) >= 128||(*s) == ' ';s+=2)
//	{
//		for (k = 0;k<GB2313_NUM;k++) 
//		{
//			if ((GB_16[k].Index[0]==*(s))&&(GB_16[k].Index[1]==*(s+1)))
//			{
//				OLED_Set_Pos(col+j*16,row);
//				for (i=0;i<16;i++)
//				{
//					OLED_WR_Byte(GB_16[k].Msk[i],OLED_DATA);
//				}
//				OLED_Set_Pos(col+j*16,row+1);
//				for (i=0;i<16;i++)
//				{
//					OLED_WR_Byte(GB_16[k].Msk[i+16],OLED_DATA);
//				}
//				break;/* 找到后，跳出循环 */
//			}
//		}
//		
//		/* 字库中没有这个字 */
//		if(k>=GB2313_NUM)
//		{
//			OLED_Set_Pos(col+j*16,row);
//			for (i=0;i<16;i++)
//			{
//				OLED_WR_Byte(GB_16[SPACE_NUM].Msk[i],OLED_DATA);
//			}
//			OLED_Set_Pos(col+j*16,row+1);
//			for (i=0;i<16;i++)
//			{
//				OLED_WR_Byte(GB_16[SPACE_NUM].Msk[i+16],OLED_DATA);
//			}
//		}
//		
//		j++;
//	}
//}
//
///*
// * 名称: oled_inplay_GB2312_16x16
// * 功能: 反色显示16x16 GB2312汉字
// * 输入: row:	垂直页，0~3
// *		 col:	水平点，0~127
// * 输出: 无
// */
//void oled_inplay_GB2312_16x16(uint8 row,uint8 col,uint8 *s)
//{
//	static uint16 i,j,k;
//	row = row * 2;
//	j = 0;
//	/* 只处理汉字和空格 */
//	for(;(*s) >= 128||(*s) == ' ';s+=2)
//	{
//		for (k = 0;k<GB2313_NUM;k++) 
//		{
//			if ((GB_16[k].Index[0]==*(s))&&(GB_16[k].Index[1]==*(s+1)))
//			{
//				OLED_Set_Pos(col+j*16,row);
//				for (i=0;i<16;i++)
//				{
//					OLED_WR_Byte(~GB_16[k].Msk[i],OLED_DATA);
//				}
//				OLED_Set_Pos(col+j*16,row+1);
//				for (i=0;i<16;i++)
//				{
//					OLED_WR_Byte(~GB_16[k].Msk[i+16],OLED_DATA);
//				}
//				break;/* 找到后，跳出循环 */
//			}
//		}
//		
//		/* 字库中没有这个字 */
//		if(k>=GB2313_NUM)
//		{
//			OLED_Set_Pos(col+j*16,row);
//			for (i=0;i<16;i++)
//			{
//				OLED_WR_Byte(~GB_16[SPACE_NUM].Msk[i],OLED_DATA);
//			}
//			OLED_Set_Pos(col+j*16,row+1);
//			for (i=0;i<16;i++)
//			{
//				OLED_WR_Byte(~GB_16[SPACE_NUM].Msk[i+16],OLED_DATA);
//			}
//		}
//		
//		j++;
//	}
//}

/* 
 * 名称: lcd_printf_display
 * 功能: 以格式化输出的方式显示字符串 row~(0,7) col~(0,127)
 * 输入: 行号，列号,格式化字符串
 * 输出: 无
 */
int lcd_printf_display(uint8 row,uint8 col,const char *format ,...)
{
	va_list arg;
	int rv;
	uint8 temp[32];

	va_start(arg, format);
	rv = vsprintf((char *)temp, format, arg);
	va_end(arg);
	
	oled_display_ASCII_8x16(row,col,(uint8 *)temp);
	
	return rv;
}

/* 
 * 名称: lcd_printf_inplay
 * 功能: 以格式化输出的方式反色显示字符串 row~(0,7) col~(0,127)
 * 输入: 行号，列号,格式化字符串
 * 输出: 无
 */
int lcd_printf_inplay(uint8 row,uint8 col,const char *format ,...)
{
	va_list arg;
	int rv;
	uint8 temp[32];

	va_start(arg, format);
	rv = vsprintf((char *)temp, format, arg);
	va_end(arg);
	
	oled_inplay_ASCII_8x16(row,col,(uint8 *)temp);
	
	return rv;
}





