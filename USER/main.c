/*************************************************
频率在100Hz到390Hz区间内显示可调
    在039Hz到390Hz区间内输出可调
*************************************************/

//原子级
#include "sys.h"
#include "delay.h"
#include "usart.h"

//用户级
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "dac.h"
#include "adc.h"
#include "timer.h"
#include "wavetab.h"

#include "math.h"

extern u16 Wave_Input_Temp_12_bits_Array[256];
extern u16 Wave_Input_12_bits_Array[256];

u8 Slow_Fre_Flag=1;
u8 Fast_Fre_Flag=0;

u8 WaveType_Count=0;
u8 SquareWaveType_Count=0;/////////////////////////////////新增改变占空比！！！！！！！！！！！！！！！！！！！！！！！！！

u16 SawtoothWave_12_bits_Array[256];
u16 SquareWave_12_bits_Array[256];
u16 Altered_SquareWave_12_bits_Array[256];

enum Show_WaveTypeDef	{Show_SineWave,Show_SawtoothWave,Show_SquareWave,Show_Altered_SquareWave}	show_wave	=	Show_SineWave;
enum Fre_TypeDef		{Slow,Fast}											fre			=	Fast;


u8 User_Frequence_Array[3];
int User_Frequence=390;



u16 SineWave_12_bits_Array[256] = { //256个正弦波采样点
 2048, 2098, 2148, 2198, 2248, 2298, 2348, 2398, 2447, 2496,
 2545, 2594, 2642, 2690, 2737, 2785, 2831, 2877, 2923, 2968,
 3013, 3057, 3100, 3143, 3185, 3227, 3267, 3307, 3347, 3385,
 3423, 3460, 3496, 3531, 3565, 3598, 3631, 3662, 3692, 3722,
 3750, 3778, 3804, 3829, 3854, 3877, 3899, 3920, 3940, 3958,
 3976, 3992, 4007, 4021, 4034, 4046, 4056, 4065, 4073, 4080,
 4086, 4090, 4093, 4095, 4095, 4095, 4093, 4090, 4086, 4080,
 4073, 4065, 4056, 4046, 4034, 4021, 4007, 3992, 3976, 3958,
 3940, 3920, 3899, 3877, 3854, 3829, 3804, 3778, 3750, 3722,
 3692, 3662, 3631, 3598, 3565, 3531, 3496, 3460, 3423, 3385,
 3347, 3307, 3267, 3227, 3185, 3143, 3100, 3057, 3013, 2968,
 2923, 2877, 2831, 2785, 2737, 2690, 2642, 2594, 2545, 2496,
 2447, 2398, 2348, 2298, 2248, 2198, 2148, 2098, 2047, 1997,
 1947, 1897, 1847, 1797, 1747, 1697, 1648, 1599, 1550, 1501,
 1453, 1405, 1358, 1310, 1264, 1218, 1172, 1127, 1082, 1038,
 995, 952, 910, 868, 828, 788, 748, 710, 672, 635,
 599, 564, 530, 497, 464, 433, 403, 373, 345, 317,
 291, 266, 241, 218, 196, 175, 155, 137, 119, 103,
 88, 74, 61, 49, 39, 30, 22, 15, 9, 5,
 2, 0, 0, 0, 2, 5, 9, 15, 22, 30,
 39, 49, 61, 74, 88, 103, 119, 137, 155, 175,
 196, 218, 241, 266, 291, 317, 345, 373, 403, 433,
 464, 497, 530, 564, 599, 635, 672, 710, 748, 788,
 828, 868, 910, 952, 995, 1038, 1082, 1127, 1172, 1218,
 1264, 1310, 1358, 1405, 1453, 1501, 1550, 1599, 1648, 1697,
 1747, 1797, 1847, 1897, 1947, 1997 };

void Generate_SawtoothWave_Array(void)
{
	int i=0;
	
	SawtoothWave_12_bits_Array[0]=2048;
	SawtoothWave_12_bits_Array[63]=0;
	SawtoothWave_12_bits_Array[192]=4095;
	SawtoothWave_12_bits_Array[255]=2048;
	
	for(i=0;i<63;i++)
	{
		SawtoothWave_12_bits_Array[i+1]=SawtoothWave_12_bits_Array[i]-32;
	}
	for(i=64;i<191;i++)
	{
		SawtoothWave_12_bits_Array[i+1]=SawtoothWave_12_bits_Array[i]+32;
	}
	for(i=192;i<254;i++)
	{
		SawtoothWave_12_bits_Array[i+1]=SawtoothWave_12_bits_Array[i]-32;
	}
}

void Generate_SquareWave_Array(void)
{
	int i=0;
	
	SquareWave_12_bits_Array[0] = 4095;
	
	for(i=0;i<120;i++)
	{
		SquareWave_12_bits_Array[i] = 4095;
	}
	
	for(i=120;i<136;i++)
	{
		SquareWave_12_bits_Array[i+1] = SquareWave_12_bits_Array[i] - 273;
	}
	
	for(i=136;i<240;i++)
	{
		SquareWave_12_bits_Array[i] = 0;
	}
	
	for(i=240;i<255;i++)
	{
		SquareWave_12_bits_Array[i+1] = SquareWave_12_bits_Array[i] + 273;
	}
}

