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
#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif
#include "asr_flash.h"

/* Logic partition on flash devices */
const asr_logic_partition_t asr_partitions[] =
{
    [PARTITION_BOOTLOADER] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "Bootloader",
        .partition_start_addr       = BOOTLOADER_FLASH_START_ADDR,
        .partition_length           = BOOTLOADER_MAX_SIZE,    //64k bytes
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_DIS,
    },
    [PARTITION_PARAMETER_1] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "Info",
        .partition_start_addr       = INFO_FLASH_START_ADDR,
        .partition_length           = INFO_MAX_SIZE, // 4k bytes
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_PARAMETER_2] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "KV",
        .partition_start_addr       = KV_FLASH_START_ADDR,
        .partition_length           = KV_MAX_SIZE, //128k bytes: minimum 8k
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_PARAMETER_3] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "OLL", //offline log
        .partition_start_addr       = OLL_FLASH_START_ADDR,
        .partition_length           = OLL_MAX_SIZE, //16k bytes
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_PARAMETER_4] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "NVDS", //offline log
        .partition_start_addr       = NVDS_FLASH_START_ADDR,
        .partition_length           = NVDS_MAX_SIZE, //8k bytes
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_APPLICATION] =
    {
        .partition_owner            = FLASH_EMBEDDED,
        .partition_description      = "Application",
        .partition_start_addr       = APP_FLASH_START_ADDR,
        .partition_length           = APP_MAX_SIZE, //768k bytes
        .partition_options          = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_OTA_TEMP] =
    {
        .partition_owner           = FLASH_EMBEDDED,
        .partition_description     = "OTA Storage",
        .partition_start_addr      = OTA_FLASH_START_ADDR,
        .partition_length          = OTA_MAX_SIZE, //768k bytes
        .partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    },
    [PARTITION_MATTER_CONFIG] =
    {
        .partition_owner           = FLASH_EMBEDDED,
        .partition_description     = "MATTER Config",
        .partition_start_addr      = MATTER_FLASH_START_ADDR,
        .partition_length          = MATTER_CONFIG_MAX_SIZE, //8k bytes
        .partition_options         = PAR_OPT_READ_EN | PAR_OPT_WRITE_EN,
    }
};

#ifdef LOW_LEVEL_FLASH_RW_SUPPORT
#include "alto_flash_kv.h"
int32_t asr_flash_get_wifi_mac(flash_mac_addr_t* addr)
{
    int32_t inBufLen = sizeof(flash_mac_addr_t);
    alto_flash_kv_get(FLASH_MAC_ADDR_TOKEN_NAME,addr,&inBufLen);
   // printf("%s:%d addr=%02x %02x %02x %02x %02x %02x token=0x%x\n",__FUNCTION__,__LINE__,addr->mac[0],addr->mac[1],addr->mac[2],addr->mac[3],addr->mac[4],addr->mac[5],addr->token);
    return 0;
}
int32_t asr_flash_set_wifi_mac(flash_mac_addr_t* addr)
{
   // printf("%s:%d addr=%02x %02x %02x %02x %02x %02x token=0x%x\n",__FUNCTION__,__LINE__,addr->mac[0],addr->mac[1],addr->mac[2],addr->mac[3],addr->mac[4],addr->mac[5],addr->mac,addr->token);
    return alto_flash_kv_set(FLASH_MAC_ADDR_TOKEN_NAME,addr,sizeof(flash_mac_addr_t),0);
}
#endif

int32_t asr_flash_get_wifi_info(flash_ap_info_t* pst_wlan_info)
{
    int32_t inBufLen = sizeof(flash_ap_info_t);
    return alto_flash_kv_get(FLASH_AP_INFO_NAME, pst_wlan_info, &inBufLen);
}
int32_t asr_flash_set_wifi_info(flash_ap_info_t* pst_wlan_info)
{
    return alto_flash_kv_set(FLASH_AP_INFO_NAME, pst_wlan_info, sizeof(flash_ap_info_t), 0);
}