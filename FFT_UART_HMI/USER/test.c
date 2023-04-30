#include "sys.h"
#include "usart.h"		
#include "delay.h"
#include "led.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "key.h"
#include "hmi.h"
#include "includes.h" 
#include "encoder.h"

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO		4		//开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE		36
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	

// 主任务
#define MAIN_TASK_PRIO		0												// 设置任务优先级
#define MAIN_STK_SIZE			256											// 设置任务堆栈大小
__align(8) OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];		// 任务堆栈，设置8字节对齐
void main_task(void *pdata);											// 任务函数

// 用户输入处理任务
#define USER_INPUT_TASK_PRIO	1										// 设置任务优先级
#define USER_INPUT_STK_SIZE		30									// 设置任务堆栈大小
OS_STK USER_INPUT_TASK_STK[USER_INPUT_STK_SIZE];	// 任务堆栈	
void user_input_task(void *pdata);								// 任务函数

// LED任务			
#define LED_TASK_PRIO			2												// 设置任务优先级
#define LED_STK_SIZE			40											// 设置任务堆栈大小
OS_STK LED_TASK_STK[LED_STK_SIZE];								// 任务堆栈	
void led_task(void *pdata);												// 任务函数
			
// 测试任务			
#define TEST_TASK_PRIO		3												// 设置任务优先级
#define TEST_STK_SIZE			64											// 设置任务堆栈大小
OS_STK TEST_TASK_STK[TEST_STK_SIZE];							// 任务堆栈	
void test_task(void *pdata);											// 任务函数

OS_EVENT * msg_key1;			// 按键1邮箱事件块指针
OS_EVENT * msg_key2;			// 按键2邮箱事件块指针
OS_EVENT * msg_key3;			// 按键3邮箱事件块指针
OS_EVENT * msg_key_enc;		// 编码器按键邮箱事件块指针
OS_EVENT * msg_enc_delta;		// 编码器位置邮箱事件块指针
OS_EVENT * msg_cpuload;			// CPU负载邮箱事件块指针

char text_log[32];				// 测试日志

// 主函数
int main(void)
{	
	Stm32_Clock_Init(9);		// 系统时钟设置
	delay_init(72);					// 延时初始化
	UART1_Init(72, 115200);	// 串口1初始化为115200
	LED_Init();							// 初始化LED
	ADC1_Init();						// 初始化ADC1
	KEY_Init();							// 按键初始化函数
	Encoder_Init();					// 初始化编码器
	
	DMA1_CH1_Init((uint32_t)&ADC1->DR, (uint32_t)&adc_buf);		// 初始化DMA1通道1(ADC1)
	DMA1_CH4_Init((uint32_t)&USART1->DR, (uint32_t)&tx_buff);	// 初始化DMA1通道4(UART1_TX)
	DMA1_CH5_Init((uint32_t)&USART1->DR, (uint32_t)&rx_buff);	// 初始化DMA1通道5(UART1_RX)
	
	DMA1_CH1_TC_CallBack(ADC1_DMA_CallBack);			// 设置DMA传输完成中断回调函数
	DMA1_CH4_TC_CallBack(UART1_TX_DMA_CallBack);	// 设置DMA传输完成中断回调函数
	
	DMA1_CH1_Start(NPT);
	DMA1_CH5_Start(UART1_RX_BUFF_SIZE);
	
	TIM3_TRGO_Init();				// 定时器初始化
	TIM3_TRGO_Freq(20000);	// 定时器更新频率20kHz
	
	// 创建开始任务
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	OSStart();
}

// 开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	
	msg_key1 = OSMboxCreate((void*)0);			// 创建消息邮箱
	msg_key2 = OSMboxCreate((void*)0);			// 创建消息邮箱
	msg_key3 = OSMboxCreate((void*)0);			// 创建消息邮箱
	msg_key_enc = OSMboxCreate((void*)0);		// 创建消息邮箱
	msg_enc_delta = OSMboxCreate((void*)0);	// 创建消息邮箱
	msg_cpuload = OSMboxCreate((void*)0);		// 创建消息邮箱
	
	OSStatInit();							// 初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			// 进入临界区(无法被中断打断)
	
