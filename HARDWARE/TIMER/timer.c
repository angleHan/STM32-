#include "timer.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"
#include "math.h"
#include "key.h"
#include "dac.h"
#include "adc.h"

//全局变量
u16 Global_delay_time = 5000; //延时时间
u16 Global_key_num = 0;		  //按键
u16 Global_press_time = 0;	//按键时间
u16 last_press = 0;			  //长按控制变量
int Global_lcd_x = 0;		  //LCDX轴变量
int Global_lcd_y = 0;		  //LCDY轴变量
u16 time = 0;				  // 时间记录器
u16 count = 0;				  // 绘图控制变量
int xdir = 1;				  //x递增控制器
int ydir = 1;				  //y递增控制器
int xdir_dac = 1;
int ydir_dac = 1;
double temp = 0;			  // sin函数 	 
int counter = 0;
int Global_lcd_x_dac = 0;		  
int Global_lcd_y_dac = 0;		  

// 三角波
void triangle_wave(void);
// 方波
void square_wave(void);
// 正弦波
void sin_wave(void);	 

// 三角波
void triangle_wave_dac(void);
// 方波
void square_wave_dac(void);
// 正弦波
void sin_wave_dac(void);

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!




void TIM3_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}


void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	
	//定时器TIM2初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

	TIM_Cmd(TIM2, ENABLE);  //使能TIMx					 
}

//定时器3中断服务程序
int flag = 1;
int count3 = 0;
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除TIMx的中断待处理位:TIM 中断源
		
		// 按键值
		Global_key_num = KEY_Scan(1);
		// 时间记录器
		//time++;

		if (Global_key_num)
		{
			switch(Global_key_num)
			{
				case WKUP_PRES:
					//Adc_Init();		  		//ADC初始化
	        //Dac1_Init();
					counter = 1;
				  Global_lcd_x = 0;		  
					Global_lcd_y = 0;
					Global_lcd_x_dac = 0;		  
					Global_lcd_y_dac = 0;
				  LCD_Clear(WHITE);
				  break;
				case KEY0_PRES:
					//Adc_Init();		  		//ADC初始化
	        //Dac1_Init();
					counter = 2;
				  Global_lcd_x = 0;		  
					Global_lcd_y = 0;
					Global_lcd_x_dac = 0;		  
					Global_lcd_y_dac = 0;
				  //LCD_Init();
				  LCD_Clear(WHITE);
				  //Dac1_Init();
				  break;
				case KEY1_PRES:
					//Adc_Init();		  		//ADC初始化
					//Dac1_Init();
					counter = 3;
				  Global_lcd_x = 0;		  
					Global_lcd_y = 0;
					Global_lcd_x_dac = 0;		  
					Global_lcd_y_dac = 0;
				  //LCD_Init();
				  LCD_Clear(WHITE);
				  break;
				default:
					counter = 1;
				  break;
				
			}
		}
		
		if (counter == 1)
		{
			//LCD_ShowxNum(124,150,(arr+1)/10,4,16,0);
			sin_wave();
			sin_wave_dac();
			LCD_DrawPoint(Global_lcd_x, Global_lcd_y + 60);
		  LCD_DrawPoint(Global_lcd_x_dac, Global_lcd_y_dac + 360);
		}
		else if (counter == 2)
		{
			//LCD_ShowxNum(124,150,(arr+1)/10,4,16,0);
			square_wave();
			square_wave_dac();
			LCD_DrawPoint(Global_lcd_x, Global_lcd_y + 60);
		  LCD_DrawPoint(Global_lcd_x_dac, Global_lcd_y_dac + 360);
		}
		else if (counter == 3)
		{
			//LCD_ShowxNum(124,150,(arr+1)/10,4,16,0);
			triangle_wave();
			triangle_wave_dac();
			LCD_DrawPoint(Global_lcd_x, Global_lcd_y + 60);
		  LCD_DrawPoint(Global_lcd_x_dac, Global_lcd_y_dac + 360);
		}
		//LCD_DrawPoint(Global_lcd_x, Global_lcd_y + 60);
//		if(count3 <= 2)
//		{
//			LCD_DrawPoint(Global_lcd_x, Global_lcd_y + 60);
//		  LCD_DrawPoint(Global_lcd_x_dac, Global_lcd_y_dac + 360);
//			count3 = count3+1;
//		}
//		count3 = 0;
	}
}




//// 三角波
void triangle_wave(void)
{

		// X轴步进量
		if (Global_lcd_x == 0)
			xdir = 1;
		// 控制范围
		if (Global_lcd_x >= 240)
		{
			Global_lcd_x = 0;
			Global_lcd_y = 0;
		}
		// Y三角波绘制
		if (Global_lcd_y == 0)
			ydir = 1;
		if (Global_lcd_y >= 50)
			ydir = -1;

		Global_lcd_x = Global_lcd_x + xdir; //
		Global_lcd_y = Global_lcd_y + ydir; //
		//flag = 1;

}

