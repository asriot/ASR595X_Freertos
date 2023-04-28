/*
 * Copyright (c) 2022 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdbool.h>
#ifdef _UART_LITE_EN_
#include "uart_lite.h"
#else
#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif
#ifdef CODE_USE_ALTO
#include "asr_uart.h"
#include "asr_pinmux.h"
#else
#include "asr_uart.h"
#include "asr_pinmux.h"
#endif
#include "printf_uart.h"
#ifdef UART_PRINTF_DMA_ENABLE
#include "asr_dma.h"
#include "lega_rtos_api.h"
lega_semaphore_t lega_semaphore_uart_tx_dma;
#endif
uint8_t g_printf_uart = UART_NUM;
uint8_t g_origin_uart = UART_NUM;
volatile bool asr_log_en_flag = true;
void printf_uart_register(uint8_t uart_idx)
{
    #if 0
    uint32_t uart_clk_reg = *(volatile uint32_t *)(UART_CLK_REG_ADDR);

    if(uart_idx == UART0_INDEX)
    {
        if((uart_clk_reg & UART0_SCLK_EN) == 0)
        {
            goto ERR;
        }
    }
    else if(uart_idx == UART1_INDEX)
    {
        if((uart_clk_reg & UART1_SCLK_EN) == 0)
        {
            goto ERR;
        }
    }
    else if(uart_idx == UART2_INDEX)
    {
        if((uart_clk_reg & UART2_SCLK_EN) == 0)
        {
            goto ERR;
        }
    }
    else
    {
        g_printf_uart = uart_idx;
        return;
    }
#endif
    g_printf_uart = uart_idx;
    asr_log_en_flag = true;
    //printf("printf use uart %d\n",uart_idx);
    return;
#if 0
ERR:
    g_printf_uart = UART_NUM;
#endif
}
#endif
void asr_log_enable(void)
{
    if(asr_log_en_flag == false)
    {
        g_printf_uart = g_origin_uart;
        g_origin_uart = UART_NUM;
        asr_log_en_flag = true;
    }
}
void asr_log_disable(void)
{
    if(asr_log_en_flag == true)
    {
        g_origin_uart = g_printf_uart;
        g_printf_uart = UART_NUM;
        asr_log_en_flag = false;
    }
}
#ifdef UART_PRINTF_DMA_ENABLE
void printf_uart_dma_send(uint8_t uart_idx,char*buf,uint32_t size)
{
    lega_rtos_get_semaphore(&lega_semaphore_uart_tx_dma,LEGA_WAIT_FOREVER);
    asr_dma_uart_tx(uart_idx, (uint8_t *)buf, size);
}
void dma_tx_callback(uint32_t channel)
{
    if(((channel==DMA_CH_UART0_TX) && (g_printf_uart==0))
     ||((channel==DMA_CH_UART1_TX) && (g_printf_uart==1))
     ||((channel==DMA_CH_UART2_TX) && (g_printf_uart==2))
     )
     {
         lega_rtos_set_semaphore(&lega_semaphore_uart_tx_dma);
     }
}
#endif
void uart_put_buf(const char *buf, uint32_t size)
{
#ifdef UART_PRINTF_DMA_ENABLE
    printf_uart_dma_send(g_printf_uart, (char *)buf, size);
#else
//    uint32_t i=0;
    asr_uart_dev_t uart_tmp;
    uart_tmp.port=g_printf_uart;
//    for(i=0;i<size;i++)
    asr_uart_send(&uart_tmp, (unsigned char*)buf,  size, 10000);
//        UART_SendData(getUartxViaIdx(g_printf_uart),(unsigned char)buf[i]);
#endif
}

void uart_put_char(char character)
{
#ifndef _UART_LITE_EN_
//#if 0
//def UART_PRINTF_DMA_ENABLE
//    printf_uart_dma_send(g_printf_uart,&character,sizeof(character));
//#else
    uartx_put_char(uart_get_uartx_via_idx(g_printf_uart), (unsigned char)(character));
//#endif
#else
       // UART_SendData(UART_INDEX, (unsigned char)(character));
	asr_uart_dev_t uart_tmp;
    uart_tmp.port=g_printf_uart;
	//asr_uart_send(&uart_tmp, (unsigned char*)&character,  1, 10000);
    uartx_put_char(uart_get_uartx_via_idx(g_printf_uart), (unsigned char)(character));
#endif
}

void printf_uart_init(asr_uart_dev_t* uart)
{
    asr_uart_dev_t uart_config_struct  = {0};
    uart_config_struct.priv=uart->priv;
//    uart_config_struct.port = uart_idx;
//    switch(uart_idx){
    switch(uart->port){
        case UART0_INDEX:
            asr_pinmux_config(PAD0,PF_UART0);
            asr_pinmux_config(PAD1,PF_UART0);
#ifdef UART_PRINTF_DMA_ENABLE
            asr_dma_callback_register(DMA_CH_UART0_TX,dma_tx_callback);
#endif
            break;
        case UART1_INDEX:
            asr_pinmux_config(PAD2,PF_UART1);
            asr_pinmux_config(PAD3,PF_UART1);
#ifdef UART_PRINTF_DMA_ENABLE
            asr_dma_callback_register(DMA_CH_UART1_TX,dma_tx_callback);
#endif
            break;
        case UART2_INDEX:
            asr_pinmux_config(PAD12,PF_UART2);
            asr_pinmux_config(PAD13,PF_UART2);
#ifdef UART_PRINTF_DMA_ENABLE
            asr_dma_callback_register(DMA_CH_UART2_TX,dma_tx_callback);
#endif
            break;

        default:
            break;
    }

    asr_uart_struct_init(uart);
    uart->priv=uart_config_struct.priv;
    asr_uart_init(uart);
//    alto_uart_start(getUartxViaIdx(uart_idx));
    printf_uart_register(uart->port);
#ifdef UART_PRINTF_DMA_ENABLE
    asr_dma_init();
    asr_uart_dma_config(uart,UART_DMA_TX_EN,ENABLE);
    lega_rtos_init_semaphore(&lega_semaphore_uart_tx_dma,1);
#endif
    printf("set uart%d as printf log uart...\n",uart->port);
}

void printf_uart_log_init(uint8_t uart_idx)
{
    asr_uart_dev_t uart_config_struct  = {0};
    uart_config_struct.port = uart_idx;
    asr_uart_struct_init(&uart_config_struct);
    printf_uart_init(&uart_config_struct);
}
#ifdef PRINTF2_SUPPORT
uint8_t g_printf2_uart = UART_NUM;
void printf2_uart_register(uint8_t uart_idx)
{
    g_printf2_uart = uart_idx;
}
void uart_put_char_2(char character)
{
    uartx_put_char(uart_get_uartx_via_idx(g_printf2_uart),(unsigned char)character);
}
#endif