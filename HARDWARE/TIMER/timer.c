#include "timer.h"
#include "led.h"
#include "lcd.h"
#include "wavetab.h"
//#include "adc.h"
#include "usart.h"


float Output_Frequence;
u16 Output_Frequence_Integer_Part;
u16 Output_Frequence_Decimal_Part;

float Input_Wave_Frequence;

int During_Capture_Flag = 0;
int Input_Capture_Count = 0;


extern u16 SineWave_12_bits_Array[256];
extern u16 SawtoothWave_12_bits_Array[256];
extern u16 SquareWave_12_bits_Array[256];
extern u16 Altered_SquareWave_12_bits_Array[256];

extern u16 Wave_Input_Temp_12_bits_Array[256];
extern u16 Wave_Input_12_bits_Array[256];

extern enum Show_WaveTypeDef	{Show_SineWave,Show_SawtoothWave,Show_SquareWave,Show_Altered_SquareWave}	show_wave;
extern enum Fre_TypeDef			{Slow,Fast}											fre;

enum StatusTypeDef				{begin,end}											status=end;

extern u8 Slow_Fre_Flag;
extern u8 Fast_Fre_Flag;

extern u8 User_Frequence_Array[3];
extern int User_Frequence;

/******************************************************************************/
int dac_convert_counter;
/******************************************************************************/


//通用定时器4中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M

void Display_TIM4_Init(void)
{
    TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	NVIC_InitTypeDef 			NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能
	
	
	
	/*******************************100ms应该更新一次显示屏*************************************************/
	TIM_TimeBaseStructure.TIM_Period = 1000-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;				//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 			//根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE );					//使能指定的TIM4中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;				//TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//先占优先级1级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);								//初始化NVIC寄存器


	TIM_Cmd(TIM4, ENABLE);	//使能TIM4					 
}

void Calculate_ADC_Frequence_TIM5_Init(void)
{
    TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	NVIC_InitTypeDef 			NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE); //时钟使能
	
	
	
	/*******************************100us中断计数一次*************************************************/
	TIM_TimeBaseStructure.TIM_Period = 10-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 720-1;				//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); 			//根据指定的参数初始化TIMx的时间基数单位
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE );					//使能指定的TIM5中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;				//TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//先占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);								//初始化NVIC寄存器


	TIM_Cmd(TIM5, ENABLE);	//使能TIM5					 
}

void Calculate_Wave_Frequence(void)
{
	Output_Frequence = 72000000 / (TIM2->ARR + 1)/(TIM2->PSC + 1) / 256 / 1.0;
	Output_Frequence_Integer_Part = (u16)Output_Frequence;
	Output_Frequence_Decimal_Part = 10000.0 * (Output_Frequence - Output_Frequence_Integer_Part);
}

