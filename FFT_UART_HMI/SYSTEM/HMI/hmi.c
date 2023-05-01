#include "hmi.h"
#include "key.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "stm32_dsp.h"
#include "table_fft.h"
#include "math.h"
#include "tim.h"
#include "includes.h"
#include "fft_windows.h"

extern OS_EVENT * msg_key1;				// 按键1邮箱事件块指针
extern OS_EVENT * msg_key2;				// 按键2邮箱事件块指针
extern OS_EVENT * msg_key3;				// 按键3邮箱事件块指针
extern OS_EVENT * msg_key_enc;		// 编码器按键邮箱事件块指针
extern OS_EVENT * msg_enc_delta;	// 编码器位置邮箱事件块指针
extern OS_EVENT * msg_cpuload;		// CPU负载邮箱事件块指针

uint32_t lBufInArray[NPT];			/* FFT 运算的输入数组 */
uint32_t lBufOutArray[NPT/2];		/* FFT 运算的输出数组 */
uint32_t lBufMagArray[NPT/2];		/* 各谐波分量的幅值 */

HMI_STATE_E hmi_state = HMI_Default;
SUB_FFT_Object_T sub_fft;				// 频谱仪子系统对象
SUB_Menu_Object_T sub_menu;			// 菜单子系统对象
SUB_Set_Object_T sub_set;				// 设置子系统对象

Window_E fft_win_type = Window_Rectangle;	// 窗函数

static char hmi_cmd[48];
static uint8_t send_data[256];
static float sampling_freq = 20000;	// 采样频率，单位Hz
static int16_t npt_pos = 0;					// FFT坐标点位置
static float npt_freq = 0;					// FFT坐标点对应频率
static float npt_power = 0;					// FFT坐标点对应幅值

static uint8_t err;
static int16_t enc_dalta = 0;
//static uint8_t x_zoom = 1;			// x轴缩放
static uint8_t y_zoom = 1;			// y轴缩放

