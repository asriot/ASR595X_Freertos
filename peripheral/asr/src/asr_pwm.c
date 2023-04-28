#include "asr_port.h"
#include "asr_common.h"
#include "asr_port_peripheral.h"
#include "asr_pwm.h"


asr_cap_callback_func g_asr_cap_callback_handler[CAP_CH_NUM]={0};

void PWM_IRQHandler(void)
{
    asr_intrpt_enter();
    for(int i = 0; i < CAP_CH_NUM; i++)
    {
        //cap irq
        if(PWM->PWM_IN_INT_STA & (0x0001 << i))
        {
            //clear cap interrupt
            PWM->PWM_IN_INT_CLR = (0x0001 << i);
            if(g_asr_cap_callback_handler[i])
            {
                g_asr_cap_callback_handler[i]();
            }
        }
    }
    asr_intrpt_exit();
}

//pwm freq and duty cycle config
void asr_pwm_cfg(asr_pwm_dev_t *pwm)
{
    uint32_t tmp_value,temp_pload;
    uint16_t temp_cmp;

    #if (COUNT_UP_MODE == PWM_COUNT_MODE)
    temp_pload = ((PWM_CLOCK / pwm->config.freq / ((CNT_CLK_DIV_EN_CFG == CNT_CLK_DIV_EN) ? (1 << ((CLK_DIV_CFG >> 24) + 1)) : 1)) - 1);
    #else
    temp_pload = ((PWM_CLOCK / pwm->config.freq / ((CNT_CLK_DIV_EN_CFG == CNT_CLK_DIV_EN) ? (1 << ((CLK_DIV_CFG >> 24) + 1)) : 1) / 2) - 1);
    #endif
    temp_cmp = (uint16_t)(((float)(temp_pload+1) * (1-pwm->config.duty_cycle) +0.5) - 1);

    switch(pwm->port)
    {
        case PWM_OUTPUT_CH0:
            PWM->PWM_GLA_CFG |= PWM_COUNT_MODE; //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD0 & (~(0x0000FFFF));
            tmp_value |= temp_pload;
            PWM->PWMLOAD0 = tmp_value;
            tmp_value = PWM->PWMCMP0 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP0 = tmp_value;
            PWM->PWM_DB_CFG0 = 0;
            break;
        case PWM_OUTPUT_CH1:
            PWM->PWM_GLA_CFG |= PWM_COUNT_MODE; //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD0 & (~(0x0000FFFF));
            tmp_value |= temp_pload;
            PWM->PWMLOAD0 = tmp_value;
            tmp_value = PWM->PWMCMP0 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP0 = tmp_value;
            PWM->PWM_DB_CFG0 = 0;
            break;
        case PWM_OUTPUT_CH2:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 1); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD0 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD0 = tmp_value;

            tmp_value = PWM->PWMCMP1 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP1 = tmp_value;
            PWM->PWM_DB_CFG0 = 0;
            break;
        case PWM_OUTPUT_CH3:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 1); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD0 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD0 = tmp_value;

            tmp_value = PWM->PWMCMP1 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP1 = tmp_value;
            PWM->PWM_DB_CFG0 = 0;
            break;
        case PWM_OUTPUT_CH4:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 2); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD1 & (~(0x0000FFFF));
            tmp_value |= temp_pload;
            PWM->PWMLOAD1 = tmp_value;

            tmp_value = PWM->PWMCMP2 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP2 = tmp_value;
            PWM->PWM_DB_CFG1 = 0;
            break;
        case PWM_OUTPUT_CH5:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 2); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD1 & (~(0x0000FFFF));
            tmp_value |= temp_pload;
            PWM->PWMLOAD1 = tmp_value;

            tmp_value = PWM->PWMCMP2 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP2 = tmp_value;
            PWM->PWM_DB_CFG1 = 0;
            break;
        case PWM_OUTPUT_CH6:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 3); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD1 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD1 = tmp_value;

            tmp_value = PWM->PWMCMP3 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP3 = tmp_value;
            PWM->PWM_DB_CFG1 = 0;
            break;
        case PWM_OUTPUT_CH7:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 3); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD1 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD1 = tmp_value;

            tmp_value = PWM->PWMCMP3 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP3 = tmp_value;
            PWM->PWM_DB_CFG1 = 0;
            break;
        case PWM_OUTPUT_CH8:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 4); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD2 & (~(0x0000FFFF));
            tmp_value |= ((uint16_t)temp_pload);
            PWM->PWMLOAD2 = tmp_value;

            tmp_value = PWM->PWMCMP4 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP4 = tmp_value;
            PWM->PWM_DB_CFG2 = 0;
            break;
        case PWM_OUTPUT_CH9:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 4); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD2 & (~(0x0000FFFF));
            tmp_value |= ((uint16_t)temp_pload);
            PWM->PWMLOAD2 = tmp_value;

            tmp_value = PWM->PWMCMP4 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP4 = tmp_value;
            PWM->PWM_DB_CFG2 = 0;
            break;
        case PWM_OUTPUT_CH10:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 5); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD2 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD2 = tmp_value;

            tmp_value = PWM->PWMCMP5 & (~(0x0000FFFF));
            tmp_value |= temp_cmp;
            PWM->PWMCMP5 = tmp_value;
            PWM->PWM_DB_CFG2 = 0;
            break;
        case PWM_OUTPUT_CH11:
            PWM->PWM_GLA_CFG |= (PWM_COUNT_MODE << 5); //0: count-up mode, 1: count-up/down mode
            tmp_value = PWM->PWMLOAD2 & (~(0xFFFF0000));
            tmp_value |= ((uint16_t)temp_pload << 16);
            PWM->PWMLOAD2 = tmp_value;

            tmp_value = PWM->PWMCMP5 & (~(0xFFFF0000));
            tmp_value |= temp_cmp << 16;
            PWM->PWMCMP5 = tmp_value;
            PWM->PWM_DB_CFG2 = 0;
            break;
        default:
            break;
    }
}


