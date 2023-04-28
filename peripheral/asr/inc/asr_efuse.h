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
#ifndef _ASR_EFUSE_H_
#define _ASR_EFUSE_H_
#include "asr_port.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __EFUSE
{
    __IO uint32_t CFG_TYPE; //0x00
    __IO uint32_t WR_TYPE;
    __IO uint32_t START;
    __IO uint32_t RD_CNT;
    __IO uint32_t WR_CNT; //0x10
    __IO uint32_t DIV_CNT;
    __IO uint32_t B_ADDR;
    __IO uint32_t PGM_DATA;
    __IO uint32_t RDBK_DATA; //0x20
    __I  uint32_t RSVD;
    __IO uint32_t INT_EN;
    __IO uint32_t INT_CLR;
    __IO uint32_t E_ENABLE; //0x30
} EFUSE_TypeDef;

#define EFUSE ((EFUSE_TypeDef *)(EFUSE_CTRL_BASE))

void asr_efuse_write_enable(void);

void asr_efuse_init(uint8_t write_en);

uint8_t asr_efuse_byte_read(uint16_t addr);

uint32_t asr_efuse_word_read(uint16_t addr);

void asr_efuse_multi_read(uint16_t start_addr, uint16_t size_in_bytes, uint8_t *pData);

void asr_efuse_reset(void);

#ifdef __cplusplus
}
#endif

#endif //_ASR_EFUSE_H_
