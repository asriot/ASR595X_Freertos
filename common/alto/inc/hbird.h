/******************************************************************************
 * @file     hbird.h
 * @brief    NMSIS Core Peripheral Access Layer Header File for
 *           Nuclei HummingBird evaluation SoC which support Nuclei N/NX class cores
 * @version  V1.00
 * @date     22. Nov 2019
 ******************************************************************************/
/*
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HBIRD_H__
#define __HBIRD_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Nuclei
  * @{
  */


/** @addtogroup hbird
  * @{
  */


/** @addtogroup Configuration_of_NMSIS
  * @{
  */



/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition                                ================ */
/* =========================================================================================================================== */

typedef enum IRQn
{
/* =======================================  Nuclei Core Specific Interrupt Numbers  ======================================== */
    Reserved0_IRQn            =   0,              /*!<  Internal reserved */
    Reserved1_IRQn            =   1,              /*!<  Internal reserved */
    Reserved2_IRQn            =   2,              /*!<  Internal reserved */
    SysTimerSW_IRQn           =   3,              /*!<  System Timer SW interrupt */
    Reserved3_IRQn            =   4,              /*!<  Internal reserved */
    Reserved4_IRQn            =   5,              /*!<  Internal reserved */
    Reserved5_IRQn            =   6,              /*!<  Internal reserved */
    SysTimer_IRQn             =   7,              /*!<  System Timer Interrupt */
    Reserved6_IRQn            =   8,              /*!<  Internal reserved */
    Reserved7_IRQn            =   9,              /*!<  Internal reserved */
    Reserved8_IRQn            =  10,              /*!<  Internal reserved */
    Reserved9_IRQn            =  11,              /*!<  Internal reserved */
    Reserved10_IRQn           =  12,              /*!<  Internal reserved */
    Reserved11_IRQn           =  13,              /*!<  Internal reserved */
    Reserved12_IRQn           =  14,              /*!<  Internal reserved */
    Reserved13_IRQn           =  15,              /*!<  Internal reserved */
    Reserved14_IRQn           =  16,              /*!<  Internal reserved */
    Reserved15_IRQn           =  17,              /*!<  Internal reserved */
    Reserved16_IRQn           =  18,              /*!<  Internal reserved */
    irq19_IRQn                  =  19,              /*!<  19 Interrupt */
    irq20_IRQn                  =  20,              /*!<  20 Interrupt */
    irq21_IRQn                  =  21,              /*!<  21 Interrupt */
    irq22_IRQn                  =  22,              /*!<  22 Interrupt */
    irq23_IRQn                  =  23,              /*!<  23 Interrupt */
    irq24_IRQn                  =  24,              /*!<  24 Interrupt */
    RW_BLE_IRQn               =  25,              /*!<  BLE Interrupt */
    I2S_IRQn                  =  26,              /*!<  I2S Interrupt */
    PLF_WAKEUP_IRQn           =  27,              /*!<  WiFi Platform Wake-Up Interrupt */
    irq28_IRQn                  =  28,              /*!<  28 Interrupt */
    PWM_IRQn                  =  29,              /*!<  29 Interrupt */
    AUX_ADC_IRQn              =  30,              /*!<  ADC Interrupt */
    irq31_IRQn                  =  31,              /*!<  31 Interrupt */
    D_SX_UNLOCK_IRQn          =  32,              /*!<  RF added: D_SX_UNLOCK Interrupt */
    D_APLL_UNLOCK_IRQn        =  33,              /*!<  RF added: D_APLL_UNLOCK Interrupt */
    SDIO_IRQn                 =  34,              /*!<  SDIO Combined Interrupt */
    I2C1_IRQn                 =  35,              /*!<  I2C1 Interrupt */
    I2C0_IRQn                 =  36,              /*!<  I2C0 Interrupt */
    SPI2_IRQn                 =  37,              /*!<  SPI2 Interrupt */
    SPI1_IRQn                 =  38,              /*!<  SPI1 Interrupt */
    SPI0_IRQn                 =  39,              /*!<  SPI0 Interrupt */
    UART2_IRQn                =  40,              /*!<  UART2 Interrupt */
    UART1_IRQn                =  41,              /*!<  UART1 Interrupt */
    UART0_IRQn                =  42,              /*!<  UART0 Interrupt */
    DMA_IRQn                  =  43,              /*!<  Generic DMA Ctrl Interrupt */
    CRYPTOCELL310_IRQn        =  44,              /*!<  CryptoCell 310 Interrupt */
    TIMER_IRQn                =  45,              /*!<  Timer Interrupt */
    GPIO_IRQn                 =  46,              /*!<  GPIO Interrupt */
    FLASH_IRQn                =  47,              /*!<  FLASH Interrupt */
    WDG_IRQn                  =  48,              /*!<  Window WatchDog */
    SLEEP_IRQn                =  49,              /*!<  Sleep Wake-Up Interrupt */
    ASRW_WI_IP_IRQn           =  50,              /*!<  ASR WIFI IP Interrupt */
    SOC_INT_MAX,
} IRQn_Type;

/* =========================================================================================================================== */
/* ================                                  Exception Code Definition                                ================ */
/* =========================================================================================================================== */

