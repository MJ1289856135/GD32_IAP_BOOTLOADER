#include "bsp_usart.h"
#include "stdio.h"
#include <stdarg.h>

#define DEBUG_PRINTF

#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
    int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}

#ifdef __GNUC__
/* 必须保证 USART 已初始化 */
int _write(int fd, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
        usart_data_transmit(EVAL_COM0, (uint8_t)ptr[i]);
        while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    }
    return len;
}
#else
/* 对 Keil 或其他编译器，用 fputc 重定向 printf */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
    while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    return ch;
}
#endif


volatile uint8_t g_usart2RxBuffer[USART2_RX_BUF_SIZE] = {'\0'};
uint8_t g_usart2TxBuffer[USART2_TX_BUF_SIZE] = {'\0'};

/**
 * @description: 日志打印
 * @param {char} *fmt
 * @return {*}
 */
void LogPrintf(const char *fmt, ...)
{
#ifdef  DEBUG_PRINTF
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);   // 或者用你的串口 / RTT 输出函数
	va_end(args);
#endif
}

/***************************************************************
 *  @brief     串口0配置
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void Usart0Config()
{
    /* enable USART clock */
    rcu_periph_clock_enable(EVAL_COM0_CLK);
    rcu_periph_clock_enable(USART_0_PORT_CLK);
    /* connect port to USARTx_Tx */
    gpio_af_set(USART_0_PORT, EVAL_COM0_AF, USART_0_TX_PIN);
    /* connect port to USARTx_Rx */
    gpio_af_set(USART_0_PORT, EVAL_COM0_AF, USART_0_RX_PIN);
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(USART_0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_0_TX_PIN);
    gpio_output_options_set(USART_0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_0_TX_PIN);
    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(USART_0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_0_RX_PIN);
    gpio_output_options_set(USART_0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_0_RX_PIN);
    // USART 配置
    usart_deinit(EVAL_COM0);
    usart_baudrate_set(EVAL_COM0, 115200);
    usart_parity_config(EVAL_COM0, USART_PM_NONE);                   // 无校验
    usart_word_length_set(EVAL_COM0, USART_WL_8BIT);                 // 8位数据
    usart_stop_bit_set(EVAL_COM0, USART_STB_1BIT);                   // 1个停止位-
    usart_hardware_flow_coherence_config(EVAL_COM0, USART_HCM_NONE); // 无硬件流控
    usart_receive_config(EVAL_COM0, USART_RECEIVE_ENABLE);
    usart_transmit_config(EVAL_COM0, USART_TRANSMIT_ENABLE);
    usart_enable(EVAL_COM0);
}

/***************************************************************
 *  @brief     串口2DMA配置
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void Usart2DMAConfig()
{
    dma_single_data_parameter_struct dma_init_struct;
    rcu_periph_clock_enable(RCU_DMA0);

    // TX
    dma_deinit(DMA0, USART2_DMA_TX_CH);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (uint32_t)g_usart2TxBuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = USART2_TX_BUF_SIZE;
    dma_init_struct.periph_addr = ((uint32_t)&USART_DATA(USART2));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, USART2_DMA_TX_CH, &dma_init_struct);
    /* configure DMA mode */
    dma_channel_subperipheral_select(DMA0, USART2_DMA_TX_CH, DMA_SUBPERI4);
    dma_circulation_disable(DMA0, USART2_DMA_TX_CH);
    dma_channel_enable(DMA0, USART2_DMA_TX_CH);

    // RX
    dma_deinit(DMA0, USART2_DMA_RX_CH);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uint32_t)g_usart2RxBuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = USART2_RX_BUF_SIZE;
    dma_init_struct.periph_addr = ((uint32_t)&USART_DATA(USART2));
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, USART2_DMA_RX_CH, &dma_init_struct);
    /* configure DMA mode */
    dma_channel_subperipheral_select(DMA0, USART2_DMA_RX_CH, DMA_SUBPERI4);
    dma_circulation_disable(DMA0, USART2_DMA_RX_CH);
    dma_channel_enable(DMA0, USART2_DMA_RX_CH);
}

