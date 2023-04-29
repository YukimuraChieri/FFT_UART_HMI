#ifndef __DMA_H
#define __DMA_H

#include "sys.h"

typedef void(*DMA_CallBack_Ptr)(void);

void DMA1_CH1_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1ͨ��1��ʼ������(ADC1)
void DMA1_CH4_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1ͨ��4��ʼ������(UART1_TX)
void DMA1_CH5_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1ͨ��5��ʼ������(UART1_RX)

void DMA1_CH1_Start(uint16_t len);		//DMA1ͨ��1��ʼ
void DMA1_CH1_Stop(void);							//DMA1ͨ��1ֹͣ

void DMA1_CH4_Start(uint16_t len);		//DMA1ͨ��4��ʼ
void DMA1_CH4_Stop(void);							//DMA1ͨ��4ֹͣ

void DMA1_CH5_Start(uint16_t len);		//DMA1ͨ��5��ʼ
void DMA1_CH5_Stop(void);							//DMA1ͨ��5ֹͣ

void DMA1_CH1_TC_CallBack(DMA_CallBack_Ptr tc_cb);	// ����DMA��������жϻص�����
void DMA1_CH4_TC_CallBack(DMA_CallBack_Ptr tc_cb);	// ����DMA��������жϻص�����


#endif

