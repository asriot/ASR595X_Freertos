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
#ifndef __TLV_DECODE_H_
#define __TLV_DECODE_H_

#define MAX_NAME_LEN 16

#define MIXTURE_PRIVATECLASS 0xFF00
#define SINGLE_PRIVATECLASS 0xDF

#define MIXCLASS_PARTITION_TAG 0xFF01
#define MIXCLASS_BOOTCONFIG_TAG 0xFF71
#define MIXCLASS_MATTERCONFIG_TAG 0xFF72
#define MIXCLASS_USERCONFIG_TAG 0xFF73

#pragma pack(1)
typedef struct tlv_area_header
{
    uint32_t magic_num;
    uint32_t crc32_value;
    uint32_t data_len;
    uint8_t data[0];
} * tlv_area_header_t;
#pragma pack()

#pragma pack(1)
typedef struct tlv_header
{
    uint16_t tag;
    uint8_t data_len_flag;
    uint16_t data_len;
    uint8_t data[0];
} * tlv_header_t;
#pragma pack()

typedef struct asr_tlv_context
{
    uint8_t valid;
    tlv_area_header_t tlv_header;
} asr_tlv_context;

#ifndef tlv_htonl
#define tlv_htonl(a)              \
    ((((a) >> 24) & 0x000000ff) | \
     (((a) >> 8) & 0x0000ff00) |  \
     (((a) << 8) & 0x00ff0000) |  \
     (((a) << 24) & 0xff000000))
#endif

#ifndef tlv_htons
#define tlv_htons(a)         \
    ((((a) >> 8) & 0x00ff) | \
     (((a) << 8) & 0xff00))
#endif

typedef int (*asr_tlv_iterator_cb)(tlv_header_t tlv, void *arg1, void *arg2);

int asr_tlv_init(asr_tlv_context *ctx, uint32_t addr);

int asr_tlv_find_by_name(asr_tlv_context *ctx, tlv_header_t *tlv, uint16_t mix_class_tag, char *name);

int asr_tlv_poll_class_members(asr_tlv_context *ctx, uint16_t mix_class_tag, asr_tlv_iterator_cb callback, void *arg1, void *arg2);

#endif // __TLV_DECODE_H_
