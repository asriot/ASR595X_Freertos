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
/**
 ****************************************************************************************
 *
 * @file soc_init.h
 *
 * @brief soc init
 *
 *
 ****************************************************************************************
 */

#ifndef __SOC_INIT__
#define __SOC_INIT__

#ifdef __cplusplus
extern "C" {
#endif

/** @brief  soc basic init, call at the beginning of main function
 */
int soc_pre_init(void);

/** @brief  soc and rtos basic init, call after soc_pre_init()
 */
int soc_init(void);

/** @brief  register uart for printf log.
 */
void printf_uart_register(uint8_t uart_idx);

void soc_delay_us(uint32_t nus);

#ifdef __cplusplus
}
#endif

#endif //__SOC_INIT__
