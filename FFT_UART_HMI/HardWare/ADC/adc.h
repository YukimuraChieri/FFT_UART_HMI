#ifndef __ADC_H
#define __ADC_H	

#include "sys.h"
#include "config.h"
#include <stdio.h>
								  
#define ADC_CH0  0	//ͨ��0(������PA0)

// �������ADCת�������Ҳ��DMA��Ŀ���ַ,��ͨ���ɼ�256��
extern uint16_t adc_buf[NPT];
extern uint8_t adc_dma_flag;	// ADC DMA������ɱ�־

void ADC1_Init(void);					// ADC1ͨ��0��ʼ��
void ADC1_DMA_CallBack(void);	// ADC1 DMA�ص�����

#endif 















