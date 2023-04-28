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
#ifdef AT_USER_DEBUG
#include <math.h>
#include <string.h>
#include "lega_at_api.h"
#include "lega_rtos_api.h"
#include "asr_common.h"
#include "asr_uart.h"
#include "asr_rv32.h"
#include "asr_flash.h"
#include "asr_flash_kv.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pmu.h"
#include "lega_version.h"
#ifdef ASR_PROFILE_ON
#include "dbg_profiling.h"
#endif
extern pmu_state_t sleep_mode;


#define UART_RXBUF_LEN            128
#define MIN_USEFUL_DEC            32
#define MAX_USEFUL_DEC            127
#define UART_CMD_NB               5
#define ARGCMAXLEN                14
#define AT_QUEUE_SIZE             UART_CMD_NB

uint8_t uart_idx=0;
uint8_t cur_idx = 0;
uint8_t start_idx = 0;
uint8_t uart_buf_len[UART_CMD_NB] = {0};
char uart_buf[UART_CMD_NB][UART_RXBUF_LEN];
lega_thread_t AT_Task_Handler = 0;
lega_queue_t  at_task_msg_queue = 0;

asr_uart_dev_t lega_at_uart;

_at_user_info at_user_info = {0};
struct cli_cmd_t *cli_cmd = NULL;
lega_semaphore_t at_cmd_protect;

#define AT_CMD_TIMEOUT          3000
#define TXPWR_ADD_MAX           2.0
#define TXPWR_11B_MAX           18.0
#define TXPWR_MAX               20
#define CHN_FREQ_INTERV         5
#define CHN1_FREQ               2412

float rf_txpwr_max = TXPWR_ADD_MAX;

extern float pwr_user_set_k;
extern _at_user_info at_user_info;
extern void lega_rfinit_pwr_cal(void);

char at_dbgflg = 1;

void lega_at_response(lega_at_rsp_status_t status)
{
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    if(status == CONFIG_OK)
    {
        at_rspinfor("OK");
        lega_rtos_set_semaphore(&at_cmd_protect);
    }
    else if(status == WAIT_PEER_RSP)
    {
        lega_rtos_set_semaphore(&at_cmd_protect);
    }
    else if(status != RSP_NULL)
    {
        at_rspinfor("FAIL:-%d",status);
        lega_rtos_set_semaphore(&at_cmd_protect);
    }
    lega_rtos_exit_critical();
}

/*
 ************************************************************
 *                    COMMON AT CMD START
 *
 ************************************************************
 */
static int lega_at_dbg(int argc, char **argv)
{
    int onoff = 0;

    if(argc < 2)
    {
        return PARAM_MISS;
    }

    onoff = convert_str_to_int(argv[1]);
    if((onoff > 1) || (onoff < 0))
    {
        return PARAM_RANGE;
    }
    else
    {
        at_dbgflg = onoff;
        return CONFIG_OK;
    }
}

static int lega_at_reset(int argc, char **argv)
{
    dbg_at("doing...");
    lega_at_response(CONFIG_OK);
    delay(5000);
    asr_system_reset();

    return RSP_NULL;
}
extern const char *lega_get_wifi_version(void);
static int lega_at_show_version(int argc, char **argv)
{
    at_rspdata("version sdk:%s", lega_get_wifi_version());
    at_rspdata("version app:%s", LEGA_VERSION_STR);
    at_rspdata("version at:%s", LEGA_AT_VERSION);

    return CONFIG_OK;
}
static int lega_at_echo(int argc, char **argv)
{
    int echo = 0;

    if(argc < 2)
    {
        return PARAM_MISS;
    }

    echo = convert_str_to_int(argv[1]);
    if((echo > 1) || (echo < 0))
    {
        return PARAM_RANGE;
    }
    else
    {
        at_user_info.uart_echo = echo?0:1;
        return CONFIG_OK;
    }
}
static int lega_at_recovery(int argc, char **argv)
{
    int ret = 0;

    dbg_at("doing...");
#if defined(CFG_PLF_DUET) && defined(CFG_DUET_5822S) || defined(CFG_PLF_RV32)
    ret = asr_flash_erase(PARTITION_PARAMETER_2, 0, KV_MAX_SIZE);
    ret |= asr_flash_erase(PARTITION_PARAMETER_4, 0, NVDS_MAX_SIZE);
#else
    lega_rtos_enter_critical();
    ret = lega_flash_erase(PARTITION_PARAMETER_2, 0, KV_MAX_SIZE);
    lega_rtos_exit_critical();
#endif

    if(ret)
        return CONFIG_FAIL;
    else
    {
        lega_at_response(CONFIG_OK);
        asr_system_reset();
        return RSP_NULL;
    }
}

