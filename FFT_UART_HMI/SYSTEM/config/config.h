#ifndef __CONFIG_H
#define __CONFIG_H

// 频谱仪系统配置头文件

#define	NPT  256							/* 采样点数 */
#define PI2 6.28318530717959
//采样率计算
//分辨率：Fs/NPT 
#define Fs	20000
//#define Fs	9984
//取9984能出来整数的分辨率 9984/256 = 39Hz

// 最小采样频率(Hz)
#define MIN_SAM_FREQ 10000.0
// 最大采样频率(Hz)
#define MAX_SAM_FREQ 80000.0

// 曲线图高度
#define SCOPE_HIGH	190
// 曲线图宽度
#define SCOPE_WIDTH	256

#endif

