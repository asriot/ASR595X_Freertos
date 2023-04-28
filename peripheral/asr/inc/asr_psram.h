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
#ifndef __ASR_PSRAM_PSRAM_H
#define __ASR_PSRAM_PSRAM_H

#include "asr_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSRAM_CHIP_LY68L6400

#ifdef PSRAM_CHIP_LY68L6400
#define PSRAM_FLASH_SIZE                    0x800000
#else  //PSRAM_CHIP_APS1604_SQR
#define PSRAM_FLASH_SIZE                    0x200000
#endif

#define PSRAM_MCR           (PSRAM_REG_BASE + 0x000)
#define PSRAM_IPCR          (PSRAM_REG_BASE + 0x008)
#define PSRAM_FLSHCR        (PSRAM_REG_BASE + 0x00C)
#define PSRAM_BUF0CR        (PSRAM_REG_BASE + 0x010)
#define PSRAM_BUF1CR        (PSRAM_REG_BASE + 0x014)
#define PSRAM_BUF2CR        (PSRAM_REG_BASE + 0x018)
#define PSRAM_BUF3CR        (PSRAM_REG_BASE + 0x01C)
#define PSRAM_BFGENCR       (PSRAM_REG_BASE + 0x020)
#define PSRAM_SOCCR         (PSRAM_REG_BASE + 0x024)
#define PSRAM_BUF0IND       (PSRAM_REG_BASE + 0x030)
#define PSRAM_BUF1IND       (PSRAM_REG_BASE + 0x034)
#define PSRAM_BUF2IND       (PSRAM_REG_BASE + 0x038)
#define PSRAM_SFAR          (PSRAM_REG_BASE + 0x100)
#define PSRAM_SMPR          (PSRAM_REG_BASE + 0x108)
#define PSRAM_RBSR          (PSRAM_REG_BASE + 0x10C)
#define PSRAM_RBCT          (PSRAM_REG_BASE + 0x110)
#define PSRAM_TBSR          (PSRAM_REG_BASE + 0x150)
#define PSRAM_TBDR          (PSRAM_REG_BASE + 0x154)
#define PSRAM_TBCT          (PSRAM_REG_BASE + 0x158)
#define PSRAM_SR            (PSRAM_REG_BASE + 0x15C)
#define PSRAM_FR            (PSRAM_REG_BASE + 0x160)
#define PSRAM_RSER          (PSRAM_REG_BASE + 0x164)
#define PSRAM_SPNDST        (PSRAM_REG_BASE + 0x168)
#define PSRAM_SPTRCLR       (PSRAM_REG_BASE + 0x16C)
#define PSRAM_SFA1AD        (PSRAM_REG_BASE + 0x180)
#define PSRAM_SFA2AD        (PSRAM_REG_BASE + 0x184)
#define PSRAM_SFB1AD        (PSRAM_REG_BASE + 0x188)
#define PSRAM_SFB2AD        (PSRAM_REG_BASE + 0x18C)
#define PSRAM_DLPV          (PSRAM_REG_BASE + 0x190)
#define PSRAM_RBDR0         (PSRAM_REG_BASE + 0x200)
#define PSRAM_RBDR1         (PSRAM_REG_BASE + 0x204)
#define PSRAM_RBDR2         (PSRAM_REG_BASE + 0x208)
#define PSRAM_LUTKEY        (PSRAM_REG_BASE + 0x300)
#define PSRAM_LCKCR         (PSRAM_REG_BASE + 0x304)
#define PSRAM_LUT0          (PSRAM_REG_BASE + 0x310)
#define PSRAM_LUT1          (PSRAM_REG_BASE + 0x314)
#define PSRAM_LUT2          (PSRAM_REG_BASE + 0x318)
#define PSRAM_LUT3          (PSRAM_REG_BASE + 0x31C)

