#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "timer.h"
#include "exti.h"
#include "beep.h"
#include "dac.h"
#include "adc.h"
 
void init(void);

extern int Global_lcd_x;
extern int Global_lcd_y;
extern int Global_lcd_x_dac;
extern int Global_lcd_y_dac;

int arr = 9;
extern int flag;

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
extern int Global_key_num;
extern int counter;

 int main(void)
 {		
	  init();

    while(1)
	{
		
			
	}	 

}

// 初始化函数
void init(void)
{
	u8 lcd_id[12];   //存放LCD ID字符串
	delay_init();	//延时函数初始化
	uart_init(9600); //串口初始化为9600
	//uart2_init(115200);
	LED_Init();		 //初始化与LED连接的硬件接口
	LCD_Init();
	KEY_Init();
	Adc_Init();		  		//ADC初始化
	Dac1_Init();

	POINT_COLOR = RED;
	sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id); //将LCD ID打印到lcd_id数组。
	

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
	TIM3_Int_Init(9, 7199);						// 1Khz的计数频率，计数到99为10ms,若arr为9，则为10Khz
	//TIM2_Int_Init(999, 7199);
	LCD_Clear(WHITE);								// 清屏
}

//void UART_Handle()
//{
//	
//}
