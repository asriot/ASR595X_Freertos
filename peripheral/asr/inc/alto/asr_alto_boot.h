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
#ifndef _ASR_ALTO_BOOT_H_
#define _ASR_ALTO_BOOT_H_

#include <nmsis_gcc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    RETENTION SRAM BEGIN
*/
struct ASR_RETENTION_SRAM
{
    __IO uint8_t RTC_DATE[16];
    __IO uint32_t BOOT_CFG;
    __IO uint32_t BOOT_TYPE;
    __IO uint32_t BOOT_TYPE_RESET;     /* hw sw wdg reset flag */
    __IO uint32_t BOOT_TYPE_SW_RESET;  /* sw reset flag */
    uint8_t RSVD1[32];
};

#define RETENTION_RAM_ADDR                0x40008000
#define RETENTION_SRAM                    ((struct ASR_RETENTION_SRAM *)(RETENTION_RAM_ADDR))
#define ALTO_RETENTION_SRAM_CUSTOM_SIZE   64
//#define RTC_TIME_RETENTION_RAM_ADDR       RETENTION_RAM_ADDR

/*
    RETENTION SRAM END
*/

#define AON_RST_CHECK_REG                   (ALWAYS_ON_REGFILE + 0x0C)
#define AON_RST_CHECK_FLAG                  (0x00000001<<17)
#define RET_RAM_SOFT_RST_FLAG               0xAA55A501
#define RET_RAM_DS_RST_FLAG                 0xAA55A502

#define BOOT_TYPE_MAGIC_ID      0x55AA5A00
#define PWR_ON_RST              (BOOT_TYPE_MAGIC_ID + 0x00)
#define HARDWARE_PIN_RST        (BOOT_TYPE_MAGIC_ID + 0x01)
#define SOFTWARE_RST            (BOOT_TYPE_MAGIC_ID + 0x02) //including SystemReset and WDG RST
#define DEEP_SLEEP_RST          (BOOT_TYPE_MAGIC_ID + 0x03)
#define WDG_RST                 (BOOT_TYPE_MAGIC_ID + 0x04)
#define UNKNOWN_RST             (BOOT_TYPE_MAGIC_ID + 0xFF)

#define AON_CPU_RESET_STATUS                (ALWAYS_ON_REGFILE + 0x94)
#define RBK_CPU_REQ_RESET                   0x00000001
#define RBK_WDG_REQ_RESET                   0x00000002

void asr_cfg_boot_type(void);
void asr_set_ds_boot_type(void);
uint32_t asr_get_boot_type(void);
void asr_set_sw_reset_boot_type(uint32_t sw_type);
uint32_t asr_get_sw_reset_boot_type(void);
void asr_cfg_boot_reset_type(void);
uint32_t asr_sys_reset_type_get(void);

#ifdef __cplusplus
}
#endif

#endif //_ASR_ALTO_BOOT_H_