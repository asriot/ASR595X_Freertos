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
#include "asr_port.h"
#include "asr_common.h"
#include "asr_dma.h"
#include "asr_pinmux.h"
#include "asr_gpio.h"
#include "asr_port_peripheral.h"
#include "asr_uart.h"
#include "asr_rf_spi.h"
#include "asr_i2s.h"
#include "asr_adc.h"
#ifdef I2S_DEMO
#include "asr_alto_apll_rf.h"
#endif
/************************************  gpio  **********************************************/
void port_gpio_input_enable(Pad_Num_Type pad_num)
{
}

void  port_gpio_enable_irq(Pad_Num_Type pad_num)
{
    uint32_t reg_value, gpio_irq_bit;
    gpio_irq_bit = (0X01<<(GPIO_IRQ_BIT_OFFSET));
    reg_value = REG_RD(IRQ_EN_REG) & (~ gpio_irq_bit);
    REG_WR(IRQ_EN_REG, (reg_value | gpio_irq_bit));//enable gpio irq
    ECLIC_Register_IRQ(GPIO_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                        ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}
void port_gpio_init_clk(Pad_Num_Type pad_num)
{
    uint32_t controller_clk_enable_bit = 0;
    uint32_t peri_clk_enable_bit = 0;
    uint32_t tmp_value=0;
    if(pad_num < ASR_GPIO_NUM_PER_GROUP)
    {
        controller_clk_enable_bit = (0x01 << GPIO_CONTROLLER_CLK_BIT_OFFSET);
        peri_clk_enable_bit = (0x01 << GPIO_PERI_CLK_BIT_OFFSET);
    }
    else if(pad_num >= ASR_GPIO_NUM_PER_GROUP && pad_num < 2*ASR_GPIO_NUM_PER_GROUP)
    {
        controller_clk_enable_bit = (0x01 << (2 + GPIO_CONTROLLER_CLK_BIT_OFFSET));
        peri_clk_enable_bit = (0x01 << (2 + GPIO_PERI_CLK_BIT_OFFSET));
    }
    else
        return;
    tmp_value = REG_RD(PERI_CLK_EN_REG0) & (~(controller_clk_enable_bit | peri_clk_enable_bit));
    REG_WR(PERI_CLK_EN_REG0, (tmp_value | (controller_clk_enable_bit | peri_clk_enable_bit)));
}

void port_pad_config(Pad_Num_Type pad_num, Pad_Pull_Type pull_type)
{
    switch(pull_type)
    {
        case PULL_UP:
            *(volatile uint32_t *)(HW_CTRL_PE_PS) &= ~(0x1 << (pad_num));
            *(volatile uint32_t *)(PAD_PE_REG) |= (0x1 << (pad_num));
            *(volatile uint32_t *)(PAD_PS_REG) |= (0x1 << (pad_num));
            break;
        case PULL_DOWN:
            *(volatile uint32_t *)(HW_CTRL_PE_PS) &= ~(0x1 << (pad_num));
            *(volatile uint32_t *)(PAD_PE_REG) |= (0x1 << (pad_num));
            *(volatile uint32_t *)(PAD_PS_REG) &= ~(0x1 << (pad_num));
            break;
        case PULL_NONE:
            *(volatile uint32_t *)(HW_CTRL_PE_PS) &= ~(0x1 << (pad_num));
            *(volatile uint32_t *)(PAD_PE_REG) &= ~(0x1 << (pad_num));
            break;
        default:
            break;
     }

}
/*****************************  uart  ****************************************/


void port_uart_enable_irq(uint8_t uart_idx)
{
    uint32_t tmp_value;
    uint32_t uartx_irq_bit = (0X01<<(UART_IRQ_BIT_OFFSET + uart_idx));
    IRQn_Type uart_irq = UART0_IRQn - uart_idx;
    void *irq_handler = NULL;
    switch(uart_idx)
    {
        case UART0_INDEX:
            irq_handler = UART0_IRQHandler;
            break;
        case UART1_INDEX:
            irq_handler = UART1_IRQHandler;
            break;
        case UART2_INDEX:
            irq_handler = UART2_IRQHandler;
            break;
        default:
            break;
    }
    if(!irq_handler)
        return;
    tmp_value = REG_RD(IRQ_EN_REG) & (~ uartx_irq_bit);
    REG_WR(IRQ_EN_REG, (tmp_value | uartx_irq_bit));
    ECLIC_Register_IRQ(uart_irq, ECLIC_NON_VECTOR_INTERRUPT,
                ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}

void port_uart_disable_irq(uint8_t uart_idx)
{
    uint32_t tmp_value;
    uint32_t uartx_irq_bit = (0X01<<(UART_IRQ_BIT_OFFSET + uart_idx));
    IRQn_Type uart_irq = UART0_IRQn - uart_idx;

    tmp_value = REG_RD(IRQ_DIS_REG) & (~ uartx_irq_bit);
    REG_WR(IRQ_DIS_REG, (tmp_value | uartx_irq_bit));
    ECLIC_DisableIRQ(uart_irq);

}

void port_uart_init_clk(uint8_t uart_idx)
{//may refine later, use common api to init all peripheral clk
    uint32_t bus_clk_enable_bit = (0x01 << (uart_idx+UART_BUS_CLK_BIT_OFFSET));
    uint32_t peri_clk_enable_bit = (0x01 << (uart_idx+UART_PERI_CLK_BIT_OFFSET));
    uint32_t tmp_value=0;

    //enable clk
    tmp_value = REG_RD(PERI_CLK_EN_REG1) & (~(bus_clk_enable_bit | peri_clk_enable_bit));
    REG_WR(PERI_CLK_EN_REG1, (tmp_value | (bus_clk_enable_bit | peri_clk_enable_bit)));

    //APB CLK DIV
    tmp_value = REG_RD(APB_CLK_DIV_REG);
    tmp_value &= (~(0X03<<2));
    REG_WR(APB_CLK_DIV_REG, (tmp_value | (APBCLK_DIV_CFG)));
}
void port_uart_deinit_clk(uint8_t uart_idx)
{
    uint32_t bus_clk_enable_bit = 0x01 << (uart_idx+UART_BUS_CLK_BIT_OFFSET);
    uint32_t peri_clk_enable_bit = 0x01 << (uart_idx+UART_PERI_CLK_BIT_OFFSET);
    uint32_t tmp_value=0;

    tmp_value = REG_RD(PERI_CLK_DIS_REG1) & (~(bus_clk_enable_bit|peri_clk_enable_bit));
    REG_WR(PERI_CLK_DIS_REG1, (tmp_value | (bus_clk_enable_bit|peri_clk_enable_bit)));

}

/*****************************  SPI  ****************************************/

void port_spi_enable_irq(uint8_t spi_idx)
{
    IRQn_Type spi_irq = SPI0_IRQn;
    void *irq_handler = NULL;
    uint32_t spix_irq_bit = (0X01<<(SPI_IRQ_BIT_OFFSET + spi_idx));
    uint32_t tmp_value;
    switch(spi_idx)
    {
        case SPI0_INDEX:
            irq_handler = SPI0_IRQHandler;
            spi_irq = SPI0_IRQn;
            break;
        case SPI1_INDEX:
            irq_handler = SPI1_IRQHandler;
            spi_irq = SPI1_IRQn;
            break;
        case SPI2_INDEX:
            irq_handler = SPI2_IRQHandler;
            spi_irq = SPI2_IRQn;
            break;

        default:
            return;
    }
    if(!irq_handler)
        return;
    tmp_value = REG_RD(IRQ_EN_REG) & (~ spix_irq_bit);
    REG_WR(IRQ_EN_REG, (tmp_value | spix_irq_bit));
    ECLIC_Register_IRQ(spi_irq, ECLIC_NON_VECTOR_INTERRUPT,
                ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}

void port_spi_disable_irq(uint8_t spi_idx)
{
    IRQn_Type spi_irq = SPI0_IRQn;
    uint32_t tmp_value;
    uint32_t spix_irq_bit = (0X01<<(SPI_IRQ_BIT_OFFSET + spi_idx));
    switch(spi_idx)
    {
        case SPI0_INDEX:
            spi_irq = SPI0_IRQn;
            break;
        case SPI1_INDEX:
            spi_irq = SPI1_IRQn;
            break;
        case SPI2_INDEX:
            spi_irq = SPI2_IRQn;
            break;

        default:
            return;
    }
    tmp_value = REG_RD(IRQ_DIS_REG) & (~ spix_irq_bit);
    REG_WR(IRQ_DIS_REG, (tmp_value | spix_irq_bit));
    ECLIC_DisableIRQ(spi_irq);

}

void port_spi_init_clk(uint8_t spi_idx)
{//may refine later, use common api to init all peripheral clk
    uint32_t bus_clk_enable_bit = (0x01 << (spi_idx+SPI_BUS_CLK_BIT_OFFSET));
    uint32_t peri_clk_enable_bit = (0x01 << (spi_idx+SPI_PERI_CLK_BIT_OFFSET));
    uint32_t tmp_value=0;

    //enable clk
    tmp_value = REG_RD(PERI_CLK_EN_REG1) & (~(bus_clk_enable_bit | peri_clk_enable_bit));
    REG_WR(PERI_CLK_EN_REG1, (tmp_value | (bus_clk_enable_bit | peri_clk_enable_bit)));

    //APB CLK DIV
    tmp_value = REG_RD(APB_CLK_DIV_REG);
    tmp_value &= (~(0X03<<2));
    REG_WR(APB_CLK_DIV_REG, (tmp_value | (APBCLK_DIV_CFG)));
}
void port_spi_deinit_clk(uint8_t spi_idx)
{

    uint32_t bus_clk_enable_bit = 0x01 << (spi_idx+SPI_BUS_CLK_BIT_OFFSET);
    uint32_t peri_clk_enable_bit = 0x01 << (spi_idx+SPI_PERI_CLK_BIT_OFFSET);
    uint32_t tmp_value=0;

    tmp_value = REG_RD(PERI_CLK_DIS_REG1) & (~(bus_clk_enable_bit|peri_clk_enable_bit));
    REG_WR(PERI_CLK_DIS_REG1, (tmp_value | (bus_clk_enable_bit|peri_clk_enable_bit)));
}


/*****************************  dma  ****************************************/

void port_dma_enable_irq(void)
{
    uint32_t tmp_value;
    //enable syscon irq bit
    tmp_value = REG_RD(IRQ_EN_REG) & (~ DMA_IRQ_BIT);
    REG_WR(IRQ_EN_REG, (tmp_value | DMA_IRQ_BIT));
    ECLIC_Register_IRQ(DMA_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}

void port_dma_disable_irq(void)
{
    uint32_t tmp_value;

    tmp_value = REG_RD(IRQ_DIS_REG) & (~ DMA_IRQ_BIT);
    REG_WR(IRQ_DIS_REG, (tmp_value | DMA_IRQ_BIT));
    ECLIC_DisableIRQ(DMA_IRQn);

}

void port_dma_init_clk(void)
{
    uint32_t tmp_value=0;
    //enable clk
    tmp_value = REG_RD(PERI_CLK_EN_REG0) & (~DMA_CLK_EN);
    REG_WR(PERI_CLK_EN_REG0, (tmp_value | (DMA_CLK_EN)));
}
void port_dma_deinit_clk(void)
{
    uint32_t tmp_value=0;

    tmp_value = REG_RD(PERI_CLK_DIS_REG0) & (~DMA_CLK_EN);
    REG_WR(PERI_CLK_DIS_REG0, (tmp_value | DMA_CLK_EN));

}

void port_dma_init(void)
{
    DMA_HANDSHAKE_CFG0 = 0XFFFFFFFF;
    DMA_HANDSHAKE_CFG1 = 0XFFFFFFFF;
    //OPEN DMA CLOCK
    port_dma_init_clk();
    //open DMA interrupt
    port_dma_enable_irq();
}

void port_dma_finalize(void)
{
    DMA_HANDSHAKE_CFG0 = 0X0;
    DMA_HANDSHAKE_CFG1 = 0X0;
    //close DMA CLOCK
    port_dma_deinit_clk();
    //open DMA interrupt
    port_dma_disable_irq();
}

uint8_t port_dma_get_uart_ch(uint8_t uart_idx,uint8_t tx_mode)
{
    uint8_t dma_chan = 0;
    if(tx_mode == 1)
    {
        if( uart_idx == 2)
        {
            dma_chan = 4; //uart2 tx channel
        }
        else if( uart_idx == 1)
        {
            dma_chan = 2; //uart1 tx channel
        }
        else if( uart_idx == 0)
        {
            dma_chan = 0; //uart0 tx channel
        }
    }
    else
    {
        if( uart_idx == 2)
        {
            dma_chan = 5; //uart2 rx channel
        }
        else if( uart_idx == 1)
        {
            dma_chan = 3; //uart1 rx channel
        }
        else if( uart_idx == 0)
        {
            dma_chan = 1; //uart0 rx channel
        }
    }
    return dma_chan;
}

uint8_t port_dma_get_spi_ch(uint8_t spi_idx,uint8_t tx_mode)
{
    uint8_t dma_chan = 0;
    if(tx_mode == 1)
    {
        if( spi_idx == 2)
        {
            dma_chan = 10; //spi2 tx channel
        }
        else if( spi_idx == 1)
        {
            dma_chan = 8; //spi1 tx channel
        }
        else if( spi_idx == 0)
        {
            dma_chan = 6; //spi0 tx channel
        }
    }
    else
    {
        if( spi_idx == 2)
        {
            dma_chan = 11; //spi2 rx channel
        }
        else if( spi_idx == 1)
        {
            dma_chan = 9; //spi1 rx channel
        }
        else if( spi_idx == 0)
        {
            dma_chan = 7; //spi0 rx channel
        }
    }
    return dma_chan;
}
/*****************************  timer  ****************************************/

void port_timer_init_clk(uint8_t timer_idx)
{
    uint32_t reg_value = 0;
    // Set Timer Clock Enable
    reg_value = REG_RD(PERI_CLK_EN_REG1) & (~TIMER_BUS_CLK_BIT);
    REG_WR(PERI_CLK_EN_REG1, (reg_value | (TIMER_BUS_CLK_BIT)));
}

void port_timer_deinit_clk(uint8_t timer_idx)
{
    uint32_t reg_value = 0;
    // Set Timer Clock disable
    reg_value = REG_RD(PERI_CLK_DIS_REG1) & (~TIMER_BUS_CLK_BIT);
    REG_WR(PERI_CLK_DIS_REG1, (reg_value | (TIMER_BUS_CLK_BIT)));
}

void port_timer_enable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_EN_REG) & (~ TIMER_IRQ_BIT);
    REG_WR(IRQ_EN_REG, (reg_value | TIMER_IRQ_BIT));

    ECLIC_Register_IRQ(TIMER_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}

void port_timer_disable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_DIS_REG) & (~ TIMER_IRQ_BIT);
    REG_WR(IRQ_DIS_REG, (reg_value | TIMER_IRQ_BIT));

    ECLIC_DisableIRQ(TIMER_IRQn);
}

/*****************************  watchdog  **************************************/

DEBUG_SECTION void port_wdg_init_clk(void)
{
    uint32_t reg_value = 0;
    // Set Wdg Clock Enable
    reg_value = REG_RD(PERI_CLK_EN_REG1) & (~WDG_BUS_CLK_BIT);
    REG_WR(PERI_CLK_EN_REG1, (reg_value | (WDG_BUS_CLK_BIT)));
    reg_value = REG_RD(WDG_APB_DIV_REG);
    REG_WR(WDG_APB_DIV_REG,(reg_value | (WDG_APB_DIV<<4)));

}

DEBUG_SECTION void port_wdg_deinit_clk(void)
{
    uint32_t reg_value = 0;
    // Set Wdg Clock disable
    reg_value = REG_RD(PERI_CLK_DIS_REG1) & (~WDG_BUS_CLK_BIT);
    REG_WR(PERI_CLK_DIS_REG1, (reg_value | (WDG_BUS_CLK_BIT)));
}

DEBUG_SECTION void port_wdg_enable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_EN_REG) & (~ WDG_IRQ_BIT);
    REG_WR(IRQ_EN_REG, (reg_value | WDG_IRQ_BIT));
    ECLIC_Register_IRQ(WDG_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
}