int pwm = 50;
// 方波
void square_wave(void)
{

		count++;
		// 第一条横线
		if (count <= 50)
		{
			xdir = 1;
			ydir = 0;
		}
		// 第一条竖线
		if (count > pwm && count <= 100)
		{
			xdir = 0;
			ydir = 1;
		}
		// 第二条横线
		if (count > 100 && count <= 150)
		{
			xdir = 1;
			ydir = 0;
		}
		// 第2条竖线
		if (count > 150 && count <= 200)
		{
			xdir = 0;
			ydir = -1;
		}
		// 第3条横线
		if (count > 200 && count <= 250)
		{
			xdir = 1;
			ydir = 0;
		}
		// 第3条竖线
		if (count > 250 && count <= 300)
		{
			xdir = 0;
			ydir = 1;
		}
		// 第4条横线
		if (count > 300 && count <= 350)
		{
			xdir = 1;
			ydir = 0;
		}
		// 第4条竖线
		if (count > 350 && count <= 400)
		{
			xdir = 0;
			ydir = -1;
		}
		// 第5条横线
		if (count > 400 && count <= 450)
		{
			xdir = 1;
			ydir = 0;
		}
		// 定范围
		if (Global_lcd_x >= 240)
		{
			Global_lcd_x = 0;
			Global_lcd_y = 0;
			count = 0;
		}

		Global_lcd_x = Global_lcd_x + xdir; //
		Global_lcd_y = Global_lcd_y + ydir; //
		
}

// 正弦波

void sin_wave(void)
{


		if (Global_lcd_x == 0)
			xdir = 1;
		if (Global_lcd_x >= 240)
		{
			Global_lcd_x = 0;
			Global_lcd_y = 0;
		}

		Global_lcd_x = Global_lcd_x + xdir; //
		temp = Global_lcd_x;
		if ((int)temp % 2 == 0)
			Global_lcd_y = 30 * sin(3 * temp); //
		  // Global_lcd_y = sin(temp);
		else
			Global_lcd_y = -30 * sin(3 * temp);
		  // Global_lcd_y = -sin(temp);
		
		Global_lcd_y = (int)Global_lcd_y;


}

int temp_t=0;
int count2 = 0;


// 三角波
void triangle_wave_dac(void)
{

		// X轴步进量
		if (Global_lcd_x == 0)
			xdir = 1;
		// 控制范围
		if (Global_lcd_x >= 240)
		{
			Global_lcd_x = 0;
			Global_lcd_y = 0;
		}
		// Y三角波绘制
		if (Global_lcd_y == 0)
			ydir = 1;
		if (Global_lcd_y >= 50)
			ydir = -1;

		Global_lcd_x = Global_lcd_x + xdir; //
		Global_lcd_y = Global_lcd_y + ydir; //
		
		
		DAC_SetChannel1Data(DAC_Align_12b_R, Global_lcd_y);
		//Dac1_Set_Vol(Global_lcd_y);

		Global_lcd_y_dac=DAC_GetDataOutputValue(DAC_Channel_1);
		Global_lcd_y_dac=Get_Adc_Average(ADC_Channel_1,10);
			

		
		//Global_lcd_x_dac=Global_lcd_x_dac/10;
		
		Global_lcd_x_dac = Global_lcd_x;

}

// 方波
void square_wave_dac(void)
{
	/////////////////////////////////////
	
	count++;

	// 第一条横线
	if (count <= 50)
	{
		xdir = 1;
		ydir = 0;
	}
	// 第一条竖线
	if (count > 50 && count <= 100)
	{
		xdir = 0;
		ydir = 1;
	}
	// 第二条横线
	if (count > 100 && count <= 150)
	{
		xdir = 1;
		ydir = 0;
	}
	// 第2条竖线
	if (count > 150 && count <= 200)
	{
		xdir = 0;
		ydir = -1;
	}
	// 第3条横线
	if (count > 200 && count <= 250)
	{
		xdir = 1;
		ydir = 0;
	}
	// 第3条竖线
	if (count > 250 && count <= 300)
	{
		xdir = 0;
		ydir = 1;
	}
	// 第4条横线
	if (count > 300 && count <= 350)
	{
		xdir = 1;
		ydir = 0;
	}
	// 第4条竖线
	if (count > 350 && count <= 400)
	{
		xdir = 0;
		ydir = -1;
	}
	// 第5条横线
	if (count > 400 && count <= 450)
	{
		xdir = 1;
		ydir = 0;
	}
	// 定范围
	if (Global_lcd_x >= 240)
	{
		Global_lcd_x = 0;
		Global_lcd_y = 0;
		count = 0;
	}

	Global_lcd_x = Global_lcd_x + xdir; 
	Global_lcd_y = Global_lcd_y + ydir; 
	
	DAC_SetChannel1Data(DAC_Align_12b_R, Global_lcd_y*100);
	
	Global_lcd_y_dac=DAC_GetDataOutputValue(DAC_Channel_1);
	Global_lcd_y_dac=Get_Adc_Average(ADC_Channel_1,10);
	
	Global_lcd_x_dac=Global_lcd_x;
		
		


}

// 正弦波

void sin_wave_dac(void)
{

		if (Global_lcd_x == 0)
		{
			xdir = 1;
		}
			
		if (Global_lcd_x >= 240)
		{
			Global_lcd_x = 0;
			Global_lcd_y = 0;
		}

		Global_lcd_x = Global_lcd_x + xdir; //
		temp = Global_lcd_x;
		if ((int)temp % 2 == 0)
			Global_lcd_y = 30 * sin(3 * temp); //
		else
			Global_lcd_y = -30 * sin(3 * temp);
		Global_lcd_y = (int)Global_lcd_y;
		
		DAC_SetChannel1Data(DAC_Align_12b_R, Global_lcd_y);

		Global_lcd_y_dac=DAC_GetDataOutputValue(DAC_Channel_1);
		Global_lcd_y_dac=Get_Adc_Average(ADC_Channel_1,10);
		
		Global_lcd_x_dac = Global_lcd_x;



}









