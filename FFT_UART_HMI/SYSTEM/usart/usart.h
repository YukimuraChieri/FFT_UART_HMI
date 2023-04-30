#ifndef __USART_H
#define __USART_H

#include "sys.h"
#include <string.h>

/* 串口1发送缓冲大小 */
#define UART1_TX_BUFF_SIZE		2048
/* 串口1接收缓冲大小 */
#define UART1_RX_BUFF_SIZE		32


extern uint8_t tx_buff[UART1_TX_BUFF_SIZE];	// 串口发送缓冲
extern uint8_t rx_buff[UART1_RX_BUFF_SIZE];	// 串口接收缓冲
extern uint16_t uart_rx_len;		// 串口接收长度

void UART1_Init(uint32_t pclk2, uint32_t bound);	// 串口1初始化
void UART1_TX_Bytes(const uint8_t* data, uint16_t len);	// 串口1发送数据
uint8_t UART1_RX_Bytes(uint8_t* data, uint16_t len);			// 串口1接收数据

void UART1_TX_DMA_CallBack(void);	// 串口发送完成DMA回调函数


#endif


