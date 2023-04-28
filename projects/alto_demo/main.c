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
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lega_wlan_api.h"
#include "tcpip.h"
#include "lega_at_api.h"
#include "soc_init.h"
#include "lwip_app_iperf.h"
#include "lega_version.h"
#include "asr_uart.h"
#include "asr_pinmux.h"
#include "asr_flash_kv.h"
#include "asr_sec_hw_common.h"
#include "printf_uart.h"
#include "lega_ota_utils.h"
#if (CFG_EASY_LOG_ENABLE==1)
#include "elog.h"
#include "elog_cfg.h"
#endif
#define LEGA_UART0_INDEX UART0_INDEX
#define LEGA_UART1_INDEX UART1_INDEX
#define LEGA_UART2_INDEX UART2_INDEX


#define     TEST_TASK_NAME              "timestamp_task"
#define     TEST_TASK_PRIORITY          2
#define     TEST_STACK_SIZE             2048
TaskHandle_t timestamp_task_Handler;
#ifdef CLOUD_TEST
void cloud_test(void);
#endif

void timestamp_task(void *arg)
{
    int timestamp = 10000;
    while(1)
    {
        printf("daemon task...\n");
        lega_rtos_delay_milliseconds(timestamp);
#ifdef CLOUD_TEST
        cloud_test();
#endif
    }
}

#define UART1_TX_PIN PAD2
#define UART1_RX_PIN PAD3

void at_uart_init(void)
{
    int32_t len = 0;
    uint8_t kv_param_ok = 0;
    memset(&lega_at_uart,0,sizeof(lega_at_uart));
    len = sizeof(lega_at_uart);

    if((asr_flash_kv_get("uart_config_def", &lega_at_uart,&len) == 0)&&(lega_at_uart.config.baud_rate !=0)\
        &&(lega_at_uart.config.baud_rate !=0xffffffff))
    {
        if(!((lega_at_uart.config.data_width > DATA_8BIT)||(lega_at_uart.config.stop_bits > STOP_2BIT)\
            ||(lega_at_uart.config.flow_control > FLOW_CTRL_CTS_RTS)||(lega_at_uart.config.parity > PARITY_EVEN)))
       {
            kv_param_ok = 1;
            lega_at_uart.config.mode = TX_RX_MODE;
        }
    }

    if(!kv_param_ok)
    {
        lega_at_uart.config.baud_rate=UART_BAUDRATE_115200;
        lega_at_uart.config.data_width = DATA_8BIT;
        lega_at_uart.config.flow_control = FLOW_CTRL_DISABLED;
        lega_at_uart.config.parity = PARITY_NO;
        lega_at_uart.config.stop_bits = STOP_1BIT;
        lega_at_uart.config.mode = TX_RX_MODE;
    }

    lega_at_uart.port=UART1_INDEX;
    asr_pinmux_config(UART1_TX_PIN,PF_UART1);
    asr_pinmux_config(UART1_RX_PIN,PF_UART1);

    //register uart callback func for receiving at command
    lega_at_uart.priv = (void *)(at_handle_uartirq);
  //  asr_uart_init(&lega_at_uart);
    printf_uart_init(&lega_at_uart);
}

extern void lega_sram_rf_pta_init(void);
extern void lega_recovery_phy_fsm_config(void);
extern void lega_wlan_efuse_read(void);

int main(void)
{
    //don't run any code before soc_pre_init.
    soc_pre_init();

    soc_init();

    alto_flash_kv_init();

    //uart init and register uart for receiving at command
    at_uart_init();

    //ota roll back,just for flash_remapping
    ota_roll_back_pro();

    //register uart for printf log, the used uart should be init before.
    printf_uart_register(LEGA_UART1_INDEX);

    //register uart for at log, the used uart should be init before.
    printf2_uart_register(LEGA_UART1_INDEX);
#if (CFG_EASY_LOG_ENABLE==1)
    elog_init();
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_start();
#endif
    printf("\napp version: %s\n",LEGA_VERSION_STR);

    lega_wlan_efuse_read();

    lega_sram_rf_pta_init();

    lega_recovery_phy_fsm_config();

#ifdef SECURITY_ENGINE_INIT
    asr_security_engine_init();
#endif

    lega_at_init();

    lega_at_cmd_register_all();

    lega_wlan_init();

    tcpip_init(NULL,NULL);

    lega_wifi_iperf_init();

    xTaskCreate(timestamp_task,TEST_TASK_NAME,TEST_STACK_SIZE>>2,0,TEST_TASK_PRIORITY,&timestamp_task_Handler);

    vTaskStartScheduler();

}
