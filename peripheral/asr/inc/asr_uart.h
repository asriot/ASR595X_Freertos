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
#ifndef _ASR_UART_H_
#define _ASR_UART_H_
#include <errno.h>

#include "asr_port.h"
#include "asr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* UART BAUDRATE*/
#define UART_BAUDRATE_110                       (110)
#define UART_BAUDRATE_300                       (300)
#define UART_BAUDRATE_600                       (600)
#define UART_BAUDRATE_1200                      (1200)
#define UART_BAUDRATE_2400                      (2400)
#define UART_BAUDRATE_4800                      (4800)
#define UART_BAUDRATE_9600                      (9600)
#define UART_BAUDRATE_14400                     (14400)
#define UART_BAUDRATE_19200                     (19200)
#define UART_BAUDRATE_38400                     (38400)
#define UART_BAUDRATE_57600                     (57600)
#define UART_BAUDRATE_115200                    (115200)
#define UART_BAUDRATE_230400                    (230400)
#define UART_BAUDRATE_460800                    (460800)
#define UART_BAUDRATE_921600                    (921600)

/* fifo depth */
#define UART_TX_FIFO_DEPTH               (16)
#define UART_RX_FIFO_DEPTH               (16)

/* UART flags */
#define UART_FLAG_TX_FIFO_EMPTY          (1<<7)
#define UART_FLAG_RX_FIFO_FULL           (1<<6)
#define UART_FLAG_TX_FIFO_FULL           (1<<5)
#define UART_FLAG_RX_FIFO_EMPTY          (1<<4)
#define UART_FLAG_BUSY                   (1<<3)

/* UART interrutps */
#define UART_RX_INTERRUPT                (1<<4)    /* tx interrupt is only set when tx fifo level transisions through a level,  not based on fifo level itself */
#define UART_TX_INTERRUPT                (1<<5)
#define UART_RX_TIMEOUT_INTERRUPT        (1<<6)
#define UART_ERROR_FRAME_INTERRUPT                 (1<<7)
#define UART_ERROR_PARITY_INTERRUPT                (1<<8)
#define UART_ERROR_BREAK_INTERRUPT                 (1<<9)
#define UART_ERROR_OVERRUN_INTERRUPT               (1<<10)
#define UART_ALL_IRQ             (0x7FF)

#define UART_DMA_TX_EN                   (1<<1)
#define UART_DMA_RX_EN                   (1<<0)

#ifdef FPGA_PTA_32M
#define UART_CLK                         (32000000)
#else
#define UART_CLK                         system_peri_clk //(52000000)
#endif

typedef void (*asr_uart_callback_func_t)(uint8_t);
extern asr_uart_callback_func_t g_asr_uart_callback_handler[UART_NUM];


/*
 * UART fifo_threshold
 * TX/RX FIFO Level, both FIFO of depth 16
*/
typedef enum {
    FIFO_1_8_FULL = 0, //1/8
    FIFO_1_4_FULL = 1, //1/4
    FIFO_HALF_FULL= 2, //1/2
    FIFO_3_4_FULL = 3, //3/4
    FIFO_7_8_FULL = 4,  //7/8
    FIFO_NULL
} asr_uart_fifo_threshold_t;

/* UART register block */
typedef struct __UART
{
    __IO      uint32_t  DR          ;/* 0x0  */
    __IO      uint32_t  RSC_ECR     ;/* 0x4  */
    __I       uint32_t  RSV0[4]     ;/* 0x8~0x14  */
    __I       uint32_t  FR          ;/* 0x18 */
    __I       uint32_t  RSV1        ;/* 0x1C */
    __IO      uint32_t  ILPR        ;/* 0x20 */
    __IO      uint32_t  IBRD        ;/* 0x24 */
    __IO      uint32_t  FBRD        ;/* 0x28 */
    __IO      uint32_t  LCR_H       ;/* 0x2C */
    __IO      uint32_t  CR          ;/* 0x30 */
    __IO      uint32_t  IFLS        ;/* 0x34 */
    __IO      uint32_t  IMSC        ;/* 0x38 */
    __I       uint32_t  RIS         ;/* 0x3C */
    __I       uint32_t  MIS         ;/* 0x40 */
    __O       uint32_t  ICR         ;/* 0x44 */
    __IO      uint32_t  DMACR       ;/* 0x48 */
    __I       uint32_t  RSV2[997]   ;/* 0x04C~0xFDC */
    __I       uint32_t  ID[8]         ;/* 0xFE0~0xFFC*/
} UART_TypeDef;

