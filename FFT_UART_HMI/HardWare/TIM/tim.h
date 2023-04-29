#ifndef __TIM_H
#define __TIM_H

#include "sys.h"

void TIM3_TRGO_Init(uint16_t psc, uint16_t arr);	// 定时器3初始化
void TIM3_TRGO_Freq(uint32_t freq);		// 设置TIM3触发频率


#endif

