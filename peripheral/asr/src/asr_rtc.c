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
#include <time.h>
#include "asr_rtc.h"
#include "asr_common.h"
#include "asr_port_peripheral.h"

#ifdef RTC_TIME_RETENTION_RAM_ADDR
time_t *p_glo_time = (time_t *)(RTC_TIME_RETENTION_RAM_ADDR);
#else
volatile time_t glo_time = 0;//dec mode
volatile time_t *p_glo_time = &glo_time;
#endif
static uint8_t rtc_dec2bcd(uint8_t decValue)
{
    if(decValue>=100) return 0;

    return  ((decValue/10) * 16) + (decValue%10);
}

static uint8_t rtc_bcd2dec(uint8_t bcdValue)
{
    if(bcdValue>0x99) return 0;

    return ((bcdValue/16)*10) + (bcdValue%16);
}

static void rtc_set_apb_clk_dev()
{
    uint32_t tmp_value = REG_RD(APB_CLK_DIV_REG);
    tmp_value &= (~(0X03<<2));
    REG_WR(APB_CLK_DIV_REG, (tmp_value | (APBCLK_DIV_CFG)));
}
static void rtc_set_init_time()
{
    struct tm tm1 = {0};
    tm1.tm_year = RTC_INIT_YEAR;
    tm1.tm_mon  = RTC_INIT_MONTH;
    tm1.tm_mday = RTC_INIT_DATE;
    tm1.tm_hour = RTC_INIT_HOUR;
    tm1.tm_min  = RTC_INIT_MINUTE;
    tm1.tm_sec  = RTC_INIT_SECOND;
    *p_glo_time = mktime(&tm1);
}

static time_t rtc_get_glo_time()
{
    return (*p_glo_time);
}
static void rtc_set_glo_time(uint8_t format,const asr_rtc_time_t *time)
{
    struct tm tm1 = {0};
    if(!time) return;
    if(format == HAL_RTC_FORMAT_DEC)
    {
        tm1.tm_year     = time->year;
        tm1.tm_mon      = time->month;
        tm1.tm_mday     = time->date;
        tm1.tm_hour     = time->hr;
        tm1.tm_min      = time->min;
        tm1.tm_sec      = time->sec;
        tm1.tm_wday     = time->weekday;
    }
    else
    {
        tm1.tm_year     = rtc_bcd2dec(time->year);
        tm1.tm_mon      = rtc_bcd2dec(time->month);
        tm1.tm_mday     = rtc_bcd2dec(time->date);
        tm1.tm_hour     = rtc_bcd2dec(time->hr);
        tm1.tm_min      = rtc_bcd2dec(time->min);
        tm1.tm_sec      = rtc_bcd2dec(time->sec);
        tm1.tm_wday     = rtc_bcd2dec(time->weekday);
    }
    *p_glo_time = mktime(&tm1);
}

static time_t rtc_get_time_passed()
{
    time_t time_passed = 0;
    uint32_t rtc_date_count = RTC->CURRENT_DATE;
    time_passed = (((rtc_date_count >> 17) & 0xFF) * 86400 + ((rtc_date_count >> 12) & 0x1F) * 3600 + ((rtc_date_count >> 6) & 0x3F) * 60 + (rtc_date_count & 0x3F));
    return time_passed;
}

static void rtc_enable_irq()
{
    port_rtc_enable_irq();
}
static void rtc_disable_irq()
{
    port_rtc_disable_irq();
}

void SLEEP_IRQHandler(void)
{
    asr_intrpt_enter();
    //RTC int sts check and clear
    if(REG_RD(RTC_IRQ_STS_REG) & (1 << RTC_ALARM_WAKEUP_BIT_OFFSET))
    {
        time_t time = 0;
        //struct tm *p_tm1 = NULL;
        time_t timePassed = RTC_MAX_DAY * SECOND_PER_DAY - 1;
        //clear irq
        RTC->CTRL &= ~RTC_INT_ENABLE;
        //update global rtc time
        time = rtc_get_glo_time();
        time += timePassed;
        //set glo time
        glo_time = time;

        delay(50);

        while(REG_RD(RTC_IRQ_STS_REG) & (1 << RTC_ALARM_WAKEUP_BIT_OFFSET))
        {
            REG_WR(RTC_IRQ_STS_REG,(1 << RTC_ALARM_WAKEUP_BIT_OFFSET));
        }
        //enable int
        RTC->CTRL |= RTC_INT_ENABLE;
//        printf("CNT_TICK:%d\t",(int)RTC->CNT_TICK);
        //p_tm1 = gmtime(&time);

        //printf("current date: %d-%d-%d %d:%d:%d\n", (int)(p_tm1->tm_year+1900), (int)(p_tm1->tm_mon), (int)(p_tm1->tm_mday), (int)(p_tm1->tm_hour), (int)(p_tm1->tm_min), (int)(p_tm1->tm_sec));
    }
#if 0
    else if(REG_RD(RTC_IRQ_STS_REG) & (1 << LPO_TIMER_WAKEUP_BIT_OFFSET))
    {
        printf("LPO1 irq handler,VALUE 0X%X\r\n",(unsigned int)LPO1_CURRENT_TICK);
        LPO1_CTRL = 0X00;
        delay(50);
        REG_WR(RTC_IRQ_STS_REG_ADDR,(1 << LPO1_IRQ_BIT));
        while(REG_RD(RTC_IRQ_STS_REG_ADDR) & (1 << LPO1_IRQ_BIT));
        LPO1_CTRL = 0X01;
    }
    else if(REG_RD(RTC_IRQ_STS_REG) & (1 << LPO2_TIMER_WAKEUP_BIT_OFFSET))
    {
        printf("LPO2 irq handler,VALUE 0X%X\r\n",(unsigned int)LPO2_CURRENT_TICK);
        LPO2_CTRL = 0X00;
        delay(50);
        REG_WR(RTC_IRQ_STS_REG_ADDR,(1 << LPO2_IRQ_BIT));
        while(REG_RD(RTC_IRQ_STS_REG_ADDR) & (1 << LPO2_IRQ_BIT));
        LPO2_CTRL = 0X01;
    }
#endif
    asr_intrpt_exit();
}

