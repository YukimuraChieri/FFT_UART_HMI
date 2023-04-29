#include "led.h"

void LED_Init(void)
{
	RCC->APB2ENR|=1<<4;		//使能PORTC时钟
	RCC->APB2ENR|=1<<2;		//使能PORTA时钟
	
	GPIOC->CRH&=0XFF0FFFFF; 
	GPIOC->CRH|=0X00300000;//PC.13 推挽输出   	 
  GPIOC->ODR|=1<<13;     //PC.13 输出高
	
	GPIOA->CRL&=0XFFFFFFF0; 
	GPIOA->CRL|=0X00000003;//PA.0 推挽输出   	 
  GPIOA->ODR|=1<<0;     //PA.0 输出高
}
