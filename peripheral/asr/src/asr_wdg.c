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

#include "asr_port.h"
#include "asr_wdg.h"
#include "asr_port_peripheral.h"


void hw_watchdog_unlock(void)
{
    WATCHDOG->LOCK = WDG_LOCK_TOKEN;
}

void hw_watchdog_lock(void)
{
    WATCHDOG->LOCK = ~(WDG_LOCK_TOKEN);
}

void hw_watchdog_disable(void)
{
    hw_watchdog_unlock();
    WATCHDOG->CONTROL = 0x0;
    WATCHDOG->LOAD = 0xffffffff;
    hw_watchdog_lock();
}

void hw_watchdog_isr(unsigned int delay)
{
    hw_watchdog_unlock();
    WATCHDOG->CONTROL = WDG_INTEN;
    WATCHDOG->LOAD = delay;
    hw_watchdog_lock();
}

void hw_watchdog_reset(unsigned int delay)
{
    hw_watchdog_unlock();
    WATCHDOG->LOAD = delay;
    WATCHDOG->CONTROL = WDG_RESEN | WDG_INTEN;
    hw_watchdog_lock();
}

void hw_watchdog_isr_clr(void)
{
    hw_watchdog_unlock();
    WATCHDOG->INTCLR = 0x1;
    hw_watchdog_lock();
}

void WDG_IRQHandler(void)
{
    asr_intrpt_enter();
    asr_wdg_dev_t asr_wdg_dev;
    asr_wdg_dev.port = 0;
    asr_wdg_reload(&asr_wdg_dev);
    asr_intrpt_exit();
}


/**
 * This function will initialize the on board CPU hardware watch dog
 *
 * @param[in]  wdg  the watch dog device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_wdg_init(asr_wdg_dev_t *wdg)
{
//    uint32_t reg_value;
    if(NULL == wdg)
    {
        return EIO;
    }
    if(0 == wdg->port)
    {
        //OPEN WDG CLOCK
        port_wdg_init_clk();
        //WDG CLOCK DIV SET
        hw_watchdog_reset(wdg->config.timeout * (WDG_CLOCK /(WDG_APB_DIV+1)/1000 / 2)); //1000 for ms, 2 for watchdog feature
        //ENABLE WDG IRQ
        port_wdg_enable_irq();
        return 0;
    }
    else
    {
        return EIO;
    }
}

/**
 * stop hardware watch dog.
 *
 */
void asr_wdg_stop(void)
{
    hw_watchdog_unlock();
    WATCHDOG->CONTROL = WDG_RESEN;
    hw_watchdog_lock();
}

/**
 * restart hardware watch dog.
 *
 */
void asr_wdg_start(void)
{
    hw_watchdog_unlock();
    WATCHDOG->CONTROL = WDG_RESEN | WDG_INTEN;
    hw_watchdog_lock();
}

/**
 * Reload watchdog counter.
 *
 * @param[in]  wdg  the watch dog device
 */
void asr_wdg_reload(asr_wdg_dev_t *wdg)
{
    if(NULL == wdg)
    {
        return;
    }
    if(0 == wdg->port)
    {
        hw_watchdog_isr_clr();
    }
}

/**
 * This function performs any platform-specific cleanup needed for hardware watch dog.
 *
 * @param[in]  wdg  the watch dog device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_wdg_finalize(asr_wdg_dev_t *wdg)
{
    if(NULL == wdg)
    {
        return EIO;
    }
    if(0 == wdg->port)
    {
        //DIS WDG IRQ
        port_wdg_disable_irq();
        hw_watchdog_disable();
        // Set WDG Clock Disable
        port_wdg_deinit_clk();
        return 0;
    }
    else
    {
        return EIO;
    }
}

