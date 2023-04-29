#include "dma.h"
#include "delay.h"
#include <stdio.h>

// DMA�жϻص�����ָ��
DMA_CallBack_Ptr DMA1_CH1_TC_CallBack_Ptr = NULL;
DMA_CallBack_Ptr DMA1_CH4_TC_CallBack_Ptr = NULL;


// DMA1ͨ��1��ʼ������(ADC1)
void DMA1_CH1_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR |= 1<<0;			//����DMA1ʱ��
	delay_ms(5);							//�ȴ�DMAʱ���ȶ�
	
	/* initialize DMA channel1 */
	DMA1_Channel1->CCR = 0X00000000;	//��λ
	DMA1_Channel1->CCR |= 0<<4;  		//�������
	DMA1_Channel1->CCR |= 0<<5;  		//��ѭ������ģʽ
	DMA1_Channel1->CCR |= 0<<6; 		//�����ַ������ģʽ
	DMA1_Channel1->CCR |= 1<<7; 	 	//�洢������ģʽ
	DMA1_Channel1->CCR |= 1<<8; 	 	//�������ݿ��Ϊ16λ
	DMA1_Channel1->CCR |= 1<<10; 		//�洢�����ݿ��16λ
	DMA1_Channel1->CCR |= 2<<12; 		//�����ȼ�
	DMA1_Channel1->CCR |= 0<<14; 		//�Ǵ洢�����洢��ģʽ
	
	DMA1_Channel1->CNDTR = 0;    		// ����DMAͨ���Ĵ�������
	
	DMA1_Channel1->CPAR = PerAddr;	//����DMA�������ַ
	DMA1_Channel1->CMAR = MemAddr;	//����DMA�ڴ����ַ(ת���������ĵ�ַ)
	
	/* DMA interrupt configuration */
	DMA1_Channel1->CCR|=1<<1;				// ʹ��DMA��������ж�
	MY_NVIC_Init(1, 0, DMA1_Channel1_IRQn, 2);	//��ռ1�������ȼ�0����2	
}

// DMA1ͨ��4��ʼ������(UART1_TX)
void DMA1_CH4_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR|=1<<0;	//����DMA1ʱ��
	delay_ms(5);				//�ȴ�DMAʱ���ȶ�
	
	/* configure DMA mode */
	DMA1_Channel4->CPAR=PerAddr;	//DMA1 �����ַ 
	DMA1_Channel4->CMAR=MemAddr;	//DMA1,�洢����ַ
	DMA1_Channel4->CNDTR=0;				// ����DMAͨ���Ĵ�������
	DMA1_Channel4->CCR=0X00000000;	//��λ
	DMA1_Channel4->CCR|=1<<4;  			//�Ӵ洢����
	DMA1_Channel4->CCR|=0<<5;  			//��ͨģʽ
	DMA1_Channel4->CCR|=0<<6; 			//�����ַ������ģʽ
	DMA1_Channel4->CCR|=1<<7; 	 		//�洢������ģʽ
	DMA1_Channel4->CCR|=0<<8; 	 		//�������ݿ��Ϊ8λ
	DMA1_Channel4->CCR|=0<<10; 			//�洢�����ݿ��8λ
	DMA1_Channel4->CCR|=1<<12; 			//�е����ȼ�
	DMA1_Channel4->CCR|=0<<14; 			//�Ǵ洢�����洢��ģʽ
	
	/* DMA interrupt configuration */
	DMA1_Channel4->CCR|=1<<1;				// ʹ��DMA��������ж�
	MY_NVIC_Init(1, 0, DMA1_Channel4_IRQn, 2);	//��ռ1�������ȼ�0����2	
}