void Generate_Altered_SquareWave_Array(void)
{
	int i=0;
	
	Altered_SquareWave_12_bits_Array[0] = 4095;
	
	for(i=0;i<80;i++)
	{
		Altered_SquareWave_12_bits_Array[i] = 4095;
	}
	
	for(i=80;i<96;i++)
	{
		Altered_SquareWave_12_bits_Array[i+1] = Altered_SquareWave_12_bits_Array[i] - 273;
	}
	
	for(i=96;i<240;i++)
	{
		Altered_SquareWave_12_bits_Array[i] = 0;
	}
	
	for(i=240;i<255;i++)
	{
		Altered_SquareWave_12_bits_Array[i+1] = Altered_SquareWave_12_bits_Array[i] + 273;
	}
}

int main(void)
{
	u16 t;  
	u16 len;
	
	delay_init();
	uart_init(115200);
	
	Generate_SawtoothWave_Array();
	Generate_SquareWave_Array();
	Generate_Altered_SquareWave_Array();
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	SineWave_Output_TIM2_DAC1_DMA2_Init();//传输
	SineWave_Input_TIM3_ADC1_DMA1_Init();//采集
	Calculate_ADC_Frequence_TIM5_Init();
	
	LED_Init();	
	LCD_Init();
	KEY_Init();
//	TPAD_Init(6);			//初始化触摸按键
	
	Display_TIM4_Init();
	
	POINT_COLOR=RED;	
//	LCD_Clear(WHITE);
	
	while(1)
	{
		u8 key;
//		LED0=!LED0;	
//		//delay_ms(50);
//		//delay_us(10);
//		delay_ms(1); 
		
		key=KEY_Scan(0);
		if( key == KEY0_PRES )
		{
			LCD_Clear(WHITE);
			
			if(SquareWaveType_Count == 0)
				SquareWaveType_Count=1;
			else if(SquareWaveType_Count == 1)
				SquareWaveType_Count=0;
			
			if(SquareWaveType_Count == 0)
			{
				show_wave = Show_SquareWave;
			}
			
			if(SquareWaveType_Count == 1)
			{
				show_wave = Show_Altered_SquareWave;
			}
			
//			TIM2->PSC = 720-1;//由26ms变为2.6ms//DAC
//			TIM3->PSC = 720-1;//由26ms变为2.6ms//ADC
//			//TIM4->ARR = 260-1;//由26ms变为2.6ms//LCD
			
			Show();
		}
		
		if( key == KEY1_PRES )
		{
			LCD_Clear(WHITE);
					if(WaveType_Count == 0)
						WaveType_Count=1;
					else if(WaveType_Count == 1)
						WaveType_Count=2;
					else if(WaveType_Count == 2)
						WaveType_Count=0;
					
					if(WaveType_Count == 0)
					{
						show_wave = Show_SineWave;
					}
					if(WaveType_Count == 1)
					{
						show_wave = Show_SawtoothWave;
					}
					
					if(WaveType_Count == 2)
					{
						show_wave = Show_SquareWave;
					}
					
					Show();
		}
		
//		if( key == WK_UP )
//		{
//			LCD_Clear(WHITE);
//			if(WaveType_Count == 0)
//				WaveType_Count=1;
//			else if(WaveType_Count == 1)
//				WaveType_Count=0;

//			
//			if(WaveType_Count == 0)
//			{
//				show_wave = Show_SineWave;
//				//DMA2_Channel3->CMAR = (uint32_t)SineWave_12_bits_Array;//可以改变送哪种波的数据
//			}
//			if(WaveType_Count == 1)
//			{
//				show_wave = Show_SawtoothWave;
//				//DMA2_Channel3->CMAR = (uint32_t)SawtoothWave_12_bits_Array;//锯齿波
//			}
////			if(WaveType_Count == 2)
////				DMA2_Channel3->CMAR = (uint32_t)SquareWave_12_bits_Array;//方波
//			
//			
//			//DMA1_Channel1->CMAR = //可以改变采集完数据后把
//		}		

		
		if(USART_RX_STA&0x8000)
		{					   
			
			if(USART_RX_BUF[0] == '0')
				User_Frequence_Array[0] = 0x00;
			else
				User_Frequence_Array[0] = USART_RX_BUF[0]-0x30;
			
			if(USART_RX_BUF[1] == '0')
				User_Frequence_Array[1] = 0x00;
			else
				User_Frequence_Array[1] = USART_RX_BUF[1]-0x30;
			
			if(USART_RX_BUF[2] == '0')
				User_Frequence_Array[2] = 0x00;
			else
				User_Frequence_Array[2] = USART_RX_BUF[2]-0x30;
			
			User_Frequence = 100 * User_Frequence_Array[0]+10*User_Frequence_Array[1]+1*User_Frequence_Array[2];
			
			TIM2->ARR = 72000000 / 72 / 256 / User_Frequence - 1;
			TIM3->ARR = 72000000 / 72 / 256 / User_Frequence - 1;
						
			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			for(t=0;t<len;t++)
			{
				USART_SendData(USART1, USART_RX_BUF[t]);//向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
			}
			printf("\r\n\r\n");//插入换行
			USART_RX_STA=0;
		}
	}	
}