void Show(void)
{
	int i=0;
	float k1=0.25;
	float k2=0.5;
	
	//Output_Frequence = 72000000 / (TIM2->ARR + 1)*(TIM2->PSC + 1) / 256 * 1.0;
	
	Calculate_Wave_Frequence();
	

				
			/************第一个整波形**************/
			for(i=0;i<255;i++)
			{
				//LCD_DrawPoint(i,150+12 * sin(i));
				if(show_wave == Show_SineWave)
					LCD_DrawPoint( k1*i *390/User_Frequence    ,k2* SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				if(show_wave == Show_SawtoothWave)
					LCD_DrawPoint( k1*i *390/User_Frequence   ,k2* SawtoothWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的锯齿波数表数据
				if(show_wave == Show_SquareWave)
					LCD_DrawPoint( k1*i *390/User_Frequence  ,k2* SquareWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的方波数表数据
				if(show_wave == Show_Altered_SquareWave)
					LCD_DrawPoint( k1*i *390/User_Frequence  ,k2* Altered_SquareWave_12_bits_Array[i] * 200 / 4096);
				
				//LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				
				LCD_DrawPoint( k1*i *390/User_Frequence  , 150 + k2* Wave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
				//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
			}
			
			/************第二个整波形**************/
			for(i=0;i<255;i++)
			{
				//LCD_DrawPoint(i,150+12 * sin(i));
				if(show_wave == Show_SineWave)
					LCD_DrawPoint( k1*255 *390/User_Frequence + k1*i *390/User_Frequence    ,k2* SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				if(show_wave == Show_SawtoothWave)
					LCD_DrawPoint( k1*255 *390/User_Frequence + k1*i *390/User_Frequence  ,k2* SawtoothWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的锯齿波数表数据
				if(show_wave == Show_SquareWave)
					LCD_DrawPoint( k1*255 *390/User_Frequence + k1*i *390/User_Frequence ,k2* SquareWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的方波数表数据
				if(show_wave == Show_Altered_SquareWave)
					LCD_DrawPoint( k1*255 *390/User_Frequence + k1*i *390/User_Frequence ,k2* Altered_SquareWave_12_bits_Array[i] * 200 / 4096);
				
				//LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				
				LCD_DrawPoint( k1*255 *390/User_Frequence + k1*i *390/User_Frequence , 150 + k2* Wave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
				//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
			}
			
			/************第二个整波形**************/
			for(i=0;i<255;i++)
			{
				//LCD_DrawPoint(i,150+12 * sin(i));
				if(show_wave == Show_SineWave)
					LCD_DrawPoint( k1*510 *390/User_Frequence + k1*i *390/User_Frequence    ,k2* SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				if(show_wave == Show_SawtoothWave)
					LCD_DrawPoint( k1*510 *390/User_Frequence + k1*i *390/User_Frequence  ,k2* SawtoothWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的锯齿波数表数据
				if(show_wave == Show_SquareWave)
					LCD_DrawPoint( k1*510 *390/User_Frequence + k1*i *390/User_Frequence ,k2* SquareWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的方波数表数据
				if(show_wave == Show_Altered_SquareWave)
					LCD_DrawPoint( k1*510 *390/User_Frequence + k1*i *390/User_Frequence ,k2* Altered_SquareWave_12_bits_Array[i] * 200 / 4096);
				
				//LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
				
				LCD_DrawPoint( k1*510 *390/User_Frequence + k1*i *390/User_Frequence , 150 + k2* Wave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
				//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
			}
		
		
		/***********显示当前DAC输出的频率************/
		LCD_ShowString( 0, 150 + 220, 200, 12, 16, "Frequence1:" );
		//LCD_ShowxNum(120, 150 + 220, 72000000/(TIM2->ARR + 1)*(TIM2->PSC + 1)/256*1.0 , 9, 16, 0);
		LCD_ShowxNum(85, 150 + 220, Output_Frequence_Integer_Part , 9, 16, 0);
		//LCD_ShowChar(160, 150 + 220, '.', 16, 0 );
		LCD_ShowString( 165, 150 + 220, 200, 12, 16, ".0000" );
		//LCD_ShowxNum(165, 150 + 220, Output_Frequence_Decimal_Part , 9, 16, 0);
		
		LCD_ShowString( 0, 150 + 220 + 16, 200, 12, 16, "Frequence2:" );
		LCD_ShowxNum(85, 150 + 220 + 16, Output_Frequence_Integer_Part , 9, 16, 0);
		LCD_ShowString( 165, 150 + 220 + 16, 200, 12, 16, ".0000" );
		//printf("%f\r\n",Output_Frequence);
		//printf("%f\r\n",Input_Wave_Frequence);
		//printf("%d\r\n",Input_Capture_Count);
		//printf("%d\r\n",ADC1->DR);
		
		
		
		
		
	
}

//定时器4中断服务程序
void TIM4_IRQHandler(void)   //TIM4中断
{
	static int i=0;
	
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM4更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIM4更新中断标志 
		/******************************************************************************/	
		
		
		//LCD_Clear(WHITE);//每次进来的时候先清一次屏
		
		//该定时器溢出设置为26ms，假设回来按键控制或者串口控制这个溢出时间是可以自己控制的
//		for(i=0;i<255;i++)
//		{
//			//LCD_DrawPoint(i,150+12 * sin(i));
//			LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
//			
//			LCD_DrawPoint( i, 150 + SineWave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
//			//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
//		}
//		
		i++;
		if(i==10)
		{
			i=0;
			LCD_Clear(WHITE);
		}
		
		Show();
//		if( Slow_Fre_Flag ==1 )
//		{
//			Slow_Fre_Flag = 0;
//			for(i=0;i<255;i++)
//			{
//				//LCD_DrawPoint(i,150+12 * sin(i));
//				if(show_wave == Show_SineWave)
//					LCD_DrawPoint( 2*i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
//				if(show_wave == Show_SawtoothWave)
//					LCD_DrawPoint( 2*i,SawtoothWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的锯齿波数表数据
//				if(show_wave == Show_SquareWave)
//					LCD_DrawPoint( 2*i,SquareWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的方波数表数据
//				
//				LCD_DrawPoint( 2*i, 150 + Wave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
//				//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
//			}
//		}
//		
//		if( Fast_Fre_Flag ==1 )
//		{
//			Fast_Fre_Flag = 0;
//			for(i=0;i<255;i++)
//			{
//				//LCD_DrawPoint(i,150+12 * sin(i));
//				if(show_wave == Show_SineWave)
//					LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
//				if(show_wave == Show_SawtoothWave)
//					LCD_DrawPoint( i,SawtoothWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的锯齿波数表数据
//				if(show_wave == Show_SquareWave)
//					LCD_DrawPoint( i,SquareWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的方波数表数据
//				
//				
//				//LCD_DrawPoint( i,SineWave_12_bits_Array[i] * 200 / 4096);//准备在LCD屏幕的上面显示自己设置的正弦波数表数据
//				
//				LCD_DrawPoint( i, 150 + Wave_Input_12_bits_Array[i] * 200 / 4096 );//准备在LCD屏幕的下面显示输入检测到的数表数据
//				//LCD_DrawPoint(i,150 + Sine12bit[i] * 150 / 4096);
//			}
//		}
		
		
		/******************************************************************************/
	}
}


void TIM5_IRQHandler(void)   //TIM5中断
{
//	static int During_Capture_Flag = 0;
//	static int Input_Capture_Count = 0;
	
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  //检查TIM5更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIM5更新中断标志
	
		
		
		/*************************************/
//		if( ( Wave_Input_12_bits_Array[0] > 2000 && Wave_Input_12_bits_Array[0] < 2100 ) || During_Capture_Flag == 1 )
//		{			
//			Input_Capture_Count++;
//			
//			if( Wave_Input_12_bits_Array[0] > 2000 && Wave_Input_12_bits_Array[0] < 2100 )
//			{
//				if( status == begin )
//				{
//					Input_Wave_Frequence = 1000 / (0.1 * Input_Capture_Count);
//					During_Capture_Flag = 0;
//					Input_Capture_Count=0;
//					status = end;
//				}	
//				if( status == end )
//					status = begin;
//			}
//			
//			During_Capture_Flag = 1;
//		}
		Input_Capture_Count++;
//		if( ADC1->DR > 2000 && ADC1->DR < 2100 )
//		{
//			if(flag == 1)
//			{
//				Input_Capture_Count=0;
//				flag = 2;
//			}
//			if(flag == 2)
//			{
//				
//			}
//		}
		/*************************************/
		
		
	}
}









