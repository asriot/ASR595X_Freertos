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
#ifndef __SYSTEM_COREDUMP_H
#define __SYSTEM_COREDUMP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SYSTEM_COREDUMP

#define GENERAL_SOC_MEMORY_SIZE  (192*1024+96*1024)
#define SOC_VERSION              0x2
#define COREDUMP_READ_ONCE_SIZE  512

#define COREDUMP_INFO_SIZE       sizeof(struct coredump_info)
#define MEMORY_START_ADDRESS     0x20FD0000
#define ASRW_START_ADDRESS       0x60000000
#define COREDUMP_SW_VERSION_LEN  32
#define COREDUMP_ASSER_MSG_LEN   64
#define COREDUMP_END_STRING      "$#$"
#define COREDUMP_REAL_END_STRING "@^@"
#define COREDUMP_MAGIC_NUMBER    0xCFEDCFED
//#define STR(x)                   #x
#define RF_ADDR_END              0xE5
#define COREDUMP_FLASH_WRITE_MS  16000
// 111111111100010000000111101111, (bit 4: DMA_CFG(can't read), bit 9~15: reserved, bit 17~19: uart(can't read), bit 30~31: reserved)
#define PERI_USABLE_MASK_ALL     0x3FF101EF
#define PERI_PER_GROUP_SIZE      0x1000
#define PERI_MAX_GROUP_NUM       33
#define COREDUMP_USAGE_STRING    "coredump [ram | flash] [all | pos [memory | wifi | info | rf | peri]]"

// (36KB)
#define ALTO_PERI_ADDR1_START     0x40000000
#define ALTO_PERI_ADDR1_END       0x40008FFF

// 4KB
#define ALTO_PERI_ADDR2_START     0x40010000
#define ALTO_PERI_ADDR2_END       0x40010FFF

// 40KB
#define ALTO_PERI_ADDR3_START     0x40014000
#define ALTO_PERI_ADDR3_END       0x4001DFFF

// 64KB
#define ALTO_PERI_ADDR4_START     0x40030000
#define ALTO_PERI_ADDR4_END       0x4003FFFF

// shared memory(191KB)
#define SHARED_MEMORY_ADDR_START 0x60000000
#define SHARED_MEMORY_ADDR_END   0x6002FFFF

// alto DFE(120Bytes)
#define ALTO_DFE_ADDR_START      0x60940000
#define ALTO_DFE_ADDR_END        0x60940078

// ASRW MAC Address Base(64KB)
#define MAC_BASE_ADDR_START      0x60B00000
#define MAC_BASE_ADDR_END        0x60B0FFFF

// ASRW Modem (PHY) Address Base(0x60C00000) and RIUKarst Register, RIU Register Address Base(0x60C0B000)
// (64KB)
#define MDM_PHY_BASE_ADDR_START  0x60C00000
#define MDM_PHY_BASE_ADDR_END    0x60C0FFFF

#define ASRW_RW_MEMORY_SIZE      ((SHARED_MEMORY_ADDR_END-SHARED_MEMORY_ADDR_START+1)+  \
                                 (ALTO_DFE_ADDR_END-ALTO_DFE_ADDR_START+1)+             \
                                 (MAC_BASE_ADDR_END-MAC_BASE_ADDR_START+1)+             \
                                 (MDM_PHY_BASE_ADDR_END-MDM_PHY_BASE_ADDR_START+1))

enum coredump_pos
{
    COREDUMP_POS_MEMORY,
    COREDUMP_POS_WIFI,
    COREDUMP_POS_INFO,
    COREDUMP_POS_PERI,
    COREDUMP_POS_RF,
    COREDUMP_POS_MAX
};

enum coredump_type
{
    COREDUMP_TYPE_HARDFAULT,
    COREDUMP_TYPE_ASSERT,
    COREDUMP_TYPE_WDG,
    COREDUMP_TYPE_MAX
};

enum PERI_TYPE
{
    SYS_CON,
    GPIO0,
    GPIO1,
    QSPI_CFG,
    DMA_CFG,
    OTP,
    SEC_FLASH_CTRL_CFG,
    SDIO,
    RETENTION_MST,
    WDT = 16,
    UART0,
    UART1,
    UART2,
    SPI0,
    SPI1,
    SPI2,
    TIMER,
    PWM,
    ADC,
    I2C0,
    I2C1,
    CACHE_CFG,
    CYPT310_CFG,
};

struct coredump_info
{
    unsigned int  ra;
    unsigned int  sp;
    unsigned int  gp;
    unsigned int  tp;
    unsigned int  t0;
    unsigned int  t1;
    unsigned int  t2;
    unsigned int  fp;
    unsigned int  s1;
    unsigned int  a0;
    unsigned int  a1;
    unsigned int  a2;
    unsigned int  a3;
    unsigned int  a4;
    unsigned int  a5;
    unsigned int  a6;
    unsigned int  a7;
    unsigned int  s2;
    unsigned int  s3;
    unsigned int  s4;
    unsigned int  s5;
    unsigned int  s6;
    unsigned int  s7;
    unsigned int  s8;
    unsigned int  s9;
    unsigned int  s10;
    unsigned int  s11;
    unsigned int  t3;
    unsigned int  t4;
    unsigned int  t5;
    unsigned int  t6;

    unsigned int mepc;   // Exception Return
    unsigned int mcause; // EXCCODE in mcause
    unsigned int mtval;
    unsigned int mstatus;
    unsigned int msubm;

    unsigned char soc_version;
    unsigned char coredump_type;
    unsigned char sw_version[COREDUMP_SW_VERSION_LEN];
    unsigned char asser_msg[COREDUMP_ASSER_MSG_LEN];
    unsigned int magic_number;
    unsigned int length;
};

void coredump_read(unsigned char dump_pos, bool src_is_ram);
void coredump_read_all(bool src_is_ram, bool wifi_readable);
void coredump_write(struct coredump_info *dump_info);
void coredump_command_register(int argc, char **argv);
void handle_coredump_cmd(char *pwbuf, int blen, int argc, char **argv);
void coredump_flash_dump(void *cpu_info, char *file, int line);
void coredump_process(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //__SYSTEM_COREDUMP_H

