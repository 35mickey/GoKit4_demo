#include "bsp.h"
#include "stdlib.h"
#include "math.h"
#include "oled.h"
#include "Asc8X16E.h"
#include "GB2312_16X16.h"

/*******************************
OLED���Դ��Ÿ�ʽ����.
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
	 * ����: OLED_WR_Byte
	 * ����: ��SSD1106д��һ���ֽڡ�
	 * ����: dat:Ҫд�������/����
	 *		 cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
	 * ���: ��
	 */
	void OLED_WR_Byte(u8 dat, u8 cmd)
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
	 * ����: OLED_WR_Byte
	 * ����: ��SSD1106д��һ���ֽڡ�
	 * ����: dat:Ҫд�������/����
	 *		 cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
	 * ���: ��
	 */
	void OLED_WR_Byte(u8 dat, u8 cmd)
	{
		u8 i;
		if (cmd)
			OLED_DC_Set();
		else
			OLED_DC_Clr();
		OLED_CS_Clr();
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
		OLED_CS_Set();
		OLED_DC_Set();
	}
#endif

/*
 * ����: OLED_Set_Pos
 * ����: ������ʾ��ʼ��ַ
 * ����: x:ˮƽ�㣬0~127
 *		 y:��ֱ�㣬0~63
 * ���: ��
 */
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD);
}

/*
 * ����: oled_display_On
 * ����: ����OLED��ʾ 
 * ����: ��
 * ���: ��
 */
void oled_display_on(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X14, OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  //DISPLAY ON
}

/*
* ����: oled_display_Off
* ����: �ر�OLED��ʾ
* ����: ��
* ���: ��
*/    
void oled_display_off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X10, OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  //DISPLAY OFF
}

/*
* ����: oled_Clear
* ����: ��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!	 
* ����: ��
* ���: ��
*/     
void oled_clear(void)
{
	u8 i, n;
	for (i = 0; i<8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD);  //����ҳ��ַ��0~7��
		OLED_WR_Byte(0x02, OLED_CMD);      //������ʾλ�á��е͵�ַ
		OLED_WR_Byte(0x10, OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for (n = 0; n<128; n++)
			OLED_WR_Byte(0, OLED_DATA);
	} //������ʾ
}

/*
* ����: oled_init
* ����: ��ʼ��OLED
* ����: ��
* ���: ��
*/     
void oled_init(void)
{ 	
	OLED_RST_Set();
	OLED_DELAY(100);
	OLED_RST_Clr();
	OLED_DELAY(100);
	OLED_RST_Set(); 
					  
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0���·��� 0xc8����
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
 * ����: oled_display_128x64
 * ����: ��ʾһ��128x64��ͼƬ
 * ����: ��
 * ���: ��
 */
void oled_display_128x64(u8 *dp)
{
	u16 i,j;
	for(j=0;j<8;j++)
	{
		OLED_Set_Pos(0,j);
		for (i=0;i<128;i++)
		{
			OLED_WR_Byte(*dp,OLED_DATA); //д���ݵ�LCD,ÿд��һ��8 λ�����ݺ��е�ַ�Զ���1
			dp++;
		}
	}
}

/*
 * ����: oled_display_ASCII_8x16
 * ����: ��ʾ8x16 ASCII
 * ����: page:	��ֱҳ��0~7
 *		 colemn:ˮƽ�㣬0~127
 * ���: ��
 */
void oled_display_ASCII_8x16(u8 page,u8 column,u8 *s)
{
	u8 i,k;
	for(k = 0;k<strlen((char *)s);k++)
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
 * ����: oled_inplay_ASCII_8x16
 * ����: ��ɫ��ʾ8x16 ASCII
 * ����: page:	��ֱҳ��0~7
 *		 colemn:ˮƽ�㣬0~127
 * ���: ��
 */
void oled_inplay_ASCII_8x16(u8 page,u8 column,u8 *s)
{
	u8 i,k;
	for(k = 0;k<strlen((char *)s);k++)
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

/*
 * ����: oled_display_GB2312_16x16
 * ����: ��ʾ16x16 GB2312����
 * ����: row:	��ֱҳ��0~3
 *		 col:	ˮƽ�㣬0~127
 * ���: ��
 */
void oled_display_GB2312_16x16(u8 row,u8 col,u8 *s)
{
	static u16 i,j,k;
	row = row * 2;
	j = 0;
	/* ֻ�����ֺͿո� */
	for(;(*s) >= 128||(*s) == ' ';s+=2)
	{
		for (k = 0;k<GB2313_NUM;k++) 
		{
			if ((GB_16[k].Index[0]==*(s))&&(GB_16[k].Index[1]==*(s+1)))
			{
				OLED_Set_Pos(col+j*16,row);
				for (i=0;i<16;i++)
				{
					OLED_WR_Byte(GB_16[k].Msk[i],OLED_DATA);
				}
				OLED_Set_Pos(col+j*16,row+1);
				for (i=0;i<16;i++)
				{
					OLED_WR_Byte(GB_16[k].Msk[i+16],OLED_DATA);
				}
				break;/* �ҵ�������ѭ�� */
			}
		}
		
		/* �ֿ���û������� */
		if(k>=GB2313_NUM)
		{
			OLED_Set_Pos(col+j*16,row);
			for (i=0;i<16;i++)
			{
				OLED_WR_Byte(GB_16[SPACE_NUM].Msk[i],OLED_DATA);
			}
			OLED_Set_Pos(col+j*16,row+1);
			for (i=0;i<16;i++)
			{
				OLED_WR_Byte(GB_16[SPACE_NUM].Msk[i+16],OLED_DATA);
			}
		}
		
		j++;
	}
}

/*
 * ����: oled_inplay_GB2312_16x16
 * ����: ��ɫ��ʾ16x16 GB2312����
 * ����: row:	��ֱҳ��0~3
 *		 col:	ˮƽ�㣬0~127
 * ���: ��
 */
void oled_inplay_GB2312_16x16(u8 row,u8 col,u8 *s)
{
	static u16 i,j,k;
	row = row * 2;
	j = 0;
	/* ֻ�����ֺͿո� */
	for(;(*s) >= 128||(*s) == ' ';s+=2)
	{
		for (k = 0;k<GB2313_NUM;k++) 
		{
			if ((GB_16[k].Index[0]==*(s))&&(GB_16[k].Index[1]==*(s+1)))
			{
				OLED_Set_Pos(col+j*16,row);
				for (i=0;i<16;i++)
				{
					OLED_WR_Byte(~GB_16[k].Msk[i],OLED_DATA);
				}
				OLED_Set_Pos(col+j*16,row+1);
				for (i=0;i<16;i++)
				{
					OLED_WR_Byte(~GB_16[k].Msk[i+16],OLED_DATA);
				}
				break;/* �ҵ�������ѭ�� */
			}
		}
		
		/* �ֿ���û������� */
		if(k>=GB2313_NUM)
		{
			OLED_Set_Pos(col+j*16,row);
			for (i=0;i<16;i++)
			{
				OLED_WR_Byte(~GB_16[SPACE_NUM].Msk[i],OLED_DATA);
			}
			OLED_Set_Pos(col+j*16,row+1);
			for (i=0;i<16;i++)
			{
				OLED_WR_Byte(~GB_16[SPACE_NUM].Msk[i+16],OLED_DATA);
			}
		}
		
		j++;
	}
}



