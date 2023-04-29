#include "tim.h"

// ��ʱ��3��ʼ��
void TIM3_TRGO_Init(void)
{
	RCC->APB1ENR|=1<<1;		// TIM3ʱ��ʹ��
	TIM3->ARR=3599;  				// �趨�������Զ���װֵ   
	TIM3->PSC=0;  				// Ԥ��Ƶ��
	TIM3->CR2|=(2<<4);		// ��ʱ��3�����¼�����TRGO
	TIM3->CR1|=0x01;    	// ʹ�ܶ�ʱ��3
}

// ����TIM3����Ƶ��
void TIM3_TRGO_Freq(float freq)
{
	uint16_t arr;
	
	TIM3->CR1 &= ~(uint16_t)0x01;	// �رն�ʱ��3
	
	arr = ((uint16_t)(72E6 / freq)) - 1;
	
	TIM3->ARR = arr;
	
	TIM3->CR1 |= 0x01;    				// ʹ�ܶ�ʱ��3
}

