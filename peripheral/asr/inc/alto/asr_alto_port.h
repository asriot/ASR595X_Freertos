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
#ifndef __ASR_ALTO_PORT_H_
#define __ASR_ALTO_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
     ALTO FLASH BEGIN
*/

#define FLASH_BASE          0x40003000
#define FLASH_ADDR_START    0x80000000
#define FLASH_ADDR_END      0x87FFFFFF

/*
    ALTO FLASH END
*/


/*
    ALTO EFUSE BEGIN
*/

#define EFUSE_CTRL_BASE     0x40005000
#define EFUSE_READ_OP_WAIT_CYCLE        5
#define EFUSE_WRITE_OP_WAIT_CYCLE       (SYSTEM_CLOCK/100000) //10us
#define EFUSE_DIV_OP_WAIT_CYCLE         31

/*
    ALTO EFUSE END
*/

/*
    ALTO DMA BEGIN
*/

#define DMA_REG_BASE   (0x40004000)
#define DMA_BUFFER_REG (0X4000C000)
typedef enum
{
    DMA_CH_UART0_TX = 0,
    DMA_CH_UART0_RX,
    DMA_CH_UART1_TX,
    DMA_CH_UART1_RX,
    DMA_CH_UART2_TX,
    DMA_CH_UART2_RX,
    DMA_CH_SPI0_TX,
    DMA_CH_SPI0_RX,
    DMA_CH_SPI1_TX,
    DMA_CH_SPI1_RX,
    DMA_CH_SPI2_TX,
    DMA_CH_SPI2_RX,
    DMA_CH_I2C0_RX,
    DMA_CH_I2C0_TX,
    DMA_CH_I2C1_RX,
    DMA_CH_I2C1_TX,
}DMA_CHANNEL;


void DMA_IRQHandler(void);

/*
    ALTO DMA END
*/

/*
    ALTO SPI-CTRL BEGIN
*/

#define SPI_CTRL_REG_BASE   0x4000B000

/*
    ALTO SPI-CTRL END
*/

/*
    ALTO GPIO BEGIN
*/

#define GPIO_GROUP0_REG_BASE 0x40001000
#define GPIO_GROUP1_REG_BASE 0x40002000
#define ASR_GPIO_TOTAL_NUM   24

#define GPIO0_INDEX 0
#define GPIO1_INDEX 1
#define GPIO2_INDEX 2
#define GPIO3_INDEX 3
#define GPIO4_INDEX 4
#define GPIO5_INDEX 5
#define GPIO6_INDEX 6
#define GPIO7_INDEX 7
#define GPIO8_INDEX 8
#define GPIO9_INDEX 9
#define GPIO10_INDEX 10
#define GPIO11_INDEX 11
#define GPIO12_INDEX 12
#define GPIO13_INDEX 13
#define GPIO14_INDEX 14
#define GPIO15_INDEX 15
#define GPIO16_INDEX 16
#define GPIO17_INDEX 17
#define GPIO18_INDEX 18
#define GPIO19_INDEX 19
#define GPIO20_INDEX 20
#define GPIO21_INDEX 21
#define GPIO22_INDEX 22
#define GPIO23_INDEX 23

void GPIO_IRQHandler(void);
/*
    ALTO GPIO END
*/

 /* ALTO I2C BEGIN
 */
#define I2C0_BASE     0x4008A000
#define I2C1_BASE     0x4008B000

#define I2C0_INDEX    0
#define I2C1_INDEX    1
#define I2C_NUM       2

void I2C0_IRQHandler();
void I2C1_IRQHandler();

/*
 * ALTO I2C END
 */

/*
    ALTO UART BEGIN
*/
#define UART0_BASE (0x40080000+0x1000)
#define UART1_BASE (0x40080000+0x2000)
#define UART2_BASE (0x40080000+0x3000)
#define UART3_BASE NULL

#define UART0_INDEX    0
#define UART1_INDEX    1
#define UART2_INDEX    2
#define UART_NUM       3

void UART0_IRQHandler();
void UART1_IRQHandler();
void UART2_IRQHandler();


//#define UART0_BUS_CLK_EN       (0X01<<1)
//#define UART0_PERI_CLK_EN      (0X01<<14)
//#define UART1_BUS_CLK_EN       (0X01<<2)
//#define UART1_PERI_CLK_EN      (0X01<<15)

//#define UART0_IRQ_BIT           (0X01<<8)
//#define UART1_IRQ_BIT           (0X01<<9)
/*
    ALTO UART END
*/

/*
    ALTO SPI BEGIN
*/

#define SPI0_BASE (0x40080000+0x4000)
#define SPI1_BASE (0x40080000+0x5000)
#define SPI2_BASE (0x40080000+0x6000)


#define SPI0_INDEX    0
#define SPI1_INDEX    1
#define SPI2_INDEX    2
#define SPI_NUM       3

void SPI0_IRQHandler();
void SPI1_IRQHandler();
void SPI2_IRQHandler();


/*
    ALTO SPI END
*/


/*
    ALTO TIMER BEGIN
*/
#define TIMER0_BASE (0x40080000+0x7000)
#define TIMER1_BASE (0x40080000+0x7020)
#define TIMER2_BASE NULL
#define TIMER3_BASE NULL

#define TIMER1_INDEX    0
#define TIMER2_INDEX    1
#define TIMER_NUM 2

void TIMER_IRQHandler(void);

