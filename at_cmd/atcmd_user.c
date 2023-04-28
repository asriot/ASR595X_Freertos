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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lega_at_api.h"
#ifdef CFG_PLF_RV32
#include "alto_common.h"
#include "asr_flash.h"
#include "asr_flash_kv.h"
#else
#include "lega_common.h"
#endif
#include "lega_wlan_api.h"
/*
 ************************************************************
 *                    USER AT CMD START
 *
 ************************************************************
 */

int at_test_1(int argc, char **argv)
{
    int tempValue;
    switch(argc)
    {
        case 1:
            printf("%s...%s\n",__func__, argv[0]);
            break;
        case 2:
            printf("%s...%s\n", __func__, argv[1]);
            break;
        case 3:
            tempValue = convert_str_to_int(argv[2]);
            if(tempValue == LEGA_STR_TO_INT_ERR)
            {
                printf("error param\n");
                break;
            }
            printf("%s...%s...%d\n", __func__, argv[1], tempValue);
            break;
        default:
            printf("error param\n");
            break;
    }

    return CONFIG_OK;
}

int at_test_2(int argc, char **argv)
{
    printf("%s...\n",__func__);
    return CONFIG_OK;
}

extern void lega_wlan_clear_pmk(void);
int ap_conn(int argc, char **argv)
{
    lega_wlan_init_type_t conf;
    memset(&conf,0,sizeof(lega_wlan_init_type_t));
    conf.wifi_mode = STA;

    printf("doing...");
    switch(argc)
    {
        case 1:
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            if ((strcmp(argv[1],"sta") == 0))
            {
                conf.dhcp_mode = WLAN_DHCP_CLIENT;
                if(strlen((char *)argv[2]) > 32)
                {
                    return PARAM_RANGE;
                }

                strcpy((char*)conf.wifi_ssid, (char*)argv[2]);

                if(argc >= 4)
                {
                    if((strlen(argv[3]) > 4)&&(strlen(argv[3]) < 65))
                        strcpy((char*)conf.wifi_key, (char*)argv[3]);
                    else if((strlen(argv[3]) == 1) && (convert_str_to_int(argv[3]) == 0))
                        memset(conf.wifi_key, 0, 64);
                    else
                    {
                        printf("param4 error, input 0 or pwd(length >= 5)\n");
                        return PARAM_RANGE;
                    }
                }

                if(argc >= 5)
                {
                    conf.channel = convert_str_to_int(argv[4]);
                    if((conf.channel > 13) || (conf.channel < 1))
                    {
                        printf("param5 error,channel range:1~13");
                        return PARAM_RANGE;
                    }
                }
            }

            break;
        default:
            printf("param num error,1~7\r\n");
            return PARAM_RANGE;
    }

    lega_wlan_clear_pmk();
    lega_wlan_open(&conf);

    return RSP_NULL;
}

int at_kv_clear(int argc, char **argv)
{
    asr_flash_erase(PARTITION_PARAMETER_2, 0, KV_MAX_SIZE);
    printf("kv clear done\r\n");
    return CONFIG_OK;
}

/*
 ************************************************************
 *                    USER AT CMD END
 *
 ************************************************************
 */

cmd_entry comm_test1 = {
        .name = "at_test1",
        .help = "at_test1",
        .function = at_test_1,
};
cmd_entry comm_test2 = {
        .name = "at_test2",
        .help = "at_test2",
        .function = at_test_2,
};

cmd_entry comm_conn = {
    .name = "wifi_open",
    .help = "wifi_open sta SSID PASSWORD",
    .function = ap_conn,
};

cmd_entry comm_at_kv_clear = {
    .name = "at_kv_clear",
    .help = "at_kv_clear",
    .function = at_kv_clear,
};

void lega_at_user_cmd_register(void)
{
    lega_at_cmd_register(&comm_test1);
    lega_at_cmd_register(&comm_test2);
    lega_at_cmd_register(&comm_conn);
    lega_at_cmd_register(&comm_at_kv_clear);
}
#endif
