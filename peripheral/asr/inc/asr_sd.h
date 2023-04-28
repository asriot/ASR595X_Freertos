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
#ifndef _ASR_SD_H_
#define _ASR_SD_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_TRC(fmt, ...)             printf(fmt "\n", ## __VA_ARGS__)

#define SD_SOURCE_CLK_MHZ            360
#define SD_BUF_BLK_CNT               8 // range = [1, 8], 512B ~ 4K

/* Standard sd commands (6.0)              type     argument      response */
   /* class 0 */
#define SD_CMD_GO_IDLE_STATE         0    /* bc                            */
#define SD_CMD_ALL_SEND_CID          2    /* bcr                       R2  */
#define SD_CMD_SET_RELATIVE_ADDR     3    /* bcr                       R6  */
#define SD_CMD_SELECT_CARD           7    /* ac     [31:16] RCA        R1b */
#define SD_CMD_SEND_IF_COND          8    /* bcr    [11:8] VHS         R7
                                                    [7:0] check pattern    */
#define SD_CMD_SEND_CSD              9    /* ac     [31:16] RCA        R2  */
#define SD_CMD_SEND_CID              10   /* ac     [31:16] RCA        R2  */
#define SD_CMD_STOP_TRANSMISSION     12   /* ac                        R1b */

  /* class 2 */
#define SD_CMD_SET_BLOCKLEN          16   /* ac     [31:0] block len   R1  */
#define SD_CMD_READ_SINGLE_BLOCK     17   /* adtc   [31:0] data addr   R1  */
#define SD_CMD_READ_MULTIPLE_BLOCK   18   /* adtc   [31:0] data addr   R1  */

  /* class 4 */
#define SD_CMD_SET_BLOCK_COUNT       23   /* ac     [31:0] block cnt   R1  */
#define SD_CMD_WRITE_BLOCK           24   /* adtc   [31:0] data addr   R1  */
#define SD_CMD_WRITE_MULTIPLE_BLOCK  25   /* adtc   [31:0] data addr   R1  */

  /* class 5 */
#define SD_CMD_ERASE_WR_BLK_START    32   /* ac     [31:0] data addr   R1  */
#define SD_CMD_ERASE_WR_BLK_END      33   /* ac     [31:0] data addr   R1  */
#define SD_CMD_ERASE                 38   /* ac     [31:0] erase func  R1b */

  /* class 8 */
#define SD_CMD_APP_CMD               55   /* ac     [31:16] RCA        R1  */

  /* Application specific commands */
#define SD_CMD_APP_SET_BUS_WIDTH     6    /* ac     [1:0] bus width    R1  */
#define SD_CMD_APP_SEND_OP_COND      41   /* bcr    [31:0] OCR         R3  */
#define SD_CMD_APP_SEND_SCR          51   /* adtc                      R1  */

#define SD_RESP_NONE                 0
#define SD_RESP_R1                   1
#define SD_RESP_R1B                  2
#define SD_RESP_R2                   3
#define SD_RESP_R3                   4
#define SD_RESP_R4                   5
#define SD_RESP_R5                   6
#define SD_RESP_R6                   7
#define SD_RESP_R7                   8

#define SD_CARD_V1_1_STD_CAP         0
#define SD_CARD_V2_0_STD_CAP         1
#define SD_CARD_V2_0_HIGH_CAP        2

#define SD_DATA_HOST_TO_CARD         0
#define SD_DATA_CARD_TO_HOST         1

#define SD_CHECK_PATTERN             0xAA

#define SD_BUS_WIDTH_1               0
#define SD_BUS_WIDTH_4               2

#define SD_CARD_4BIT_BUS_SUPPORT     0x04

/* SD Status in R1 */
#define SD_CARD_STATUS_APP_CMD       (1 << 5)

/* SD OCR Register Masks */
#define SD_OCR_CARD_POWER_UP_STATUS  (1 << 31)
#define SD_OCR_CARD_CAP_STATUS       (1 << 30)

