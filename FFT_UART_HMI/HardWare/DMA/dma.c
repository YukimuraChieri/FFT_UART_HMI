#include "dma.h"
#include "delay.h"
#include <stdio.h>

// DMA中断回调函数指针
DMA_CallBack_Ptr DMA1_CH1_TC_CallBack_Ptr = NULL;
DMA_CallBack_Ptr DMA1_CH4_TC_CallBack_Ptr = NULL;


// DMA1通道1初始化函数(ADC1)
void DMA1_CH1_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR |= 1<<0;			//开启DMA1时钟
	delay_ms(5);							//等待DMA时钟稳定
	
	/* initialize DMA channel1 */
	DMA1_Channel1->CCR = 0X00000000;	//复位
	DMA1_Channel1->CCR |= 0<<4;  		//从外设读
	DMA1_Channel1->CCR |= 0<<5;  		//非循环缓存模式
	DMA1_Channel1->CCR |= 0<<6; 		//外设地址非增量模式
	DMA1_Channel1->CCR |= 1<<7; 	 	//存储器增量模式
	DMA1_Channel1->CCR |= 1<<8; 	 	//外设数据宽度为16位
	DMA1_Channel1->CCR |= 1<<10; 		//存储器数据宽度16位
	DMA1_Channel1->CCR |= 2<<12; 		//高优先级
	DMA1_Channel1->CCR |= 0<<14; 		//非存储器到存储器模式
	
	DMA1_Channel1->CNDTR = 0;    		// 定义DMA通道的传输数量
	
	DMA1_Channel1->CPAR = PerAddr;	//定义DMA外设基地址
	DMA1_Channel1->CMAR = MemAddr;	//定义DMA内存基地址(转换结果保存的地址)
	
	/* DMA interrupt configuration */
	DMA1_Channel1->CCR|=1<<1;				// 使能DMA传输完成中断
	MY_NVIC_Init(1, 0, DMA1_Channel1_IRQn, 2);	//抢占1，子优先级0，组2	
}

// DMA1通道4初始化函数(UART1_TX)
void DMA1_CH4_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR|=1<<0;	//开启DMA1时钟
	delay_ms(5);				//等待DMA时钟稳定
	
	/* configure DMA mode */
	DMA1_Channel4->CPAR=PerAddr;	//DMA1 外设地址 
	DMA1_Channel4->CMAR=MemAddr;	//DMA1,存储器地址
	DMA1_Channel4->CNDTR=0;				// 定义DMA通道的传输数量
	DMA1_Channel4->CCR=0X00000000;	//复位
	DMA1_Channel4->CCR|=1<<4;  			//从存储器读
	DMA1_Channel4->CCR|=0<<5;  			//普通模式
	DMA1_Channel4->CCR|=0<<6; 			//外设地址非增量模式
	DMA1_Channel4->CCR|=1<<7; 	 		//存储器增量模式
	DMA1_Channel4->CCR|=0<<8; 	 		//外设数据宽度为8位
	DMA1_Channel4->CCR|=0<<10; 			//存储器数据宽度8位
	DMA1_Channel4->CCR|=1<<12; 			//中等优先级
	DMA1_Channel4->CCR|=0<<14; 			//非存储器到存储器模式
	
	/* DMA interrupt configuration */
	DMA1_Channel4->CCR|=1<<1;				// 使能DMA传输完成中断
	MY_NVIC_Init(1, 0, DMA1_Channel4_IRQn, 2);	//抢占1，子优先级0，组2	
}


// DMA1通道5初始化函数(UART1_RX)
void DMA1_CH5_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR|=1<<0;	//开启DMA1时钟
	delay_ms(5);				//等待DMA时钟稳定
	
	/* configure DMA mode */
	DMA1_Channel5->CPAR=PerAddr;		//DMA1 外设地址 
	DMA1_Channel5->CMAR=MemAddr;		//DMA1,存储器地址
	DMA1_Channel5->CNDTR=0;					//定义DMA通道的传输数量
	DMA1_Channel5->CCR=0X00000000;	//复位
	DMA1_Channel5->CCR|=0<<4;  			//从外设读
	DMA1_Channel5->CCR|=0<<5;  			//普通模式
	DMA1_Channel5->CCR|=0<<6; 			//外设地址非增量模式
	DMA1_Channel5->CCR|=1<<7; 	 		//存储器增量模式
	DMA1_Channel5->CCR|=0<<8; 	 		//外设数据宽度为8位
	DMA1_Channel5->CCR|=0<<10; 			//存储器数据宽度8位
	DMA1_Channel5->CCR|=1<<12; 			//中等优先级
	DMA1_Channel5->CCR|=0<<14; 			//非存储器到存储器模式
}

//DMA1通道1开始
void DMA1_CH1_Start(uint16_t len)
{
	DMA1_Channel1->CNDTR=len;									// 定义DMA通道的传输数量
	DMA1_Channel1->CCR |= (uint16_t)0x0001;		// 使能DMA传输
}

//DMA1通道1停止
void DMA1_CH1_Stop(void)
{
	DMA1_Channel1->CCR &= ~(1<<0);	// 关闭DMA1通道1
}

//DMA1通道4开始
void DMA1_CH4_Start(uint16_t len)
{
	DMA1_Channel4->CNDTR=len;									// 定义DMA通道的传输数量
	DMA1_Channel4->CCR |= (uint16_t)0x0001;		// 使能DMA传输
}

//DMA1通道4停止
void DMA1_CH4_Stop(void)
{
	DMA1_Channel4->CCR &= ~(1<<0);	// 关闭DMA1通道4
}

//DMA1通道5开始
void DMA1_CH5_Start(uint16_t len)
{
	DMA1_Channel5->CNDTR=len;									// 定义DMA通道的传输数量
	DMA1_Channel5->CCR |= (uint16_t)0x0001;		// 使能DMA传输
}

//DMA1通道5停止
void DMA1_CH5_Stop(void)
{
	DMA1_Channel5->CCR &= ~(1<<0);	// 关闭DMA1通道5
}

// 设置DMA传输完成中断回调函数
void DMA1_CH1_TC_CallBack(DMA_CallBack_Ptr tc_cb)
{
	DMA1_CH1_TC_CallBack_Ptr = tc_cb;
}

// 设置DMA传输完成中断回调函数
void DMA1_CH4_TC_CallBack(DMA_CallBack_Ptr tc_cb)
{
	DMA1_CH4_TC_CallBack_Ptr = tc_cb;
}

// DMA1通道1中断服务函数
void DMA1_Channel1_IRQHandler(void)
{
	// 是否发生传输完成中断
	if ( 0 != (DMA1->ISR & (1<<1)) )
	{
		DMA1->IFCR |= (1<<1);	// 清除中断标志
		DMA1_CH1_TC_CallBack_Ptr();	// 执行回调函数
	}
}


// DMA1 CH4中断服务函数
void DMA1_Channel4_IRQHandler(void)
{
	// 是否发生传输完成中断
	if ( 0 != (DMA1->ISR & (1<<13)) )
	{
		DMA1->IFCR |= (1<<13);	// 清除中断标志
		DMA1_CH4_TC_CallBack_Ptr();	// 执行回调函数
	}
}