// DMA1ͨ��5��ʼ������(UART1_RX)
void DMA1_CH5_Init(uint32_t PerAddr, uint32_t MemAddr)
{
	/* enable DMA clock */
	RCC->AHBENR|=1<<0;	//����DMA1ʱ��
	delay_ms(5);				//�ȴ�DMAʱ���ȶ�
	
	/* configure DMA mode */
	DMA1_Channel5->CPAR=PerAddr;		//DMA1 �����ַ 
	DMA1_Channel5->CMAR=MemAddr;		//DMA1,�洢����ַ
	DMA1_Channel5->CNDTR=0;					//����DMAͨ���Ĵ�������
	DMA1_Channel5->CCR=0X00000000;	//��λ
	DMA1_Channel5->CCR|=0<<4;  			//�������
	DMA1_Channel5->CCR|=0<<5;  			//��ͨģʽ
	DMA1_Channel5->CCR|=0<<6; 			//�����ַ������ģʽ
	DMA1_Channel5->CCR|=1<<7; 	 		//�洢������ģʽ
	DMA1_Channel5->CCR|=0<<8; 	 		//�������ݿ��Ϊ8λ
	DMA1_Channel5->CCR|=0<<10; 			//�洢�����ݿ��8λ
	DMA1_Channel5->CCR|=1<<12; 			//�е����ȼ�
	DMA1_Channel5->CCR|=0<<14; 			//�Ǵ洢�����洢��ģʽ
}

//DMA1ͨ��1��ʼ
void DMA1_CH1_Start(uint16_t len)
{
	DMA1_Channel1->CNDTR=len;									// ����DMAͨ���Ĵ�������
	DMA1_Channel1->CCR |= (uint16_t)0x0001;		// ʹ��DMA����
}

//DMA1ͨ��1ֹͣ
void DMA1_CH1_Stop(void)
{
	DMA1_Channel1->CCR &= ~(1<<0);	// �ر�DMA1ͨ��1
}

//DMA1ͨ��4��ʼ
void DMA1_CH4_Start(uint16_t len)
{
	DMA1_Channel4->CNDTR=len;									// ����DMAͨ���Ĵ�������
	DMA1_Channel4->CCR |= (uint16_t)0x0001;		// ʹ��DMA����
}

//DMA1ͨ��4ֹͣ
void DMA1_CH4_Stop(void)
{
	DMA1_Channel4->CCR &= ~(1<<0);	// �ر�DMA1ͨ��4
}

//DMA1ͨ��5��ʼ
void DMA1_CH5_Start(uint16_t len)
{
	DMA1_Channel5->CNDTR=len;									// ����DMAͨ���Ĵ�������
	DMA1_Channel5->CCR |= (uint16_t)0x0001;		// ʹ��DMA����
}

//DMA1ͨ��5ֹͣ
void DMA1_CH5_Stop(void)
{
	DMA1_Channel5->CCR &= ~(1<<0);	// �ر�DMA1ͨ��5
}

// ����DMA��������жϻص�����
void DMA1_CH1_TC_CallBack(DMA_CallBack_Ptr tc_cb)
{
	DMA1_CH1_TC_CallBack_Ptr = tc_cb;
}

// ����DMA��������жϻص�����
void DMA1_CH4_TC_CallBack(DMA_CallBack_Ptr tc_cb)
{
	DMA1_CH4_TC_CallBack_Ptr = tc_cb;
}

// DMA1ͨ��1�жϷ�����
void DMA1_Channel1_IRQHandler(void)
{
	// �Ƿ�����������ж�
	if ( 0 != (DMA1->ISR & (1<<1)) )
	{
		DMA1->IFCR |= (1<<1);	// ����жϱ�־
		DMA1_CH1_TC_CallBack_Ptr();	// ִ�лص�����
	}
}


// DMA1 CH4�жϷ�����
void DMA1_Channel4_IRQHandler(void)
{
	// �Ƿ�����������ж�
	if ( 0 != (DMA1->ISR & (1<<13)) )
	{
		DMA1->IFCR |= (1<<13);	// ����жϱ�־
		DMA1_CH4_TC_CallBack_Ptr();	// ִ�лص�����
	}
}