#define TIMER_CLK 52000000

/*
    ALTO TIMER END
*/

/*
    ALTO WDG BEGIN
*/

#define WDOG_BASE       0x40080000
//WDG APB CLOCK 0-15 BIT 4-7
#define APB_DIV_1   0
#define APB_DIV_2   1
#define APB_DIV_3   2
#define APB_DIV_4   3
#define APB_DIV_5   4
#define APB_DIV_6   5
#define APB_DIV_7   6
#define APB_DIV_8   7
#define APB_DIV_9   8
#define APB_DIV_10  9
#define APB_DIV_11  10
#define APB_DIV_12  11
#define APB_DIV_13  12
#define APB_DIV_14  13
#define APB_DIV_15  14
#define APB_DIV_16  15
#define WDG_APB_DIV APB_DIV_1


void WDG_IRQHandler(void);

#define WDG_CLOCK  (52000000)

/*
    ALTO WDG END
*/

/*
    ALTO PWM BEGIN
*/
#define PWM_REG_BASE 0x40088000

#define PWM_CH_NUM 8
#define CAP_CH_NUM 4

void PWM_IRQHandler();
#define PWM_CLOCK (52000000)

/*
    ALTO PWM END
*/


/*
    ALTO RTC BEGIN
*/
#define RTC_REG_BASE 0x40000A20
void SLEEP_IRQHandler();
#define RTC_NUM 1

#define RTC_RCO32K_REG   (SYS_CON_REG_BASE + 0xA44)
/*
    ALTO RTC END
*/

/*
    ALTO I2S BEGIN
*/
#define I2S_BASE            0x4008D000

void I2S_IRQHandler(void);

/*
    ALTO I2S END
*/

/*
    ALTO PSRAM BEGIN
*/
#define PSRAM_REG_BASE      0x4000A000
#define PSRAM_AMBA_BASE     0x30000000
/*
    ALTO PSRAM END
*/

/*
 * ALTO USB BEGIN
 */
#define USB_REG_BASE        0x40002000
#define USB_PHY_BASE        0x40003000
/*
 * ALTO USB END
 */

/*
 * ALTO ADC BEGIN
 */
typedef enum
{
  ADC_SAMPLE_1M,
  ADC_SAMPLE_500K,
  ADC_SAMPLE_250K,
  ADC_SAMPLE_125K
}AUX_ADC_SMP_RATE;

#define  AUX_ADC_CLK  (1<<12)
#define  ADC_SAMPLE_SEL ADC_SAMPLE_125K
#define  AUX_ADC_IRQ  (1<<20)

#define SYS_REG_BASE                  0x40000000
#define SYS_REG_BASE_CLKCTRL_ENABLE   ((SYS_REG_BASE + 0x844))
#define SYS_REG_BASE_CLKCTRL_DISABLE  ((SYS_REG_BASE + 0x84C))
#define SYS_REG_BASE_IRQ_ENABLE       ((SYS_REG_BASE + 0x944))
#define SYS_REG_BASE_IRQ_DISABLE      ((SYS_REG_BASE + 0x948))

#define SYS_REG_BASE_XOCTRL2          ((SYS_REG_BASE + 0xA70))
#define SYS_REG_BASE_REF_ROOT_CLK     ((SYS_REG_BASE + 0x80C))
#define SYS_REG_BASE_WIFI_CLK         ((SYS_REG_BASE + 0x85C))
#define SYS_REG_BASE_AUXADC_CFG           ((SYS_REG_BASE + 0x89000))
typedef void (*asr_adc_callback_func)(void *arg);

typedef struct
{
    union
    {
        struct
        {
            volatile uint32_t adc_resv : 24;
            volatile uint32_t adc_int_mode : 1;
            volatile uint32_t adc_int_en : 1;
            volatile uint32_t adc_int_clr : 1;
            volatile uint32_t adc_resv1 : 5;
        }BITS_ADC_CTRL;
        volatile uint32_t ADC_CTRL; /* adc control */
    };
    volatile uint32_t ADC_DATA;
}adc_struct;
#define ASR_ADC_CFG_STRUCT  (( adc_struct *)(SYS_REG_BASE_AUXADC_CFG))
//#define BIT(pos) (1U<<(pos))
/* Modem Config */
#define MDM_CLKGATEFCTRL0_ADDR          0x60C00874


typedef enum
{
    ADC_CHANNEL_NUM0,
    ADC_CHANNEL_NUM1,
    ADC_CHANNEL_NUM2,
    ADC_CHANNEL_NUM3,
    ADC_CHANNEL_NUM4,
    ADC_CHANNEL_NUM5,
    ADC_CHANNEL_NUM6,
    ADC_CHANNEL_NUM7,
    ADC_CHANNEL_TEMN,
    ADC_CHANNEL_TEMP
}asr_adc_channel_t;

typedef struct
{
    uint32_t sampling_cycle;  /* sampling period in number of ADC clock cycles */
}asr_adc_config_t;
typedef enum
{
    MOD_TRIG,
    MOD_CNT10
}AUX_ADC_MOD;

typedef struct
{
    uint8_t           port;   /* adc port */
    asr_adc_config_t config; /* adc config */
    void              *priv;   /* priv data */
} asr_adc_dev_t;

void AUX_ADC_IRQHandler(void);
/*
 * ALTO ADC END
 */

#ifdef __cplusplus
}
#endif

#endif