/***************************************************************
 *  @brief     串口2配置
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void Usart2Config()
{
    /* enable USART clock */
    rcu_periph_clock_enable(EVAL_COM2_CLK);
    rcu_periph_clock_enable(USART_2_PORT_CLK);
    /* connect port to USARTx_Tx */
    gpio_af_set(USART_2_PORT, EVAL_COM2_AF, USART_2_TX_PIN);
    /* connect port to USARTx_Rx */
    gpio_af_set(USART_2_PORT, EVAL_COM2_AF, USART_2_RX_PIN);
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(USART_2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_2_TX_PIN);
    gpio_output_options_set(USART_2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_2_TX_PIN);
    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(USART_2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_2_RX_PIN);
    gpio_output_options_set(USART_2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_2_RX_PIN);
    /* USART configure */
    usart_deinit(EVAL_COM2);
    usart_baudrate_set(EVAL_COM2, 19200);
    usart_receive_config(EVAL_COM2, USART_RECEIVE_ENABLE);
    usart_transmit_config(EVAL_COM2, USART_TRANSMIT_ENABLE);
    usart_enable(EVAL_COM2);
    // DMA配置
    Usart2DMAConfig();
    usart_dma_receive_config(EVAL_COM2, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(EVAL_COM2, USART_TRANSMIT_DMA_ENABLE);
    // 中断配置
    nvic_irq_enable(USART2_IRQn, 6, 0);
    // 开启串口空闲中断
    usart_flag_clear(EVAL_COM2, USART_FLAG_IDLE);
    usart_interrupt_enable(EVAL_COM2, USART_INT_IDLE);
}

/***************************************************************
 *  @brief     串口初始化
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void AllUsartInit()
{
    Usart0Config(); // 调试串口
}

/***************************************************************
 *  @brief     串口2DMA发送字符串
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample 	usage:     函数的使用方法
 **************************************************************/
void Usart2DmaSend(char *sendBuffer, int count)
{
	if (count > USART2_TX_BUF_SIZE) 
	{
		count = USART2_TX_BUF_SIZE;
	}
	
    memset(g_usart2TxBuffer, 0, sizeof(g_usart2TxBuffer));
    memcpy(g_usart2TxBuffer, sendBuffer, count);
    dma_channel_disable(DMA0, USART2_DMA_TX_CH);
    dma_flag_clear(DMA0, USART2_DMA_TX_CH, DMA_FLAG_FTF);
    dma_memory_address_config(DMA0, USART2_DMA_TX_CH, 0, (uint32_t)g_usart2TxBuffer);
    dma_transfer_number_config(DMA0, USART2_DMA_TX_CH, count);
    dma_channel_enable(DMA0, USART2_DMA_TX_CH);

    while (RESET == dma_flag_get(DMA0, USART2_DMA_TX_CH, DMA_FLAG_FTF)) ;
   
        
}

/***************************************************************
 *  @brief     串口2发送字节
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void Usart2SendByte(uint8_t data)
{
    usart_data_transmit(EVAL_COM2, data);
    while (usart_flag_get(EVAL_COM2, USART_FLAG_TBE) == RESET);
}

/***************************************************************
 *  @brief     串口2发送字符
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample 	usage:     函数的使用方法
 **************************************************************/
void Usart2SendStr(char *str)
{
    uint8_t i = 0;
#if 0		
	do
	{
		Usart2SendByte(*(str+i));
		i++;
	}while(*(str+i)!='\0');
	
	while(usart_flag_get(EVAL_COM2,USART_FLAG_TC)==RESET);	//判断是否发送完成
#else
    while (*(str + i) != '\0')
    {
        i++;
    }

    Usart2DmaSend(str, i);

#endif
}

/***************************************************************
 *  @brief     串口2接收中断
 *  @param     参数
 *  @return
 *  @note      备注
 *  @Sample usage:     函数的使用方法
 **************************************************************/
void USART2_IRQHandler(void)
{

    int16_t Usart2ReceiveSize;


    if (usart_interrupt_flag_get(EVAL_COM2, USART_INT_FLAG_IDLE) != RESET)
    {
        // 清除空闲中断标志位
        usart_interrupt_flag_clear(EVAL_COM2, USART_INT_FLAG_IDLE);
        // 清除接收完成标志
        usart_data_receive(EVAL_COM2);

        dma_channel_disable(DMA0, USART2_DMA_RX_CH);

        Usart2ReceiveSize = sizeof(g_usart2RxBuffer) - dma_transfer_number_get(DMA0, USART2_DMA_RX_CH);

        if (Usart2ReceiveSize != 0)
        {

            memset((void *)g_usart2RxBuffer, 0, sizeof(g_usart2RxBuffer));
        }

        dma_memory_address_config(DMA0, USART2_DMA_RX_CH, DMA_MEMORY_0, (uint32_t)g_usart2RxBuffer);
        dma_transfer_number_config(DMA0, USART2_DMA_RX_CH, sizeof(g_usart2RxBuffer));
        dma_flag_clear(DMA0, USART2_DMA_RX_CH, DMA_FLAG_FTF);
        dma_channel_enable(DMA0, USART2_DMA_RX_CH);

    }


}
