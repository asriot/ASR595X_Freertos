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
/**
 ****************************************************************************************
 *
 * @file asr_rv32.h
 *
 * @brief define risc-v SOC architecture
 *
 *
 ****************************************************************************************
 */


/*************************   **************************************/
#ifndef __ASR_RV32_H__
#define __ASR_RV32_H__

#include "hbird.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITstatus;
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
#define XTAL_26M               26000000

#define SYSTEM_CORE_CLOCK_INIT  (52000000)
#define SYSTEM_BUS_CLOCK_INIT   (52000000)
#define SYSTEM_PERI_CLOCK_INIT  (52000000)

extern uint32_t system_bus_clk;
extern uint32_t system_core_clk;

#define SYSTEM_CORE_CLOCK      system_core_clk
#define SYSTEM_BUS_CLOCK       system_bus_clk

#define SYSTEM_CLOCK           SYSTEM_PERI_CLOCK_INIT

#define SYSTEM_CORE_CLOCK_160M  (160000000)
#define SYSTEM_CORE_CLOCK_80M   (80000000)
#define SYSTEM_BUS_CLOCK_80M    (80000000)


#define SYSTEM_CLOCK_NORMAL    (80000000)

#define HAPS_FPGA 0
#define V7_FPGA 1
#define FPGA_PLATFORM V7_FPGA

#define PINMUX_CTRL_REG0                0x40000004 //pad0-7
#define PINMUX_CTRL_REG1                0x40000008 //pad8-15
#define PINMUX_CTRL_REG2                0x4000000C //pad16-23
#define PINMUX_CTRL_REG3                0x40000010 //pad24-31

#define SYS_REG_BASE                    0x40000000
#define SYS_REG_BASE_FLASH_CLK          ((SYS_REG_BASE + 0x808))
#define PERI_CLK_CFG                    (SYS_REG_BASE + 0x850)

#define WIFI_BLE_FLASH_CLK_CTRL_REG     (0x40000804)
#define APB_PERI_CLK_CTRL_REG           (0x40000808)
#define MCU_CLK_HARD_MODE_REG           (0x40000814)
#define ADC_SDIO_BLE_DEBUG_CTRL_REG     (0x40000908)

#define SYS_REG_BASE_CLK1_ENABLE        (SYS_REG_BASE + 0x840)
#define CC310_CLOCK_ENABLE              (1 << 11)

#define SYS_REG_BASE_CLK1_DISABLE       (SYS_REG_BASE + 0x848)
#define CC310_CLOCK_DISABLE             (1 << 11)

#define REG_INTERRUPT_ENABLE            (SYS_REG_BASE + 0x944)
#define REG_INTERRUPT_DISABLE           (SYS_REG_BASE + 0x948)

#define SYS_CRM_SYS_CLK_CTRL1           *((volatile uint32_t *)(SYS_REG_BASE + 0x950))
#define SYS_CRM_REG_0x804               *((volatile uint32_t *)(SYS_REG_BASE + 0x804))
#define SYS_CRM_UART2_FRAC_DIV          *((volatile uint32_t *)(SYS_REG_BASE + 0x82C))

#define ALWAYS_ON_REGFILE           0x40000A00
#define REG_AHB_BUS_CTRL            *((volatile uint32_t *)(ALWAYS_ON_REGFILE + 0x90))

#define REG_PLF_WAKEUP_INT_EN       (0x1<<23)
#define SDIO_HCLK_EN                    (1 << 4)
#define PWM_CLK_EN                      (1 << 2)
#define WDG_CLK_EN                      (1 << 1)
#define TIMER_SCLK_EN                   0x1

#define SYS_CRM_WIFI_BLK_CLK        *((volatile uint32_t *)(SYS_REG_BASE + 0x85C))
#define MDM_CLKGATEFCTRL0_ADDR      0x60C00874
#define MDM_CLKGATEFCTRL0           *((volatile uint32_t *)(MDM_CLKGATEFCTRL0_ADDR))
#define RTC_REG_RCO32K_ADDR         0x40000A44
#define RTC_REG_RCO32K              *((volatile uint32_t *)(RTC_REG_RCO32K_ADDR))
#define SYS_CRM_CLR_HCLK_REC        *((volatile uint32_t *)(SYS_REG_BASE + 0x844))

#define TRX_PD_CTRL1_REG_ADDR       0x06
#define TRX_PD_CTRL2_REG_ADDR       0x07
#define APLL_PD_CTRL_REG_ADDR       0x0D
#define APLL_RST_CTRL_REG_ADDR      0x0E
#define XO_PD_CTRL_REG_ADDR         0x0F
#define APLL_CLK_PHY_REG_ADDR       0x6B
#define APLL_FCAL_FSM_CTRL_ADDR     0x6E

