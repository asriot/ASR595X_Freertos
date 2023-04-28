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
#include <stdio.h>
#include "asr_rv32.h"
#include "soc_init.h"
#include "asr_wdg.h"
#include "asr_alto_boot.h"
#include "asr_flash.h"
#include "asr_common.h"
#include "systick_delay.h"
#include "lega_wlan_api.h"
#include "asr_flash_alg.h"
#ifndef ALIOS_SUPPORT
#include "FreeRTOSConfig.h"
#endif
#include "pmu.h"

extern pmu_state_t current_state;


#define REG_AHB_BUS_CTRL   *((volatile uint32_t *)(ALWAYS_ON_REGFILE + 0x90))
extern void BLE_IRQHandler(void);

/***********************************************************
* set IRQ priority not enable
*
**********************************************************/

void ECLIC_Mode_Init()
{
    ECLIC_Register_IRQ_Mode(UART1_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, UART1_IRQHandler);
    ECLIC_Register_IRQ_Mode(UART0_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, UART0_IRQHandler);
    ECLIC_Register_IRQ_Mode(UART2_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, UART2_IRQHandler);

    ECLIC_Register_IRQ_Mode(ASRW_WI_IP_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_LEVEL_TRIGGER, INTERRUPT_LEVEL_HIGH, 0, intc_irq);

    ECLIC_Register_IRQ_Mode(D_APLL_UNLOCK_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, D_APLL_UNLOCK_IRQHandler);
    ECLIC_Register_IRQ_Mode(SLEEP_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, SLEEP_IRQHandler);
    ECLIC_Register_IRQ_Mode(D_SX_UNLOCK_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, D_SX_UNLOCK_IRQHandler);

    ECLIC_Register_IRQ_Mode(WDG_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, WDG_IRQHandler);
    //ECLIC_Register_IRQ_Mode(FLASH_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, FLASH_IRQHandler);
    ECLIC_Register_IRQ_Mode(GPIO_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, GPIO_IRQHandler);
    ECLIC_Register_IRQ_Mode(TIMER_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, TIMER_IRQHandler);
    //ECLIC_Register_IRQ_Mode(CRYPTOCELL310_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, CRYPTOCELL310_IRQHandler);
    //ECLIC_Register_IRQ_Mode(DMA_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, DMA_IRQHandler);
    //ECLIC_Register_IRQ_Mode(SPI0_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, SPI0_IRQHandler);

    //ECLIC_Register_IRQ_Mode(SPI1_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, SPI1_IRQHandler);
    //ECLIC_Register_IRQ_Mode(SPI2_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, SPI2_IRQHandler);
    //ECLIC_Register_IRQ_Mode(I2C0_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, I2C0_IRQHandler);
    //ECLIC_Register_IRQ_Mode(I2C1_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, I2C1_IRQHandler);
    //ECLIC_Register_IRQ_Mode(SDIO_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, SDIO_IRQHandler);
    //ECLIC_Register_IRQ_Mode(PLF_WAKEUP_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
    //    ECLIC_POSTIVE_EDGE_TRIGGER, INTERRUPT_LEVEL_NORMAL, 0, PLATFORM_WAKEUP_IRQHandler);
#ifdef CFG_DUET_BLE
    ECLIC_Register_IRQ_Mode(RW_BLE_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
        ECLIC_LEVEL_TRIGGER, INTERRUPT_LEVEL_HIGHEST, 0, BLE_IRQHandler);
#endif
    //ECLIC_ClearPendingIRQ(ASRW_WI_IP_IRQn);
    //ECLIC_EnableIRQ(ASRW_WI_IP_IRQn);
}

/***********************************************************
* watchdog init config
*
**********************************************************/
asr_wdg_dev_t asr_wdg;
void wdg_init(void)
{
    asr_wdg.port = 0;
    asr_wdg.config.timeout = WDG_TIMEOUT_MS;
    asr_wdg_init(&asr_wdg);
}

