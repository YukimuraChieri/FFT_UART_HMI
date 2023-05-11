#include "usart.h"
#include "delay.h"

uint8_t tx_buff[UART1_TX_BUFF_SIZE] = {0};	// 串口发送缓冲
uint8_t rx_buff[UART1_RX_BUFF_SIZE] = {0};	// 串口接收缓冲
__IO uint8_t DMA_TX_FLAG = 0;		// DMA发送完成标志
__IO uint8_t DMA_RX_FLAG = 0;		// DMA接收完成标志
uint16_t uart_rx_len = 0;		// 串口接收长度

//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率 
void UART1_Init(uint32_t pclk2, uint32_t bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;
	
	temp=(float)(pclk2*1000000)/(bound*16);//得到USARTDIV
	mantissa=temp;				 //得到整数部分
	fraction=(temp-mantissa)*16; //得到小数部分	 
	mantissa<<=4;
	mantissa+=fraction;
	
	/* enable GPIOA clock */
	RCC->APB2ENR|=1<<2;

	/* enable USART1 clock */
	RCC->APB2ENR|=1<<14;

	/* connect port to USART1_Tx and USART1_Rx */
	GPIOA->CRH&=0XFFFFF00F;
	GPIOA->CRH|=0X000008B0;

	/* USART configure */
	RCC->APB2RSTR|=1<<14;   //复位串口1
	RCC->APB2RSTR&=~(1<<14);//停止复位
	
	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C;  //1位停止,无校验位
	
	USART1->CR3|=1<<7;		//使能串口1的DMA发送
	USART1->CR3|=1<<6;		//使能串口1的DMA接收
	
	USART1->CR1|=1<<4;    //使能串口空闲中断
	MY_NVIC_Init(3,3,USART1_IRQn,2);	//组2，最低优先级
}

// 串口发送数据，使用DMA1通道4
void UART1_TX_Bytes(const uint8_t* data, uint16_t len)
{	
	uint8_t timeout_cnt = 0;		// 超时计数
	if (len < UART1_TX_BUFF_SIZE)
	{
		// 等待上个数据传输完成，时间小于50ms
		while((0 != DMA_TX_FLAG) && (timeout_cnt < 50))
		{
			delay_ms(1);
			timeout_cnt++;
		}
		DMA1_Channel4->CCR &= ~(1<<0);	// 关闭DMA1通道4
		memcpy(tx_buff, data, len);		// 拷贝传输数据到发送缓冲
		DMA1_Channel4->CNDTR = len;		// 定义DMA的传输量
		DMA_TX_FLAG = 1;							//发送标志置1
		/* enable DMA channel4 */
		DMA1_Channel4->CCR |= 1<<0;		// 开启DMA1通道4传输
	}
}

// 串口接收数据，使用DMA1通道5
uint8_t UART1_RX_Bytes(uint8_t* data, uint16_t len)
{
	uint8_t ret_val = 0;
	if ((0 != DMA_RX_FLAG) && (len <= uart_rx_len))
	{
		DMA_RX_FLAG = 0;
		memcpy(data, rx_buff, len);
		ret_val = 1;
	}
	return ret_val;	// 读取成功，返回1
}

// 串口1中断处理函数
void USART1_IRQHandler(void)
{
	if(USART1->SR&(1<<4))	//总线空闲信号
	{
		USART1->SR;		//清除IDLE信号
		USART1->DR;
		DMA1_Channel5->CCR &= ~(1<<0);	// 关闭DMA1通道5
		uart_rx_len = UART1_RX_BUFF_SIZE - DMA1_Channel5->CNDTR;	// 计算接收数据长度
		if (uart_rx_len > UART1_RX_BUFF_SIZE)	// 接收数据超过接收缓冲最大长度
		{
			uart_rx_len = UART1_RX_BUFF_SIZE;
		}
		
		DMA_RX_FLAG = 1;
		
		DMA1_Channel5->CNDTR=UART1_RX_BUFF_SIZE;	//DMA1,传输数据量
		DMA1_Channel5->CCR|=1<<0;					// 开启DMA传输
	}
}

// 串口发送完成DMA回调函数
void UART1_TX_DMA_CallBack(void)
{
	DMA_TX_FLAG = 0;		// 清空发送标志
}


