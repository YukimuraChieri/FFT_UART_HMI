#ifndef __USART_H
#define __USART_H

#include "sys.h"
#include <string.h>

/* ����1���ͻ����С */
#define UART1_TX_BUFF_SIZE		2048
/* ����1���ջ����С */
#define UART1_RX_BUFF_SIZE		32


extern uint8_t tx_buff[UART1_TX_BUFF_SIZE];	// ���ڷ��ͻ���
extern uint8_t rx_buff[UART1_RX_BUFF_SIZE];	// ���ڽ��ջ���
extern uint16_t uart_rx_len;		// ���ڽ��ճ���

void UART1_Init(uint32_t pclk2, uint32_t bound);	// ����1��ʼ��
void UART1_TX_Bytes(const uint8_t* data, uint16_t len);	// ����1��������
uint8_t UART1_RX_Bytes(uint8_t* data, uint16_t len);			// ����1��������

void UART1_TX_DMA_CallBack(void);	// ���ڷ������DMA�ص�����


#endif


