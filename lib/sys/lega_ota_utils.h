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
#ifndef _LEGA_OTA_UTILS_H_
#define _LEGA_OTA_UTILS_H_

#ifndef _LEGA_BOOTLOADER_
#include "ota_port.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*CRC16*/
typedef struct {
    unsigned short crc;
} lega_crc16_ctx;

#define IMAGE_ROLL_BACK_WDG_TIMEOUT_MS      (5000)

void lega_crc16_init(lega_crc16_ctx *inCtx);

void lega_crc16_update(lega_crc16_ctx *inCtx, const void *inSrc, unsigned int inLen);

void lega_crc16_final(lega_crc16_ctx *inCtx, unsigned short *outResult);

void ota_wdg_enable(void);

void ota_wdg_disable(void);

void flash_ota_info_update(struct OTA_INFO *p_ota_info);

void ota_roll_back_pro(void);

#ifdef __cplusplus
}
#endif

#endif //_LEGA_OTA_UTILS_H_