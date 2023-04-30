#ifndef __LED_H
#define __LED_H
#include "sys.h"

#define LED PCout(13)

void LED_Init(void);
void TIM3_Int_Init(u16 arr,u16 psc);

#endif