/**
 * Initialises a PWM pin
 *
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_init(asr_pwm_dev_t *pwm)
{
    if(NULL == pwm)
    {
        return -1;
    }
    if(pwm->port >= PWM_CH_NUM)
    {
        return -1;
    }
    //pwm clk enable
    port_pwm_init_clk();

    PWM->PWMCFG0 &= ~(1 << pwm->port); //disable pwm
    //PWM->PWMCFG0 |= (CNT_CLK_DIV_EN | CLK_DIV_CFG);
    asr_pwm_cfg(pwm);
    PWM->PWMCFG1 = 0; //invert control
    return 0;
}

/**
 * Initialises a PWM CAP pin
 *
 *
 * @param[in]  cap  the PWM CAP device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_cap_init(asr_cap_dev_t *cap)
{
    uint32_t reg_value;
    if(NULL == cap)
    {
        return -1;
    }
    if(cap->port >= CAP_CH_NUM)
    {
        return -1;
    }
    //pwm clk enable
    port_pwm_init_clk();

    PWM->PWMCFG0 &= ~(3 << (cap->port*2)); //disable pwm
    PWM->PWM_IN_CFG &= ~(1 << (cap->port)); //disable cap
    PWM->PWM_IN_INT_CFG &= ~(1 << (cap->port));
    PWM->PWMCFG0 &= ~(1 << (cap->port+12));
    switch(cap->port)
    {
        case PWM_CAP_CH0:
            reg_value = PWM->PWM_IN_MAT0& (~(0x0000FFFF));
            reg_value |= ((uint16_t)cap->config.cap_num);
            PWM->PWM_IN_MAT0 = reg_value;
            break;
        case PWM_CAP_CH1:
            reg_value = PWM->PWM_IN_MAT0& (~(0xFFFF0000));
            reg_value |= ((uint16_t)cap->config.cap_num << 16);
            PWM->PWM_IN_MAT0 = reg_value;
            break;
        case PWM_CAP_CH2:
            reg_value = PWM->PWM_IN_MAT1& (~(0x0000FFFF));
            reg_value |= ((uint16_t)cap->config.cap_num);
            PWM->PWM_IN_MAT1 = reg_value;
            break;
        case PWM_CAP_CH3:
            reg_value = PWM->PWM_IN_MAT1& (~(0xFFFF0000));
            reg_value |= ((uint16_t)cap->config.cap_num << 16);
            PWM->PWM_IN_MAT1 = reg_value;
            break;
        case PWM_CAP_CH4:
            reg_value = PWM->PWM_IN_MAT2& (~(0x0000FFFF));
            reg_value |= ((uint16_t)cap->config.cap_num);
            PWM->PWM_IN_MAT2 = reg_value;
            break;
        case PWM_CAP_CH5:
            reg_value = PWM->PWM_IN_MAT2& (~(0xFFFF0000));
            reg_value |= ((uint16_t)cap->config.cap_num << 16);
            PWM->PWM_IN_MAT2 = reg_value;
            break;
    }
    reg_value = PWM->PWM_IN_CFG;
    reg_value &= ~(3 << (cap->port*2+6));
    reg_value &= ~(1 << (cap->port));
    reg_value |= (cap->config.cap_mode << (cap->port*2+6));
    reg_value |= (1 << (cap->port));
    PWM->PWM_IN_CFG = reg_value;
    reg_value = PWM->PWM_IN_INT_CFG;
    reg_value &= ~(1 << (cap->port+6));
    reg_value |= (1 << (cap->port));
    PWM->PWM_IN_INT_CFG = reg_value;

    port_pwm_enable_irq();
    PWM->PWMCFG0 |= (1 << (cap->port+12));
    return 0;
}

/**
 * Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_start(asr_pwm_dev_t *pwm)
{
    if(NULL == pwm)
    {
        return -1;
    }
    if(pwm->port >= PWM_CH_NUM)
    {
        return -1;
    }
    PWM->PWMCFG0 |= (1 << pwm->port);
    return 0;
}

/**
 * Stops output on a PWM pin
 *
 * @param[in]  pwm  the PWM device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_stop(asr_pwm_dev_t *pwm)
{
    if(NULL == pwm)
    {
        return -1;
    }
    if(pwm->port >= PWM_CH_NUM)
    {
        return -1;
    }
    PWM->PWMCFG0 &= ~(1 << pwm->port);
    return 0;
}

/**
 * change the para of pwm
 *
 * @param[in]  pwm  the PWM device
 * @param[in]  para the para of pwm
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_para_chg(asr_pwm_dev_t *pwm, asr_pwm_config_t para)
{
    if(NULL == pwm)
    {
        return -1;
    }
    if(pwm->port >= PWM_CH_NUM)
    {
        return -1;
    }
    //asr_pwm_stop(pwm);
    pwm->config = para;
    asr_pwm_cfg(pwm);
    //asr_pwm_start(pwm);
    return 0;
}

/**
 * De-initialises an PWM interface, Turns off an PWM hardware interface
 *
 * @param[in]  pwm  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_pwm_finalize(asr_pwm_dev_t *pwm)
{
    //pwm clock disable
    //uint32_t reg_value;
    if(NULL == pwm)
    {
        return -1;
    }
    if(pwm->port >= PWM_CH_NUM)
    {
        return -1;
    }
    //one clk enable for 12 pwm channel
    //reg_value = REG_RD(PERI_CLK_CFG);
    //REG_WR(PERI_CLK_CFG, (reg_value|(PWM_CLK_DIS)));
    return asr_pwm_stop(pwm);
}

/**
 * De-initialises an PWM CAP interface, Turns off an PWM CAP hardware interface
 *
 * @param[in]  cap  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_cap_finalize(asr_cap_dev_t *cap)
{
    //uint32_t reg_value;
    if(NULL == cap)
    {
        return -1;
    }
    if(cap->port >= CAP_CH_NUM)
    {
        return -1;
    }
    //one clk enable for 12 pwm channel
    //reg_value = REG_RD(PERI_CLK_CFG);
    //REG_WR(PERI_CLK_CFG, (reg_value|(PWM_CLK_DIS)));
    PWM->PWM_IN_CFG &= ~(1 << (cap->port));
    PWM->PWM_IN_INT_CFG &= ~(1 << (cap->port));
    PWM->PWMCFG0 &= ~(1 << (cap->port+12));
    return 0;
}
