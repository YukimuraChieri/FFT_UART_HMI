#include "led.h"

void LED_Init(void)
{
	RCC->APB2ENR|=1<<4;		//ʹ��PORTCʱ��
	RCC->APB2ENR|=1<<2;		//ʹ��PORTAʱ��
	
	GPIOC->CRH&=0XFF0FFFFF; 
	GPIOC->CRH|=0X00300000;//PC.13 �������   	 
  GPIOC->ODR|=1<<13;     //PC.13 �����
	
	GPIOA->CRL&=0XFFFFFFF0; 
	GPIOA->CRL|=0X00000003;//PA.0 �������   	 
  GPIOA->ODR|=1<<0;     //PA.0 �����
}
