 #include "adc.h"
 #include "delay.h"
 
 /*********************************************
采样按照：
认为正弦信号为40Hz（实测其实是39.1Hz），即信号周期25ms，一个信号周期采样256个点那么定时器应该设置为25ms/256=97.65625约等于98us溢出一次
ADC使用DMA
*********************************************/
 
 
u16 Wave_Input_Temp_12_bits_Array[256];
u16 Wave_Input_12_bits_Array[256];

extern u16 SineWave_12_bits_Array[256];
extern u16 SawtoothWave_12_bits_Array[256];
extern u16 SquareWave_12_bits_Array[256];
extern u16 Altered_SquareWave_12_bits_Array[256];

extern enum Show_WaveTypeDef	{Show_SineWave,Show_SawtoothWave,Show_SquareWave,Show_Altered_SquareWave}	show_wave;
 
 
////初始化ADC
////这里我们仅以规则通道为例
////我们默认将开启通道0~3																	   
//void  Adc_Init(void)
//{ 	
//	ADC_InitTypeDef ADC_InitStructure; 
//	GPIO_InitTypeDef GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
// 

//	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

//	//PA1 作为模拟通道输入引脚                         
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

//	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

//	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
//	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
//	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
//	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

//  
//	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
//	
//	ADC_ResetCalibration(ADC1);	//使能复位校准  
//	 
//	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
//	
//	ADC_StartCalibration(ADC1);	 //开启AD校准
// 
//	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
// 
////	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

//}				  
////获得ADC值
////ch:通道值 0~3
//u16 Get_Adc(u8 ch)   
//{
//  	//设置指定ADC的规则组通道，一个序列，采样时间
//	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
//  
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
//	 
//	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

//	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
//}

//u16 Get_Adc_Average(u8 ch,u8 times)
//{
//	u32 temp_val=0;
//	u8 t;
//	for(t=0;t<times;t++)
//	{
//		temp_val+=Get_Adc(ch);
//		delay_ms(5);
//	}
//	return temp_val/times;
//} 	 



void SineWave_Input_TIM3_ADC1_DMA1_Init(void)//ADC1_Mode_Config(uint32_t ADC_ConvertedVal)
{
        
	GPIO_InitTypeDef			GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
	ADC_InitTypeDef				ADC_InitStructure;
	DMA_InitTypeDef				DMA_InitStructure;
	NVIC_InitTypeDef			NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	  //使能（ADC的通道1）PORTA通道时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	  //使能TIM2的时钟用于产生更新事件从而定时触发ADC
   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		  //使能DMA1时钟
	
	
	/***********GPIO***************/	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	/***********TIM***************/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 10-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 72-1;//定时器触发100us采样一个点
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update); //TRGO触发ADC1
	TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);

	TIM_Cmd(TIM3,ENABLE); 
	
	
	/***********ADC***************/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	ADC_DeInit(ADC1); 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; 
	ADC_InitStructure.ADC_NbrOfChannel = 1; 
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//TIM3的TRGO更新事件触发ADC 
	ADC_InitStructure.ADC_DataAlign =ADC_DataAlign_Right; 
	ADC_Init(ADC1,&ADC_InitStructure); 
	
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);//使能外部触发
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_55Cycles5);//ADC1的Channel1
	
	ADC_DMACmd(ADC1, ENABLE);/***********ADC的DMA****************/
	
	ADC_Cmd(ADC1,ENABLE); 

	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1)); 
	ADC_StartCalibration(ADC1); 
	while(ADC_GetCalibrationStatus(ADC1));
	
	
	
	/***********DMA***************/
	DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t ) ( & ( ADC1->DR ) );// 外设基址为：ADC 数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave_Input_Temp_12_bits_Array;// 存储器地址，实际上就是一个内部SRAM的变量
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 256;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址自增 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;/****循环传输模式****/ 	
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;// DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;// 禁止存储器到存储器模式，因为是从外设到存储器
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);//DMA1的Channel1负责ADC1

	DMA_Cmd(DMA1_Channel1 , ENABLE);
	
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE); //传输结束中断
	
	
	/***********NVIC***************/
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	

}


void DMA1_Channel1_IRQHandler(void)
{        
	int i=0;
	
	
	for(i=0;i<256;i++)
	{
		Wave_Input_12_bits_Array[i] = Wave_Input_Temp_12_bits_Array[i];//把缓冲区的临时值放到稳定区中
	}        
	
	if(show_wave == Show_SineWave)
		DMA2_Channel3->CMAR = (uint32_t)SineWave_12_bits_Array;
	if(show_wave == Show_SawtoothWave)
		DMA2_Channel3->CMAR = (uint32_t)SawtoothWave_12_bits_Array;
	if(show_wave == Show_SquareWave)
		DMA2_Channel3->CMAR = (uint32_t)SquareWave_12_bits_Array;
	
	
	if(show_wave == Show_Altered_SquareWave)
		DMA2_Channel3->CMAR = (uint32_t)Altered_SquareWave_12_bits_Array;
	
	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_ClearFlag(DMA1_FLAG_TC1);
}





























