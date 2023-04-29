#ifndef __CONFIG_H
#define __CONFIG_H

// 系统配置头文件

#define	NPT  256							/* 采样点数 */
#define PI2 6.28318530717959
//采样率计算
//分辨率：Fs/NPT 
//#define Fs	10000
#define Fs	9984
//取9984能出来整数的分辨率 9984/256 = 39Hz

#endif

