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
#ifndef __SYSTICK_DELAY_H
#define __SYSTICK_DELAY_H

#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint32_t alto_systick_csr_get();
void alto_systick_csr_set(uint32_t ctrl);

void delay_us(uint32_t us); // 9cycle
void delay_ms(uint32_t ms);
void delay_s(uint32_t s);

#ifdef __cplusplus
}
#endif

#endif //__SYSTICK_DELAY_H





