static int lega_at_set_max_rftxpwr(int argc, char **argv)
{
    int max_txpwr;
    if(argc < 2)
    {
        return PARAM_MISS;
    }

    max_txpwr = convert_str_to_int(argv[1]);
    if((max_txpwr > TXPWR_MAX) || (max_txpwr < 0))
    {
        dbg_at("max txpwr range [0, %d] dbm.\r\n", TXPWR_MAX);
        return PARAM_RANGE;
    }
    else
    {
        at_user_info.max_txpwr = max_txpwr;

        // max power base on 11B tx power
        rf_txpwr_max = at_user_info.max_txpwr -TXPWR_11B_MAX;

        return CONFIG_OK;
    }
}

extern uint16_t lega_rf_get_center_freq(void);
extern void lega_rfinit_pwr_cal(void);
extern void lega_ram_cfg_clip(uint8_t chn);
extern void lega_dfecail_write(uint16_t chn);
static int lega_at_add_rftxpow(int argc, char**argv)
{
    uint8_t chn = 0;
    float gain = 1.0;

    if(argc < 2)
    {
        return PARAM_MISS;
    }

    gain = atof(argv[1]);

    if (gain < (0 - TXPWR_MAX))
    {
        return PARAM_RANGE;
    }

    if(rf_txpwr_max < 0)
        gain += rf_txpwr_max;

    if(gain > rf_txpwr_max)
    {
        dbg_at("tx limit to %u dbm\n",at_user_info.max_txpwr);

        return PARAM_RANGE;
    }

    pwr_user_set_k = pow(10.0,((double)gain/20.0));

    lega_rfinit_pwr_cal();
    chn = (lega_rf_get_center_freq() -CHN1_FREQ-CHN_FREQ_INTERV)/CHN_FREQ_INTERV;
    lega_dfecail_write(chn);
    lega_ram_cfg_clip(chn);

    return CONFIG_OK;
}