//efuse memory
typedef struct
{
    uint8_t tx_params[9];           //0x87-0x8F
    uint8_t mac_addr0[6];           //0x90-0x95
    uint8_t reserved0;              //0x96
    uint8_t tmmt1;                  //0x97
    uint8_t tmmt2;                  //0x98
    uint8_t cus_tx_pwr[19];         //0x99-0xab
    uint8_t cal_tx_pwr0[6];         //0xac-0xb1
    uint8_t cus_tx_total_pwr[3];    //0xb2-0xb4
    uint8_t cal_tx_evm0[6];         //0xb5-0xba
    uint8_t ble_tx_pwr0[3];         //0xbb-0xbd
    uint8_t reserved1[2];           //0xbe-0xbf
    uint8_t mac_addr1[6];           //0xc0-0xc5
    uint8_t mac_addr2[6];           //0xc6-0xcb
    uint8_t cal_tx_pwr1[6];         //0xcc-0xd1
    uint8_t cal_tx_evm1[6];         //0xd2-0xd7
    uint8_t cal_tx_pwr2[6];         //0xd8-0xdd
    uint8_t cal_tx_evm2[6];         //0xde-0xe3
    uint8_t ble_tx_pwr1[3];         //0xe4-0xe6
    uint8_t ble_tx_pwr2[3];         //0xe7-0xe9
    uint8_t reserved2[3];           //0xea-0xec
    uint8_t freq_err[2];            //0xed-0xee
    uint8_t reserved3;              //0xef
    uint8_t reserved4[248];         //0xf0-0x1e7
    uint8_t vendor_id[8];           //0x1e8-0x1ef
    uint8_t reserved5[16];          //0x1f0-0x1ff
}efuse_info_t;
#define EFUSE_INFO_START_ADDR       0x87
#define EFUSE_TMMT_PN               0x97
#define EFUSE_TMMT_PN_LEN           2
#define EFUSE_INFO_LEN              (sizeof(efuse_info_t))
#define EFUSE_INFO_CHIP_TYPE_ADDR   0x1F6

#ifndef CFG_FLASH_MAX_SIZE
#define FLASH_MAX_SIZE              0x200000
#else
#define FLASH_MAX_SIZE              CFG_FLASH_MAX_SIZE
#endif

#if (0x200000 == FLASH_MAX_SIZE)
    #ifdef _FLASH_DIRECT_BOOT_EN_
        #define BOOTLOADER_FLASH_START_ADDR 0x00000000
        #define INFO_FLASH_START_ADDR       0x00010000
        #define APP_FLASH_START_ADDR        0x00040000
        #define OTA_FLASH_START_ADDR        0x00100000
        #define KV_FLASH_START_ADDR         0x001E0000
        #define OLL_FLASH_START_ADDR        0x001E4000
    #else
        #define BOOTLOADER_FLASH_START_ADDR 0x80000000
        #define INFO_FLASH_START_ADDR       0x80010000
        #define APP_FLASH_START_ADDR        0x80012000
        #define OTA_FLASH_START_ADDR        0x80100000
        #define KV_FLASH_START_ADDR         0x801EE000
        #define NVDS_FLASH_START_ADDR       0x801FC000
        #define OLL_FLASH_START_ADDR        0x801FE000
    #endif

    #define BOOTLOADER_MAX_SIZE         0x10000
    #define INFO_MAX_SIZE               0x2000
    #define APP_MAX_SIZE                0xEE000
    #define OTA_MAX_SIZE                0xEE000
    #define KV_MAX_SIZE                 0xE000
    #define NVDS_MAX_SIZE               0x2000
    #define OLL_MAX_SIZE                0x1000
#elif (0x400000 == FLASH_MAX_SIZE)
    #ifdef _FLASH_DIRECT_BOOT_EN_
        #define BOOTLOADER_FLASH_START_ADDR 0x00000000
        #define INFO_FLASH_START_ADDR       0x00010000
        #define APP_FLASH_START_ADDR        0x00040000
        #define OTA_FLASH_START_ADDR        0x00200000
        #define KV_FLASH_START_ADDR         0x001E0000
        #define OLL_FLASH_START_ADDR        0x001E4000
    #else
        #define BOOTLOADER_FLASH_START_ADDR 0x80000000
        #define INFO_FLASH_START_ADDR       0x80010000
        #define APP_FLASH_START_ADDR        0x80012000
        #define OTA_FLASH_START_ADDR        0x80200000
        #define KV_FLASH_START_ADDR         0x801EE000
        #define NVDS_FLASH_START_ADDR       0x801FC000
        #define OLL_FLASH_START_ADDR        0x801FE000
        #define MATTER_FLASH_START_ADDR     0x801C1000
    #endif

    #define BOOTLOADER_MAX_SIZE         0x10000
    #define INFO_MAX_SIZE               0x2000
    #define APP_MAX_SIZE                0x1BD000
    #define OTA_MAX_SIZE                0x1BD000
    #define KV_MAX_SIZE                 0xE000
    #define NVDS_MAX_SIZE               0x2000
    #define OLL_MAX_SIZE                0x1000
    #define MATTER_CONFIG_MAX_SIZE      0x2000
#endif

#ifndef LEGA_RTOS_SUPPORT
#define lega_intrpt_enter()
#define lega_intrpt_exit()
#endif

#ifdef __cplusplus
}
#endif

#endif //__ASR_RV32_H__
