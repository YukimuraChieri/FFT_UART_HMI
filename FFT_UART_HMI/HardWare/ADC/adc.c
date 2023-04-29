#include "adc.h"
#include "delay.h"

// 用来存放ADC转换结果，也是DMA的目标地址,单通道采集256次
uint16_t adc_buf[NPT] = {0};
// ADC DMA传输完成标志
uint8_t adc_dma_flag = 0;

// 初始化ADC1																	   
void ADC1_Init(void)
{
	/* enable GPIOA clock */
 	RCC->APB2ENR|=1<<2;
	
	/* connect port to ADC1_CH0 */
	GPIOA->CRL&=0XFFFFFFF0;
	
	/* enable ADC0 clock */
	RCC->APB2ENR|=1<<9;    	//ADC1时钟使能
	RCC->APB2RSTR|=1<<9;   	//ADC1复位
	RCC->APB2RSTR&=~(1<<9);	//复位结束	    
	RCC->CFGR&=~(3<<14);   	//分频因子清零
	
	//SYSCLK/DIV2=12M ADC时钟设置为12M,ADC最大时钟不能超过14M!
	//否则将导致ADC准确度下降! 
	RCC->CFGR |=	2<<14;      	 
	ADC1->CR1 &=	0XF0FFFF;	//工作模式清零
	ADC1->CR1 |=	0<<16;   	//独立工作模式  
	ADC1->CR1 |=	(1<<8); 	//使能扫描模式	  
	ADC1->CR2 &=	~(1<<1); 	//单次转换模式
	ADC1->CR2 |=	1<<20;   	//使用用外部触发(SWSTART)!!!	必须使用一个事件来触发
	ADC1->CR2 &=	~(1<<11);	//右对齐	 
	ADC1->SQR1 &=	~(0XF<<20);
	ADC1->SQR1 |=	1<<20;		//8个转换在规则序列中 也就是只转换规则序列1 		
	
	//设置通道0的采样时间
	ADC1->SMPR2&=~(7<<(3*0));	//通道0采样时间清空	  
 	ADC1->SMPR2|=6<<(3*0);		//通道0 71.5周期,提高采样时间可以提高精确度	 
	
	//设置规则序列
	ADC1->SQR3 &= ~(0x1F<<(5*0));	// 清空规则序列的第1个转换通道
	ADC1->SQR3 |= (0<<(5*0));			// 设置规则序列的第1个转换通道
	
//	ADC1->SQR3 &= ~(0x1F<<(5*1));	// 清空规则序列的第2个转换通道
//	ADC1->SQR3 |= (0<<(5*1));			// 设置规则序列的第2个转换通道
//	
//	ADC1->SQR3 &= ~(0x1F<<(5*2));	// 清空规则序列的第3个转换通道
//	ADC1->SQR3 |= (0<<(5*2));			// 设置规则序列的第3个转换通道
//	
//	ADC1->SQR3 &= ~(0x1F<<(5*3));	// 清空规则序列的第4个转换通道
//	ADC1->SQR3 |= (0<<(5*3));			// 设置规则序列的第4个转换通道
	
	ADC1->CR2 &=	~(7<<17);	
	ADC1->CR2 |= (4<<17);		// 定时器3的TRGO事件触发规则通道转换
	ADC1->CR2 |= (1<<20);		// 使用外部事件启动转换
	
	ADC1->CR2 |= (1<<8);		// 使用DMA模式
	
	ADC1->CR2|=1<<0;				//开启AD转换器
	ADC1->CR2|=1<<3;				//使能复位校准  
	while(ADC1->CR2&1<<3);	//等待校准结束 			 
  //该位由软件设置并由硬件清除。在校准寄存器被初始化后该位将被清除。 		 
	ADC1->CR2|=1<<2;        //开启AD校准	   
	while(ADC1->CR2&1<<2);  //等待校准结束
	//该位由软件设置以开始校准，并在校准结束时由硬件清除  
}

// ADC1 DMA回调函数
void ADC1_DMA_CallBack(void)
{
	adc_dma_flag = 1;
}

