#include "usart.h"

uint8_t tx_buff[UART1_TX_BUFF_SIZE] = {0};	// ���ڷ��ͻ���
uint8_t rx_buff[UART1_RX_BUFF_SIZE] = {0};	// ���ڽ��ջ���
uint8_t DMA_TX_FLAG = 0;		// DMA������ɱ�־
uint8_t DMA_RX_FLAG = 0;		// DMA������ɱ�־
uint16_t uart_rx_len = 0;		// ���ڽ��ճ���

//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������ 
void UART1_Init(uint32_t pclk2, uint32_t bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;
	
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������	 
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
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ
	
	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ
	
	USART1->CR3|=1<<7;		//ʹ�ܴ���1��DMA����
	USART1->CR3|=1<<6;		//ʹ�ܴ���1��DMA����
	
	USART1->CR1|=1<<4;    //ʹ�ܴ��ڿ����ж�
	MY_NVIC_Init(3,3,USART1_IRQn,2);	//��2��������ȼ�
}

// ���ڷ������ݣ�ʹ��DMA1ͨ��4
void UART1_TX_Bytes(const uint8_t* data, uint16_t len)
{	
	if (len < UART1_TX_BUFF_SIZE)
	{
		while(0 != DMA_TX_FLAG);				// �ȴ��ϸ����ݴ������
		DMA1_Channel4->CCR &= ~(1<<0);	// �ر�DMA1ͨ��4
		memcpy(tx_buff, data, len);		// �����������ݵ����ͻ���
		DMA1_Channel4->CNDTR = len;		// ����DMA�Ĵ�����
		DMA_TX_FLAG = 1;							//���ͱ�־��1
		/* enable DMA channel4 */
		DMA1_Channel4->CCR |= 1<<0;		// ����DMA1ͨ��4����
	}
}

// ���ڽ������ݣ�ʹ��DMA1ͨ��5
uint8_t UART1_RX_Bytes(uint8_t* data, uint16_t len)
{
	uint8_t ret_val = 0;
	if ((0 != DMA_RX_FLAG) && (len <= uart_rx_len))
	{
		DMA_RX_FLAG = 0;
		memcpy(data, rx_buff, len);
		ret_val = 1;
	}
	return ret_val;	// ��ȡ�ɹ�������1
}

// ����1�жϴ�����
void USART1_IRQHandler(void)
{
	if(USART1->SR&(1<<4))	//���߿����ź�
	{
		USART1->SR;		//���IDLE�ź�
		USART1->DR;
		DMA1_Channel5->CCR &= ~(1<<0);	// �ر�DMA1ͨ��5
		uart_rx_len = UART1_RX_BUFF_SIZE - DMA1_Channel5->CNDTR;	// ����������ݳ���
		if (uart_rx_len > UART1_RX_BUFF_SIZE)	// �������ݳ������ջ�����󳤶�
		{
			uart_rx_len = UART1_RX_BUFF_SIZE;
		}
		
		DMA_RX_FLAG = 1;
		
		DMA1_Channel5->CNDTR=UART1_RX_BUFF_SIZE;	//DMA1,����������
		DMA1_Channel5->CCR|=1<<0;					// ����DMA����
	}
}

// ���ڷ������DMA�ص�����
void UART1_TX_DMA_CallBack(void)
{
	DMA_TX_FLAG = 0;		// ��շ��ͱ�־
}


