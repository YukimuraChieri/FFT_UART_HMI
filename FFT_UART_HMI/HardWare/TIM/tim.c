#include "tim.h"

// 定时器3初始化
void TIM3_TRGO_Init(void)
{
	RCC->APB1ENR|=1<<1;		// TIM3时钟使能
	TIM3->ARR=3599;  				// 设定计数器自动重装值   
	TIM3->PSC=0;  				// 预分频器
	TIM3->CR2|=(2<<4);		// 定时器3更新事件触发TRGO
	TIM3->CR1|=0x01;    	// 使能定时器3
}

// 设置TIM3触发频率
void TIM3_TRGO_Freq(float freq)
{
	uint16_t arr;
	
	TIM3->CR1 &= ~(uint16_t)0x01;	// 关闭定时器3
	
	arr = ((uint16_t)(72E6 / freq)) - 1;
	
	TIM3->ARR = arr;
	
	TIM3->CR1 |= 0x01;    				// 使能定时器3
}