static int lega_at_sleep_mode_set(int argc, char **argv)
{
    int deep_sleep_ms = 0;
    int wakeup_gpio = 0;

    if (argc != 2 && argc != 4)
    {
        dbg_at("param num error!!!");
        return PARAM_MISS;
    }

    if (strcmp(argv[1], "light") == 0)
    {
        sleep_mode = PMU_STATE_LIGHTSLEEP;
        return CONFIG_OK;
    }
#ifdef LIGHT_SLEEP_SUPPORT
    if (strcmp(argv[1], "light_socoff") == 0)
    {
        sleep_mode = PMU_STATE_LIGHTSLEEP_SOC_OFF;
        return CONFIG_OK;
    }
#endif
    else if (strcmp(argv[1], "modem") == 0)
    {
        sleep_mode = PMU_STATE_MODEMSLEEP;
        return CONFIG_OK;
    }
    else if (strcmp(argv[1], "disable") == 0)
    {
        sleep_mode = PMU_STATE_ACTIVE;
        return CONFIG_OK;
    }
    else if (strcmp(argv[1], "deep") == 0)
    {
        if (argc != 4)
        {
            dbg_at("set deep sleep mode, param num error !!!");
            return PARAM_MISS;
        }
        deep_sleep_ms = convert_str_to_int(argv[2]);
        if(deep_sleep_ms == LEGA_STR_TO_INT_ERR || deep_sleep_ms <= 0)
        {
            dbg_at("set deep sleep time, param error !!!");
            return CONFIG_FAIL;
        }
        wakeup_gpio = convert_str_to_int(argv[3]);
        if (wakeup_gpio == LEGA_STR_TO_INT_ERR || !lega_drv_wakeup_gpio_is_valid(wakeup_gpio))
        {
            dbg_at("set deep sleep mode, gpio pin invalid(only pin12/pin13)");
            return CONFIG_FAIL;
        }
        sleep_mode = PMU_STATE_DEEPSLEEP;
        lega_at_response(CONFIG_OK);
        lega_drv_goto_deepsleep(deep_sleep_ms, wakeup_gpio);

        return RSP_NULL;
    }
    else
    {
        dbg_at("unsupported mode %s\n", argv[1]);
        return PARAM_RANGE;
    }
}
static char* uart_flowCtl_to_str(asr_uart_flow_control_t flowCtl)
{
    switch(flowCtl){
        case FLOW_CTRL_DISABLED:     return "DISABLE";
        case FLOW_CTRL_CTS:            return "CTS";
        case FLOW_CTRL_RTS:            return "RTS";
        case FLOW_CTRL_CTS_RTS:        return "CTS_RTS";
        default:                     return "invalid";
    }
}

static char* uart_parity_to_str(asr_uart_parity_t parity)
{
    switch(parity){
        case PARITY_NO:      return "None";
        case PARITY_ODD:     return "ODD";
        case PARITY_EVEN:    return "EVEN";
        default:             return "invalid";
    }
}

