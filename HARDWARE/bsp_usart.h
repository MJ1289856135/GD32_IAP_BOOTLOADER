#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f4xx.h"
#include "stdio.h"
#include "string.h"


#define EVAL_COM0                        	USART0
#define EVAL_COM0_CLK                    	RCU_USART0
#define EVAL_COM0_AF                        GPIO_AF_7
#define USART_0_PORT_CLK					RCU_GPIOA
#define USART_0_PORT    					GPIOA
#define USART_0_TX_PIN  					GPIO_PIN_9
#define USART_0_RX_PIN						GPIO_PIN_10

#define EVAL_COM2                        	USART2
#define EVAL_COM2_CLK                    	RCU_USART2
#define EVAL_COM2_AF                        GPIO_AF_7
#define USART_2_PORT_CLK					RCU_GPIOD
#define USART_2_PORT    					GPIOD
#define USART_2_TX_PIN  					GPIO_PIN_8
#define USART_2_RX_PIN						GPIO_PIN_9

 
#define USART2_DMA_RX_CH			        DMA_CH1
#define USART2_DMA_TX_CH			        DMA_CH3


#define USART2_TX_BUF_SIZE      10
#define USART2_RX_BUF_SIZE      10



void AllUsartInit(void);
void Usart2SendStr(char *str);
void LogPrintf(const char *fmt, ...);

#endif /*__BSP_USART_H*/

