#include "encoder.h"

// 定义编码器按键对象
KEY_Object_T KEY_Enc_Obj;

// 初始化编码器
void Encoder_Init(void)
{
	// 编码器按键IO设置
	RCC->APB2ENR|=1<<2;    		// 使能PORTA时钟	 
	GPIOA->CRL&=0XFFFFFF0F; 	// PA1输入
	GPIOA->CRL|=0X00000080;	
  GPIOA->ODR|=1<<1;					// PA1输入上拉
	// 编码器相线IO设置
	RCC->APB2ENR|=1<<3;    		// 使能PORTB时钟	 
	GPIOB->CRL&=0X00FFFFFF; 	// PB6/7输入
	GPIOB->CRL|=0X88000000;	
  GPIOB->ODR|=3<<6;					// PB6/7输入上拉
	
	RCC->APB1ENR |= 1<<2;	// TIM4时钟使能
	TIM4->ARR = 0xFFFF;		// 设定计数器自动重装值   
	TIM4->PSC = 0;  			// 预分频器
	TIM4->CR1 &= ~(uint16_t)(1<<4);    // 定时器4向上计数
	TIM4->SMCR |= (1<<0);	// 编码器模式1
	TIM4->CCER |= (1<<0);	// 通道1输入使能
	TIM4->CCER |= (1<<4);	// 通道2输入使能
	TIM4->CCMR1 |= (1<<0);	// IC1映射TI1
	TIM4->CCMR1 |= (1<<8);	// IC2映射TI2
	TIM4->CCMR1 |= (6<<4);	// 输入捕获1滤波器设置
	
	TIM4->CNT = 0;				// 清空计数器
	TIM4->CR1 |= 0x01;    // 使能定时器4
	
	KEY_Enc_Obj.state = KEY_Up;	// 初始化编码器按键对象状态
	KEY_Enc_Obj.action = KEY_Action_None;	// 初始化按键动作
}


int16_t GetEncDelta(void)
{
	static int8_t last_pos = 0;
	int8_t current_pos = 0;
	int8_t retval = 0;
	
	current_pos = (int8_t)(TIM4->CNT >> 1);
	
	retval = (current_pos - last_pos);
	
	last_pos = current_pos;
	
	return retval;
}


