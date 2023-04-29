#include "adc.h"
#include "delay.h"

// �������ADCת�������Ҳ��DMA��Ŀ���ַ,��ͨ���ɼ�256��
uint16_t adc_buf[NPT] = {0};
// ADC DMA������ɱ�־
uint8_t adc_dma_flag = 0;

// ��ʼ��ADC1																	   
void ADC1_Init(void)
{
	/* enable GPIOA clock */
 	RCC->APB2ENR|=1<<2;
	
	/* connect port to ADC1_CH0 */
	GPIOA->CRL&=0XFFFFFFF0;
	
	/* enable ADC0 clock */
	RCC->APB2ENR|=1<<9;    	//ADC1ʱ��ʹ��
	RCC->APB2RSTR|=1<<9;   	//ADC1��λ
	RCC->APB2RSTR&=~(1<<9);	//��λ����	    
	RCC->CFGR&=~(3<<14);   	//��Ƶ��������
	
	//SYSCLK/DIV2=12M ADCʱ������Ϊ12M,ADC���ʱ�Ӳ��ܳ���14M!
	//���򽫵���ADC׼ȷ���½�! 
	RCC->CFGR |=	2<<14;      	 
	ADC1->CR1 &=	0XF0FFFF;	//����ģʽ����
	ADC1->CR1 |=	0<<16;   	//��������ģʽ  
	ADC1->CR1 |=	(1<<8); 	//ʹ��ɨ��ģʽ	  
	ADC1->CR2 &=	~(1<<1); 	//����ת��ģʽ
	ADC1->CR2 |=	1<<20;   	//ʹ�����ⲿ����(SWSTART)!!!	����ʹ��һ���¼�������
	ADC1->CR2 &=	~(1<<11);	//�Ҷ���	 
	ADC1->SQR1 &=	~(0XF<<20);
	ADC1->SQR1 |=	1<<20;		//8��ת���ڹ��������� Ҳ����ֻת����������1 		
	
	//����ͨ��0�Ĳ���ʱ��
	ADC1->SMPR2&=~(7<<(3*0));	//ͨ��0����ʱ�����	  
 	ADC1->SMPR2|=6<<(3*0);		//ͨ��0 71.5����,��߲���ʱ�������߾�ȷ��	 
	
	//���ù�������
	ADC1->SQR3 &= ~(0x1F<<(5*0));	// ��չ������еĵ�1��ת��ͨ��
	ADC1->SQR3 |= (0<<(5*0));			// ���ù������еĵ�1��ת��ͨ��
	
//	ADC1->SQR3 &= ~(0x1F<<(5*1));	// ��չ������еĵ�2��ת��ͨ��
//	ADC1->SQR3 |= (0<<(5*1));			// ���ù������еĵ�2��ת��ͨ��
//	
//	ADC1->SQR3 &= ~(0x1F<<(5*2));	// ��չ������еĵ�3��ת��ͨ��
//	ADC1->SQR3 |= (0<<(5*2));			// ���ù������еĵ�3��ת��ͨ��
//	
//	ADC1->SQR3 &= ~(0x1F<<(5*3));	// ��չ������еĵ�4��ת��ͨ��
//	ADC1->SQR3 |= (0<<(5*3));			// ���ù������еĵ�4��ת��ͨ��
	
	ADC1->CR2 &=	~(7<<17);	
	ADC1->CR2 |= (4<<17);		// ��ʱ��3��TRGO�¼���������ͨ��ת��
	ADC1->CR2 |= (1<<20);		// ʹ���ⲿ�¼�����ת��
	
	ADC1->CR2 |= (1<<8);		// ʹ��DMAģʽ
	
	ADC1->CR2|=1<<0;				//����ADת����
	ADC1->CR2|=1<<3;				//ʹ�ܸ�λУ׼  
	while(ADC1->CR2&1<<3);	//�ȴ�У׼���� 			 
  //��λ���������ò���Ӳ���������У׼�Ĵ�������ʼ�����λ��������� 		 
	ADC1->CR2|=1<<2;        //����ADУ׼	   
	while(ADC1->CR2&1<<2);  //�ȴ�У׼����
	//��λ�����������Կ�ʼУ׼������У׼����ʱ��Ӳ�����  
}

// ADC1 DMA�ص�����
void ADC1_DMA_CallBack(void)
{
	adc_dma_flag = 1;
}
