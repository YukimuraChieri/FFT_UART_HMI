#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

#define KEY1_Pin	PAin(3)	// 按键开关1
#define KEY2_Pin	PAin(4)	// 按键开关2
#define KEY3_Pin	PAin(5)	// 按键开关3

// 按键状态枚举类型定义
typedef enum
{
	KEY_Up = 0,			// 按键已抬起
	KEY_Pre_Down,		// 按键预按下
	KEY_Down,				// 按键已按下
	KEY_Pre_Up			// 按键预抬起
}KEY_STATE_E;

// 按键动作枚举类型定义
typedef enum
{
	KEY_Action_Down = 0,	// 按键按下动作
	KEY_Action_Up,				// 按键抬起动作
	KEY_Action_None				// 按键无动作
}KEY_Action_E;

// 按键对象结构体类型定义
typedef struct
{
	KEY_STATE_E state;		// 按键状态
	uint16_t cnt;					// 延时计数
	KEY_Action_E action;	// 按键动作
}KEY_Object_T;


// 声明按键对象
extern KEY_Object_T KEY1_Obj, KEY2_Obj, KEY3_Obj;

void KEY_Init(void);				// 按键初始化函数
void KEY_StateMachine(KEY_Object_T* key, uint8_t pin);	// 按键状态机
KEY_Action_E KEY_Get_Flag(KEY_Object_T* key);	// 获取按键标志



#endif