#define PSRAM_FLASH_A1_BASE      PSRAM_AMBA_BASE
#define PSRAM_FLASH_A1_TOP       (PSRAM_FLASH_A1_BASE + PSRAM_FLASH_SIZE)
#define PSRAM_FLASH_A2_BASE      PSRAM_FLASH_A1_TOP
#define PSRAM_FLASH_A2_TOP       PSRAM_FLASH_A1_TOP
#define PSRAM_FLASH_B1_BASE      PSRAM_FLASH_A1_TOP
#define PSRAM_FLASH_B1_TOP       PSRAM_FLASH_A1_TOP
#define PSRAM_FLASH_B2_BASE      PSRAM_FLASH_A1_TOP
#define PSRAM_FLASH_B2_TOP       PSRAM_FLASH_A1_TOP

#define PSRAM_CMD_STOP           0
#define PSRAM_CMD_CMD            1
#define PSRAM_CMD_ADDR           2
#define PSRAM_CMD_DUMMY          3
#define PSRAM_CMD_MODE           4
#define PSRAM_CMD_MODE2          5
#define PSRAM_CMD_MODE4          6
#define PSRAM_CMD_READ           7
#define PSRAM_CMD_WRITE          8
#define PSRAM_CMD_JMP_ON_CS      9
#define PSRAM_CMD_ADDR_DDR       10
#define PSRAM_CMD_MODE_DDR       11
#define PSRAM_CMD_MODE2_DDR      12
#define PSRAM_CMD_MODE4_DDR      13
#define PSRAM_CMD_READ_DDR       14
#define PSRAM_CMD_WRITE_DDR      15
#define PSRAM_CMD_DATA_LEARN     16
#define PSRAM_CMD_CMD_DDR        17
#define PSRAM_CMD_CADDR          18
#define PSRAM_CMD_CADDR_DDR      19

#define PSRAM_SEQ_ID_READ        5
#define PSRAM_SEQ_ID_WRITE_EVICT 6
#define PSRAM_SEQ_ID_WRITE       7
#define PSRAM_SEQ_ID_READ_ID     8
#define PSRAM_SEQ_ID_QUAD_ENABLE 9
#define PSRAM_SEQ_ID_QUAD_EXIT   10
#define PSRAM_SEQ_ID_MODE_REG_READ   11
#define PSRAM_SEQ_ID_MODE_REG_WRITE  12

#define FLASH_CMD_READ_ID           0x9F
#define FLASH_CMD_READ              0x03
#define FLASH_CMD_FAST_READ         0x0B
#define FLASH_CMD_FAST_READ_QUAD    0xEB
#define FLASH_CMD_WRITE             0x02
#define FLASH_CMD_QUAD_WRITE        0x38
#define FLASH_CMD_WRAPPED_READ      0x8B
#define FLASH_CMD_WRAPPED_WRITE     0x82
#define FLASH_CMD_MODE_REG_READ     0xB5
#define FLASH_CMD_MODE_REG_WRITE    0xB1
#define FLASH_CMD_ENTER_QUAD_MODE   0x35
#define FLASH_CMD_EXIT_QUAD_MODE    0xF5
#define FLASH_CMD_RESET_ENABLE      0x66
#define FLASH_CMD_RESET             0x99
#define FLASH_CMD_BURST_LENGTH_TOGGLE   0xC0

typedef enum {
    PSRAM_CHANNEL_4_9,
    PSRAM_CHANNEL_16_21
} asr_psram_channel;

typedef enum {
    PSRAM_MODE_SPI,
    PSRAM_MODE_QSPI
} asr_psram_mode;

/**
 * psram set the channel used
 * @note  this function must be called before psram config
 * @param[in]  channel  The sets of gpio pads used for psram
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int asr_psram_set_channel(asr_psram_channel channel);

/**
 * psram config
 * @note  this function must be called before use psram address space
 * @param[in]  mode  The psram config mode
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int asr_psram_config(asr_psram_mode mode);

#ifdef __cplusplus
}
#endif

#endif //__ASR_PSRAM_PSRAM_H
