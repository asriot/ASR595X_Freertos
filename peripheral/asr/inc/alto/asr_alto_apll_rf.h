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
#ifndef __ASR_RF_H__
#define __ASR_RF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000



/********************************************
 * @ TX CLIP
 *   fix DSSS and OFDM power problem
 *   set diff rate with diff tx power
 ********************************************/
#define TX_CLIP 1

/********************************************
 * @ DCDC powersave mode open/close
 *   1 for open dcdc powersave mode, 0 for close
 * if close, no pulse in current graph, but average current higher
 ********************************************/

#define INIT_FREQ        2412
#define INIT_PRIM_FREQ   INIT_FREQ
#define INIT_CENTER_FREQ INIT_FREQ
#define PHY_CHNL_BW      PHY_CHNL_BW_20

//debug print reg
#define RF_DEBUG 0

#define RETRY_T             2

#define FREQ_CHN14          2484
#define FREQ_CHN1           2412

#define IODELAY_ADC0        0x00
#define IODELAY_ADC1        0x00

#define COMBO_SPI_CTRL_ADDR     0x4000B000
#define TRANS_MODE_OFT          0x0800
#define PRESCALER_OFT           0x0804
#define ADDR_REG_OFT            0x0808
#define READNOTWRITE_OFT        0x080c
#define WDATA_REG_OFT           0x0810
#define RDATA_REG_OFT           0x0814
#define START_FLAG_OFT          0x0818

#define  MAXITERATIONS      5

#define CORDIC_ITERNUM      14

#define SYS_REG_BASE_CORE_CLK          ((SYS_REG_BASE + 0x804))
#define SYS_REG_BASE_FLASH_SEL_CLK     ((SYS_REG_BASE + 0x808))
#define SYS_REG_BASE_MAC_CORE_CLK      ((SYS_REG_BASE + 0x810))
#define SYS_REG_BASE_REF_ROOT_CLK      ((SYS_REG_BASE + 0x80C))
#define SYS_REG_BASE_WIFI_CLK          ((SYS_REG_BASE + 0x85C))
#define SYS_REG_BASE_DIG32_SEL         ((SYS_REG_BASE + 0x908))

#define SYS_REG_BASE_PMU_CTRL          ((SYS_REG_BASE + 0xA20))
#define SYS_REG_BASE_AUXADC            ((SYS_REG_BASE + 0x19000))
#define SYS_REG_DCDC_REG3              (SYS_REG_BASE + 0xA50)
#define SYS_REG_BASE_XOCTRL2           ((SYS_REG_BASE + 0xA70))
#define SYS_REG_BASE_XODCCA_CTRL       ((SYS_REG_BASE + 0xA74))

#define TX_PWR_CAL_CHAN_NUM (3)
#define TX_PWR_CAL_CHAN_DIVIDE_1 (5)
#define TX_PWR_CAL_CHAN_DIVIDE_2 (9)



#define RW_BLE_BB_EM_ADDR_BASE      0x62008000
#define RW_BLE_MDM_ADDR_BASE        0x62006000
#define RW_PTA_BASE_ADDR            0x60920000

enum check_type
{
    D_RCM_EOC = 0x1,
    D_PVM_EOC,
    APLL_FCAL_DONE,
    DBL_CAL_DONE,
    SX_VCO_FULL_CAL_DONE,
    SX_LO_MIX_CAL_DONE,
    DCOC_BIT0,
    DCOC_BIT1,
    DCOC_BIT2,
    DCOC_BIT3,
    DCOC_BIT4,
    DCOC_BIT5,
    TXPWR_CAL_AUXADC_DOWN,
    RCO_CAL_EN,
    NULL_BIT
};

#define RF_INIT10_SET_BIT (1<<0)
#define RF_INIT10_CAL_BIT (1<<1)

void asr_soc_wifi_clk_enable(void);
void asr_soc_mac_phy_clock_enable(void);

#ifdef __cplusplus
}
#endif

#endif //__ASR_RF_H__

