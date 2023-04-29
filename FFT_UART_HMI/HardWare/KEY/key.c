#include "key.h"
#include "delay.h"

// 定义按键对象
KEY_Object_T KEY1_Obj, KEY2_Obj, KEY3_Obj;


// 按键初始化函数
void KEY_Init(void)
{
	// 按键IO设置
	RCC->APB2ENR|=1<<2;    		// 使能PORTA时钟	 
	GPIOA->CRL&=0XFF000FFF; 	// PA3/4/5输入
	GPIOA->CRL|=0X00888000;	
  GPIOA->ODR|=7<<3;					// PA3/4/5输入上拉
	
	KEY1_Obj.state = KEY_Up;	// 初始化按键1对象状态
	KEY2_Obj.state = KEY_Up;	// 初始化按键2对象状态
	KEY3_Obj.state = KEY_Up;	// 初始化按键3对象状态
	
	KEY1_Obj.action = KEY_Action_None;	// 初始化按键动作
	KEY2_Obj.action = KEY_Action_None;	// 初始化按键动作
	KEY3_Obj.action = KEY_Action_None;	// 初始化按键动作
}


// 按键状态机
void KEY_StateMachine(KEY_Object_T* key, uint8_t pin)
{
	switch(key->state)
	{
		case KEY_Up: {									// 按键已抬起状态
			if (0 == pin)									// 按键引脚低电平
			{		
				key->state = KEY_Pre_Down;	// 切换至按键预按下状态
				key->cnt = 0;								// 清空延时计数值
			}
		} break;
		
		case KEY_Pre_Down: {						// 按键预按下状态
			if (10 == key->cnt)						// 延时计数达到10ms
			{
				if (0 != pin)								// pin脚为高电平
				{
					key->state = KEY_Up;			// 切换回按键已抬起状态
				}
				else
				{
					key->state = KEY_Down;					// 切换至按键已按下状态
					key->action = KEY_Action_Down;	// 按键按下动作
				}
			}
			key->cnt++;
		} break;
		
		case KEY_Down: {								// 按键已按下状态
			if (0 != pin)									// 按键引脚高电平
			{	
				key->state = KEY_Pre_Up;		// 切换至按键预抬起状态
				key->cnt = 0;								// 清空延时计数值
			}
		} break;
		
		case KEY_Pre_Up: {							// 按键预抬起状态
			if (10 == key->cnt)						// 延时计数达到10ms
			{
				if (0 == pin)								// pin脚为低电平
				{
					key->state = KEY_Down;		// 切换回按键已按下状态
				}
				else
				{
					key->state = KEY_Up;					// 切换至按键已按下状态
					key->action = KEY_Action_Up;	// 按键抬起动作
				}
			}
			key->cnt++;
		} break;
	}
}

// 获取按键标志，返回0
KEY_Action_E KEY_Get_Flag(KEY_Object_T* key)
{
	KEY_Action_E action = KEY_Action_None;
	
	if (KEY_Action_None != key->action)
	{
		action = key->action;
		key->action = KEY_Action_None;
	}
	
	return action;
}

