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

uint32_t lBufInArray[NPT];		/* FFT ������������� */
uint32_t lBufOutArray[NPT/2];	/* FFT ������������ */
uint32_t lBufMagArray[NPT/2];	/* ��г�������ķ�ֵ */

HMI_STATE_E hmi_state = HMI_Default;
SUB_Object_T sub_spectrum;			// Ƶ������ϵͳ����
SUB_Object_T sub_oscilloscope;	// ʾ������ϵͳ����
static char hmi_cmd[32];
static uint8_t send_data[256];
static uint8_t sampling_freq = 20;	// ����Ƶ�ʣ���λkHz
static uint16_t npt_pos = 0;				// FFT�����λ��
static float npt_freq = 0;				// FFT������ӦƵ��
static uint16_t npt_power = 0;		// FFT������Ӧ��ֵ

// HMIϵͳ״̬��
void HMI_StateMachine(void)
{
	static uint16_t cnt = 0;
		
	switch(hmi_state)
	{
		case HMI_Start: {		// ��ʼ����
			if (1500 == cnt)	// ��ʱ1500ms
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			cnt++;
		} break;
		
		case HMI_Menu: {	// �˵�����
			if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// ����1���¶���
			{
				hmi_state = HMI_Spectrum;	// �л���Ƶ���ǽ���
				sprintf(hmi_cmd, "page 2\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sub_spectrum.state = SUB_Default;
				sub_spectrum.exit_signal = 0;
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// ����2���¶���
			{
				hmi_state = HMI_Oscilloscope;	// �л���ʾ��������
				sprintf(hmi_cmd, "page 3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sub_oscilloscope.state = SUB_Default;
				sub_oscilloscope.exit_signal = 0;				
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// ����3���¶���
			{
				hmi_state = HMI_Setting;	// �л������ý���
				sprintf(hmi_cmd, "page 4\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case HMI_Spectrum: {	// Ƶ���ǽ���
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// ����3���¶���
			{
				sub_spectrum.exit_signal = 1;		// ����Ƶ������ϵͳ��ֹ�ź�
			}
			else if (SUB_Exit == sub_spectrum.state)	// ��ϵͳ�˳�
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_StateMachine(&sub_spectrum);	// Ƶ������ϵͳ״̬��
			}
		} break;
		
		case HMI_Oscilloscope: {	// ʾ��������
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// ����3���¶���
			{
				sub_oscilloscope.exit_signal = 1;	// ����ʾ������ϵͳ��ֹ�ź�
			}
			else if (SUB_Exit == sub_oscilloscope.state)		// ��ϵͳ�˳�
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				SUB_StateMachine(&sub_oscilloscope);	// ʾ������ϵͳ״̬��
			}
		} break;
		
		case HMI_Setting: {	// ���ý���
			if (KEY_Action_Down == KEY_Get_Flag(&KEY3_Obj))	// ����3���¶���
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// ����2���¶���
			{
				if (sampling_freq < 80)	// ����Ƶ������80kHz
				{
					sampling_freq ++;	// ����Ƶ�ʼ�1kHz
				}
				TIM3_TRGO_Freq((uint32_t)sampling_freq * 1000);	// ����TIM3����Ƶ��
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", sampling_freq);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// ����1���¶���
			{
				if (sampling_freq > 10)	// ����Ƶ������10kHz
				{
					sampling_freq --;	// ����Ƶ�ʼ�1kHz
				}
				TIM3_TRGO_Freq((uint32_t)sampling_freq * 1000);	// ����TIM3����Ƶ��
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", sampling_freq);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			
		} break;
		
		default: {					// ��ʼ״̬
			hmi_state = HMI_Start;		// �л�����ʼ����
			sprintf(hmi_cmd, "page 0\xFF\xFF\xFF");
			UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
		} break;
	}
}


void SUB_StateMachine(SUB_Object_T* sub)	// ��ϵͳ״̬��
{
	uint8_t rx_data[32];
	uint16_t i;
	
	switch(sub->state)
	{
		case SUB_SendCMD: {		// �����״̬
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// ���յ���ʾ����Ӧ��
			{
				if (0 == memcmp(rx_data, "\xFE\xFF\xFF\xFF", 4))	// �Ƿ�Ϊ͸������
				{
					sub->state = SUB_SendData;	// �л�����������״̬
					
					if (HMI_Spectrum == hmi_state)	// Ƶ��������
					{
						//�������
						for(i = 0; i < NPT; i++)
						{
							/******************************************************************
								������Ϊ��Ƭ����ADCֻ�ܲ����ĵ�ѹ ������Ҫǰ����ֱ��ƫִ
								����ֱ��ƫִ�� ����ϼ�ȥ2048��һ�� �ﵽ�������ڲ�����Ŀ��	
							******************************************************************/
							lBufInArray[i] = (((int16_t)adc_buf[i])-2048) << 16;
						}
//						Creat_Single();
						//cr4_fft_1024_stm32(lBufOutArray, lBufInArray, NPT);		//FFT�任
						cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
						GetPowerMag();																					//ȡֱ��������Ӧ��ADֵ
						for(i=0; i<NPT/2; i++)
						{
							send_data[2*i] = 0;
							send_data[2*i+1] = (uint8_t)(lBufMagArray[i]/6);
						}
					}
					else if (HMI_Oscilloscope == hmi_state)		// ʾ��������
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
		
		case SUB_SendData: {	// ���ݷ���״̬
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// ���յ���ʾ����Ӧ��
			{
				if (0 == memcmp(rx_data, "\xFD\xFF\xFF\xFF", 4))	// �Ƿ�Ϊ͸�����
				{
					sub->state = SUB_Default;	// �л�����ʼ״̬
					DMA1_CH1_Start(NPT);	// ����DMA1 CH1
				}
			}
		} break;
		
		case SUB_Default: {		// ��ʼ״̬
			if (0 != sub->exit_signal)		// ���յ���ֹ�ź�
			{
				sub->state = SUB_Exit;			// ��ϵͳ�˳�
				sub->exit_signal = 0;				// �˳���ɣ���ֹ�ź���0
			}
			else if (0 != adc_dma_flag)
			{
				sub->state = SUB_SendCMD;		// �л�����������״̬
				adc_dma_flag = 0;						// ������ɱ�־��0
				DMA1_CH1_Stop();						// DMA1ͨ��1ֹͣ
				// ��������͸������
				sprintf(hmi_cmd, "addt s0.id,0,%d\xFF\xFF\xFF", NPT);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}	
			else if (HMI_Spectrum == hmi_state)	// Ƶ���ǹ���
			{
				if (KEY_Action_Down == KEY_Get_Flag(&KEY2_Obj))	// ����2���¶���
				{
					if (npt_pos < 255)	// npt���������255
					{
						npt_pos ++;				// npt������1
					}
					npt_freq = (float)sampling_freq * 1000 / NPT * npt_pos;	// FFT������ӦƵ��
					npt_power = lBufMagArray[npt_pos];								// FFT������Ӧ��ֵ
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%d\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == KEY_Get_Flag(&KEY1_Obj))	// ����1���¶���
				{
					if (npt_pos > 0)	// npt���������0
					{
						npt_pos --;			// npt������1
					}
					npt_freq = (float)sampling_freq * 1000 / NPT * npt_pos;	// FFT������ӦƵ��
					npt_power = lBufMagArray[npt_pos];								// FFT������Ӧ��ֵ
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%d\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		case SUB_Exit: {		// �˳�״̬
		} break;
	}
}


//��ȡFFT���г������
void GetPowerMag(void)
{
	int16_t lX,lY;
	float X,Y,Mag;
	int16_t i;
	for(i=0; i<NPT/2; i++)
	{
		lX  = (lBufOutArray[i] << 16) >> 16;
		lY  = (lBufOutArray[i] >> 16);
	
		//����32768�ٳ�65536��Ϊ�˷��ϸ������������
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

/************FFT���*****************/
//������ ����һ���ź�
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

