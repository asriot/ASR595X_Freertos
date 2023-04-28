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
#include "asr_alto_efuse.h"
#include "asr_efuse.h"
#include "asr_alto_port.h"


efuse_blk_struct_t efuse_blk[EFUSE_MAX_BLK] = {{ EFUSE_USER_BLK0_ADDR, EFUSE_ADDR0, EFUSE_USER_BLK0_LENTH}
                                              , { EFUSE_USER_BLK1_ADDR, EFUSE_ADDR1, EFUSE_USER_BLK1_LENTH}
                                              , { EFUSE_USER_BLK2_ADDR, EFUSE_ADDR2,EFUSE_USER_BLK2_LENTH}
                                              , { EFUSE_USER_BLK3_ADDR, EFUSE_ADDR3,EFUSE_USER_BLK3_LENTH}
                                              , { EFUSE_USER_BLK4_ADDR, EFUSE_ADDR4,EFUSE_USER_BLK4_LENTH}
                                              , { EFUSE_USER_BLK5_ADDR, EFUSE_ADDR5, EFUSE_USER_BLK5_LENTH}
                                              , { EFUSE_USER_BLK6_ADDR, EFUSE_ADDR6, EFUSE_USER_BLK6_LENTH}
                                              , { EFUSE_USER_BLK7_ADDR, EFUSE_ADDR7, EFUSE_USER_BLK7_LENTH}
                                              , { EFUSE_USER_BLK8_ADDR, EFUSE_ADDR8,EFUSE_USER_BLK8_LENTH}
                                              , { MAGIC_NUM_EFUSE_ADDR, MAGIC_NUM_EFUSE_ADDR, EFUSE_MAGIC_NUM_LENTH}
                                               , { EFUSE_BOOT_EN_EFUSE_ADDR, EFUSE_BOOT_EN_EFUSE_ADDR, EFUSE_BOOT_EN_LENTH}
                                               , { CERT_KEY_EFUSE_ADDR, CERT_KEY_EFUSE_ADDR,EFUSE_CERT_KEY_LENTH}
                                               , { FLASH_ENC_KEY_EFUSE_ADDR, FLASH_ENC_KEY_EFUSE_ADDR,EFUSE_FLASH_TAG_LENTH}
                                               , { MAC0_EFUSE_ADDR, MAC0_EFUSE_ADDR,EFUSE_MAC0_LENTH}
                                               , { RFCALI_START_FUSE_ADDR, RFCALI_START_FUSE_ADDR, EFUSE_CALI0_LENTH}
                                               , { MAC1_RFCALI_START_EFUSE_ADDR, MAC1_RFCALI_START_EFUSE_ADDR, EFUSE_MAC_CALI_LENTH}
                                               , { DIGLDO_EFUSE_ADDR, DIGLDO_EFUSE_ADDR, EFUSE_DIGLDO_LENTH}
                                               , { MIDEA_FT_EFUSE_ADDR, MIDEA_FT_EFUSE_ADDR,EFUSE_ID_FT_LENTH}};






