#ifndef __ADC_H
#define __ADC_H	

#include "sys.h"
#include "config.h"
#include <stdio.h>
								  
#define ADC_CH0  0	//通道0(连接在PA0)

// 用来存放ADC转换结果，也是DMA的目标地址,单通道采集256次
extern uint16_t adc_buf[NPT];
extern uint8_t adc_dma_flag;	// ADC DMA传输完成标志

void ADC1_Init(void);					// ADC1通道0初始化
void ADC1_DMA_CallBack(void);	// ADC1 DMA回调函数

#endif 















