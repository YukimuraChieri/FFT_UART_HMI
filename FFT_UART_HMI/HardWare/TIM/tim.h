#ifndef __TIM_H
#define __TIM_H

#include "sys.h"

void TIM3_TRGO_Init(uint16_t psc, uint16_t arr);	// ��ʱ��3��ʼ��
void TIM3_TRGO_Freq(uint32_t freq);		// ����TIM3����Ƶ��


#endif

