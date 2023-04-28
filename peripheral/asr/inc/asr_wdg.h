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
#ifndef _ASR_WDG_H_
#define _ASR_WDG_H_
#include <stdint.h>
#include <errno.h>

#include "asr_port.h"
#include "asr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    CANON WDG BEGIN
*/
struct WDOG
{
    __IO uint32_t LOAD;
    __I  uint32_t VALUE;
    __IO uint32_t CONTROL;
    __O  uint32_t INTCLR;
    __I  uint32_t RIS;
    __I  uint32_t MIS; //0x14
    __I  uint32_t DUMMY0[0x2FA];
    __IO uint32_t LOCK; //0xC00
    __I  uint32_t DUMMY1[0xBF];
    __IO uint32_t ITCR; //0xF00
    __O  uint32_t ITOP; //0xF04
    __I  uint32_t DUMMY2[0x32];
    __I  uint32_t PERIPHID4; //0xFD0
    __I  uint32_t PERIPHID5;
    __I  uint32_t PERIPHID6;
    __I  uint32_t PERIPHID7;
    __I  uint32_t PERIPHID0;
    __I  uint32_t PERIPHID1;
    __I  uint32_t PERIPHID2;
    __I  uint32_t PERIPHID3;
    __I  uint32_t PCELLID0;
    __I  uint32_t PCELLID1;
    __I  uint32_t PCELLID2;
    __I  uint32_t PCELLID3;
};


#define WATCHDOG ((struct WDOG *)(WDOG_BASE))

#define WDG_LOCK_TOKEN 0x1ACCE551
#define WDG_RESEN (1 << 1)
#define WDG_INTEN 1

/*
    CANON WDG END
*/

typedef struct {
    uint32_t timeout;  /* Watchdag timeout */
} asr_wdg_config_t;

typedef struct {
    uint8_t      port;   /* wdg port */
    asr_wdg_config_t config; /* wdg config */
    void        *priv;   /* priv data */
} asr_wdg_dev_t;

#define WDG_TIMEOUT_MS (10000*(WDG_CLOCK/1000/2))  // between 5s and 10s

/**
 * This function will initialize the on board CPU hardware watch dog
 *
 * @param[in]  wdg  the watch dog device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_wdg_init(asr_wdg_dev_t *wdg);

/**
 * This function for stop hardware watch dog.
 *
 * @param[in]  NULL
 *
 * @return  NULL
 */
void asr_wdg_stop(void);

/**
 * This function for restart hardware watch dog.
 *
 * @param[in]  NULL
 *
 * @return  NULL
 */
void asr_wdg_start(void);
/**
 * Reload watchdog counter.
 *
 * @param[in]  wdg  the watch dog device
 */

void asr_wdg_reload(asr_wdg_dev_t *wdg);

/**
 * This function performs any platform-specific cleanup needed for hardware watch dog.
 *
 * @param[in]  wdg  the watch dog device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_wdg_finalize(asr_wdg_dev_t *wdg);

#ifdef __cplusplus
}
#endif

#endif //_ASR_WDG_H_