// HMI系统状态机
void HMI_StateMachine(void)
{
	static uint16_t cnt = 0;
	
	switch(hmi_state)
	{
		case HMI_Start: {			// 开始界面
			if (500 == cnt)	// 延时500ms
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sub_menu.state = SUB_Menu_Default;
				sprintf(hmi_cmd, "page Menu\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
			{
				sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			cnt++;
		} break;
		
		case HMI_Menu: {			// 菜单界面
			if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				if (SUB_Menu_FFT == sub_menu.state)
				{
					hmi_state = HMI_Spectrum;	// 切换到频谱仪界面
					y_zoom = 1;
					sprintf(hmi_cmd, "page Spectrum\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sub_fft.state = SUB_FFT_Default;
					sub_fft.exit_signal = 0;
					sub_fft.cursor_update = 0;
					npt_pos = 0;
					sprintf(hmi_cmd, "f1.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f2.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*2);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f3.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*3);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f4.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f5.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*5);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f6.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*6);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (SUB_Menu_Setting == sub_menu.state)
				{
					hmi_state = HMI_Setting;	// 切换到设置界面
					sub_set.state = SUB_Set_Default;
					sprintf(hmi_cmd, "page Setting\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
			else
			{
				SUB_Menu_StateMachine(&sub_menu);	// 菜单子系统状态机
			}
			
			if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
			{
				sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case HMI_Spectrum: {	// 频谱仪界面
			if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub_fft.exit_signal = 1;		// 发送频谱仪子系统中止信号
			}
			else if (SUB_FFT_Exit == sub_fft.state)	// 子系统退出
			{
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sub_menu.state = SUB_Menu_Default;
				sprintf(hmi_cmd, "page Menu\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_FFT_StateMachine(&sub_fft);	// 频谱仪子系统状态机
			}
		} break;
		
		case HMI_Setting: {		// 设置界面
			if (SUB_Set_Exit == sub_set.state)	// 子系统退出
			{
				TIM3_TRGO_Freq(sampling_freq);		// 设置TIM3触发频率
				
				hmi_state = HMI_Menu;	// 切换到菜单界面
				sub_menu.state = SUB_Menu_Default;
				sprintf(hmi_cmd, "page Menu\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_Set_StateMachine(&sub_set);		// 设置子系统状态机
				
				if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
				{
					sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		default: {						// 初始状态
			hmi_state = HMI_Start;		// 切换到开始界面
			sprintf(hmi_cmd, "page Start\xFF\xFF\xFF");
			UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
		} break;
	}
}


// 频谱仪子系统状态机
void SUB_FFT_StateMachine(SUB_FFT_Object_T* sub)
{
	uint8_t rx_data[32];
	uint32_t temp = 0;
	uint16_t i;
	uint32_t adc_win = 0;
	
	switch(sub->state)
	{
		case SUB_FFT_SendCMD: {		// 透传命令发送状态
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// 接收到显示屏的应答
			{
				if (0 == memcmp(rx_data, "\xFE\xFF\xFF\xFF", 4))	// 是否为透传就绪
				{
					sub->state = SUB_FFT_SendData;	// 切换到发送数据状态
					if (0 != sub->cursor_update)	//填充游标数组
					{
						sub->cursor_update = 0;
						
						memset(send_data, 0x00, sizeof(send_data));
						send_data[2*npt_pos] = SCOPE_HIGH;
					}
					else	//填充频谱数组
					{
						if (Window_Rectangle == fft_win_type)		// 矩形窗
						{
							for(i = 0; i < NPT; i++)
							{
								/******************************************************************
									这里因为单片机的ADC只能测正的电压 所以需要前级加直流偏执
									加入直流偏执后 软件上减去2048即一半 达到负半周期测量的目的	
								******************************************************************/
								lBufInArray[i] = (uint32_t)(((int16_t)adc_buf[i])-2048) << 16;
							}
						}
						else if (Window_Hanning == fft_win_type)	// 汉宁窗
						{
							for(i = 0; i < NPT; i++)
							{
								adc_win = (int32_t)(((float)(((int16_t)adc_buf[i])-2048)) * HanningWindow_256[i]);
								lBufInArray[i] = adc_win << 16;
							}
						}
//						Creat_Single();
//						cr4_fft_1024_stm32(lBufOutArray, lBufInArray, NPT);		//FFT变换
						cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
						GetPowerMag();																					//取直流分量对应的AD值
						for(i=0; i<NPT/2; i++)
						{
							temp = lBufMagArray[i]/25 * y_zoom;
							if (temp < 256)
							{
								send_data[2*i] = (uint8_t)temp;
							}
							else
							{
								send_data[2*i] = 255;
							}
							send_data[2*i+1] = 0;
						}
					}
					UART1_TX_Bytes(send_data, NPT);
				}
			}
		} break;
		
		case SUB_FFT_SendData: {	// 透传数据发送状态
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// 接收到显示屏的应答
			{
				if (0 == memcmp(rx_data, "\xFD\xFF\xFF\xFF", 4))	// 是否为透传完成
				{
					sub->state = SUB_FFT_Default;	// 切换到初始状态
					DMA1_CH1_Start(NPT);	// 启动DMA1 CH1
				}
			}
		} break;
		
		case SUB_FFT_Default: {		// 初始状态
			if (0 != sub->exit_signal)		// 接收到中止信号
			{
				sub->state = SUB_FFT_Exit;			// 子系统退出
				sub->exit_signal = 0;				// 退出完成，中止信号置0
			}
			else if (0 != adc_dma_flag)
			{
				sub->state = SUB_FFT_SendCMD;		// 切换到发送命令状态
				adc_dma_flag = 0;						// 传输完成标志置0
				DMA1_CH1_Stop();						// DMA1通道1停止
				// 发送数据透传命令
				sprintf(hmi_cmd, "addt s0.id,0,%d\xFF\xFF\xFF", NPT);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
				if (0 != enc_dalta)	// 编码器增量不为0
				{
					sub->state = SUB_FFT_SendCMD;		// 切换到发送命令状态
					sub->cursor_update = 1;			// 游标更新
					
					npt_pos += enc_dalta;				// npt坐标点加编码器增量
					
					if (npt_pos > (NPT/2)-1)		// npt坐标点上限NPT/2 - 1
					{
						npt_pos = 0;							// npt坐标点归0
					}
					else if (npt_pos < 0)				// npt坐标点下限0
					{
						npt_pos = (NPT/2) - 1;		// npt坐标点归(NPT/2)
					}
					// 发送数据透传命令
					sprintf(hmi_cmd, "addt s0.id,1,%d\xFF\xFF\xFF", NPT);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key3, 1, &err))	// 按键3按下动作
				{
					sub->state = SUB_FFT_SendCMD;		// 切换到发送命令状态
					sub->cursor_update = 1;			// 游标更新
					
					npt_pos = GetMaxIndex();		// 获取最大幅度对应频率
					// 发送数据透传命令
					sprintf(hmi_cmd, "addt s0.id,1,%d\xFF\xFF\xFF", NPT);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key2, 1, &err))	// 按键2按下动作
				{
					y_zoom ++;	// 改变Y轴缩放
					if (y_zoom > 3)
					{
						y_zoom = 1;
					}
					sprintf(hmi_cmd, "t6.txt=\"Y Zoom:%u.0\"\xFF\xFF\xFF", y_zoom);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					
					sprintf(hmi_cmd, "v1.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v2.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*2);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v3.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*3);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v4.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else
				{
					npt_freq = sampling_freq / NPT * npt_pos;						// FFT坐标点对应频率
					npt_power = (float)lBufMagArray[npt_pos]/4095*3.3;	// FFT坐标点对应幅值
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%.2f\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					
					if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
					{
						sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
						UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					}
				}
			}
		} break;
		
		case SUB_FFT_Exit: {		// 退出状态
		} break;
	}
}

// 菜单子系统状态机
void SUB_Menu_StateMachine(SUB_Menu_Object_T* sub)
{
	switch(sub->state)
	{
		case SUB_Menu_FFT: {			// 频谱仪选择状态
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // 正转
			{
				sub->state = SUB_Menu_Setting;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=4\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // 反转
			{
				sub->state = SUB_Menu_Default;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Menu_Setting: {	// 设置选择状态
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // 正转
			{
				sub->state = SUB_Menu_Default;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // 反转
			{
				sub->state = SUB_Menu_FFT;
				sprintf(hmi_cmd, "p0.pic=2\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Menu_Default: {	// 初始状态
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 != enc_dalta)	// 编码器增量不为0
			{
				if (0 < enc_dalta) // 正转
				{
					sub->state = SUB_Menu_FFT;
					sprintf(hmi_cmd, "p0.pic=2\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (0 > enc_dalta) // 反转
				{
					sub->state = SUB_Menu_Setting;
					sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "p1.pic=4\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		case SUB_Menu_Exit: {			// 退出状态
		} break;
	}
}

// 设置子系统状态机
void SUB_Set_StateMachine(SUB_Set_Object_T* sub)
{
	switch(sub->state)
	{
		case SUB_Set_Sampling_Item: {		/* 采样频率项目状态 */
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // 正转
			{
				sub->state = SUB_Set_Window_Item;
				sprintf(hmi_cmd, "t2.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t2.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // 反转
			{
				sub->state = SUB_Set_Default;
			}
			else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub->state = SUB_Set_Sampling_Selet;
				sprintf(hmi_cmd, "n0.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "n0.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
			// Exit状态执行操作
			if (SUB_Set_Sampling_Item != sub->state)
			{
				sprintf(hmi_cmd, "t0.bco=17663\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t0.pco=0\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Set_Sampling_Selet: {	/* 采样频率选中状态 */
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 != enc_dalta)	// 编码器增量不为0，即旋钮发生改变
			{
				sampling_freq += (float)enc_dalta * 1000;
				
				// 采样频率限幅
				if (sampling_freq > MAX_SAM_FREQ)				// 采样频率上限
				{
					sampling_freq = MAX_SAM_FREQ;			
				}
				else if (sampling_freq < MIN_SAM_FREQ)	// 采样频率下限
				{
					sampling_freq = MIN_SAM_FREQ;
				}
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", (int32_t)(sampling_freq/1000));
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub->state = SUB_Set_Sampling_Item;
				sprintf(hmi_cmd, "t0.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t0.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
			// Exit状态执行操作
			if (SUB_Set_Sampling_Selet != sub->state)
			{
				sprintf(hmi_cmd, "n0.bco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "n0.pco=0\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Set_Window_Item: {			/* 窗函数项目状态 */
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // 正转
			{
				sub->state = SUB_Set_Default;
			}
			else if (0 > enc_dalta) // 反转
			{
				sub->state = SUB_Set_Sampling_Item;
				sprintf(hmi_cmd, "t0.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t0.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub->state = SUB_Set_Window_Selet;
				sprintf(hmi_cmd, "t4.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t4.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
			// Exit状态执行操作
			if (SUB_Set_Window_Item != sub->state)
			{
				sprintf(hmi_cmd, "t2.bco=17663\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t2.pco=0\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Set_Window_Selet: {		/* 窗函数选中状态 */
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 != enc_dalta) // 编码器动作
			{
				if (Window_Rectangle == fft_win_type)
				{
					fft_win_type = Window_Hanning;
					sprintf(hmi_cmd, "t4.txt=\"汉宁窗\"\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "Spectrum.window.txt=\"Win:Hann\"\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (Window_Hanning == fft_win_type)
				{
					fft_win_type = Window_Rectangle;
					sprintf(hmi_cmd, "t4.txt=\"矩形窗\"\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "Spectrum.window.txt=\"Win:Rect\"\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
			else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub->state = SUB_Set_Window_Item;
				sprintf(hmi_cmd, "t2.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t2.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
			// Exit状态执行操作
			if (SUB_Set_Window_Selet != sub->state)
			{
				sprintf(hmi_cmd, "t4.bco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t4.pco=0\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case SUB_Set_Default: {					/* 初始状态 */
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // 正转
			{
				sub->state = SUB_Set_Sampling_Item;
				sprintf(hmi_cmd, "t0.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t0.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // 反转
			{
				sub->state = SUB_Set_Window_Item;
				sprintf(hmi_cmd, "t2.bco=1024\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "t2.pco=65535\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// 编码器按键按下动作
			{
				sub->state = SUB_Set_Exit;
			}
		} break;
		
		case SUB_Set_Exit: {						/* 退出状态 */
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
				 1024*sin(PI2 * i * 3000.0 / Fs)+
				 512*sin(PI2 * i * 5000.0 / Fs);
		lBufInArray[i] = ((signed short)fx) << 16;	
	}
}

// 获取最大幅度对应频率
uint16_t GetMaxIndex(void)
{
	uint16_t i, max_index = 0;
	uint32_t max_val = 0;
	
	for (i = 0; i < NPT/2; i++)
	{
		if (lBufMagArray[i] > max_val)
		{
			max_val = lBufMagArray[i];
			max_index = i;
		}
	}
	return max_index;
}
