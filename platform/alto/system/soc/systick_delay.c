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
#include "systick_delay.h"
#ifdef ALIOS_SUPPORT
#include <stdio.h>
#include <k_config.h>
#include <k_err.h>
#include <k_sys.h>
#include <k_time.h>
#define SYSTICK_AMEND 1
#ifdef SYSTICK_AMEND
#include "alto_timer.h"
#endif
#else
#include "FreeRTOS.h"
#include "task.h"
#endif
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
static u8  fac_us=0;
static u16 fac_ms=0;

#ifdef ALIOS_SUPPORT
#ifdef SYSTICK_AMEND
#define SYSTICK_AMEND_MAX_TIME 50000 //ms

alto_timer_dev_t tim={0};
sys_time_t sys_time=0;
volatile uint32_t sys_timer1_cnt = 0;

void timer1_callback(void)
{
    sys_timer1_cnt++;
}
#endif

#define SysTick_Handler eclic_mtip_handler

void SysTick_Handler(void)
{
    krhino_intrpt_enter();
    krhino_tick_proc();
#if  0//ef SYSTICK_AMEND
    sys_time = krhino_sys_tick_get();
    if(sys_time == 1)//first time, init timer
    {
        tim.port=1;
        tim.config.period = SYSTICK_AMEND_MAX_TIME*1000;//50s
        tim.config.reload_mode = TIMER_RELOAD_AUTO;
        tim.config.cb = timer1_callback;

        alto_timer_init(&tim);
        alto_timer_start(&tim);
    }
    else
    {//check hw time
        uint64_t passed_time = SYSTICK_AMEND_MAX_TIME*(sys_timer1_cnt+1) - alto_timer_get(&tim)/1000;
        while(passed_time > sys_time){
            krhino_tick_proc();
            passed_time -= 1;
        }
    }
#endif
    krhino_intrpt_exit();
}

#if 0
uint32_t alto_systick_csr_get()
{
    uint32_t ul_systick_ctrl;
    ul_systick_ctrl = SysTick->CTRL;
    return ul_systick_ctrl;
}
void alto_systick_csr_set(uint32_t ctrl)
{
    SysTick->CTRL = ctrl;
}

void delay_init(u8 SYSCLK)
{
    u32 reload;
    //SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    fac_us=SYSCLK;
    reload=SYSCLK;
    reload= reload*1000000/RHINO_CONFIG_TICKS_PER_SECOND;

    fac_ms=1000/RHINO_CONFIG_TICKS_PER_SECOND;
    SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD=reload;
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
}
#endif

#else

extern void xPortSysTickHandler(void);

void SysTick_Handler(void)
{
    if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}


void delay_init(u8 SYSCLK)
{
    u32 reload;
    //SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    fac_us=SYSCLK;
    reload=SYSCLK;
    reload= reload*1000000/configTICK_RATE_HZ;

    fac_ms=1000/configTICK_RATE_HZ;

    SysTick_Config(reload);
}
#endif

#if 0
void delay_us(u32 nus)
{
    u32 ticks;
    u32 told,tnow,tcnt=0;
    u32 reload=SysTick->LOAD;
    ticks=nus*fac_us;
    told=SysTick->VAL;
    while(1)
    {
        tnow=SysTick->VAL;
        if(tnow!=told)
        {
            if(tnow<told)tcnt+=told-tnow;
            else tcnt+=reload-tnow+told;
            told=tnow;
            if(tcnt>=ticks)break;
        }
    };
}
#endif


#ifdef FPGA_PTA_32M
#define CPU_CLK                         (32000000)
#else
#define CPU_CLK                         (80000000)
#endif
#define DELAY_CYCLE                        (9)

uint32_t us_factor = (CPU_CLK/1000000)/DELAY_CYCLE;
uint32_t ms_factor = (CPU_CLK/1000)/DELAY_CYCLE;
uint32_t s_factor =(CPU_CLK)/DELAY_CYCLE;

uint32_t us_cycle=0, ms_cycle=0, s_cycle=0;


//Os
void delay_us(uint32_t us) // 9cycle
{
    us_cycle = us * us_factor;
    while(us_cycle > 0)
    {
        us_cycle --;
        asm("addi x0,x0,0");
    }
}

void delay_ms(uint32_t ms)
{
    ms_cycle = ms * ms_factor;
    while(ms_cycle > 0)
    {
        ms_cycle--;
        asm("addi x0,x0,0");
    }
}

void delay_s(uint32_t s)
{
    s_cycle = s* s_factor;
    while(s_cycle > 0)
    {
        s_cycle--;
        asm("addi x0,x0,0");
    }
}