typedef enum EXCn {
/* =======================================  Nuclei N/NX Specific Exception Code  ======================================== */
    InsUnalign_EXCn          =   0,              /*!<  Instruction address misaligned */
    InsAccFault_EXCn         =   1,              /*!<  Instruction access fault */
    IlleIns_EXCn             =   2,              /*!<  Illegal instruction */
    Break_EXCn               =   3,              /*!<  Beakpoint */
    LdAddrUnalign_EXCn       =   4,              /*!<  Load address misaligned */
    LdFault_EXCn             =   5,              /*!<  Load access fault */
    StAddrUnalign_EXCn       =   6,              /*!<  Store or AMO address misaligned */
    StAccessFault_EXCn       =   7,              /*!<  Store or AMO access fault */
    UmodeEcall_EXCn          =   8,              /*!<  Environment call from User mode */
    MmodeEcall_EXCn          =  11,              /*!<  Environment call from Machine mode */
    NMI_EXCn                 = 0xfff,            /*!<  NMI interrupt */
} EXCn_Type;

/* =========================================================================================================================== */
/* ================                           Processor and Core Peripheral Section                           ================ */
/* =========================================================================================================================== */

/* ToDo: set the defines according your Device */
/* ToDo: define the correct core revision */
#if __riscv_xlen == 32

#ifndef __NUCLEI_CORE_REV
#define __NUCLEI_N_REV            0x0104    /*!< Core Revision r1p4 */
#else
#define __NUCLEI_N_REV            __NUCLEI_CORE_REV
#endif

#elif __riscv_xlen == 64

#ifndef __NUCLEI_CORE_REV
#define __NUCLEI_NX_REV           0x0100    /*!< Core Revision r1p0 */
#else
#define __NUCLEI_NX_REV           __NUCLEI_CORE_REV
#endif

#endif /* __riscv_xlen == 64 */

/* ToDo: define the correct core features for the hbird */
#define __ECLIC_PRESENT           1                     /*!< Set to 1 if ECLIC is present */
#define __ECLIC_BASEADDR          0x00130000UL          /*!< Set to ECLIC baseaddr of your device */

//#define __ECLIC_INTCTLBITS        3                     /*!< Set to 1 - 8, the number of hardware bits are actually implemented in the clicintctl registers. */
#define __ECLIC_INTNUM            51                    /*!< Set to 1 - 1024, total interrupt number of ECLIC Unit */
#define __SYSTIMER_PRESENT        1                     /*!< Set to 1 if System Timer is present */
#define __SYSTIMER_BASEADDR       0x00120000UL          /*!< Set to SysTimer baseaddr of your device */

/*!< Set to 0, 1, or 2, 0 not present, 1 single floating point unit present, 2 double floating point unit present */
#if !defined(__riscv_flen)
#define __FPU_PRESENT             0
#elif __riscv_flen == 32
#define __FPU_PRESENT             1
#else
#define __FPU_PRESENT             2
#endif

#define __DSP_PRESENT             1                     /*!< Set to 1 if DSP is present */
#define __PMP_PRESENT             1                     /*!< Set to 1 if PMP is present */
#define __PMP_ENTRY_NUM           16                    /*!< Set to 8 or 16, the number of PMP entries */
#define __ICACHE_PRESENT          1                     /*!< Set to 1 if I-Cache is present */
#define __DCACHE_PRESENT          1                     /*!< Set to 1 if D-Cache is present */
#define __Vendor_SysTickConfig    0                     /*!< Set to 1 if different SysTick Config is used */
#define __Vendor_EXCEPTION        0                     /*!< Set to 1 if vendor exception hander is present */

/** @} */ /* End of group Configuration_of_CMSIS */


#include <nmsis_core.h>                         /*!< Nuclei N/NX class processor and core peripherals */
/* ToDo: include your system_hbird.h file
         replace 'Device' with your device name */
#include "system_hbird.h"                    /*!< hbird System */


/* ========================================  Start of section using anonymous unions  ======================================== */
#if   defined (__GNUC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

#define RTC_FREQ                    32768

#define CPU_AON_FREQ                80000000
// The TIMER frequency use CPU CLK not the default RTC frequency
#define SOC_TIMER_FREQ              CPU_AON_FREQ
/* =========================================================================================================================== */
/* ================                            Device Specific Peripheral Section                             ================ */
/* =========================================================================================================================== */

// Interrupt Numbers
#define SOC_ECLIC_NUM_INTERRUPTS    32
#define SOC_ECLIC_INT_GPIO_BASE     19

// Interrupt Handler Definitions
#define SOC_MTIMER_HANDLER          eclic_mtip_handler
#define SOC_SOFTINT_HANDLER         eclic_msip_handler


/* =========================================  End of section using anonymous unions  ========================================= */
#if defined (__GNUC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */


/* ToDo: add here your device peripherals pointer definitions
         following is an example for timer */

// Helper functions
#define _REG8(p, i)             (*(volatile uint8_t *) ((p) + (i)))
#define _REG32(p, i)            (*(volatile uint32_t *) ((p) + (i)))
#define _REG32P(p, i)           ((volatile uint32_t *) ((p) + (i)))

#define GPIO_REG(offset)        _REG32(GPIO_BASE, offset)
#define PWM0_REG(offset)        _REG32(PWM0_BASE, offset)
#define PWM1_REG(offset)        _REG32(PWM1_BASE, offset)
#define PWM2_REG(offset)        _REG32(PWM2_BASE, offset)
#define SPI0_REG(offset)        _REG32(QSPI0_BASE, offset)
#define SPI1_REG(offset)        _REG32(QSPI1_BASE, offset)
#define SPI2_REG(offset)        _REG32(QSPI2_BASE, offset)
#define UART0_REG(offset)       _REG32(UART0_BASE, offset)
#define UART1_REG(offset)       _REG32(UART1_BASE, offset)
#define I2C_REG(offset)         _REG8(I2C_BASE, offset)

// Misc

uint32_t get_cpu_freq();
void delay_1ms(uint32_t count);

/** @} */ /* End of group hbird */

/** @} */ /* End of group Nuclei */

#ifdef __cplusplus
}
#endif

#endif  /* __HBIRD_H__ */
