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
#ifndef _ASR_PWM_H_
#define _ASR_PWM_H_

#include <stdint.h>
#include "asr_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_OUTPUT_CH0  0
#define PWM_OUTPUT_CH1  1
#define PWM_OUTPUT_CH2  2
#define PWM_OUTPUT_CH3  3
#define PWM_OUTPUT_CH4  4
#define PWM_OUTPUT_CH5  5
#define PWM_OUTPUT_CH6  6
#define PWM_OUTPUT_CH7  7
#define PWM_OUTPUT_CH8  8
#define PWM_OUTPUT_CH9  9
#define PWM_OUTPUT_CH10 10
#define PWM_OUTPUT_CH11 11
//#define PWM_CH_NUM 12

#define PWM_CAP_CH0  0
#define PWM_CAP_CH1  1
#define PWM_CAP_CH2  2
#define PWM_CAP_CH3  3
#define PWM_CAP_CH4  4
#define PWM_CAP_CH5  5
//#define CAP_CH_NUM 6

#define COUNT_UP_MODE 0
#define COUNT_UP_DOWN_MODE 1
#define PWM_COUNT_MODE COUNT_UP_MODE


#define CNT_CLK_DIV_EN  (0x00000001 << 27)
#define CNT_CLK_DIV_DIS (~(0x00000001 << 27))
#define CNT_CLK_DIV_EN_CFG CNT_CLK_DIV_DIS

#define CLK_DIV_2 (0)
#define CLK_DIV_4 (0x00000001 << 24)
#define CLK_DIV_8 (0x00000002 << 24)
#define CLK_DIV_16 (0x00000003 << 24)
#define CLK_DIV_32 (0x00000004 << 24)
#define CLK_DIV_64 (0x00000005 << 24)
#define CLK_DIV_128 (0x00000006 << 24)
#define CLK_DIV_CFG CLK_DIV_2

#define PWM0_INVERT_EN (0x00000001 << 0)
#define PWM1_INVERT_EN (0x00000001 << 1)
#define PWM2_INVERT_EN (0x00000001 << 2)
#define PWM3_INVERT_EN (0x00000001 << 3)
#define PWM4_INVERT_EN (0x00000001 << 4)
#define PWM5_INVERT_EN (0x00000001 << 5)
#define PWM6_INVERT_EN (0x00000001 << 6)
#define PWM7_INVERT_EN (0x00000001 << 7)
#define PWM8_INVERT_EN (0x00000001 << 8)
#define PWM9_INVERT_EN (0x00000001 << 9)
#define PWM10_INVERT_EN (0x00000001 << 10)
#define PWM11_INVERT_EN (0x00000001 << 11)
#define PWMX_INVERT_EN 0



/* PWM register block */
typedef struct __PWM
{
    __IO uint32_t PWMCFG0;
    __IO uint32_t PWMCFG1;
    __IO uint32_t PWM_TRIG_CFG0;
    __IO uint32_t PWM_TRIG_CFG1;
    __IO uint32_t PWM_TRIG_CFG2;
    __IO uint32_t PWMINCFG0;
    __IO uint32_t PWMINCFG1;
    __I  uint32_t PWMRAW0;
    __I  uint32_t PWMRAW1;
    __IO uint32_t PWM_INT_CLR0;

    __IO uint32_t PWM_INT_CLR1;
    __I  uint32_t PWM_INT_STA0;
    __I  uint32_t PWM_INT_STA1;
    __IO uint32_t PWM_GLA_CFG;
    __IO uint32_t PWMLOAD0;
    __IO uint32_t PWMLOAD1;
    __IO uint32_t PWMLOAD2;
    __I  uint32_t PWMCOUNT0;
    __I  uint32_t PWMCOUNT1;
    __I  uint32_t PWMCOUNT2;

    __IO uint32_t PWMCMP0;
    __IO uint32_t PWMCMP1;
    __IO uint32_t PWMCMP2;
    __IO uint32_t PWMCMP3;
    __IO uint32_t PWMCMP4;
    __IO uint32_t PWMCMP5;
    __IO uint32_t PWM_DB_CFG0;
    __IO uint32_t PWM_DB_CFG1;
    __IO uint32_t PWM_DB_CFG2;
    __IO uint32_t PWM_IN_CFG;

    __IO uint32_t PWM_IN_INT_CFG;
    __I  uint32_t PWM_IN_RAW;
    __IO uint32_t PWM_IN_INT_CLR;
    __I  uint32_t PWM_IN_INT_STA;
    __I  uint32_t PWM_IN_CNT0;
    __I  uint32_t PWM_IN_CNT1;
    __I  uint32_t PWM_IN_CNT2;
    __IO uint32_t PWM_IN_MAT0;
    __IO uint32_t PWM_IN_MAT1;
    __IO uint32_t PWM_IN_MAT2;

    __IO uint32_t PWM_TIM_INT_CFG;
    __I  uint32_t PWM_TIM_RAW;
    __IO uint32_t PWM_TIM_INT_CLR;
    __I  uint32_t PWM_TIM_INT_STA;
    __IO uint32_t PWM_TIM_LOAD0;
    __IO uint32_t PWM_TIM_LOAD1;
    __IO uint32_t PWM_TIM_LOAD2;
    __IO uint32_t PWM_TIM_CNT0;
    __IO uint32_t PWM_TIM_CNT1;
    __IO uint32_t PWM_TIM_CNT2;
}PWM_TypeDef;

