#ifndef __DAC_H
#define __DAC_H	 
#include "sys.h"	    
								    
//extern u16 SineWave_12_bits_Array[256];								    

void SineWave_Output_TIM2_DAC1_DMA2_Init(void);//回环模式初始化		 	 
void Dac1_Set_Vol(u16 vol);


#endif


