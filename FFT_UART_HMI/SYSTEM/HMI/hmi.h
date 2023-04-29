#ifndef __HMI_H
#define __HMI_H

#include <stdint.h>

/* HMI状态枚举定义 */
typedef enum
{
	HMI_Start = 0,		/* 开始界面 */
	HMI_Menu,					/* 菜单界面 */
	HMI_Spectrum,			/* 频谱仪界面 */
	HMI_Setting,			/* 设置界面 */
	HMI_Default				/* 初始状态 */
}HMI_STATE_E;

/* 子系统状态枚举定义 */
typedef enum
{
	SUB_SendCMD = 0,	/* 透传命令发送状态 */
	SUB_SendData,			/* 透传数据发送状态 */
	SUB_Default,			/* 初始状态 */
	SUB_Exit					/* 退出状态 */
}SUB_STATE_E;

// 子系统对象结构体类型定义
typedef struct
{
	SUB_STATE_E state;		// 子系统状态
	uint8_t exit_signal;	// 子系统退出信号
	uint8_t cursor_update;	// 游标更新状态
}SUB_Object_T;

extern HMI_STATE_E hmi_state;


void HMI_StateMachine(void);							// HMI系统状态机
void SUB_StateMachine(SUB_Object_T* sub);	// 子系统状态机
void GetPowerMag(void);
void Creat_Single(void);

#endif