/*******************************************/
/* SD Error Codes                          */
/*******************************************/
#define SD_OK                        0
#define SD_CMD_RESP_TIMEOUT          1
#define SD_CMD_CRC_ERR               2
#define SD_CMD_INDEX_ERR             3
#define SD_CHECK_PATTERN_ERR         4
#define SD_APP_NOT_SUPPORT           5
#define SD_INVALID_PARAMETER         6
#define SD_NO_DISK                   7

typedef enum
{
    SD_CLK_1M  = 1,
    SD_CLK_2M  = 2,
    SD_CLK_5M  = 5,
    SD_CLK_10M = 10,
    SD_CLK_15M = 15,
    SD_CLK_20M = 20,
} asr_sd_clk_sel_t;

enum
{
    SD_EVENT_PLUG_IN,
    SD_EVENT_PLUG_OUT
};

typedef void (*asr_sd_evt_handler_t)(int evt, void *param);

typedef struct sd_command_s sd_command_t;
typedef struct asr_sd_card_s asr_sd_card_t;

typedef void (*sdh_hw_init_t)(void);
typedef void (*sdh_hw_startup_t)(void);
typedef void (*sdh_hw_send_cmd_t)(sd_command_t *cmd);
typedef int (*sdh_hw_wait_cmd_done_t)(sd_command_t *cmd);
typedef int (*sdh_hw_wait_data_done_t)(void);
typedef void (*sdh_hw_enable_4bit_bus_t)(asr_sd_card_t *card);
typedef void (*sdh_hw_config_clk_t)(asr_sd_clk_sel_t clk_sel);

typedef struct {
    sdh_hw_init_t              init;
    sdh_hw_startup_t           startup;
    sdh_hw_send_cmd_t          send_cmd;
    sdh_hw_wait_cmd_done_t     wait_cmd_done;
    sdh_hw_wait_data_done_t    wait_data_done;
    sdh_hw_enable_4bit_bus_t   enable_4bit_bus;
    sdh_hw_config_clk_t        config_clk;
} sd_hc_driver_t;

typedef struct {
    uint16_t block_size;
    uint16_t block_cnt;
    bool transfer_dir;
    void *buf;
} sd_data_t;

struct sd_command_s
{
    uint8_t cmd_index;
    uint32_t argument;
    uint8_t resp_type;
    uint16_t resp[8];
    sd_data_t *data;
};

struct asr_sd_card_s
{
    uint32_t cid[4];
    uint32_t csd[4];
    uint32_t scr[2];
    uint32_t max_write_blk_len;
    uint32_t max_read_blk_len;
    uint16_t rca;
    uint8_t card_type;
    uint8_t bus_width_support;
    bool write_blk_partial_support;
    bool read_blk_partial_support;
    asr_sd_evt_handler_t handler;
};

void sd_process_card_insert(void);
void sd_process_card_removal(void);

void asr_sd_init(void);

void asr_sd_reg_handler(asr_sd_evt_handler_t handler);

void asr_sd_startup(void);

int asr_sd_init_card(asr_sd_card_t *card);

int asr_sd_select_card(asr_sd_card_t *card);

int asr_sd_enable_4bit_bus(asr_sd_card_t *card);

void asr_sd_config_clk(asr_sd_clk_sel_t clk_sel);

int asr_sd_erase(uint32_t start, uint32_t end, uint32_t erase_fun);

int asr_sd_write_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr, uint32_t blk_len);

int asr_sd_write_multi_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr,
                                        uint32_t blk_len, uint32_t blk_cnt);

int asr_sd_read_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr, uint32_t blk_len);

int asr_sd_read_multi_block(asr_sd_card_t *card, uint8_t *buf, uint32_t addr,
                                    uint32_t blk_len, uint32_t blk_cnt);

int asr_sd_init_disk(void *disk);

int asr_sd_write_disk(void *disk, uint8_t *buf, uint32_t sector, uint32_t count);

int asr_sd_read_disk(void *disk, uint8_t *buf, uint32_t sector, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif //_ASR_SD_H_