DEBUG_SECTION void port_wdg_disable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_DIS_REG) & (~ WDG_IRQ_BIT);
    REG_WR(IRQ_DIS_REG, (reg_value | WDG_IRQ_BIT));
    ECLIC_DisableIRQ(WDG_IRQn);
}

/*****************************  efuse  ****************************************/

void port_efuse_write_enable(void)
{
    uint16_t tmp_16;
    uint32_t tmp_32;
    // ----- Change APLL clock to 80MHz -----
//    SYS_CRM_WIFI_BLK_CLK = 0x1; // Enable WiFi core clock
//    delay(5); // wait for a few cycles for WiFi core clock settle
//    MDM_CLKGATEFCTRL0 = (0x1<<27);  // Force RC clock open

    //open 10uA current
    tmp_16 = spi_mst_read(TRX_PD_CTRL1_REG_ADDR);
    tmp_16 &= (~(0x0001 << 13)); //clear bit13 (D_PD_BG)
    spi_mst_write(TRX_PD_CTRL1_REG_ADDR, tmp_16);

    tmp_16 = spi_mst_read(TRX_PD_CTRL2_REG_ADDR);
    tmp_16 &= (~(0x0003 << 2)); //clear bit<3:2> (D_PD_TRXTOP_BIAS, D_PD_TRXTOP_LDO)
    spi_mst_write(TRX_PD_CTRL2_REG_ADDR, tmp_16);

    //open PU LDO25: set D_AON_RCO32K_REG1<13> to 1
    tmp_32 = REG_RD(RTC_RCO32K_REG);
    REG_WR(RTC_RCO32K_REG, tmp_32 | (0x00000001 << (13 + 16)));

    //adjust PU LDO25 voltage: set D_AON_RCO32K_REG1<12:9> to 4'b0011
    tmp_32 = REG_RD(RTC_RCO32K_REG);
    tmp_32 &= ~(0x0000000F << (9 + 16));
    tmp_32 |= (0x00000003 << (9 + 16));
    REG_WR(RTC_RCO32K_REG, tmp_32);
}
/*****************************  i2s  ****************************************/
void port_i2s_enable_irq(void)
{
    ECLIC_Register_IRQ(I2S_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
            ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
    REG_WR(IRQ_EN_REG, (REG_RD(IRQ_EN_REG) | I2S_IRQ_BIT));
}

void port_i2s_disable_irq(void)
{
    REG_WR(IRQ_DIS_REG, (REG_RD(IRQ_DIS_REG) | I2S_IRQ_BIT));
    ECLIC_DisableIRQ(I2S_IRQn);
}

int port_i2s_config_clk(uint32_t clk_src)
{
#ifdef I2S_DEMO
    asr_soc_wifi_clk_enable();
    asr_soc_mac_phy_clock_enable();
#endif
    if(clk_src == I2S_MCLK_SRC_FREQ72)
        spi_mst_write(0xfd, 0x20df);    //72M
    else if(clk_src == I2S_MCLK_SRC_FREQ96)
        spi_mst_write(0xfd, 0x219f);    //96M
    else if(clk_src == I2S_MCLK_SRC_FREQ120)
        spi_mst_write(0xfd, 0x211f);    //120M
    else
        return -1;
    return 0;
}

void port_i2s_enable_clk()
{
    REG_WR(PERI_CLK_EN_REG1, REG_RD(PERI_CLK_EN_REG1) | I2S_CLK_BIT);
}

void port_i2s_disable_clk()
{
    REG_WR(PERI_CLK_DIS_REG1, REG_RD(PERI_CLK_DIS_REG1) | I2S_CLK_BIT);
}

/************************************ psram **********************************************/
void port_psram_init_clk(void)
{
    REG_WR(PERI_CLK_EN_REG0, REG_RD(PERI_CLK_EN_REG0) | PSRAM_CLK_BIT);
}

int port_psram_set_channel(asr_psram_channel channel)
{
    if(channel != PSRAM_CHANNEL_4_9 && channel != PSRAM_CHANNEL_16_21)
        return -1;

    REG_WR(ADC_SDIO_BLE_DEBUG_CTRL_REG,REG_RD(ADC_SDIO_BLE_DEBUG_CTRL_REG) & (~(1<<3)));//disable sdio debug

    if(channel == PSRAM_CHANNEL_4_9)
    {
        asr_pinmux_config(PAD4, PF_PSRAM);//set pad4 mux to psram
        asr_pinmux_config(PAD5, PF_PSRAM);//set pad5 mux to psram
        asr_pinmux_config(PAD6, PF_PSRAM);//set pad6 mux to psram
        asr_pinmux_config(PAD7, PF_PSRAM);//set pad7 mux to psram
        asr_pinmux_config(PAD8, PF_PSRAM);//set pad8 mux to psram
        asr_pinmux_config(PAD9, PF_PSRAM);//set pad9 mux to psram

        REG_WR(HW_CTRL_PE_PS, REG_RD(HW_CTRL_PE_PS) & 0xfffffc0f);//set all pad pull up and down
    }
    else
    {
        asr_pinmux_config(PAD16, PF_PSRAM);//set pad16 mux to psram
        asr_pinmux_config(PAD17, PF_PSRAM);//set pad17 mux to psram
        asr_pinmux_config(PAD18, PF_PSRAM);//set pad18 mux to psram
        asr_pinmux_config(PAD19, PF_PSRAM);//set pad19 mux to psram
        asr_pinmux_config(PAD20, PF_PSRAM);//set pad20 mux to psram
        asr_pinmux_config(PAD21, PF_PSRAM);//set pad21 mux to psram

        REG_WR(HW_CTRL_PE_PS, REG_RD(HW_CTRL_PE_PS) & 0xffc0ffff);//set all pad pull up and down
    }
    return 0;
}

void port_rtc_enable_irq(void)
{
    uint32_t reg_value = 0;
    uint32_t rtc_irq_bit = 0;
    rtc_irq_bit = (0X01<<(SLEEP_IRQ_BIT_OFFSET));
    reg_value = REG_RD(IRQ_EN_REG) & (~ rtc_irq_bit);
    REG_WR(IRQ_EN_REG, (reg_value | rtc_irq_bit));//enable gpio irq
    ECLIC_Register_IRQ(SLEEP_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                            ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
}

void port_rtc_disable_irq(void)
{
    uint32_t reg_value = 0;
    uint32_t rtc_irq_bit = 0;
    rtc_irq_bit = (0X01<<(SLEEP_IRQ_BIT_OFFSET));
    reg_value = REG_RD(IRQ_DIS_REG) & (~(rtc_irq_bit));
    REG_WR(IRQ_DIS_REG, (reg_value | rtc_irq_bit));
    ECLIC_DisableIRQ(SLEEP_IRQn);
}


/*****************************  pwm  ****************************************/

void port_pwm_enable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_EN_REG) & (~ PWM_IRQ_BIT);
    REG_WR(IRQ_EN_REG, (reg_value | PWM_IRQ_BIT));

    ECLIC_Register_IRQ(PWM_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                    ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
}

void port_pwm_disable_irq(void)
{
    uint32_t reg_value = 0;
    reg_value = REG_RD(IRQ_DIS_REG) & (~ PWM_IRQ_BIT);
    REG_WR(IRQ_DIS_REG, (reg_value | PWM_IRQ_BIT));

    ECLIC_DisableIRQ(PWM_IRQn);
}

void port_pwm_init_clk(void)
{
    uint32_t reg_value = 0;
    // Set pwm Clock Enable
    reg_value = REG_RD(PERI_CLK_EN_REG1) & (~PWM_BUS_CLK_BIT);
    REG_WR(PERI_CLK_EN_REG1, (reg_value | (PWM_BUS_CLK_BIT)));
}
void port_pwm_deinit_clk(void)
{
    uint32_t reg_value = 0;
    // Set pwm Clock disable
    reg_value = REG_RD(PERI_CLK_DIS_REG1) & (~PWM_BUS_CLK_BIT);
    REG_WR(PERI_CLK_DIS_REG1, (reg_value | (PWM_BUS_CLK_BIT)));
}

asr_adc_callback_func g_asr_adc_callback_handler;

void AUX_ADC_IRQHandler_Dummy(void)
{
    uint32_t vol_value[10] = {0};
    // save CSR context
    SAVE_IRQ_CSR_CONTEXT();
    if (g_asr_adc_callback_handler)
    {
        for(uint8_t i = 0;i<10;i++)
        {
            vol_value[i] = (ASR_ADC_CFG_STRUCT->ADC_DATA & 0xFFF0) >> 4;
        }
        g_asr_adc_callback_handler(vol_value);
    }
    ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr = 1;
    while (ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr)
    {
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr = 0;
    }
    // restore CSR context
    RESTORE_IRQ_CSR_CONTEXT();
}

static uint8_t rec_mode = 0;
static void port_rf_auxadc_mode(AUX_ADC_MOD mode)
{
    uint8_t tmp=0;
    static uint8_t cfg_fg=1;
    if (cfg_fg !=1)
    {
      tmp=mode;
      if(tmp ==rec_mode)
      {

          return;
      }
    }
    cfg_fg =0;
    rec_mode=mode;
    // Enable XO CLK AUCADC, DFF's RB;D_RST_XO_CLK_AUXADC= 0
    rf_set_reg_bit(0x0F, 11, 1, 1);
    delay(100);
    rf_set_reg_bit(0xa3,12,2,ADC_SAMPLE_125K);
    // Enable AUXADC
    rf_set_reg_bit(0x06, 14, 1, 0x0);
    // Enable TMMT
    rf_set_reg_bit(0x07, 4, 1, 0x0);
    // Enable CLK AUXADC13M; D_XO_CLK_AUXADC13M_EN= 1
    rf_set_reg_bit(0x75, 14, 1, 0x1);
    // Enable XO CLK AUCADC, DFF's RB;D_RST_XO_CLK_AUXADC= 0
    rf_set_reg_bit(0x0F, 11, 1, 0);
    delay(250); // delay(2000)
    if (mode == MOD_TRIG)
    {
        rf_set_reg_bit(0xA3, 6, 1, 0x0);
    }
    else if (mode == MOD_CNT10)
    {
        rf_set_reg_bit(0xA3, 6, 1, 0x1);
    }

}

int32_t port_adc_init(asr_adc_dev_t *adc_config)
{
    uint32_t reg_value;
    uint8_t temp_chan;
    asr_gpio_dev_t config_gpio;
    if (adc_config->port > 9)
    return 0;

    config_gpio.port = adc_config->port + 4;
    config_gpio.config = ASR_INPUT_HIGH_IMPEDANCE;
    config_gpio.priv = NULL;
    asr_gpio_init(&config_gpio);
    reg_value = REG_RD(HW_CTRL_PE_PS);
    REG_WR(HW_CTRL_PE_PS, (reg_value & (~(1 << config_gpio.port)))); // cfg by// user
    reg_value = REG_RD(PAD_PE_REG);                                  //
    // REG_WR(PAD_PE_REG, (reg_value|( (1 << gpio->port))));
    REG_WR(PAD_PE_REG, (reg_value & (~(1 << config_gpio.port))));
//    adc_config->asrt_adc_handler_struct.cb = adc_config->priv;
    if (adc_config->priv)
    {
        port_rf_auxadc_mode(MOD_CNT10);
    }
    else
    {
        port_rf_auxadc_mode(MOD_TRIG);
    }
    REG_WR(SYS_REG_BASE_CLKCTRL_ENABLE,REG_RD(SYS_REG_BASE_CLKCTRL_ENABLE) | AUX_ADC_CLK);
    delay(2000);


    if (adc_config->port <= 7)
    {


        REG_WR(SYS_REG_BASE_XOCTRL2,((REG_RD(SYS_REG_BASE_XOCTRL2) & (~(uint32_t)0x7)) |(uint32_t)adc_config->port) |((uint32_t)(1<<9)));
//        REG_WR(SYS_REG_BASE_AUXADC,REG_RD(SYS_REG_BASE_AUXADC) | ((uint32_t)(BIT(9))));
        delay(2000);
    }
   else
    {
        temp_chan = adc_config->port - 8;
        REG_WR(SYS_REG_BASE_XOCTRL2,((REG_RD(SYS_REG_BASE_XOCTRL2) & (~(uint32_t)0x7)) |(uint32_t)temp_chan) &(~(uint32_t)((1<<9))));
        delay(2000);

    }
    delay(200);
    if (adc_config->priv)
    {
        g_asr_adc_callback_handler = (asr_adc_callback_func)(adc_config->priv);
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr = 0;
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_mode =MOD_CNT10;
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_en = 1;
        // inital adc interrupt as vector interrupt
        REG_WR(SYS_REG_BASE_IRQ_ENABLE,REG_RD(SYS_REG_BASE_IRQ_ENABLE) | AUX_ADC_IRQ);
        delay(1000);
        ECLIC_Register_IRQ(AUX_ADC_IRQn, ECLIC_NON_VECTOR_INTERRUPT,
                        ECLIC_LEVEL_TRIGGER, 2, 0, NULL);
    }
    else
    {
        g_asr_adc_callback_handler = NULL;
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr = 0;
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_mode = MOD_TRIG;
        ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_en = 0;
    }
    return 0;
}

int32_t port_adc_get(asr_adc_dev_t *adc_config)
{
    int32_t vol_value = 0;
    if (rf_get_reg_bit(0x06, 14, 1))
    {
        return 0;
    }
    vol_value =(int16_t) ((ASR_ADC_CFG_STRUCT->ADC_DATA & 0xFFF0) >> 4);
    if (adc_config->port < 8)
    {
        return (int32_t)(0.4149 * vol_value + 22.8);
    }
    else
    {
    return 0;
    }
}
int32_t port_tempr_get(asr_adc_dev_t *adc_config)
{
    static int16_t temp_n, temp_p;
    static uint8_t tp_flag = 0;
    if (adc_config->port < 8)
    return 0;
    if (adc_config->port == ADC_CHANNEL_TEMN)
    {
      temp_n = (int16_t)((ASR_ADC_CFG_STRUCT->ADC_DATA & 0xFFF0) >> 4);
      tp_flag |= 0x1;
    }
    else
    {
        temp_p = (int16_t)((ASR_ADC_CFG_STRUCT->ADC_DATA & 0xFFF0) >> 4);
        tp_flag |= 0x2;
    }
    if (tp_flag == 0x3)
    {
        tp_flag = 0;
        return (int32_t)((0.29 * (temp_n - temp_p)) / 5.25 + 41.5);
    }
    else
    {
        return 0;
    }
}

int32_t port_adc_finalize(asr_adc_dev_t *adc_config)
{

    ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_clr = 0;
    ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_mode = 0;
    ASR_ADC_CFG_STRUCT->BITS_ADC_CTRL.adc_int_en = 0;
    ECLIC_DisableIRQ(AUX_ADC_IRQn);
    REG_WR(SYS_REG_BASE_CLKCTRL_DISABLE,REG_RD(SYS_REG_BASE_CLKCTRL_DISABLE) | AUX_ADC_CLK);
    return 0;
}

/************************************  i2c  **********************************************/
void port_i2c_init_clk(uint8_t i2c_idx)
{
    uint32_t bus_clk_enable_bit = (0x01 << (i2c_idx+I2C_BUS_CLK_BIT_OFFSET));
    uint32_t peri_clk_enable_bit = (0x01 << (i2c_idx+I2C_PERI_CLK_BIT_OFFSET));
    uint32_t tmp_value=0;

    //enable clk
    tmp_value = REG_RD(PERI_CLK_EN_REG1) & (~(bus_clk_enable_bit | peri_clk_enable_bit));
    REG_WR(PERI_CLK_EN_REG1, (tmp_value | (bus_clk_enable_bit | peri_clk_enable_bit)));
}

void port_i2c_deinit_clk(uint8_t i2c_idx)
{

    uint32_t bus_clk_enable_bit = 0x01 << (i2c_idx+I2C_BUS_CLK_BIT_OFFSET);
    uint32_t peri_clk_enable_bit = 0x01 << (i2c_idx+I2C_PERI_CLK_BIT_OFFSET);
    uint32_t tmp_value=0;

    tmp_value = REG_RD(PERI_CLK_DIS_REG1) & (~(bus_clk_enable_bit|peri_clk_enable_bit));
    REG_WR(PERI_CLK_DIS_REG1, (tmp_value | (bus_clk_enable_bit|peri_clk_enable_bit)));
}

void port_i2c_enable_irq(uint8_t i2c_idx)
{
    IRQn_Type i2c_irq = I2C0_IRQn;
    void *irq_handler = NULL;
    uint32_t i2cx_irq_bit = (0X01<<(I2C_IRQ_BIT_OFFSET + i2c_idx));
    uint32_t tmp_value;
    switch(i2c_idx)
    {
        case I2C0_INDEX:
            irq_handler = I2C0_IRQHandler;
            i2c_irq = I2C0_IRQn;
            break;
        case I2C1_INDEX:
            irq_handler = I2C1_IRQHandler;
            i2c_irq = I2C1_IRQn;
            break;
        default:
            return;
    }
    if(!irq_handler)
        return;
    tmp_value = REG_RD(IRQ_EN_REG) & (~ i2cx_irq_bit);
    REG_WR(IRQ_EN_REG, (tmp_value | i2cx_irq_bit));
    ECLIC_Register_IRQ(i2c_irq, ECLIC_NON_VECTOR_INTERRUPT,
                ECLIC_POSTIVE_EDGE_TRIGGER, 2, 0, NULL);
}

void port_i2c_disable_irq(uint8_t i2c_idx)
{
    IRQn_Type i2c_irq = I2C0_IRQn;
    uint32_t tmp_value;
    uint32_t i2cx_irq_bit = (0X01<<(I2C_IRQ_BIT_OFFSET + i2c_idx));
    switch(i2c_idx)
    {
        case I2C0_INDEX:
            i2c_irq = I2C0_IRQn;
            break;
        case I2C1_INDEX:
            i2c_irq = I2C1_IRQn;
            break;

        default:
            return;
    }
    tmp_value = REG_RD(IRQ_DIS_REG) & (~ i2cx_irq_bit);
    REG_WR(IRQ_DIS_REG, (tmp_value | i2cx_irq_bit));
    ECLIC_DisableIRQ(i2c_irq);
}
