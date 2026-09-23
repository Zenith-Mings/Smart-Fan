#include <REGX52.H>
#include "LCD1602.h"
#include "DS18B20.h"
#include "Delay.h"
#include "AT24C02.h"
#include "Key.h"
#include "Timer0.h"

float T,TShow;
char TLow,THigh;
unsigned char KeyNum;
sbit Motor=P1^3;

unsigned char Counter,Compare;	//计数值和比较值，用于输出PWM
unsigned char Speed;

void main()
{
  //AT24C02_WriteByte(0,0xFF);
	//AT24C02_WriteByte(1,0xFF);
	Motor=0;//默认不转
	DS18B20_ConvertT();		//上电先转换一次温度，防止第一次读数据错误
	Delay(1000);			//等待转换完成
	THigh=AT24C02_ReadByte(0);	//读取温度阈值数据
	Speed=AT24C02_ReadByte(1);
	if(THigh>125 || Speed>3 || Speed<1||THigh<12)
	{
		THigh=20;			//如果阈值非法，则设为默认值
		Speed=1;
	}
	LCD_Init();
	LCD_ShowString(1,1,"T:");
	LCD_ShowString(2,1,"TH:");
	LCD_ShowString(2,9,"Speed:");
	LCD_ShowSignedNum(2,4,THigh,3);
	LCD_ShowNum(2,15,Speed,1);
	Timer0_Init();
	
	while(1)
	{
		
		KeyNum=Key();
		
		/*温度读取及显示*/
		DS18B20_ConvertT();	//转换温度
		T=DS18B20_ReadT();	//读取温度
		if(T<0)				//如果温度小于0
		{
			LCD_ShowChar(1,3,'-');	//显示负号
			TShow=-T;		//将温度变为正数
		}
		else				//如果温度大于等于0
		{
			LCD_ShowChar(1,3,'+');	//显示正号
			TShow=T;
		}
		LCD_ShowNum(1,4,TShow,3);		//显示温度整数部分
		LCD_ShowChar(1,7,'.');		//显示小数点
		LCD_ShowNum(1,8,(unsigned long)(TShow*100)%100,2);//显示温度小数部分
		/*阈值判断及显示*/
		if(KeyNum)
		{
			if(KeyNum==1)	//K1按键，THigh自增
			{
				THigh++;
				if(THigh>125){THigh=125;}
			}
			if(KeyNum==2)	//K2按键，THigh自减
			{
				THigh--;
				if(THigh<12){THigh=12;}
			}
			if(KeyNum==3)	//K3按键，调速
			{
				Speed++;
				if(Speed>3)
				{
				Speed=1;
				}
			}
			LCD_ShowSignedNum(2,4,THigh,3);	//显示阈值数据
			LCD_ShowNum(2,15,Speed,1);
			AT24C02_WriteByte(0,THigh);		//写入到At24C02中保存
			Delay(5);
			AT24C02_WriteByte(1,Speed);
			Delay(5);
		}
		if(T>THigh)			//越界判断
		{
			LCD_ShowString(1,13,"OV:H");
			
			//设置比较值，改变PWM占空比
			if(Speed==1){Compare=50;}
			if(Speed==2){Compare=75;}
			if(Speed==3){Compare=100;}
		}
		
		else
		{
			LCD_ShowString(1,13,"    ");
			
			Compare=0;
		}
	}
}

void Timer0_Routine() interrupt 1
{
	TL0 = 0x9C;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	Counter++;
	Counter%=100;	//计数值变化范围限制在0~99
	if(Counter<Compare)	//计数值小于比较值
	{
		Motor=1;		//输出1
	}
	else				//计数值大于比较值
	{
		Motor=0;		//输出0
	}
}
