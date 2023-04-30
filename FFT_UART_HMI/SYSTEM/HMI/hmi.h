#ifndef __HMI_H
#define __HMI_H

#include "config.h"
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

/* 频谱仪子系统状态枚举定义 */
typedef enum
{
	FFT_SUB_SendCMD = 0,	/* 透传命令发送状态 */
	FFT_SUB_SendData,			/* 透传数据发送状态 */
	FFT_SUB_Default,			/* 初始状态 */
	FFT_SUB_Exit					/* 退出状态 */
}FFT_SUB_STATE_E;

/* 菜单子系统状态枚举定义 */
typedef enum
{
	Menu_SUB_FFT = 0,			/* 频谱仪选择状态 */
	Menu_SUB_Setting,			/* 设置选择状态 */
	Menu_SUB_Default,			/* 初始状态 */
	Menu_SUB_Exit					/* 退出状态 */
}Menu_SUB_STATE_E;

// 频谱仪子系统对象结构体类型定义
typedef struct
{
	FFT_SUB_STATE_E state;	// 子系统状态
	uint8_t exit_signal;		// 子系统退出信号
	uint8_t cursor_update;	// 游标更新状态
}FFT_SUB_Object_T;


// 菜单子系统对象结构体类型定义
typedef struct
{
	Menu_SUB_STATE_E state;	// 子系统状态
	uint8_t exit_signal;		// 子系统退出信号
}Menu_SUB_Object_T;

extern HMI_STATE_E hmi_state;
extern uint32_t lBufInArray[NPT];				/* FFT 运算的输入数组 */
extern uint32_t lBufOutArray[NPT/2];		/* FFT 运算的输出数组 */
extern uint32_t lBufMagArray[NPT/2];		/* 各谐波分量的幅值 */

void HMI_StateMachine(void);						// HMI系统状态机
void FFT_SUB_StateMachine(FFT_SUB_Object_T* sub);		// 频谱仪子系统状态机
void Menu_SUB_StateMachine(Menu_SUB_Object_T* sub);	// 菜单子系统状态机
void GetPowerMag(void);
void Creat_Single(void);
uint16_t GetMaxIndex(void);		// 获取最大幅度对应频率

#endif

