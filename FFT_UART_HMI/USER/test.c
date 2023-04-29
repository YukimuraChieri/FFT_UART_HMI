#include "sys.h"
#include "usart.h"		
#include "delay.h"
#include "led.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "key.h"
#include "hmi.h"

int main(void)
{	
	Stm32_Clock_Init(9);		// ϵͳʱ������
	delay_init(72);					// ��ʱ��ʼ��
	UART1_Init(72, 115200);	// ����1��ʼ��Ϊ115200
	LED_Init();							// ��ʼ��LED
	ADC1_Init();						// ��ʼ��ADC1
	KEY_Init();							// ������ʼ������
	
	DMA1_CH1_Init((uint32_t)&ADC1->DR, (uint32_t)&adc_buf);		// ��ʼ��DMA1ͨ��1(ADC1)
	DMA1_CH4_Init((uint32_t)&USART1->DR, (uint32_t)&tx_buff);	// ��ʼ��DMA1ͨ��4(UART1_TX)
	DMA1_CH5_Init((uint32_t)&USART1->DR, (uint32_t)&rx_buff);	// ��ʼ��DMA1ͨ��5(UART1_RX)
	
	DMA1_CH1_TC_CallBack(ADC1_DMA_CallBack);			// ����DMA��������жϻص�����
	DMA1_CH4_TC_CallBack(UART1_TX_DMA_CallBack);	// ����DMA��������жϻص�����
	
	DMA1_CH1_Start(NPT);
	DMA1_CH5_Start(UART1_RX_BUFF_SIZE);
	
	TIM3_TRGO_Init(71, 49);	// ��ʱ������Ƶ��=72MHz/((71+1)*(49+1))=20kHz
	
  while(1)
	{
		KEY_StateMachine(&KEY1_Obj, KEY1_Pin);	// ����1״̬��
		KEY_StateMachine(&KEY2_Obj, KEY2_Pin);	// ����2״̬��
		KEY_StateMachine(&KEY3_Obj, KEY3_Pin);	// ����3״̬��
		
		HMI_StateMachine();		// HMIϵͳ״̬��
		
		delay_ms(1);					// ��ѭ������1ms
	}
}

