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

#ifndef _ASR_MATTER_CONFIG_H_
#define _ASR_MATTER_CONFIG_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif
#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER

#define ASR_CONFIG_BASE PARTITION_MATTER_CONFIG

typedef enum ASR_MATTER_PARTITION_T
{
    ASR_ITERATION_COUNT_PARTITION = 0x00,
    ASR_SALT_PARTITION,
    ASR_VERIFIER_PARTITION,
    ASR_DISCRIMINATOR_PARTITION,
    ASR_DAC_CERT_PARTITION,
    ASR_DAC_KEY_PARTITION,
    ASR_DAC_PUB_KEY_PARTITION,
    ASR_PAI_CERT_PARTITION,
    ASR_CERT_DCLRN_PARTITION,
    ASR_VENDOR_NAME_PARTITION,
    ASR_VENDOR_ID_PARTITION,
    ASR_PRODUCT_NAME_PARTITION,
    ASR_PRODUCT_ID_PARTITION,
    ASR_CHIP_ID_PARTITION,
    ASR_FACTORY_HASH_PARTITION,
    ASR_MATTER_PARTITION_MAX,
} asr_matter_partition_t;

int32_t asr_factory_config_read(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t buf_len, uint32_t * out_len);
int32_t asr_factory_config_write(uint8_t * configbuffer, uint32_t buf_len);
int32_t asr_factory_config_buffer_write(asr_matter_partition_t matter_partition, const void * buf,
                                       uint32_t buf_len);
uint8_t asr_factory_check();
int32_t asr_factory_dac_prvkey_get(uint8_t *pRdBuf, uint32_t *pOutLen);
#endif

#ifdef __cplusplus
}
#endif
#endif //_ASR_MATTER_CONFIG_H_