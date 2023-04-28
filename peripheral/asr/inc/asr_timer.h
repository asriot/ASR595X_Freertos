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
#ifndef __ASR_TIMER_H__
#define __ASR_TIMER_H__
#include <errno.h>

#include "asr_port.h"
#include "asr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASR_TIMER_COUNTER_16BIT 0
#define ASR_TIMER_COUNTER_32BIT 1
#define ASR_TIMER_COUNTER_SIZE ASR_TIMER_COUNTER_32BIT

#define ASR_TIMER_CLOCK_DIV_1    0
#define ASR_TIMER_CLOCK_DIV_16   1
#define ASR_TIMER_CLOCK_DIV_256  2
#define ASR_TIMER_CLOCK_PRESCALE ASR_TIMER_CLOCK_DIV_1

#define ASR_TIMER_FREE_RUNNING_MODE 0
#define ASR_TIMER_USER_DEFINED_COUNT_MODE 1
#define ASR_TIMER_MODE ASR_TIMER_USER_DEFINED_COUNT_MODE
typedef struct __TimerControlReg{
    uint32_t timer_oneshot           : 1 ;
    uint32_t timer_size              : 1 ;
    uint32_t timer_clkdiv            : 2 ;
    uint32_t res0                    : 1 ;
    uint32_t timer_interrupt_en      : 1 ;
    uint32_t timer_mode              : 1 ;
    uint32_t timer_enable            : 1 ;
    uint32_t res1                    :24 ;
} TimerControlReg ;

/* TIMER register block */
typedef struct __TIMER{
    __IO uint32_t LOAD;
    __I  uint32_t VALUE;
    __IO TimerControlReg CONTROL;
    __O  uint32_t INTCLR;
    __I  uint32_t RIS;
    __I  uint32_t MIS;
    __IO uint32_t BGLOAD;
} TIMER_TypeDef;

#define TIMER0     ((TIMER_TypeDef *)TIMER0_BASE)
#define TIMER1     ((TIMER_TypeDef *)TIMER1_BASE)
#define TIMER2     ((TIMER_TypeDef *)TIMER2_BASE)
#define TIMER3     ((TIMER_TypeDef *)TIMER3_BASE)


#define TIMER_RELOAD_AUTO  1  /* timer reload automatic */
#define TIMER_RELOAD_MANU  2  /* timer reload manual */

typedef void (*asr_timer_cb_t)(void *arg);

typedef struct {
    uint32_t       period;
    uint8_t        reload_mode;
    asr_timer_cb_t cb;
    void          *arg;
} asr_timer_config_t;

typedef struct {
    int8_t             port;   /* timer port */
    asr_timer_config_t config; /* timer config */
    void              *priv;   /* priv data */
} asr_timer_dev_t;

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
int32_t asr_timer_init(asr_timer_dev_t *tim);

/**
 * start a hardware timer
 *
 * @return  0 == success, EIO == failure
 */
int32_t asr_timer_start(asr_timer_dev_t *tim);

/**
 * get hardware timer remain time
 *
 * @return   success return remain time, EIO == failure
 */
int32_t asr_timer_get(asr_timer_dev_t *tim);

/**
 * reload hardware timer value
 *
 * @return   0 == success, EIO == failure
 */
int32_t asr_timer_reload(asr_timer_dev_t *tim);

/**
 * stop a hardware timer
 *
 * @param[in]  tmr  timer struct
 * @param[in]  cb   callback to be triggered after useconds
 * @param[in]  arg  passed to cb
 */
void asr_timer_stop(asr_timer_dev_t *tim);

/**
 * De-initialises an TIMER interface, Turns off an TIMER hardware interface
 *
 * @param[in]  timer  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_timer_finalize(asr_timer_dev_t *tim);

typedef struct {
    asr_timer_cb_t cb;
    void *arg;
} asr_timer_cb_func_arg_t;

#ifdef __cplusplus
}
#endif

#endif