void pmp_init(void)
{
    int region = 0;

    printf("pmp_init\r\n");

    if(__config_PMPx(region, PMP_L | PMP_R | PMP_X, 0x00000000, 17))            //rom               128K    rom code
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_W, 0x00080000, 12))          //itcm              4K      data for ble rom
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_W | PMP_X, 0x00000000, 26))  //riscv-reg         64M     systimer+eclic+jtag
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_W | PMP_X, 0x20000000, 27))  //dtcm+sram         128M    stack+data+heap
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_W | PMP_X, 0x30000000, 23))  //psram             8M      psram
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_W, 0x40000000, 30))          //reg+ram           1G      reg+wifi+ble
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L | PMP_R | PMP_X, 0x80000000, 22))          //flash             4M      code
        printf("pmp_init region(%d) configure failed!\r\n", region);

    if(__config_PMPx(++region, PMP_L, 0x00000000, 32))                          //others            4G      N/A
        printf("pmp_init region(%d) configure failed!\r\n", region);
}

extern void lega_drv_rco_cal(void);
/***********************************************************
*  init device: flash config,irq priority,rco and systick
*
**********************************************************/
void alto_devInit()
{
    //default close watchdog for step debug on keil
    //wdg_init();

    ECLIC_Mode_Init();

    pmp_init();

    //move to lega_phy_hw_init since wifi6
#ifdef DCDC_PFMMODE_CLOSE
    //alto_drv_close_dcdc_pfm();
#endif

    lega_drv_rco_cal();
}

/**********************************************************
*  use soc_init to init board
*
**********************************************************/
int soc_init(void)
{
    alto_devInit();
    return 0;
}

void HCLK_SW_IRQHandler(void)
{
    SYS_CRM_CLR_HCLK_REC = 0x1;
}



void ahb_sync_brid_open(void)
{
    unsigned int is_using_sync_down = (REG_AHB_BUS_CTRL & (0x1<<1));
    if(!is_using_sync_down || asr_get_boot_type() == DEEP_SLEEP_RST)
    {
        REG_AHB_BUS_CTRL |= (0x1<<1); //0x40000A90 bit1 sw_use_hsync

        __enable_irq();
        //NVIC_EnableIRQ(24);
        //__asm volatile("DSB");
       // __asm volatile("WFI");
        //__asm volatile("ISB");

        //delay_nop(50);
    }
}

void enable_sleep_irq_after_deepsleep(void)
{
    if (asr_get_boot_type() == DEEP_SLEEP_RST)
    {
        // In this case means wakeup from deep sleep and need process RTC interrupt
        current_state = PMU_STATE_DEEPSLEEP;
        ECLIC_ClearPendingIRQ(SLEEP_IRQn);
        ECLIC_EnableIRQ(SLEEP_IRQn);

        lega_drv_goto_active();
    }
}

extern void lega_drv_open_dcdc_pfm(void);
extern void lega_reset_rw_rf(void);
extern void lega_soc_wifi_ble_clk_disable(void);
extern void lega_enable_all_soc_interrupt(void);
extern uint8_t lega_get_chip_type(void);

/*************************************************************
*  soc init config, don't run any code before it
*
**************************************************************/
int soc_pre_init(void)
{
    DisableICache();
    DisableDCache();

    lega_get_chip_type();
#ifdef _SPI_FLASH_ENABLE_
    asr_flash_init();
#endif
    //without bootload write LDO to 3.3V, otherwise TXPWR/EVM is bad
    //REG_WR(0x40000a58,0x5FFFF);
    EnableICache();
    EnableDCache();

    // enable sleep irq here to clear interrupt status after deepsleep reset
    enable_sleep_irq_after_deepsleep();

    lega_drv_open_dcdc_pfm();
    lega_reset_rw_rf();
    //turn off all wifi/ble clock
    lega_soc_wifi_ble_clk_disable();

    //enable all soc interrupt
    lega_enable_all_soc_interrupt();

    return 0;
}
