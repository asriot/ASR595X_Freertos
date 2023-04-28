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
#ifndef __ASR_RTC_H__
#define __ASE_RTC_H__
#include "asr_port.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_INIT_YEAR           120 //2020
#define RTC_INIT_MONTH          11 //month 12
#define RTC_INIT_DATE           25
#define RTC_INIT_HOUR           8
#define RTC_INIT_MINUTE         30
#define RTC_INIT_SECOND         20
#define RTC_INIT_WEEKDAY        5 //auto modify

//RTC->CTRL
#define RTC_CNT_CYCLE_ENABLE    (1 << 15)
#define RTC_ENABLE              (1 << 14)
#define RTC_INT_ENABLE          (1 << 11)

#define RTC_TICK_CNT            32768 //1s for asic

#define RTC_REFRESH_DAY         255 //255 max unit: day
#define RTC_REFRESH_HOUR        23 // 0 - 23
#define RTC_REFRESH_MINUTE      59 // 0 - 59
#define RTC_REFRESH_SECOND      59 // 0 - 59

#define RTC_MAX_DAY             256
#define SECOND_PER_DAY          (24*3600)
typedef struct __RTC
{
    __IO uint32_t CTRL;
    __IO uint32_t CNT_TICK;
    __IO uint32_t CNT_DATE;
    __I  uint32_t DUMMY[3];
    __I  uint32_t CURRENT_TICK;
    __I  uint32_t CURRENT_DATE;
} RTC_TypeDef;

#define RTC ((RTC_TypeDef *)(RTC_REG_BASE))

#ifndef ALIOS_SUPPORT
#define HAL_RTC_FORMAT_DEC  1
#define HAL_RTC_FORMAT_BCD  2

typedef struct {
    uint8_t  format;    /* time formart DEC or BCD */
} asr_rtc_config_t;

typedef struct {
    uint8_t port;        /* rtc port */
    asr_rtc_config_t config; /* rtc config */
    void   *priv;        /* priv data */
} asr_rtc_dev_t;

/*
 * RTC time
 */
typedef struct {
    uint8_t sec;         /* DEC format:value range from 0 to 59, BCD format:value range from 0x00 to 0x59 */
    uint8_t min;         /* DEC format:value range from 0 to 59, BCD format:value range from 0x00 to 0x59 */
    uint8_t hr;          /* DEC format:value range from 0 to 23, BCD format:value range from 0x00 to 0x23 */
    uint8_t weekday;     /* DEC format:value range from 0 to  6, BCD format:value range from 0x00 to 0x06 */
    uint8_t date;        /* DEC format:value range from 1 to 31, BCD format:value range from 0x01 to 0x31 */
    uint8_t month;       /* DEC format:value range from 0 to 11, BCD format:value range from 0x00 to 0x11 */
    uint8_t year;        /* DEC format:value range from 0 to 99, BCD format:value range from 0x00 to 0x99 */
} asr_rtc_time_t;

/**
 * This function will initialize the on board CPU real time clock
 *
 *
 * @param[in]  rtc  rtc device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_init(asr_rtc_dev_t *rtc);

/**
 * This function will return the value of time read from the on board CPU real time clock.
 *
 * @param[in]   rtc   rtc device
 * @param[out]  time  pointer to a time structure
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_get_time(asr_rtc_dev_t *rtc, asr_rtc_time_t *time);

/**
 * This function will set MCU RTC time to a new value.
 *
 * @param[in]   rtc   rtc device
 * @param[out]  time  pointer to a time structure
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_set_time(asr_rtc_dev_t *rtc, const asr_rtc_time_t *time);

/**
 * De-initialises an RTC interface, Turns off an RTC hardware interface
 *
 * @param[in]  RTC  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_finalize(asr_rtc_dev_t *rtc);
#endif

#ifdef __cplusplus
}
#endif

#endif
