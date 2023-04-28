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
#include "asr_timer.h"

#include "asr_port.h"
#include "asr_port_peripheral.h"


asr_timer_cb_func_arg_t g_asr_timer_handler[TIMER_NUM];
static TIMER_TypeDef* timer_get_timerx_via_idx(uint8_t timer_idx)
{
    if(timer_idx>=TIMER_NUM)
        return NULL;
    switch(timer_idx)
    {
        case 0: return TIMER0;
        case 1: return TIMER1;
        case 2: return TIMER2;
        case 3: return TIMER3;
        default:return NULL;
    }
}

void TIMER_IRQHandler(void)
{
    uint8_t i=0;
    TIMER_TypeDef * TIMERx=NULL;

    asr_intrpt_enter();

    for(i=0;i<TIMER_NUM;i++)
    {
        TIMERx = timer_get_timerx_via_idx(i);
        if(TIMERx && TIMERx->MIS)
        {
            TIMERx->INTCLR = 1; //clear irq
            if(g_asr_timer_handler[i].cb)
            {
                g_asr_timer_handler[i].cb(g_asr_timer_handler[i].arg);
            }
        }
    }

    asr_intrpt_exit();
}

static uint8_t timer_atlease_1_enable()
{
    TIMER_TypeDef* TIMERx=NULL;
    uint8_t i = 0;
    for(i=0;i<TIMER_NUM;i++)
    {
        TIMERx = timer_get_timerx_via_idx(i);
        if(TIMERx->CONTROL.timer_enable) //at lease one timer enable, should not disable
            return 1;
    }
    return 0;
}

/**
 * init a hardware timer
 *
 * @param[in]  tmr         timer struct
 * @param[in]  period      micro seconds for repeat timer trigger
 * @param[in]  auto_reoad  set to 0, if you just need oneshot timer
 * @param[in]  cb          callback to be triggered after useconds
 * @param[in]  ch          timer channel
 * @param[in]  arg         passed to cb
 */
int32_t asr_timer_init(asr_timer_dev_t *tim)
{
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return EIO;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return EIO;

    port_timer_init_clk(tim->port);

    *(uint32_t*)(&TIMERx->CONTROL) = 0; //disable timer first
    TIMERx->LOAD = tim->config.period * (TIMER_CLK / 1000000); //1000000 for us

    if(tim->config.reload_mode == TIMER_RELOAD_MANU)
        TIMERx->CONTROL.timer_oneshot = 1;
    else if(tim->config.reload_mode == TIMER_RELOAD_AUTO)
        TIMERx->CONTROL.timer_oneshot = 0;
    else
        return EIO;

    TIMERx->CONTROL.timer_size = ASR_TIMER_COUNTER_SIZE;
    TIMERx->CONTROL.timer_clkdiv=ASR_TIMER_CLOCK_PRESCALE;

    TIMERx->CONTROL.timer_mode=ASR_TIMER_MODE;

    g_asr_timer_handler[tim->port].cb = tim->config.cb;
    g_asr_timer_handler[tim->port].arg = tim->config.arg;

    return 0;
}

/**
 * start a hardware timer
 *
 * @return  0 == success, EIO == failure
 */
int32_t asr_timer_start(asr_timer_dev_t *tim)
{
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return EIO;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return EIO;

    if(!tim->config.cb)//no callback, timer not work
        return EIO;

    TIMERx->CONTROL.timer_enable=1;
    TIMERx->CONTROL.timer_interrupt_en=1;

    port_timer_enable_irq();

    return 0;
}

/**
 * get hardware timer remain time
 *
 * @return   success return remain time, EIO == failure
 */
int32_t asr_timer_get(asr_timer_dev_t *tim)
{
    uint32_t reg_value = 0;
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return EIO;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return EIO;

    reg_value = TIMERx->VALUE; //timer current value
    return (reg_value/(TIMER_CLK / 1000000)); //time for us
}

/**
 * reload hardware timer value
 *
 * @return   0 == success, EIO == failure
 */
int32_t asr_timer_reload(asr_timer_dev_t *tim)
{
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return EIO;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return EIO;

    TIMERx->LOAD = tim->config.period * (TIMER_CLK / 1000000); //1000000 for us
    return 0;
}


/**
 * stop a hardware timer
 *
 * @param[in]  tmr  timer struct
 * @param[in]  cb   callback to be triggered after useconds
 * @param[in]  arg  passed to cb
 */
void asr_timer_stop(asr_timer_dev_t *tim)
{
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return;

    TIMERx->CONTROL.timer_enable = 0;
    TIMERx->CONTROL.timer_interrupt_en = 0;
    if(timer_atlease_1_enable())
        return;
    port_timer_disable_irq();
    return;
}

/**
 * De-initialises an TIMER interface, Turns off an TIMER hardware interface
 *
 * @param[in]  timer  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_timer_finalize(asr_timer_dev_t *tim)
{
    TIMER_TypeDef* TIMERx=NULL;

    if(!tim) return EIO;

    TIMERx = timer_get_timerx_via_idx(tim->port);
    if(!TIMERx) return EIO;

    if(TIMERx->CONTROL.timer_enable)
        asr_timer_stop(tim);

    *(uint32_t*)(&TIMERx->CONTROL) = 0;
    TIMERx->LOAD = 0;

    g_asr_timer_handler[tim->port].cb = NULL;
    g_asr_timer_handler[tim->port].arg = NULL;
    if(timer_atlease_1_enable())
        return 0;
    port_timer_deinit_clk(tim->port);
    return 0;
}
