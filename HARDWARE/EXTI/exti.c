#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "beep.h"
#include "lcd.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//外部中断 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   
//外部中断0服务程序

extern u16 Colors;
extern u16 Hun, Dcd, Unt, NumL;

void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	 //	按键端口初始化

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

    //GPIOE.2 中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource2);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line2;	//KEY2
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

   //GPIOE.3	  中断线以及中断初始化配置 下降沿触发 //KEY1
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line3;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

   //GPIOE.4	  中断线以及中断初始化配置  下降沿触发	//KEY0
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


   //GPIOA.0	  中断线以及中断初始化配置 上升沿触发 PA0  WK_UP
 	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0); 

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_Init(&EXTI_InitStructure);		//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键WK_UP所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			//使能按键KEY2所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//使能按键KEY1所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//使能按键KEY0所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
}

//外部中断0服务程序 
u16 flagCount = 0;	//连按累加
u8 flagL = 0;				//下划线位置，flagL=0为百位，flagL=1为十位，flagL=2为个位
u8 Start = 1;				//Start = 1为启动，Start = 0为停止，

//void EXTI0_IRQHandler(void)
//{
//	if(Start==0){
//		//暂停后才可更改数字和下标，去掉此功能将if删去
//		
//		delay_ms(10);//消抖
//		if(WK_UP==1)	 	 //WK_UP按键
//		{
//			//短按一次
//			delay_ms(10);
//			if(flagL==0){
//				Hun++;
//				if(Hun>5){Hun = 0;}
//				if(Hun==0 && Dcd==0 && Unt==0){
//					Hun=0;Dcd=9;Unt=9;
//				}
//			}
//			else if(flagL==1){
//				Dcd++;
//				if(Dcd>9){Dcd = 0;}
//				if(Hun==0 && Dcd==0 && Unt==0){
//					Hun=0;Dcd=9;Unt=9;
//				}
//			}
//			else if(flagL==2){
//				Unt++;
//				if(Unt>9){Unt = 0;}
//				if(Hun==0 && Dcd==0 && Unt==0){
//					Hun=0;Dcd=9;Unt=9;
//				}
//			}
//			
//			//连按状态
//			for(;WK_UP==1;flagCount++){
//				delay_ms(10);
//				if(flagCount>50){
//					delay_ms(100);
//					if(flagL==0){
//						Hun++;
//						if(Hun>5){Hun = 0;}
//						if(Hun==0 && Dcd==0 && Unt==0){
//							Hun=0;Dcd=9;Unt=9;
//						}
//						LCD_ShowNum(100,300,Hun,1,24);
//						LCD_ShowNum(150,300,Dcd,1,24);
//						LCD_ShowNum(200,300,Unt,1,24);
//					}
//					else if(flagL==1){
//						Dcd++;
//						if(Dcd>9){Dcd = 0;}
//						if(Hun==0 && Dcd==0 && Unt==0){
//							Hun=0;Dcd=9;Unt=9;
//						}
//						LCD_ShowNum(100,300,Hun,1,24);
//						LCD_ShowNum(150,300,Dcd,1,24);
//						LCD_ShowNum(200,300,Unt,1,24);
//					}
//					else if(flagL==2){
//						Unt++;
//						if(Unt>9){Unt = 0;}
//						if(Hun==0 && Dcd==0 && Unt==0){
//							Hun=0;Dcd=9;Unt=9;
//						}
//						LCD_ShowNum(100,300,Hun,1,24);
//						LCD_ShowNum(150,300,Dcd,1,24);
//						LCD_ShowNum(200,300,Unt,1,24);
//					}
//					//
//				}
//			}
//		}
//		if(WK_UP==0){
//			flagCount = 0;
//		}
//		
//	}//
//	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位  
//}
// 
////外部中断2服务程序

//void EXTI2_IRQHandler(void)
//{
//	delay_ms(10);//消抖
//	if(KEY2==0)	  //按键KEY2
//	{
//	  //短按一次
//		delay_ms(100);
//		Start = !Start;
//		//
//		for(;KEY2==0;flagCount++){
//			delay_ms(10);
//			if(flagCount>50){

//				//
//			}
//		}
//	}
//	if(KEY2==1){
//		flagCount = 0;
//	}
//	
//	EXTI_ClearITPendingBit(EXTI_Line2);  //清除LINE2上的中断标志位  
//}
////外部中断3服务程序
//void EXTI3_IRQHandler(void)
//{
//	if(Start==0){
//		//暂停后才可更改数字和下标
//		
//		delay_ms(10);//消抖
//		if(KEY1==0)	 //按键KEY1
//		{
//			//短按一次
//			if(NumL == 100){
//				POINT_COLOR = WHITE;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				NumL = 150;
//				POINT_COLOR = RED;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				LED1=!LED1;
//				flagL = 1;
//			}
//			else if(NumL == 150){
//				POINT_COLOR = WHITE;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				NumL = 200;
//				POINT_COLOR = RED;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				flagL = 2;
//			}
//			else if(NumL == 200){
//				POINT_COLOR = WHITE;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				NumL = 100;
//				POINT_COLOR = RED;
//				LCD_DrawLine(NumL,340,NumL+10,340);
//				flagL = 0;
//			}
//			delay_ms(10);
//			//
//			for(;KEY1==0;flagCount++){
//				delay_ms(10);
//				if(flagCount>50){
//					delay_ms(100);
//					
//					//
//				}
//			}
//		}
//		if(KEY1==1){
//			flagCount = 0;
//		}
//		
//	}
//	EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
//}

//u8 colorFlag = 0;
//void EXTI4_IRQHandler(void)
//{
//	delay_ms(10);//消抖
//	if(KEY0==0)	 //按键KEY0
//	{
//	  //短按一次
//		//更改画点的颜色
//		delay_ms(10);
//		colorFlag++;
//		if(colorFlag>3)colorFlag = 0;
//		switch(colorFlag)
//		{
//			case 0:Colors=RED;break;
//			case 1:Colors=BLACK;break;
//			case 2:Colors=BLUE;break;
//			case 3:Colors=GREEN;break;
//		}
//	  //连按状态
//		for(;KEY0==0;flagCount++){
//			delay_ms(10);
//			if(flagCount>50){
//			  delay_ms(100);
//				colorFlag++;
//				if(colorFlag>3)colorFlag = 0;
//				switch(colorFlag)
//				{
//					case 0:Colors=RED;break;
//					case 1:Colors=BLACK;break;
//					case 2:Colors=BLUE;break;
//					case 3:Colors=GREEN;break;
//				}
//				//连续更改画点的颜色
//			}
//		}
//	}
//	if(KEY0==1){
//		flagCount = 0;
//	}
//	
//	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
//}
 
