#include "hmi.h"
#include "key.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "stm32_dsp.h"
#include "table_fft.h"
#include "math.h"
#include "tim.h"

uint32_t lBufInArray[NPT];		/* FFT 运算的输入数组 */
uint32_t lBufOutArray[NPT/2];	/* FFT 运算的输出数组 */
uint32_t lBufMagArray[NPT/2];	/* 各谐波分量的幅值 */

HMI_STATE_E hmi_state = HMI_Default;
SUB_Object_T sub_spectrum;			// 频谱仪子系统对象
SUB_Object_T sub_oscilloscope;	// 示波器子系统对象
static char hmi_cmd[32];
static uint8_t send_data[256];
static uint8_t sampling_freq = 20;	// 采样频率，单位kHz
static uint16_t npt_pos = 0;				// FFT坐标点位置
static float npt_freq = 0;				// FFT坐标点对应频率
static uint16_t npt_power = 0;		// FFT坐标点对应幅值

// HMI系统状态机
void HMI_StateMachine(void)
{
	static uint16_t cnt = 0;
		
	switch(hmi_state)
	{
		case HMI_Start: {		// 开始界面
			if (1500 == cnt)	// 延时1500ms
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			cnt++;
		} break;
		
		case HMI_Menu: {	// 菜单界面
			if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// 按键1按下动作
			{
				hmi_state = HMI_Spectrum;	// 切换到频谱仪界面
				sprintf(hmi_cmd, "page 2\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sub_spectrum.state = SUB_Default;
				sub_spectrum.exit_signal = 0;
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// 按键2按下动作
			{
				hmi_state = HMI_Oscilloscope;	// 切换到示波器界面
				sprintf(hmi_cmd, "page 3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sub_oscilloscope.state = SUB_Default;
				sub_oscilloscope.exit_signal = 0;				
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// 按键3按下动作
			{
				hmi_state = HMI_Setting;	// 切换到设置界面
				sprintf(hmi_cmd, "page 4\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case HMI_Spectrum: {	// 频谱仪界面
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// 按键3按下动作
			{
				sub_spectrum.exit_signal = 1;		// 发送频谱仪子系统中止信号
			}
			else if (SUB_Exit == sub_spectrum.state)	// 子系统退出
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_StateMachine(&sub_spectrum);	// 频谱仪子系统状态机
			}
		} break;
		
		case HMI_Oscilloscope: {	// 示波器界面
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// 按键3按下动作
			{
				sub_oscilloscope.exit_signal = 1;	// 发送示波器子系统中止信号
			}
			else if (SUB_Exit == sub_oscilloscope.state)		// 子系统退出
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_StateMachine(&sub_oscilloscope);	// 示波器子系统状态机
			}
		} break;
		
		case HMI_Setting: {	// 设置界面
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// 按键3按下动作
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// 按键2按下动作
			{
				if (sampling_freq < 80)	// 采样频率上限80kHz
				{
					sampling_freq ++;	// 采样频率加1kHz
				}
				TIM3_TRGO_Freq((uint32_t)sampling_freq * 1000);	// 设置TIM3触发频率
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", sampling_freq);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// 按键1按下动作
			{
				if (sampling_freq > 10)	// 采样频率下限10kHz
				{
					sampling_freq --;	// 采样频率减1kHz
				}
				TIM3_TRGO_Freq((uint32_t)sampling_freq * 1000);	// 设置TIM3触发频率
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", sampling_freq);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
		} break;
		
		default: {					// 初始状态
			hmi_state = HMI_Start;		// 切换到开始界面
			sprintf(hmi_cmd, "page 0\xFF\xFF\xFF");
			UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
		} break;
	}
}


void SUB_StateMachine(SUB_Object_T* sub)	// 子系统状态机
{
	uint8_t rx_data[32];
	uint16_t i;
	
	switch(sub->state)
	{
		case SUB_SendCMD: {		// 命令发送状态
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// 接收到显示屏的应答
			{
				if (0 == memcmp(rx_data, "\xFE\xFF\xFF\xFF", 4))	// 是否为透传就绪
				{
					sub->state = SUB_SendData;	// 切换到发送数据状态
					
					if (HMI_Spectrum == hmi_state)	// 频谱仪数据
					{
						//填充数组
						for(i = 0; i < NPT; i++)
						{
							/******************************************************************
								这里因为单片机的ADC只能测正的电压 所以需要前级加直流偏执
								加入直流偏执后 软件上减去2048即一半 达到负半周期测量的目的	
							******************************************************************/
							lBufInArray[i] = (((int16_t)adc_buf[i])-2048) << 16;
						}
//						Creat_Single();
						//cr4_fft_1024_stm32(lBufOutArray, lBufInArray, NPT);		//FFT变换
						cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
						GetPowerMag();																					//取直流分量对应的AD值
						for(i=0; i<NPT/2; i++)
						{
							send_data[2*i] = 0;
							send_data[2*i+1] = (uint8_t)(lBufMagArray[i]/6);
						}
					}
					else if (HMI_Oscilloscope == hmi_state)		// 示波器数据
					{
						for(i=0; i<NPT; i++)
						{
							send_data[i] = (uint8_t)(adc_buf[i]/30);
						}
					}
					UART1_TX_Bytes(send_data, NPT);
				}
			}
		} break;
		
		case SUB_SendData: {	// 数据发送状态
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// 接收到显示屏的应答
			{
				if (0 == memcmp(rx_data, "\xFD\xFF\xFF\xFF", 4))	// 是否为透传完成
				{
					sub->state = SUB_Default;	// 切换到初始状态
					DMA1_CH1_Start(NPT);	// 启动DMA1 CH1
				}
			}
		} break;
		
		case SUB_Default: {		// 初始状态
			if (0 != sub->exit_signal)		// 接收到中止信号
			{
				sub->state = SUB_Exit;			// 子系统退出
				sub->exit_signal = 0;				// 退出完成，中止信号置0
			}
			else if (0 != adc_dma_flag)
			{
				sub->state = SUB_SendCMD;		// 切换到发送命令状态
				adc_dma_flag = 0;						// 传输完成标志置0
				DMA1_CH1_Stop();						// DMA1通道1停止
				// 发送数据透传命令
				sprintf(hmi_cmd, "addt s0.id,0,%d\xFF\xFF\xFF", NPT);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}	
			else if (HMI_Spectrum == hmi_state)	// 频谱仪功能
			{
				if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// 按键2按下动作
				{
					if (npt_pos < 255)	// npt坐标点上限255
					{
						npt_pos ++;				// npt坐标点加1
					}
					npt_freq = (float)sampling_freq * 1000 / NPT * npt_pos;	// FFT坐标点对应频率
					npt_power = lBufMagArray[npt_pos];								// FFT坐标点对应幅值
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%d\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// 按键1按下动作
				{
					if (npt_pos > 0)	// npt坐标点下限0
					{
						npt_pos --;			// npt坐标点减1
					}
					npt_freq = (float)sampling_freq * 1000 / NPT * npt_pos;	// FFT坐标点对应频率
					npt_power = lBufMagArray[npt_pos];								// FFT坐标点对应幅值
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%d\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		case SUB_Exit: {		// 退出状态
		} break;
	}
}


//获取FFT后的谐波分量
void GetPowerMag(void)
{
	int16_t lX,lY;
	float X,Y,Mag;
	int16_t i;
	for(i=0; i<NPT/2; i++)
	{
		lX  = (lBufOutArray[i] << 16) >> 16;
		lY  = (lBufOutArray[i] >> 16);
	
		//除以32768再乘65536是为了符合浮点数计算规律
		X = NPT * ((float)lX) / 32768;
		Y = NPT * ((float)lY) / 32768;
		Mag = sqrt(X * X + Y * Y)*1.0/ NPT;
		if(i == 0)
		{
			lBufMagArray[i] = (uint32_t)(Mag * 32768);
		}
		else
		{
			lBufMagArray[i] = (uint32_t)(Mag * 65536);
		}
	}
}

/************FFT相关*****************/
//测试用 生成一个信号
void Creat_Single(void)
{
	u16 i = 0;
	float fx=0.0;
	
	for(i=0; i<NPT; i++)
	{
		fx = 2048+2048*sin(PI2 * i * 500.0 / Fs)+
				 1024*sin(PI2 * i * 1000.0 / Fs)+
				 512*sin(PI2 * i * 5000.0 / Fs);
		lBufInArray[i] = ((signed short)fx) << 16;	
	}
}

