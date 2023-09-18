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

#include "lega_at_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asr_flash.h"
#include "asr_flash_kv.h"
#include "lega_wlan_api.h"

void asr_matter_reset(int type);
void asr_matter_onoff(int cmd);
void asr_matter_sensors(bool enable, int temp, int humi, int pressure);
void ShutdownChip();
void asr_matter_ota(uint32_t timeout);

int at_matter_reset(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    if (strcmp(argv[1], "fab") == 0)
    {
        printf("resetting fabric info\n");
        asr_matter_reset(0);
    }
    else if (strcmp(argv[1], "factory") == 0)
    {
        printf("resetting factory info\n");
        asr_matter_reset(1);
    }
    else if (strcmp(argv[1], "wificommissioning") == 0)
    {
        printf("resetting to only wifi commissing\n");
        asr_matter_reset(2);
    }
    printf("reset matter done\r\n");
    return CONFIG_OK;
}

int at_matter(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    if (strcmp(argv[1], "stop") == 0)
    {
        ShutdownChip();
        printf("matter stop done\n");
    }
    else if (strcmp(argv[1], "start") == 0)
    {
        printf("not support\n");
    }
    return CONFIG_OK;
}

int at_matter_onoff(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        printf("button on\n");
        asr_matter_onoff(1);
    }
    else if (strcmp(argv[1], "off") == 0)
    {
        printf("button off\n");
        asr_matter_onoff(0);
    }
    return CONFIG_OK;
}

int at_matter_sensors(int argc, char ** argv)
{
    if (argc == 2 && strcmp(argv[1], "disable") == 0)
    {
        printf("sensor simulation disabled\n");
        asr_matter_sensors(false, 0, 0, 0);
    }
    else if (argc == 4)
    {
        int temp, humi, pressure;
        temp     = strtol(argv[1], NULL, 10);
        humi     = strtol(argv[2], NULL, 10);
        pressure = strtol(argv[3], NULL, 10);
        printf("simulate temp:%d humi:%d prssure:%d done\n", temp, humi, pressure);
        asr_matter_sensors(true, temp, humi, pressure);
    }
    else
    {
        printf("para error\n");
    }
    return CONFIG_OK;
}

#if ENABLE_ASR_BRIDGE_SUBDEVICE_TEST
void Sync_SubDevice_test(void);
void Add_SubDevice_test(void);
void Remove_SubDevice_test(void);

int at_subdevice(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    if (strcmp(argv[1], "sync") == 0)
    {
        printf("sync all subdevices\n");
        Sync_SubDevice_test();
    }
    else if (strcmp(argv[1], "add") == 0)
    {
        printf("add light 2\n");
        Add_SubDevice_test();
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        printf("remove light 2\n");
        Remove_SubDevice_test();
    }
    return CONFIG_OK;
}
#endif

int at_matter_ota(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    uint32_t timeout = strtol(argv[1], NULL, 10);
    asr_matter_ota(timeout);
    return CONFIG_OK;
}

int at_matter_dis(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("para error\n");
        return CONFIG_OK;
    }
    uint32_t dis = strtol(argv[1], NULL, 10);

    if (dis < 0xFFF)
        asr_flash_kv_set("chip-factory;discriminator", &dis, 4, 1);

    return CONFIG_OK;
}
/*
 ************************************************************
 *                    USER AT CMD END
 *
 ************************************************************
 */

cmd_entry comm_at_reset_matter = {
    .name     = "matter_reset",
    .help     = "matter_reset fab/factory/wificommissioning",
    .function = at_matter_reset,
};
cmd_entry comm_at_matter = {
    .name     = "matter",
    .help     = "matter start/stop",
    .function = at_matter,
};
cmd_entry comm_at_onoff = {
    .name     = "matter_onoff",
    .help     = "matter_onoff on/off",
    .function = at_matter_onoff,
};
cmd_entry comm_at_simulate_sensors = {
    .name     = "matter_sensor_sim",
    .help     = "matter_sensor_sim disable/[temp humi pressure]",
    .function = at_matter_sensors,
};
#if ENABLE_ASR_BRIDGE_SUBDEVICE_TEST
cmd_entry comm_at_subdevice = {
    .name     = "subdevice",
    .help     = "subdevice sync/add/remove",
    .function = at_subdevice,
};
#endif
cmd_entry comm_at_ota = {
    .name     = "matter_ota",
    .help     = "matter_ota [timeout]",
    .function = at_matter_ota,
};
cmd_entry comm_at_dis = {
    .name     = "matter_dis",
    .help     = "matter_dis [Discriminator]",
    .function = at_matter_dis,
};
void lega_at_matter_cmd_register(void)
{
    lega_at_cmd_register(&comm_at_matter);
    //lega_at_cmd_register(&comm_at_reset_matter);
    lega_at_cmd_register(&comm_at_onoff);
#if ENABLE_ASR_BRIDGE_SUBDEVICE_TEST
    lega_at_cmd_register(&comm_at_subdevice);
#endif
    lega_at_cmd_register(&comm_at_simulate_sensors);
    lega_at_cmd_register(&comm_at_ota);
    lega_at_cmd_register(&comm_at_dis);
}
#endif