static int lega_at_uart_config(int argc, char **argv)
{
    int tempValue;
    asr_uart_dev_t uart_modify_dev = {0};

    if( argc < 5)
    {
        dbg_at("param num not enough!!!\r\n");
        return PARAM_MISS;
    }
    tempValue = convert_str_to_int(argv[1]);
    if(tempValue == LEGA_STR_TO_INT_ERR || tempValue <= 0)
    {
        dbg_at("baud rate param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.baud_rate = tempValue;
    tempValue = convert_str_to_int(argv[2]);
    if((tempValue > 9) || (tempValue < 5))
    {
        dbg_at("data_width param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.data_width = (tempValue - 5);
    tempValue = convert_str_to_int(argv[3]);
    if((tempValue <= 0) || (tempValue > 2))
    {
        dbg_at("stop_bits param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.stop_bits = (tempValue - 1);
    tempValue = convert_str_to_int(argv[4]);
    if((tempValue > 2) || (tempValue < 0))
    {
        dbg_at("parity param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.parity = tempValue;
    tempValue = convert_str_to_int(argv[5]);
    if((tempValue > 3) || (tempValue < 0))
    {
        dbg_at("flow_control param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.flow_control = tempValue;

    dbg_at("uart baud:%d ,databits:%d ,stop:%d ,parity:%s ,flow:%s\r\n",
        (int)uart_modify_dev.config.baud_rate,
        uart_modify_dev.config.data_width+5,
        uart_modify_dev.config.stop_bits+1,
        uart_parity_to_str(uart_modify_dev.config.parity),
        uart_flowCtl_to_str(uart_modify_dev.config.flow_control));

    uart_modify_dev.port = lega_at_uart.port;

    //register uart callback func for receiving at command
    uart_modify_dev.priv = (void *)(at_handle_uartirq);
    uart_modify_dev.config.mode = TX_RX_MODE;
    lega_at_response(CONFIG_OK);

    delay(5000);

    asr_uart_finalize(&uart_modify_dev);
    asr_uart_init(&uart_modify_dev);

    return RSP_NULL;
}

static int lega_at_uart_config_def(int argc, char **argv)
{
    asr_uart_dev_t uart_modify_dev = {0};

    int32_t len = 0;
    int tempValue;
    if( argc < 5)
    {
        dbg_at("config param num not enough!!!\r\n");
        return PARAM_MISS;
    }
    tempValue = convert_str_to_int(argv[1]);
    if(tempValue == LEGA_STR_TO_INT_ERR || tempValue <= 0)
    {
        dbg_at("baud rate param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.baud_rate = tempValue;
    tempValue = convert_str_to_int(argv[2]);
    if((tempValue > 9) || (tempValue < 5))
    {
        dbg_at("data_width param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.data_width = (tempValue - 5);
    tempValue = convert_str_to_int(argv[3]);
    if((tempValue <= 0) || (tempValue > 2))
    {
        dbg_at("stop_bits param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.stop_bits = (tempValue - 1);
    tempValue = convert_str_to_int(argv[4]);
    if((tempValue > 2) || (tempValue < 0))
    {
        dbg_at("parity param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.parity = tempValue;
    tempValue = convert_str_to_int(argv[5]);
    if((tempValue > 3)  || (tempValue < 0))
    {
        dbg_at("flow_control param err!!!\r\n");
        return PARAM_RANGE;
    }
    uart_modify_dev.config.flow_control = tempValue;

    dbg_at("uart baud:%d ,databits:%d ,stop:%d ,parity:%s ,flow:%s\r\n",
        (int)uart_modify_dev.config.baud_rate,
        uart_modify_dev.config.data_width+5,
        uart_modify_dev.config.stop_bits+1,
        uart_parity_to_str(uart_modify_dev.config.parity),
        uart_flowCtl_to_str(uart_modify_dev.config.flow_control));

    len = sizeof(uart_modify_dev);
    asr_flash_kv_set("uart_config_def", &uart_modify_dev,len,1);

    uart_modify_dev.port = lega_at_uart.port;
    uart_modify_dev.config.mode = TX_RX_MODE;
    delay(5000);//wait for all data output, or else have messy code

    //register uart callback func for receiving at command
    uart_modify_dev.priv = (void *)(at_handle_uartirq);
    asr_uart_finalize(&uart_modify_dev);
    asr_uart_init(&uart_modify_dev);
    return CONFIG_OK;
}

static int lega_at_uart_config_def_del(int argc, char **argv)
{
    asr_flash_kv_del("uart_config_def");

    return CONFIG_OK;
}

#ifdef SYSTEM_COREDUMP
#ifndef ALIOS_SUPPORT
extern void handle_coredump_cmd(char *pwbuf, int blen, int argc, char **argv);
#endif
void coredump_command_register(int argc, char **argv)
{
#ifdef ALIOS_SUPPORT
    aos_cli_register_command(&coredump_cmd);
#else
    handle_coredump_cmd(NULL, 0, argc, argv);
#endif
}

static int lega_at_coredump(int argc, char **argv)
{
    coredump_command_register(argc, argv);

    return CONFIG_OK;
}
#endif

#define VLIST_DEBUG_SIZE 1024
char v_list[VLIST_DEBUG_SIZE] = {0};
static int lega_at_vlist(int argc, char **argv)
{
    vTaskList(v_list);
    at_rspinfor("%s",v_list);
    at_rspinfor("left free heap %d\r\n",xPortGetFreeHeapSize());
    memset(v_list,0,VLIST_DEBUG_SIZE);

    return CONFIG_OK;
}

int lega_at_show_command(int argc, char **argv)
{
    int i;

    if(argc ==  1)
    {
        at_rspinfor("%s",cli_cmd->cmds[0]->help);

        for(i = 1; i < cli_cmd->cmds_num; i++)
        {
            //cmd begin with "lega_" is internal cmd
            if(memcmp(cli_cmd->cmds[i]->name, "lega", 4) != 0)
            {
                at_rspinfor("%s\r\n",cli_cmd->cmds[i]->name);
            }
        }
    }
    else if(argc == 2)
    {
        for(i = 0; i < cli_cmd->cmds_num; i++)
        {
            if(strcmp(argv[1],cli_cmd->cmds[i]->name) == 0)
            {
                if(cli_cmd->cmds[i]->help)
                {
                    at_rspinfor("%s",cli_cmd->cmds[i]->help);
                }

                break;
            }
        }
        if(i == cli_cmd->cmds_num)
        {
            at_rspinfor("help not support cmd %s ",argv[1]);
            return CONFIG_FAIL;
        }

    }
    else
    {
        return CONFIG_FAIL;
    }

    return CONFIG_OK;
}


#ifdef ASR_PROFILE_ON
//only for altos
static int lega_at_profile_init(int argc, char **argv)
{
    uint32_t profile_reg_value = *(volatile uint32_t *)(0x40000908) & (~(0xf << 4));
    (*(volatile uint32_t *)(0x40000908)) = profile_reg_value | (0xd << 4);
    dbg_at("profile set init\r\n");
    return CONFIG_OK;
}

static int lega_at_profile_set(int argc, char **argv)
{
    //pin mux part
    uint8_t pad_num;
    pad_num = convert_str_to_int(argv[1]);
    uint32_t pad_reg_addr = PINMUX_REG_BASE + 4*(pad_num>>3); // pinmux register address of pad pad_num
    uint32_t reg_offset = (pad_num%8)*4;  // offset from pad_reg_addr in bits

    dbg_at("profile set pin num: %d\r\n",pad_num);

    *(volatile uint32_t *)(pad_reg_addr) &= ~(0xf<<reg_offset); // mask
    *(volatile uint32_t *)(pad_reg_addr) |= (0x5<<reg_offset); // set pinmux value
    return CONFIG_OK;
}
#endif

/*
 ************************************************************
 *                    COMMON AT CMD END
 *
 ************************************************************
 */

cmd_entry comm_help = {
    .name = "help",
    .help = "AT command as below, use \"help name\" for more details\r\n",
    .function = lega_at_show_command,
};
cmd_entry comm_dbg = {
    .name = "dbg",
    .help = "cmd format:   dbg <onoff>\r\n\
cmd function: show at set execute prompt info\r\n\
              onoff: 0:off, 1:on, default off\r\n",
    .function = lega_at_dbg,
};

cmd_entry comm_reset = {
    .name = "reset",
    .help = "cmd format:   reset\r\n\
cmd function: soc soft reset\r\n",
    .function = lega_at_reset,
};
cmd_entry comm_version = {
    .name = "version",
    .help = "cmd format:   version\r\n\
cmd function: show release version\r\n",
    .function = lega_at_show_version,
};
cmd_entry comm_echo = {
    .name = "echo",
    .help = "cmd format:   echo 1/0\r\n\
cmd function: open or close uart display for input cmd\r\n",
    .function = lega_at_echo,
};
cmd_entry comm_recovery = {
    .name = "recovery",
    .help = "cmd format:   recovery\r\n\
cmd function: erase all kv in flash and reboot\r\n",
    .function = lega_at_recovery,
};
cmd_entry comm_setmaxpwr = {
    .name = "set_max_txpwr",
    .help = "cmd format:   set_max_txpwr power\r\n\
cmd function: set max RF tx power\r\n",
    .function = lega_at_set_max_rftxpwr,
};
cmd_entry comm_setpwr = {
    .name = "set_txpwr",
    .help = "cmd format:   set_txpwr power\r\n\
cmd function: add or decrease RF tx power base on current power\r\n",
    .function = lega_at_add_rftxpow,
};
cmd_entry comm_sleep = {
    .name = "sleep",
    .help = "cmd format:   sleep light\r\n\
cmd function: set sleep mode to light sleep\r\n\
cmd format:   sleep modem\r\n\
cmd function: set sleep mode to modem sleep\r\n\
cmd format:   sleep disable\r\n\
cmd function: disable light/modem sleep\r\n\
cmd format:   sleep deep TIME GPIO\r\n\
cmd function: enter deep sleep and wakeup after TIME or by GPIO(12/13)\r\n\
for example 'sleep deep 1000 13' means enter deep sleep now and wakeup after 1000ms or by GPIO13\r\n",
    .function = lega_at_sleep_mode_set,
};
cmd_entry comm_uart_config = {
    .name = "uart_config",
    .help = "cmd format:   uart_config baud_rate data_bit stop_bit parity_bit flow_ctrl\r\n\
data_bit:5,6,7,8,9\r\n\
stop_bit:1,2\r\n\
parity_bit:0:no parity,1:odd,2:even\r\n\
flow_ctrl:0:flow_ctrl disable,1:flow_ctrl cts,2:flow_ctrl rts,3:flow_ctrl cts rts\r\n\
cmd function: set uart config param\r\n",
    .function = lega_at_uart_config,
};
cmd_entry comm_uart_config_def = {
    .name = "uart_config_def",
    .help = "cmd format:   uart_config baud_rate data_bit stop_bit parity_bit flow_ctrl\r\n\
data_bit:5,6,7,8,9\r\n\
stop_bit:1,2\r\n\
parity_bit:0:no parity,1:odd,2:even\r\n\
flow_ctrl:0:flow_ctrl disable,1:flow_ctrl cts,2:flow_ctrl rts,3:flow_ctrl cts rts\r\n\
cmd function: set uart config param & save to kv\r\n",
    .function = lega_at_uart_config_def,
};
cmd_entry comm_uart_config_def_del = {
    .name = "uart_config_def_del",
    .help = "cmd format:   uart_config_def_del\r\n\
cmd function: del uart def config param from kv\r\n",
    .function = lega_at_uart_config_def_del,
};
#ifdef SYSTEM_COREDUMP
cmd_entry comm_coredump = {
    .name = "coredump",
    .help = "cmd format:   coredump all\r\n\
cmd function: dump the key memory and information stored after system crashed, use together with DOGO tool\r\n",
    .function = lega_at_coredump,
};
#endif
cmd_entry comm_vtasklist = {
    .name = "vtasklist",
    .help = "cmd format:   vtasklist\r\n\
cmd function: show vtasklist status\r\n",
    .function = lega_at_vlist,
};

#ifdef ASR_PROFILE_ON
cmd_entry comm_profile_init = {
    .name = "profile_init",
    .help = "cmd format:   profile\r\n\
cmd function: profile debug init\r\n",
    .function = lega_at_profile_init,
};

cmd_entry comm_profile_set = {
    .name = "profile_set",
    .help = "cmd format:   profile\r\n\
cmd function: profile debug\r\n",
    .function = lega_at_profile_set,
};
#endif

void lega_at_comm_cmd_register(void)
{
    lega_at_cmd_register(&comm_help);
    lega_at_cmd_register(&comm_dbg);
    lega_at_cmd_register(&comm_reset);
    lega_at_cmd_register(&comm_version);
    lega_at_cmd_register(&comm_echo);
    lega_at_cmd_register(&comm_recovery);
    lega_at_cmd_register(&comm_setmaxpwr);
    lega_at_cmd_register(&comm_setpwr);
    lega_at_cmd_register(&comm_sleep);
    lega_at_cmd_register(&comm_uart_config);
    lega_at_cmd_register(&comm_uart_config_def);
    lega_at_cmd_register(&comm_uart_config_def_del);
#ifdef SYSTEM_COREDUMP
    lega_at_cmd_register(&comm_coredump);
#endif
    lega_at_cmd_register(&comm_vtasklist);
#ifdef ASR_PROFILE_ON
    lega_at_cmd_register(&comm_profile_init);
    lega_at_cmd_register(&comm_profile_set);
#endif
}

/*
 ************************************************************
 *               at cmd register start
 *
 ************************************************************
 */
void lega_at_cmd_register(cmd_entry *cmd)
{
    if(cli_cmd->cmds_num > AT_MAX_COMMANDS - 1)
    {
        printf("register cmd:%s fail,reach max cmd table list.",cmd->name);
        return;
    }

    cli_cmd->cmds[cli_cmd->cmds_num] = cmd;
    cli_cmd->cmds_num++;
}


extern void lega_at_wifi_cmd_register(void);
extern void lega_at_dbg_cmd_register(void);
extern void lega_at_ate_cmd_register(void);
extern void lega_at_ble_cmd_register(void);
extern void lega_at_net_cmd_register(void);
extern void lega_at_user_cmd_register(void);
extern void lega_at_cloud_cmd_register(void);
extern void lega_at_ota_cmd_register(void);
extern void lega_at_sigma_cmd_register(void);
#ifdef CFG_RFOTA_WIFITX
extern void lega_at_rfota_wifi_cmd_register(void);
#endif
extern void lega_at_matter_cmd_register(void);
void lega_at_cmd_register_all(void)
{
    cli_cmd = lega_rtos_malloc(sizeof(struct cli_cmd_t));
    if(cli_cmd == NULL)
    {
        printf("cli_cmd malloc failed!");
        return;
    }

#ifdef CFG_RFOTA_WIFITX
    //lega_at_rfota_wifi_cmd_register();
#endif

    lega_at_comm_cmd_register();
#ifndef _VENDOR_MIDEA_
    //lega_at_wifi_cmd_register();
#endif
#ifdef CFG_DUET_BLE
    //lega_at_ble_cmd_register();
#endif
    //lega_at_test_cmd_register();
    //lega_at_dbg_cmd_register();
    //lega_at_ate_cmd_register();
    //lega_at_net_cmd_register();
    lega_at_user_cmd_register();
    //lega_at_cloud_cmd_register();
    //lega_at_ota_cmd_register();
    #ifdef CFG_SIGMA_ADAPTER
    //lega_at_sigma_cmd_register();
    #endif
    lega_at_matter_cmd_register();
}
/*
 ************************************************************
 *               at cmd register end
 *
 ************************************************************
 */

/*
 ************************************************************
 *           AT task and cmd process start
 *
 ************************************************************
 */
void at_handle_uartirq(char ch)
{
    uint32_t msg_queue_elmt = 0;

    if(AT_Task_Handler && !(((start_idx + 1)%UART_CMD_NB) == cur_idx))
    {
        if(ch =='\r')//in windows platform '\r\n' added to end
        {
            uart_buf[start_idx][uart_idx]='\0';
            uart_buf_len[start_idx]=uart_idx;
            uart_idx=0;
            if(start_idx == UART_CMD_NB -1)
                start_idx = 0;
            else
                start_idx++;

            lega_rtos_push_to_queue(&at_task_msg_queue, &msg_queue_elmt, LEGA_NO_WAIT);
        }
        else if(ch =='\b')
        {
            if(uart_idx>0)
                uart_buf[start_idx][--uart_idx] = '\0';
        }
        else if(ch > (MIN_USEFUL_DEC - 1) && ch<(MAX_USEFUL_DEC + 1))
        {
            uart_buf[start_idx][uart_idx++] = ch;
            if(uart_idx > UART_RXBUF_LEN - 1)
            {
                dbg_at("error:uart char_str length must <= 127,return\n");
                uart_idx=0;
            }
        }
    }
}

/*
 *********************************************************************
 * @brief make argv point to uart_buf address,return number of command
 *
 * @param[in] buf point to uart_buf
 * @param[in] argv point to char *argv[ARGCMAXLEN];
 *
 * @return number of command param
 *********************************************************************
 */
static int parse_cmd(char *buf, char **argv)
{
    int argc = 0;
    //buf point to uart_buf
    while((argc < ARGCMAXLEN) && (*buf != '\0'))
    {
        argv[argc] = buf;
        argc++;
        buf++;
        //space and NULL character
        while((*buf != ' ') && (*buf != '\0'))
            buf++;
        //one or more space characters
        while(*buf == ' ')
        {
            *buf = '\0';
            buf ++;
         }
    }
    return argc;
}

void lega_at_cmd_protect(void)
{
    if(lega_rtos_get_semaphore(&at_cmd_protect, AT_CMD_TIMEOUT))
        dbg_at("pre cmd is running\n");
}

void at_command_process(void)
{
    int i, argc;
    char *argv[ARGCMAXLEN];
    int ret;

    if((argc = parse_cmd(uart_buf[cur_idx], argv)) > 0)
    {
        for(i = 0; i < cli_cmd->cmds_num; i++)
        {
            if(strcmp(argv[0], cli_cmd->cmds[i]->name) == 0)
            {
                lega_at_cmd_protect();
                ret = cli_cmd->cmds[i]->function(argc, argv);
                lega_at_response(ret);
                break;
            }
        }
    }
    memset(uart_buf[cur_idx],0,UART_RXBUF_LEN);

    if(cur_idx == UART_CMD_NB - 1)
    {
        cur_idx = 0;
    }else
        cur_idx ++;
}

#ifdef ATCMD_WIFI_SUPPORT
extern int32_t lega_at_wifi_setup();
#endif
void AT_task(lega_thread_arg_t arg)
{
    uint32_t msg_queue_elmt;
    int32_t ret = 0;

#ifdef ATCMD_WIFI_SUPPORT
    ret = lega_at_wifi_setup();
#else
    ret = 1;
#endif
    if(ret)
        lega_rtos_set_semaphore(&at_cmd_protect);


    while(1)
    {
        lega_rtos_pop_from_queue(&at_task_msg_queue, &msg_queue_elmt, LEGA_WAIT_FOREVER);

        if(!at_user_info.uart_echo)
        {
            at_rspinfor("%s",uart_buf[cur_idx]);
        }

        at_command_process();
    }
}

static uint8_t mac_addr_asc_to_hex(char param)
{
    uint8_t val;

    if((param >= 48) && (param <= 57))
        val = param-48;
    else if((param >= 65) && (param <= 70))
        val = param-55;
    else if((param >= 97) && (param <= 102))
        val = param-87;
    else
    {
        dbg_at("wifi_set_mac_addr:error param\r\n");
        val = 0xA;
    }
    return val;
}

void lega_mac_addr_string_to_array(char *input_str, uint8_t *mac_addr)
{
    int i;
    for(i = 0; i < (6<<1); i++)
    {
        if(i%2 || i==1)
            mac_addr[i>>1] = mac_addr_asc_to_hex(input_str[i]) | (mac_addr[i>>1]&0xF0);
        else
            mac_addr[i>>1] = (mac_addr_asc_to_hex(input_str[i])<<4) | (mac_addr[i>>1]&0xF);
    }
}

extern void lega_at_wifi_event_cb_register(void);
extern void lega_at_net_socket_init(void);
#define     AT_TASK_NAME                "AT_task"
#define     AT_TASK_PRIORITY            (20)
#define     AT_TASK_STACK_SIZE          3072
int lega_at_init(void)
{
    if(!AT_Task_Handler)
    {
        lega_rtos_init_semaphore(&at_cmd_protect, 0);

        //lega_at_net_socket_init();
#ifdef ATCMD_WIFI_SUPPORT
        lega_at_wifi_event_cb_register();
#endif
        lega_rtos_init_queue(&at_task_msg_queue, "AT_TASK_QUEUE", sizeof(uint32_t), AT_QUEUE_SIZE);
        lega_rtos_create_thread(&AT_Task_Handler, AT_TASK_PRIORITY, AT_TASK_NAME, AT_task, AT_TASK_STACK_SIZE, 0);
    }
    return 0;
}

int lega_at_deinit(void)
{
    if(AT_Task_Handler)
    {
        lega_rtos_delete_thread(&AT_Task_Handler);
        AT_Task_Handler = NULL;
        lega_rtos_deinit_queue(&at_task_msg_queue);
        at_task_msg_queue = NULL;
    }
    return 0;
}
/*
 ************************************************************
 *           AT task and cmd process end
 *
 ************************************************************
 */

#endif
