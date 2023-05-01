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
	SUB_FFT_SendCMD = 0,	/* 透传命令发送状态 */
	SUB_FFT_SendData,			/* 透传数据发送状态 */
	SUB_FFT_Default,			/* 初始状态 */
	SUB_FFT_Exit					/* 退出状态 */
}SUB_FFT_STATE_E;

/* 菜单子系统状态枚举定义 */
typedef enum
{
	SUB_Menu_FFT = 0,			/* 频谱仪选择状态 */
	SUB_Menu_Setting,			/* 设置选择状态 */
	SUB_Menu_Default,			/* 初始状态 */
	SUB_Menu_Exit					/* 退出状态 */
}SUB_Menu_STATE_E;

/* 设置子系统状态枚举定义 */
typedef enum
{
	SUB_Set_Sampling_Item = 0,	/* 采样频率项目状态 */
	SUB_Set_Sampling_Selet,			/* 采样频率选中状态 */
	SUB_Set_Window_Item,				/* 窗函数项目状态 */
	SUB_Set_Window_Selet,				/* 窗函数选中状态 */
	SUB_Set_Default,						/* 初始状态 */
	SUB_Set_Exit								/* 退出状态 */
}SUB_Set_STATE_E;


// 频谱仪子系统对象结构体类型定义
typedef struct
{
	SUB_FFT_STATE_E state;	// 子系统状态
	uint8_t exit_signal;		// 子系统退出信号
	uint8_t cursor_update;	// 游标更新状态
}SUB_FFT_Object_T;


// 菜单子系统对象结构体类型定义
typedef struct
{
	SUB_Menu_STATE_E state;	// 子系统状态
	uint8_t exit_signal;		// 子系统退出信号
}SUB_Menu_Object_T;


// 设置子系统对象结构体类型定义
typedef struct
{
	SUB_Set_STATE_E state;	// 子系统状态
	uint8_t exit_signal;		// 子系统退出信号
}SUB_Set_Object_T;


extern HMI_STATE_E hmi_state;
extern uint32_t lBufInArray[NPT];				/* FFT 运算的输入数组 */
extern uint32_t lBufOutArray[NPT/2];		/* FFT 运算的输出数组 */
extern uint32_t lBufMagArray[NPT/2];		/* 各谐波分量的幅值 */

void HMI_StateMachine(void);						// HMI系统状态机
void SUB_FFT_StateMachine(SUB_FFT_Object_T* sub);		// 频谱仪子系统状态机
void SUB_Menu_StateMachine(SUB_Menu_Object_T* sub);	// 菜单子系统状态机
void SUB_Set_StateMachine(SUB_Set_Object_T* sub);		// 设置子系统状态机
void GetPowerMag(void);
void Creat_Single(void);
uint16_t GetMaxIndex(void);		// 获取最大幅度对应频率

#endif

