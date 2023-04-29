#include "sys.h"
#include "usart.h"		
#include "delay.h"
#include "led.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "key.h"
#include "hmi.h"

int main(void)
{	
	Stm32_Clock_Init(9);		// 系统时钟设置
	delay_init(72);					// 延时初始化
	UART1_Init(72, 115200);	// 串口1初始化为115200
	LED_Init();							// 初始化LED
	ADC1_Init();						// 初始化ADC1
	KEY_Init();							// 按键初始化函数
	
	DMA1_CH1_Init((uint32_t)&ADC1->DR, (uint32_t)&adc_buf);		// 初始化DMA1通道1(ADC1)
	DMA1_CH4_Init((uint32_t)&USART1->DR, (uint32_t)&tx_buff);	// 初始化DMA1通道4(UART1_TX)
	DMA1_CH5_Init((uint32_t)&USART1->DR, (uint32_t)&rx_buff);	// 初始化DMA1通道5(UART1_RX)
	
	DMA1_CH1_TC_CallBack(ADC1_DMA_CallBack);			// 设置DMA传输完成中断回调函数
	DMA1_CH4_TC_CallBack(UART1_TX_DMA_CallBack);	// 设置DMA传输完成中断回调函数
	
	DMA1_CH1_Start(NPT);
	DMA1_CH5_Start(UART1_RX_BUFF_SIZE);
	
	TIM3_TRGO_Init(71, 49);	// 定时器更新频率=72MHz/((71+1)*(49+1))=20kHz
	
  while(1)
	{
		KEY_StateMachine(&KEY1_Obj, KEY1_Pin);	// 按键1状态机
		KEY_StateMachine(&KEY2_Obj, KEY2_Pin);	// 按键2状态机
		KEY_StateMachine(&KEY3_Obj, KEY3_Pin);	// 按键3状态机
		
		HMI_StateMachine();		// HMI系统状态机
		
		delay_ms(1);					// 主循环周期1ms
	}
}

