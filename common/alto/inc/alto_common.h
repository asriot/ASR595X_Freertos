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
#ifndef _ALTO_COMM_H_
#define _ALTO_COMM_H_

#include "hbird.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASR_STR_TO_INT_ERR  0xFFFFFFFF
#define LEGA_STR_TO_INT_ERR  ASR_STR_TO_INT_ERR

#define INTERRUPT_LEVEL_LOW         1
#define INTERRUPT_LEVEL_NORMAL      2
#define INTERRUPT_LEVEL_HIGH        3
#define INTERRUPT_LEVEL_HIGHEST     4
#define INTERRUPT_LEVEL_SLEPP       5
#define INTERRUPT_LEVEL_DEEP_SLEEP  6
#define INTERRUPT_LEVEL_MAX         255

#define BSS_IN_ITCM_SEG __attribute__((section("BSS_DTCM")))
#ifdef ALTO_BLE_NO_BLOCK
#define FLASH_COMMON2_SEG __attribute__((section("seg_wf_flash_driver")))
void lega_enter_critical_expble(void);
void lega_exit_critical_expble(void);
#else
#define FLASH_COMMON2_SEG
#define lega_enter_critical_expble() lega_rtos_enter_critical()
#define lega_exit_critical_expble() lega_rtos_exit_critical()
#endif

#define FLASH_LWIFI_SEG __attribute__((section("seg_wf_flash_driver")))
//#define BSS_IN_ITCM_SEG __attribute__((section("BSS_ITCM")))
#define SPEED_OPTIMIZATION __attribute__((optimize("-Ofast")))

#ifdef COREDUMP_DEBUG
#define DEBUG_SECTION __attribute__((section("seg_debug_section")))
#else
#define DEBUG_SECTION
#endif

void jumpToApp(int addr);
void asr_memset(char *buf,int value, int size);
void asr_memcpy(char *dst, char *src, int size);
void delay(unsigned int cycles);
void delay_nop(unsigned int nop_count);
int convert_str_to_int(char *str);
void convert_int_to_str(unsigned int val, unsigned int type, char *ch);

/// Macro to read a register
#define REG_RD(addr)              (*(volatile uint32_t *)(addr))
/// Macro to write a register
#define REG_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)

/// Macro to read a register
#define REG_BLE_RD(addr)              (*(volatile uint32_t *)(addr))
/// Macro to write a register
#define REG_BLE_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)

void asr_write32_bit(uint32_t reg, uint8_t start_bit, uint8_t len, uint32_t src_val);
uint32_t asr_read32_bit(uint32_t reg, uint8_t start_bit, uint8_t len);
void asr_system_reset(void);

void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);
void intc_irq(void);
void D_APLL_UNLOCK_IRQHandler(void);
void D_SX_UNLOCK_IRQHandler(void);
void WDG_IRQHandler(void);
void GPIO_IRQHandler(void);
void TIMER_IRQHandler(void);
void SPI0_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

__inline __attribute__(( always_inline)) static uint32_t asr_disable_irq(void)
{
    uint32_t en = __RV_CSR_READ(CSR_MSTATUS) & MSTATUS_MIE;
    __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MIE);
    return en;
}

__inline __attribute__(( always_inline)) static void asr_enable_irq(uint32_t en)
{
    if(en)
        __RV_CSR_SET(CSR_MSTATUS, MSTATUS_MIE);
    else
        __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MIE);
}

#ifdef __cplusplus
}
#endif

#endif //_ALTO_COMM_H_