// 	OSTaskCreate(led_task,(void *)0,(OS_STK *)&LED_TASK_STK[LED_STK_SIZE-1], LED_TASK_PRIO);
//	OSTaskCreate(test_task,(void *)0,(OS_STK *)&TEST_TASK_STK[TEST_STK_SIZE-1], TEST_TASK_PRIO);
	
	OSTaskCreateExt(	main_task,
										(void *)0,
										(OS_STK *)&MAIN_TASK_STK[MAIN_STK_SIZE-1], 
										MAIN_TASK_PRIO,
										0,
										(OS_STK *)&MAIN_TASK_STK[0], 
										MAIN_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	user_input_task,	
										(void *)0,
										(OS_STK *)&USER_INPUT_TASK_STK[USER_INPUT_STK_SIZE-1], 
										USER_INPUT_TASK_PRIO,
										1,
										(OS_STK *)&USER_INPUT_TASK_STK[0], 
										USER_INPUT_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	led_task,	
										(void *)0,
										(OS_STK *)&LED_TASK_STK[LED_STK_SIZE-1], 
										LED_TASK_PRIO,
										2,
										(OS_STK *)&LED_TASK_STK[0], 
										LED_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	test_task,
										(void *)0,
										(OS_STK *)&TEST_TASK_STK[TEST_STK_SIZE-1], 
										TEST_TASK_PRIO,
										3,
										(OS_STK *)&TEST_TASK_STK[0], 
										TEST_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
	
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}

// 主任务
void main_task(void *pdata)
{
	while(1)
	{
		HMI_StateMachine();		// HMI系统状态机
		delay_ms(1);
	}
}

// 用户输入任务
void user_input_task(void *pdata)
{
	while(1)
	{
		KEY_StateMachine(&KEY1_Obj, KEY1_Pin);				// 按键1状态机
		KEY_StateMachine(&KEY2_Obj, KEY2_Pin);				// 按键2状态机
		KEY_StateMachine(&KEY3_Obj, KEY3_Pin);				// 按键3状态机
		KEY_StateMachine(&KEY_Enc_Obj, KEY_Enc_Pin);	// 编码器按键状态机
		
		OSMboxPost(msg_key1, (void*)KEY_Get_Action(&KEY1_Obj));			//发送按键1按下消息
		OSMboxPost(msg_key2, (void*)KEY_Get_Action(&KEY2_Obj));			//发送按键2按下消息
		OSMboxPost(msg_key3, (void*)KEY_Get_Action(&KEY3_Obj));			//发送按键3按下消息
		OSMboxPost(msg_key_enc, (void*)KEY_Get_Action(&KEY_Enc_Obj));	//发送编码器按键按下消息
		
		OSMboxPost(msg_enc_delta, (void*)GetEncDelta());	//发送编码器位置增量消息
		
		delay_ms(10);
	}
}


// LED任务
void led_task(void *pdata)
{
	while(1)
	{
		LED = ~LED;
		delay_ms(100);
	}
}

// 测试任务，获取CPU负载率，以及各任务栈空间使用情况
void test_task(void *pdata)
{
//	OS_STK_DATA StackData;	
	
	while(1)
	{
		OSMboxPost(msg_cpuload, (void*)OSCPUUsage);	//发送CPU负载消息
		delay_ms(500);
		
//		OSTaskStkChk(MAIN_TASK_PRIO, &StackData);
//		sprintf(text_log, "MAIN Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(USER_INPUT_TASK_PRIO, &StackData);
//		sprintf(text_log, "USER_INPUT Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(LED_TASK_PRIO, &StackData);
//		sprintf(text_log, "LED Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(TEST_TASK_PRIO, &StackData);
//		sprintf(text_log, "TEST Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
	}
}
