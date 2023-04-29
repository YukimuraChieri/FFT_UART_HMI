#ifndef __DMA_H
#define __DMA_H

#include "sys.h"

typedef void(*DMA_CallBack_Ptr)(void);

void DMA1_CH1_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1通道1初始化函数(ADC1)
void DMA1_CH4_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1通道4初始化函数(UART1_TX)
void DMA1_CH5_Init(uint32_t PerAddr, uint32_t MemAddr);	// DMA1通道5初始化函数(UART1_RX)

void DMA1_CH1_Start(uint16_t len);		//DMA1通道1开始
void DMA1_CH1_Stop(void);							//DMA1通道1停止

void DMA1_CH4_Start(uint16_t len);		//DMA1通道4开始
void DMA1_CH4_Stop(void);							//DMA1通道4停止

void DMA1_CH5_Start(uint16_t len);		//DMA1通道5开始
void DMA1_CH5_Stop(void);							//DMA1通道5停止

void DMA1_CH1_TC_CallBack(DMA_CallBack_Ptr tc_cb);	// 设置DMA传输完成中断回调函数
void DMA1_CH4_TC_CallBack(DMA_CallBack_Ptr tc_cb);	// 设置DMA传输完成中断回调函数


#endif