#define UART0     ((UART_TypeDef *)UART0_BASE)
#define UART1     ((UART_TypeDef *)UART1_BASE)
#define UART2     ((UART_TypeDef *)UART2_BASE)
#define UART3     ((UART_TypeDef *)UART3_BASE)


/*
 * UART data width
 */
typedef enum {
    DATA_5BIT,
    DATA_6BIT,
    DATA_7BIT,
    DATA_8BIT
} asr_uart_data_width_t;

/*
 * UART stop bits
 */
typedef enum {
    STOP_1BIT,
    STOP_2BIT
} asr_uart_stop_bits_t;

/*
 * UART flow control
 */
typedef enum {
    FLOW_CTRL_DISABLED,
    FLOW_CTRL_RTS,
    FLOW_CTRL_CTS,
    FLOW_CTRL_CTS_RTS
} asr_uart_flow_control_t;

/*
 * UART parity
 */
typedef enum {
    PARITY_NO,
    PARITY_ODD,
    PARITY_EVEN
} asr_uart_parity_t;

/*
 * UART mode
 */
typedef enum {
    TX_MODE,
    RX_MODE,
    TX_RX_MODE
} asr_uart_mode_t;

/*
 * UART configuration
 */
typedef struct {
    uint32_t                baud_rate;
    asr_uart_data_width_t   data_width;
    asr_uart_parity_t       parity;
    asr_uart_stop_bits_t    stop_bits;
    asr_uart_flow_control_t flow_control;
    asr_uart_mode_t         mode;
} asr_uart_config_t;

typedef struct {
    uint8_t           port;    /* uart port */
    asr_uart_config_t config;  /* uart config */
    void             *priv;    /* priv data ,should set asr_uart_callback_func_t type function, use to receive rx data in interrupt contex
                            if set NULL, no rx interrupt enable, and no rx data received
                            when support rtos, set it asr_uart_callback_handler() defined in asr_uart.c, and use asr_uart_recv()/asr_uart_recv_II() receive rx data
                                               set user defined func and asr_uart_recv()/asr_uart_recv_II() will not work
                            when support nonos, set user define func<asr_uart_callback_func_t> to receive rx data in interrupt contex, asr_uart_recv()/asr_uart_recv_II() will not work
                                                or set
                            */
} asr_uart_dev_t;

int32_t asr_uart_dma_config(asr_uart_dev_t* uart,uint8_t dma_tx_rx_sel,uint8_t new_state);
uint8_t uart_get_flag_status(UART_TypeDef* UARTx, uint8_t uart_flag);
/**
 * Initialises a UART interface
 *
 *
 * @param[in]  uart  the interface which should be initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */

int32_t asr_uart_init(asr_uart_dev_t *uart);

/**
 * Transmit data on a UART interface
 *
 * @param[in]  uart  the UART interface
 * @param[in]  data  pointer to the start of data
 * @param[in]  size  number of bytes to transmit
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_send(asr_uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout);
#ifdef RTOS_SUPPORT
void asr_uart_callback_handler(uint8_t uart_idx,uint8_t ch);

/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_recv(asr_uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t timeout);

/**
 * Receive data on a UART in terface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[out]  recv_size    number of bytes received
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_recv_II(asr_uart_dev_t *uart, void *data, uint32_t expect_size,
                      uint32_t *recv_size, uint32_t timeout);
#endif
/**
 * Deinitialises a UART interface
 *
 * @param[in]  uart  the interface which should be deinitialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_uart_finalize(asr_uart_dev_t *uart);


UART_TypeDef* uart_get_uartx_via_idx(uint8_t uart_idx);

/*
* Init uart struct, set default value
*/
void asr_uart_struct_init(asr_uart_dev_t* UART_InitStruct);

void asr_uart_set_callback(uint8_t uart_idx,asr_uart_callback_func_t func);

void uartx_put_char(UART_TypeDef* uartx, uint8_t c);

#ifdef __cplusplus
}
#endif

#endif //_ASR_UART_H_
