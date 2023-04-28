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

#if (CFG_EASY_LOG_ENABLE==1)
#if (CFG_EASY_LOG_MODULE_EN==1)
#include "elog_module.h"
#include "elog.h"
#include <stdarg.h>
#include <stdio.h>
// void (*elog_mbedtls_printf)();
module_status_t elog_module_status[] = {
    {"test",        MODULE_ELOG_ON,     ELOG_MODULE_DEFAULT_LEVEL},
    {"mbedtls",     MODULE_ELOG_ON,     ELOG_MODULE_DEFAULT_LEVEL},
    {"matter",      MODULE_ELOG_ON,     ELOG_LVL_VERBOSE},
};

void elog_module_printf(const char *tag, uint8_t enable, uint8_t level, const char *fmt, va_list args)
{
    if(enable == MODULE_ELOG_OFF)
    {
        return;
    }

    switch(level)
    {
        case ELOG_LVL_ASSERT:
             mlog_a(tag, fmt, args);
             break;
        case ELOG_LVL_ERROR:
             mlog_e(tag, fmt, args);
             break;
        case ELOG_LVL_WARN:
             mlog_w(tag, fmt, args);
             break;
        case ELOG_LVL_INFO:
             mlog_i(tag, fmt, args);
             break;
        case ELOG_LVL_DEBUG:
             mlog_d(tag, fmt, args);
             break;
        case ELOG_LVL_VERBOSE:
             mlog_v(tag, fmt, args);
             break;
        default:
             mlog_e(tag, fmt, args);
             break;
    }
    return;
}

uint8_t get_elog_module_num()
{
    uint8_t elog_module_num;
    elog_module_num = sizeof(elog_module_status)/sizeof(module_status_t);
    return elog_module_num;
}

int set_elog_module_level(uint8_t id, uint8_t level)
{
    uint8_t elog_module_num;
    elog_module_num = get_elog_module_num();
    if(id >= elog_module_num)
    {
        log_e("module id >= mudule num:%d\n", (int)elog_module_num);
        return ELOG_FAIL;
    }
    elog_module_status[id].level = level;
    return ELOG_OK;
}

int set_elog_module_swtich(uint8_t id, uint8_t enable)
{
    uint8_t elog_module_num;
    elog_module_num = get_elog_module_num();
    if(id >= elog_module_num)
    {
        log_e("module id >= mudule num:%d\n", (int)elog_module_num);
        return ELOG_FAIL;
    }
    elog_module_status[id].enable = enable;
    return ELOG_OK;
}

// void elog_module_init()
// {
//     int i = 0;
//     log_d("elog module init\n");
//     for(i = 0; i<MODULE_MAX_ID; i++)
//     {
//         elog_module_status[i].enable = MODULE_ELOG_ON;
//         elog_module_status[i].level = ELOG_MODULE_DEFAULT_LEVEL;
//     }
// }

#else
#endif
#endif
