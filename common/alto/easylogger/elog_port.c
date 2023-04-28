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
#include "FreeRTOS.h"
#include "elog.h"
#include <stdio.h>
#include "asr_common.h"
#include "printf_uart.h"
#include "task.h"

// #include <stm32f10x_conf.h>

extern void uart_put_char(char character);
/**
 * EasyLogger port initialize
 *
 * @return result
 */

ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size) {
    uart_put_buf(log,size);
    /* output to terminal */
    //printf2("%.*s", size, log);
    //TODO output to flash
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    __disable_irq();
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    __enable_irq();
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char tm_st[10];
    uint32_t tm = lega_rtos_get_time();
    sprintf(tm_st,"%ld",tm);
    return tm_st;
}


/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    static char task_num[4];
    TaskStatus_t xTaskDetails;
    TaskHandle_t xHandle = NULL;
    xHandle = xTaskGetCurrentTaskHandle();
    vTaskGetInfo( xHandle, &xTaskDetails, 0, eInvalid);
    sprintf(task_num,"%ld",xTaskDetails.xTaskNumber);
    return task_num;
}
#endif