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
#ifndef __ASR_ALTO_PINMUX_H
#define __ASR_ALTO_PINMUX_H

#include "alto.h"

#ifdef __cplusplus
extern "C"{
#endif

#define ALTO_PAD_NUM 24
typedef enum
{
    PAD0,
    PAD1,
    PAD2,
    PAD3,
    PAD4,
    PAD5,
    PAD6,
    PAD7,
    PAD8,
    PAD9,
    PAD10,
    PAD11,
    PAD12,
    PAD13,
    PAD14,
    PAD15,
    PAD16,
    PAD17,
    PAD18,
    PAD19,
    PAD20,
    PAD21,
    PAD22,
    PAD23,
    PAD_INVALID
}Pad_Num_Type;


typedef enum
{
    PF_GPIO,
    PF_SWD,PF_SWC,
    PF_UART0, PF_UART1, PF_UART2, PF_UART3,
    PF_SPI0,  PF_SPI1,  PF_SPI2,
    PF_PWM,
    PF_I2C0,  PF_I2C1,
    PF_SDIO0,
    PF_I2S,
    PF_PSRAM
}Pad_Func_Type;

#define PAD_PE_REG      0x40000014
#define PAD_PS_REG      0x40000018
#define PAD_IS_REG      0x4000001C
#define HW_CTRL_PE_PS   0x40000020

int asr_get_pad_func_val(Pad_Num_Type pad_num,Pad_Func_Type pad_type);
#ifdef ALIOS_SUPPORT
void asr_set_uart_pinmux(uint8_t uart_idx,uint8_t hw_flow_control);
#endif

#ifdef __cplusplus
}
#endif

#endif