#define PWM ((PWM_TypeDef *)(PWM_REG_BASE))

/* min freq:PWM_CLOCK/(65535*CLK_DIV_CFG),  max freq:PWM_CLOCK/3 */
typedef struct
{
    float    duty_cycle;  /* the pwm duty_cycle */
    uint32_t freq;        /* the pwm freq */
} asr_pwm_config_t;

typedef struct
{
    uint8_t      port;    /* pwm port */
    asr_pwm_config_t config;  /* pwm config */
    void        *priv;    /* priv data */
} asr_pwm_dev_t;

typedef enum {
    POSEDGE_EDGE_MODE = 0,
    NEGEDGE_EDGE_MODE = 1,
    BOTH_EDGE_MODE    = 3,
} cap_mode_t;
/* max cap num:65535 */
typedef struct
{
    uint32_t   cap_num;   /* the cap num*/
    cap_mode_t cap_mode;  /* the cap mode */
} asr_cap_config_t;
/* CAP PIN map
    PWM_CAP_CH0 PWM0
    PWM_CAP_CH1 PWM2
    PWM_CAP_CH2 PWM4
    PWM_CAP_CH3 PWM6
    PWM_CAP_CH4 PWM8
    PWM_CAP_CH5 PWM10
*/
typedef struct
{
    uint8_t      port;        /* cap port */
    asr_cap_config_t config;  /* cap config */
    void        *priv;       /* priv data */
} asr_cap_dev_t;

typedef void (*asr_cap_callback_func)(void);
extern asr_cap_callback_func g_asr_cap_callback_handler[CAP_CH_NUM];

/**
 * PWM clock select clock source
 *
 * @param[in]  clock freq type
 *
 * @note:
 */
void asr_pwm_clk_use_135M(void);

/**
 * Initialises a PWM pin
 *
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_init(asr_pwm_dev_t *pwm);

/**
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_start(asr_pwm_dev_t *pwm);

/**
 * Stops output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_stop(asr_pwm_dev_t *pwm);

/**
 * change the para of pwm
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  para the para of pwm
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_para_chg(asr_pwm_dev_t *pwm, asr_pwm_config_t para);

/**
 * De-initialises an PWM interface, Turns off an PWM hardware interface
 *
 * @param[in]  pwm  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_finalize(asr_pwm_dev_t *pwm);

/**
 * Initialises a PWM CAP pin
 *
 *
 * @param[in]  cap  the PWM CAP device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_cap_init(asr_cap_dev_t *cap);

/**
 * De-initialises an PWM CAP interface, Turns off an PWM CAP hardware interface
 *
 * @param[in]  cap  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_cap_finalize(asr_cap_dev_t *cap);

#ifdef __cplusplus
}
#endif

#endif //_ASR_PWM_H_