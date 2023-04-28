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

#ifndef _ELOG_MODULE_H_
#define _ELOG_MODULE_H_

#if (CFG_EASY_LOG_ENABLE==1)
#include <stdint.h>
#include <stdarg.h>
#include "elog.h"
#if (CFG_EASY_LOG_MODULE_EN==1)

#ifdef __cplusplus
extern "C" {
#endif

#define MODULE_MAX_ID               10
#define MODULE_ELOG_ON              1
#define MODULE_ELOG_OFF             0

typedef struct module_status {
    char *name;
    uint8_t enable;
    uint8_t level;
}module_status_t;

extern module_status_t elog_module_status[];

void elog_module_printf(const char *tag, uint8_t enable, uint8_t level, const char *fmt, va_list args);
void elog_module_output(uint8_t level, const char *tag, const char *format, va_list args);

#if LOG_LVL >= ELOG_LVL_ASSERT
    #define mlog_a(tag, fmt, args)       elog_module_output(ELOG_LVL_ASSERT, tag, fmt, args)
#else
    #define mlog_a(tag, fmt, args)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_ERROR
    #define mlog_e(tag, fmt, args)       elog_module_output(ELOG_LVL_ERROR, tag, fmt, args)
#else
    #define mlog_e(tag, fmt, args)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_WARN
    #define mlog_w(tag, fmt, args)       elog_module_output(ELOG_LVL_WARN, tag, fmt, args)
#else
    #define mlog_w(tag, fmt, args)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_INFO
    #define mlog_i(tag, fmt, args)       elog_module_output(ELOG_LVL_INFO, tag, fmt, args)
#else
    #define mlog_i(tag, fmt, args)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_DEBUG
    #define mlog_d(tag, fmt, args)       elog_module_output(ELOG_LVL_DEBUG, tag, fmt, args)
#else
    #define mlog_d(tag, fmt, args)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_VERBOSE
    #define mlog_v(tag, fmt, args)       elog_module_output(ELOG_LVL_VERBOSE, tag, fmt, args)
#else
    #define mlog_v(tag, fmt, args)       ((void)0);
#endif

int set_elog_module_level(uint8_t id, uint8_t level);
int set_elog_module_swtich(uint8_t id, uint8_t enable);
uint8_t get_elog_module_num();
// #define mbedtls_printf      elog_mbedtls_printf

#define ELOG_MODULE_DEFAULT_LEVEL           ELOG_LVL_ERROR
#define TEST_MODULE_ID                      0
#define MBED_MODULE_ID                      1
#define CHIP_MODULE_ID                      2

#define test_printf(tag, fmt, args)            elog_module_printf(tag, elog_module_status[TEST_MODULE_ID].enable, elog_module_status[TEST_MODULE_ID].level, fmt, args)
#define elog_mbedtls_printf(tag, fmt, args)    elog_module_printf(tag, elog_module_status[MBED_MODULE_ID].enable, elog_module_status[MBED_MODULE_ID].level, fmt, args)
#define elog_chip_printf(tag, fmt, args)       elog_module_printf(tag, elog_module_status[CHIP_MODULE_ID].enable, elog_module_status[CHIP_MODULE_ID].level, fmt, args)

#ifdef __cplusplus
}
#endif

#endif // CFG_EASY_LOG_MODULE_EN
#endif // CFG_EASY_LOG_ENABLE
#endif //_ELOG_MODULE_H_
