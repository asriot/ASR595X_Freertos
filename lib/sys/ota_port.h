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
#ifndef _OTA_PORT_H_
#define _OTA_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

//if defined, erase ota region in bootloader in order to reduce interference to network, becasue sector erase may cause more than 70ms time
#ifdef _OTA_REGION_ERASE_IN_BOOTLOADER_
    #define _OTA_REGION_ALWAYS_ERASE_IN_BOOTLOADER_
#endif

//if defined, fw version both in header and image will be checked to anti version roll back attack
//note that this feature is not required for ali certification, and this macro need to disabled for ali certification
//#define _FOTA_ANTI_VERSION_ROLL_BACK_EN_

#ifdef _FLASH_DIRECT_BOOT_EN_ //flash direct run
#define FLASH_OTA_INFO_ADDR         0x00010000
#else
#define FLASH_OTA_INFO_ADDR         0x80010000
#endif

#define FLASH_EMPTY_DATA            0xFFFFFFFF

#define FLASH_ACCESS_CONTROL0_DEFAULT_VALUE 0x37053977
#define FLASH_ACCESS_CONTROL1_DEFAULT_VALUE 0x000014e5

#if (defined LEGA_A0V1)
#define IMAGE_TOKEN                     "WIFI 5501 A0V1"
#elif (defined LEGA_A0V2)
#define IMAGE_TOKEN                     "WIFI 5501 A0V2"
#endif
#define IMAGE_RESERVED_DATA             0xFF

#define OTA_TAG_VALUE                   0x00000000
#define FLASH_REMAPPING_EN_VALUE        0x00000000
#define IMAGE_COMPRESS_EN_VALUE         0x00000000
#define OTA_REGION_DIRTY_FLAG_VALUE        0x00000000
#define FLASH_REMAPPING_BANK0_VALUE        0xFFFFFFFF
#define FLASH_REMAPPING_BANK1_VALUE        0x00000000

//size of image header
#define IMAGE_HEADER_SIZE               128
#define IMAGE_TOKEN_SIZE                32
#define IMAGE_APP_VERSION_MAX_SIZE      32
#define FLASH_REMAPPING_EN_SIZE         4
#define FLASH_REMAPPING_BANK_SIZE       4
#define OTA_FLAG_SIZE                   4
#define IMAGE_COMPRESS_EN_SIZE          4
#define IMAGE_LENGTH_SIZE               4
#define IMAGE_CRC_SIZE                  4
#define APP_LENGTH_SIZE                 4
#define APP_CRC_SIZE                    4
#define OTA_REGION_DIRTY_FLAG_SIZE      4
#define IMAGE_ROLL_BACK_FLAG_SIZE       4
#define IMAGE_VERIFY_DONE_SIZE          4
#define IMAGE_RESERVED_SIZE             (IMAGE_HEADER_SIZE - IMAGE_TOKEN_SIZE - IMAGE_APP_VERSION_MAX_SIZE \
                                            - FLASH_REMAPPING_EN_SIZE - FLASH_REMAPPING_BANK_SIZE - OTA_FLAG_SIZE - IMAGE_COMPRESS_EN_SIZE \
                                            - IMAGE_LENGTH_SIZE - IMAGE_CRC_SIZE - APP_LENGTH_SIZE - APP_CRC_SIZE \
                                            - OTA_REGION_DIRTY_FLAG_SIZE - IMAGE_ROLL_BACK_FLAG_SIZE - IMAGE_VERIFY_DONE_SIZE)

//size of image
#define IMAGE_APP_VERSION_SIZE          24 //e.g. app-1.0.2-20181115.1553

//offset in image header
#define TOKEN_OFFSET                    0
#define APP_VERSION_OFFSET              (TOKEN_OFFSET + IMAGE_TOKEN_SIZE)

#define IMAGE_HEADER_OFFSET             0x0 //offset in flash region
#define OTA_IMAGE_CRC_OFFSET            (IMAGE_TOKEN_SIZE + IMAGE_APP_VERSION_MAX_SIZE \
                                            + FLASH_REMAPPING_EN_SIZE + FLASH_REMAPPING_BANK_SIZE + OTA_FLAG_SIZE + IMAGE_COMPRESS_EN_SIZE \
                                            + IMAGE_LENGTH_SIZE)

//offset in pure image
#define IMAGE_APP_VERSION_OFFSET            0x100

#define _LEGA_OTA_DBG_EN_
#ifdef _LEGA_OTA_DBG_EN_
    #define LEGA_OTA_DBG_MSG printf
#else
    #define LEGA_OTA_DBG_MSG(...)
#endif

#define OTA_VERIFY_TOKEN 0xACDF160B
struct OTA_INFO
{
    char token[IMAGE_TOKEN_SIZE];
    char app_version[IMAGE_APP_VERSION_MAX_SIZE];
    uint32_t flash_remapping_en;
    uint32_t flash_remapping_bank;
    uint32_t ota_flag;
    uint32_t image_compress_en;
    uint32_t ota_image_size;
    uint32_t ota_image_crc;
    uint32_t app_image_size; //for image compress, image length after de-compress
    uint32_t app_image_crc; //for image compress, image length after de-compress
    uint32_t ota_region_dirty_flag;
    uint32_t image_roll_back_flag;
    uint32_t verify_done;
    uint8_t reserved[IMAGE_RESERVED_SIZE];
};

typedef enum {
    LEGA_OTA_FINISH,
    LEGA_OTA_BREAKPOINT,
    LEGA_OTA_VERIFY,
    LEGA_OTA_FINISH_NOVERIFY,
} LEGA_OTA_RES_TYPE_E;

typedef struct  {
    unsigned int dst_adr;
    unsigned int src_adr;
    unsigned int len;
    unsigned short crc;
    unsigned int  upg_flag;
    unsigned char boot_count;
    unsigned int  rec_size;
    unsigned int  splict_size;
    int off_bp;               /*Break point offset*/
    LEGA_OTA_RES_TYPE_E  res_type; /*result type: OTA_FINISH, OTA_BREAKPOINT*/
    unsigned short param_crc; /*Parameter crc*/
} __attribute__((packed)) lega_ota_boot_param_t;

int lega_ota_init(void *something);
int lega_ota_write(int *off_set, char *in_buf, int in_buf_len);
int lega_ota_read(int *off_set, char *out_buf, int out_buf_len);
int lega_ota_set_boot(void *something);
int lega_ota_rollback(void *something);
const char *lega_ota_get_version(unsigned char dev_type);
#ifdef AOS_COMP_MSMART
void lega_ota_clear_ota_flag(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //_OTA_PORT_H_
