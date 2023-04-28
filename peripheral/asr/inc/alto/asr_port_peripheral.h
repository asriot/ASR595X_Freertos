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
/***************************************************************
 * *
 * *  As each platform(LEGA/SONATA/DUET/CANON) peripherals have different implementation
 * *  All interface defined here
 * *
 * ************************************************************/
#ifndef _ASR_PORT_PERIPHERAL_H_
#define _ASR_PORT_PERIPHERAL_H_

#include "asr_psram.h"
#include "asr_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************  gpio  **********************************************/
void port_gpio_input_enable(Pad_Num_Type pad_num);
void port_gpio_enable_irq(Pad_Num_Type pad_num);
void port_gpio_init_clk(Pad_Num_Type pad_num);
/************************************  uart  **********************************************/
void port_uart_enable_irq(uint8_t uart_idx);
void port_uart_disable_irq(uint8_t uart_idx);
void port_uart_init_clk(uint8_t uart_idx);
void port_uart_deinit_clk(uint8_t uart_idx);

/************************************  spi  **********************************************/
void port_spi_enable_irq(uint8_t spi_idx);
void port_spi_disable_irq(uint8_t spi_idx);
void port_spi_init_clk(uint8_t spi_idx);
void port_spi_deinit_clk(uint8_t spi_idx);

/************************************ DMA  **********************************************/
void port_dma_init(void);
void port_dma_finalize(void);
uint8_t port_dma_get_uart_ch(uint8_t uart_idx,uint8_t tx_mode);
uint8_t port_dma_get_spi_ch(uint8_t spi_idx,uint8_t tx_mode);
/************************************ EFUSE  **********************************************/
void port_efuse_write_enable(void);
/************************************ WDG **********************************************/
void port_wdg_enable_irq(void);
void port_wdg_disable_irq(void);
void port_wdg_init_clk(void);
void port_wdg_deinit_clk(void);

/************************************  timer  **********************************************/
void port_timer_enable_irq(void);
void port_timer_disable_irq(void);
void port_timer_init_clk(uint8_t timer_idx);
void port_timer_deinit_clk(uint8_t timer_idx);

/************************************ I2S **********************************************/
void port_i2s_enable_irq(void);
void port_i2s_disable_irq(void);
int  port_i2s_config_clk(uint32_t clk_src);
void port_i2s_enable_clk();
void port_i2s_disable_clk();

/************************************ PSRAM **********************************************/
void port_psram_init_clk(void);
int port_psram_set_channel(asr_psram_channel channel);

/************************************ RTC **********************************************/
void port_rtc_enable_irq(void);
void port_rtc_disable_irq(void);

/************************************  pwm  **********************************************/
void port_pwm_enable_irq(void);
void port_pwm_disable_irq(void);
void port_pwm_init_clk(void);
void port_pwm_deinit_clk(void);

/************************************ ADC **********************************************/
int32_t port_adc_init(asr_adc_dev_t *adc_config);
int32_t port_adc_get(asr_adc_dev_t *adc_config);
int32_t port_tempr_get(asr_adc_dev_t *adc_config);
int32_t port_adc_finalize(asr_adc_dev_t *adc_config);
void AUX_ADC_IRQHandler_Dummy(void);

/************************************  i2c  **********************************************/
void port_i2c_enable_irq(uint8_t i2c_idx);
void port_i2c_disable_irq(uint8_t i2c_idx);
void port_i2c_init_clk(uint8_t i2c_idx);
void port_i2c_deinit_clk(uint8_t i2c_idx);

#ifdef __cplusplus
}
#endif

#endif
