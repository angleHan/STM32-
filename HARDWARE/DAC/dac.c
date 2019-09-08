#include "dac.h"

/*********************************************
思路：
DAC使用DMA
ADC使用DMA
*********************************************/




/******DAC寄存器地址声明*******/  
#define DAC_DHR12R1    (u32)&(DAC->DHR12R1)   //DAC通道1输出地址（PA4）
#define DAC_DHR12R2    (u32)&(DAC->DHR12R2)   //DAC通道2输出地址（PA5）

extern u16 SineWave_12_bits_Array[256];
extern u16 Wave_Input_12_bits_Array[256];

//PA4为DAC通道1输出管脚
void SineWave_Output_TIM2_DAC1_DMA2_Init(void)
{
  
	GPIO_InitTypeDef			GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
	DAC_InitTypeDef				DAC_InitStructure;
	DMA_InitTypeDef				DMA_InitStructure;
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	  //使能PORTA通道时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	  //使能TIM2的时钟用于产生更新事件从而触发DAC
   	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE );	  //使能DAC通道时钟 
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);		  //使能DMA2时钟
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 // 端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 //模拟输入
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4)	;//PA.4 输出高
					
	
	//1ms更新一次（arr=10，pre=7200,采样频率为10K，采一个点需要1ms），采完一整个正弦波数表需要256ms（经实验证明，这个看不出波形）
	//只有（arr=10,pre=720,采样频率为100K，采一个点需要100us），采完一整个正弦波数表需要256 x 100us = 25.6ms，正弦波频率为40Hz左右（实测39.1Hz）
	//以及（arr=10,pre=72,采样频率为1000K，采一个点需要10us），采完一整个正弦波数表需要256 x 10us = 2.56ms，正弦波频率为400Hz左右（实测391Hz）
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseStructure.TIM_Period = 10-1;//设置输出频率
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);//设置TIME输出触发为更新模式
	
	TIM_Cmd(TIM2,ENABLE);
	
	
	DAC_StructInit(&DAC_InitStructure);
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;//DAC触发为定时器2触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//不产生波形
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;//屏蔽、幅值设置
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable; //不使能输出缓存
    DAC_Init(DAC_Channel_1,&DAC_InitStructure);	 //初始化DAC通道1

	DAC_Cmd(DAC_Channel_1, ENABLE);    //使能DAC的通道1
	
	DAC_DMACmd(DAC_Channel_1, ENABLE); //使能DAC通道1的DMA
	
  
    //DAC_SetChannel1Data(DAC_Align_12b_R, 0);  //12位右对齐数据格式设置DAC值，初始化为0
	
	
	DMA_StructInit( &DMA_InitStructure);        //DMA结构体初始化
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//从寄存器读数据
    DMA_InitStructure.DMA_BufferSize = 256;//寄存器大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//宽度为半字
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//宽度为半字
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//优先级非常高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//关闭内存到内存模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//循环发送模式
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R1;//外设地址为DAC通道1的地址
	/***************************************************************************************************/
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SineWave_12_bits_Array;//波形数据表内存地址
	/***************************************************************************************************/
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);//初始化
	
	DMA_Cmd(DMA2_Channel3, ENABLE);//使能DMA2通道3
	
	
	/*************************测试波形所用的DAC2*******************************/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 // 端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 //模拟输入
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5)	;//PA.5 输出高
	
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;//DAC触发为定时器2触发
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;//不产生波形
	DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude=DAC_LFSRUnmask_Bit0;//屏蔽、幅值设置
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable; //不使能输出缓存
    DAC_Init(DAC_Channel_2,&DAC_InitStructure);	 //初始化DAC通道2

	DAC_Cmd(DAC_Channel_2, ENABLE);    //使能DAC的通道2
	
	DAC_DMACmd(DAC_Channel_2, ENABLE); //使能DAC通道2的DMA
	
	
	
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//从寄存器读数据
    DMA_InitStructure.DMA_BufferSize = 256;//寄存器大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址不递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//宽度为半字
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//宽度为半字
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;//优先级非常高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//关闭内存到内存模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//循环发送模式
	DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12R2;//外设地址为DAC通道2的地址
	/***************************************************************************************************/
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Wave_Input_12_bits_Array;//波形数据表内存地址
	/***************************************************************************************************/
	DMA_Init(DMA2_Channel4, &DMA_InitStructure);//初始化
	
	DMA_Cmd(DMA2_Channel4, ENABLE);//使能DMA2通道4
	
	
	
	

}

//设置通道1输出电压
//vol:0~3300,代表0~3.3V
void Dac1_Set_Vol(u16 vol)
{
	float temp=vol;
	temp/=1000;
	temp=temp*4096/3.3;
	DAC_SetChannel1Data(DAC_Align_12b_R,temp);//12位右对齐数据格式设置DAC值
}






