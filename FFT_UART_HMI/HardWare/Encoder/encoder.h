#ifndef __ENCODER_H
#define __ENCODER_H

#include "sys.h"
#include "key.h"

#define KEY_Enc_Pin	PAin(1)	// 编码器按键开关

// 定义编码器按键对象
extern KEY_Object_T KEY_Enc_Obj;

void Encoder_Init(void);		// 初始化编码器
int16_t GetEncDelta(void);		// 获取编码器增量值


#endif