/**
 * This function will initialize the on board CPU real time clock
 *
 *
 * @param[in]  rtc  rtc device
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_init(asr_rtc_dev_t *rtc)
{
    if(!rtc || rtc->port>=RTC_NUM)
        return EIO;
    if((rtc->config.format!=HAL_RTC_FORMAT_DEC) && (rtc->config.format!=HAL_RTC_FORMAT_BCD))
        rtc->config.format = HAL_RTC_FORMAT_DEC;

    if(RTC->CTRL&RTC_ENABLE) return 0; //for deep sleep wake up reboot case consider

    rtc_set_apb_clk_dev();
    rtc_set_init_time();

    RTC->CTRL &= ~RTC_ENABLE;
    delay(200);
    RTC->CNT_TICK = RTC_TICK_CNT;
    RTC->CNT_DATE = (RTC_REFRESH_DAY << 17 | RTC_REFRESH_HOUR << 12 | RTC_REFRESH_MINUTE << 6 | RTC_REFRESH_SECOND);
    RTC->CTRL |= RTC_CNT_CYCLE_ENABLE | RTC_INT_ENABLE | RTC_ENABLE; //sel internal RC clock, calibration needed

    REG_WR(RTC_IRQ_STS_REG,(1 << RTC_ALARM_WAKEUP_BIT_OFFSET));

    rtc_enable_irq();
    return 0;
}

/**
 * This function will return the value of time read from the on board CPU real time clock.
 *
 * @param[in]   rtc   rtc device
 * @param[out]  time  pointer to a time structure
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_get_time(asr_rtc_dev_t *rtc, asr_rtc_time_t *rtc_time)
{

    struct tm *p_tm1 = NULL;
    time_t time = 0;


    if(!rtc || !time || rtc->port>=RTC_NUM)
        return EIO;

    if((rtc->config.format!=HAL_RTC_FORMAT_DEC) && (rtc->config.format!=HAL_RTC_FORMAT_BCD))
        rtc->config.format = HAL_RTC_FORMAT_DEC;

    time = rtc_get_glo_time();

    time += rtc_get_time_passed();

    p_tm1 = gmtime(&time);

    if(rtc->config.format == HAL_RTC_FORMAT_DEC)
    {
        rtc_time->year = p_tm1->tm_year;
        rtc_time->month = p_tm1->tm_mon;
        rtc_time->date = p_tm1->tm_mday;
        rtc_time->hr = p_tm1->tm_hour;
        rtc_time->min = p_tm1->tm_min;
        rtc_time->sec = p_tm1->tm_sec;
        rtc_time->weekday = p_tm1->tm_wday;
    }
    else if(rtc->config.format == HAL_RTC_FORMAT_BCD)
    {
        rtc_time->year = rtc_dec2bcd(p_tm1->tm_year);
        rtc_time->month = rtc_dec2bcd(p_tm1->tm_mon);
        rtc_time->date = rtc_dec2bcd(p_tm1->tm_mday);
        rtc_time->hr = rtc_dec2bcd(p_tm1->tm_hour);
        rtc_time->min = rtc_dec2bcd(p_tm1->tm_min);
        rtc_time->sec = rtc_dec2bcd(p_tm1->tm_sec);
        rtc_time->weekday = rtc_dec2bcd(p_tm1->tm_wday);
    }
    else
        return EIO;

    return 0;

}

/**
 * This function will set MCU RTC time to a new value.
 *
 * @param[in]   rtc   rtc device
 * @param[out]  time  pointer to a time structure
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_set_time(asr_rtc_dev_t *rtc, const asr_rtc_time_t *time)
{
    if((NULL == rtc) || (NULL == time) || (rtc->port>=RTC_NUM))
    {
        return EIO;
    }
    if((rtc->config.format!=HAL_RTC_FORMAT_DEC) && (rtc->config.format!=HAL_RTC_FORMAT_BCD))
        rtc->config.format = HAL_RTC_FORMAT_DEC;
    RTC->CTRL &= ~RTC_ENABLE;
    delay(200);
    RTC->CTRL |= RTC_ENABLE;

    rtc_set_glo_time(rtc->config.format,time);
    return 0;
}

/**
 * De-initialises an RTC interface, Turns off an RTC hardware interface
 *
 * @param[in]  RTC  the interface which should be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t asr_rtc_finalize(asr_rtc_dev_t *rtc)
{
    if(!rtc || rtc->port>=RTC_NUM)
        return EIO;
    rtc_disable_irq();
    RTC->CTRL &= ~(RTC_ENABLE | RTC_INT_ENABLE | RTC_CNT_CYCLE_ENABLE);
    RTC->CNT_TICK = 0;
    RTC->CNT_DATE = 0;
    return 0;
}
