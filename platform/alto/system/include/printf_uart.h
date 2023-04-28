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
#ifndef __PRINTF_UART_H__
#define __PRINTF_UART_H__
#ifdef CODE_USE_ALTO
#include "asr_uart.h"
#else
#include "asr_uart.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SYSTEM_SUPPORT_OS
//#define UART_PRINTF_DMA_ENABLE
#endif
extern uint8_t g_printf_uart;
/**
 * Initialize printf uart port
 *
 *@param[in]  uart port for printf
 */
void printf_uart_register(uint8_t uart_idx);
void asr_log_enable(void);
void asr_log_disable(void);
#ifdef UART_PRINTF_DMA_ENABLE
void printf_uart_dma_send(uint8_t uart_idx,char*buf,uint32_t size);
#endif
void uart_put_buf(const char *buf, uint32_t size);
void uart_put_char(char character);
void printf_uart_init(asr_uart_dev_t* uart);
void printf_uart_log_init(uint8_t uart_idx);
#ifdef PRINTF2_SUPPORT
extern uint8_t g_printf2_uart;
void uart_put_char_2(char character);
void printf2_uart_register(uint8_t uart_idx);
#endif

#ifdef __cplusplus
}
#endif

#endif //__PRINTF_UART_H__
