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
#include "asr_common.h"
//#include "reg_access.h"
#include "asr_rf_spi.h"
#include "asr_alto_apll_rf.h"

#ifdef I2S_DEMO

#define BIT(n) 1<<(n)

/// Address of the MONOTONIC_COUNTER_2_LO register
#define NXMAC_MONOTONIC_COUNTER_2_LO_ADDR   0x60B00120
uint32_t nxmac_monotonic_counter_2_lo_get(void)
{
    return REG_RD(NXMAC_MONOTONIC_COUNTER_2_LO_ADDR);
}

void asr_disable_rf_unlock_nvic_irq(void)
{
    ECLIC_DisableIRQ(D_APLL_UNLOCK_IRQn);
    ECLIC_DisableIRQ(D_SX_UNLOCK_IRQn);
}

void mdm_rcclkforce_setf(uint8_t rcclkforce)
{
//    ASSERT_ERR((((uint32_t)rcclkforce << 27) & ~((uint32_t)0x08000000)) == 0);
    REG_WR(MDM_CLKGATEFCTRL0_ADDR, (REG_RD(MDM_CLKGATEFCTRL0_ADDR) & ~((uint32_t)0x08000000)) | ((uint32_t)rcclkforce << 27));
}

uint8_t check_wf_val(uint8_t type)
{
    uint16_t val = 0;
    uint8_t ret = 1;
    switch(type){
        case NULL_BIT:
            ret = 1;
            break;
        case D_RCM_EOC:
            val = rf_get_reg_bit(0xA6,9,1);
            if(val == 1)
                ret = 0;
            break;
        case D_PVM_EOC:
            val = rf_get_reg_bit(0xA8,6,1);
            if(val == 1)
                ret = 0;
            break;
        case APLL_FCAL_DONE:
            val = rf_get_reg_bit(0x6E,15,1);
            if(val == 1)
                ret = 0;
            break;
        case DBL_CAL_DONE:
            val = rf_get_reg_bit(0x3F,2,1);
            if(val == 1)
                ret = 0;
            break;
        case SX_LO_MIX_CAL_DONE:
            val = rf_get_reg_bit(0x4D,0,1);
            if(val == 1)
                ret = 0;
            break;
        case DCOC_BIT0:
            val = rf_get_reg_bit(0xAC,0,1);
            if(val == 0)
                ret = 0;
            break;
        case DCOC_BIT1:
            val = rf_get_reg_bit(0xAC,1,1);
            if(val == 0)
                ret = 0;
            break;
        case DCOC_BIT2:
            val = rf_get_reg_bit(0xAC,2,1);
            if(val == 0)
                ret = 0;
            break;
        case DCOC_BIT3:
            val = rf_get_reg_bit(0xAC,3,1);
            if(val == 0)
                ret = 0;
            break;
        case DCOC_BIT4:
            val = rf_get_reg_bit(0xAC,4,1);
            if(val == 0)
                ret = 0;
            break;
        case DCOC_BIT5:
            val = rf_get_reg_bit(0xAC,5,1);
            if(val == 0)
                ret = 0;
            break;
        case SX_VCO_FULL_CAL_DONE:
            val = rf_get_reg_bit(0x43,6,1);
            if(val == 1)
                ret = 0;
            break;
        case RCO_CAL_EN:
            val = rf_get_reg_bit(0xC4,5,1);
            if(val == 0)
                ret = 0;
            break;
        case TXPWR_CAL_AUXADC_DOWN:
            val = rf_get_reg_bit(0xA4,3,1);
            if(val == 1)
                ret = 0;
            break;
        default:
            break;
    }
    return ret;
}

void check_timeout_nop(uint32_t nus,uint32_t type)
{
    uint16_t ret = 0;

    while(nus--)
    {
        __asm("nop");
        if(ret == check_wf_val(type))
            return;
    }
}

void asr_rfinit0_reset_default_pdval(void)
{
    asr_disable_rf_unlock_nvic_irq();

    /* Enable RC clock */
    mdm_rcclkforce_setf(1);//can't remove in mini
    //change to SPI control
    rf_set_reg_bit(0x02,11,1,0x1);

    spi_mst_write(0x03, 0xFFFF);
    spi_mst_write(0x04, 0xFFF8);
    spi_mst_write(0x0D, 0xFFFF);
    spi_mst_write(0x0E, 0xE000);
    spi_mst_write(0x0F, 0xF800);
    spi_mst_write(0x06, 0xFFFF);
    //close D_TRXTOP_RSV_2 LDO
    rf_set_reg_bit(0x23,0,1,0x0);
    spi_mst_write(0x07, 0xFFFF);
    spi_mst_write(0x08, 0xFFFF);
    spi_mst_write(0x09, 0xFE00);
}

void asr_rfinit1_bg_pu()
{
    //D_PD_BG= 0
    rf_set_reg_bit(0x06,13,1,0x0);
    //Disable CLKTXDAC
    rf_set_reg_bit(0x08,8,1,0x0);
    //disable bb test
    rf_set_reg_bit(0x08,2,2,0x3);
}

 void asr_rfinit2_xo_pu()
{
    //PD_XO_LDO= 0, PD_XO_VREF= 0; XO_LDO_PU,LDO VREF PU
    spi_mst_write(0x0F,0x9800);
    delay(20);
    //PDD_XO_VREF= 0; XO_LDO_PU, (PDD, DELAYED)
    spi_mst_write(0x0F,0x8800);

    REG_WR(0x40000A74, 0x7171);
}

 void asr_rfinit3_rcm_pvm()
{
    uint16_t rcm_out;
    uint16_t rcm_com;
    uint32_t pmu_ctrl;

    //Enable D_AON_PD_TRSW
    REG_WR(0x40000A68, REG_RD(0x40000A68)& (~ (BIT(6))));
    //PA gain.0x120: DA=4,PA=4
    REG_WR(0x40000A70, 0x520);

    //Enable PA DA,because it's default val error
    pmu_ctrl = REG_RD(SYS_REG_BASE_PMU_CTRL);
    REG_WR(SYS_REG_BASE_PMU_CTRL, (pmu_ctrl & 0xFFFFFBFF));//bit10 clear

    //Enable TRX TOP Bias  and TRX LDOs
    //PU: ADDA_DLDO, DAC_ALDO, PA_LDO
    spi_mst_write(0x06,0x57DF);
    //Enable RCM and TRX LDOs
    //PU: RBB_LDO, RXADC_ALDO, RXADC_VBIAS, RXFE_LDO,TXTOP_LDO, TXRTOP_BIAS, RCM
    spi_mst_write(0x07,0xBD13);

    //Enable UPC LDO
    //PU: UPC_LDO
    spi_mst_write(0x08,0xEEFF);

    //TRXTOP LDO output voltage
    //TRXTOP_LDO_BM = 7, 1.55V to 1.6V
    rf_set_reg_bit(0x1E,3,3,0x7);

    //for ble rx opt
    rf_set_reg_bit(0x1E,13,3,0x7);
    //open XO RCM 26M clock
    //D_XO_CLK_RCM26M_EN =1
    spi_mst_write(0x75,0xA9C0);

    //ALDO tune to 1.2V
    rf_set_reg_bit(0x23,1,3,0x5);
    rf_set_reg_bit(0x23,8,3,0x4);
    //ble pwdt vref
    spi_mst_write(0x22,0x1964);

    //trigger D_TRXTOP_RSV_2
    rf_set_reg_bit(0x23,0,1,0x1);
    //div freq 13MHz to 26MHz
    rf_set_reg_bit(0x22,15,1,0x1);

    //set fix cap val
    rf_set_reg_bit(0x87,11,1,0x1);
    //RCM Start cal
    //D_RCM_START= 1
    spi_mst_write(0xA5,0x8800);
    check_timeout_nop(50,D_RCM_EOC);

    //Read RCM result
    rcm_out = rf_get_reg_bit(0xA6,10,6);

    if(rcm_out == 0)
    {
        //RCM Stop cal
        //D_RCM_START= 0
        spi_mst_write(0xA5,0x8000);

        check_timeout_nop(2,NULL_BIT);

        rf_set_reg_bit(0x87,10,2,0x1);
        //RCM Start cal
        //D_RCM_START= 1
        spi_mst_write(0xA5,0x8800);
        check_timeout_nop(50,D_RCM_EOC);
        //Read RCM [15:13]
        rcm_out = rf_get_reg_bit(0xA6,10,6);
    }

    //write RCM to Cbank reg
    rf_set_reg_bit(0x83,8,6,rcm_out);
    //write RCM to TIA_CC reg
    rf_set_reg_bit(0x85,3,6,rcm_out);

    //RCM Stop cal
    //D_RCM_START= 0
    spi_mst_write(0xA5,0x8000);

    //Reading RCM should be before RCM cali stopped.
    //Here we can use RCM stored in SW.
    rcm_com = rcm_out >> 3;
    //Write RCM [15:13] to D_RBB_RUN_BQOP1CC
    rf_set_reg_bit(0x82,13,3,rcm_com);
    //Write RCM [15:13] to D_RBB_RUN_BQOP2CC
    rf_set_reg_bit(0x82,7,3,rcm_com);
    //Write RCM [15:13] to D_RBB_RUN_PGAOPCC
    rf_set_reg_bit(0x84,13,3,rcm_com);

    //PVM Start cal
    //D_PVM_START= 1
    spi_mst_write(0xA7,0xC000);
    check_timeout_nop(50,D_PVM_EOC);

    //PVM Stop cal
    //D_PVM_START= 0
    spi_mst_write(0xA7,0x8000);
    //Close RCM
    //PD_RCM= 1
    spi_mst_write(0x07,0xBF13);
    //Close RCM_XO 26M
    spi_mst_write(0x75,0x29C0);

    //Config DAC ALDO output voltage
    rf_set_reg_bit(0x1D,13,3,0x3);
    rf_set_reg_bit(0x1D,10,3,0x3);

    //Config ADC LDO output voltage
    spi_mst_write(0x1C,0x7973);
    //Config ADC param
    spi_mst_write(0x88,0x8DE0);

    //LDO cfg,for a0v0,a0v1,a0v2.

    //LC tuning capacitance modify
    rf_set_reg_bit(0x77,12,4,0x3);

    //Config LODT LDO voltage
    rf_set_reg_bit(0x31,5,4,0xF);
    //Config TX Chain param
    //spi_mst_write(0x8A,0x4C99);
    //spi_mst_write(0x92,0x8620);


    spi_mst_write(0x8A,0x4C77);//5C77->4C77
    spi_mst_write(0x92,0xC620);//7600->c620

    spi_mst_write(0x8B,0x63e4);//0x6224
    spi_mst_write(0x8C,0x630);//EVM ori 630->530->630

    spi_mst_write(0x8D,0xFC0);
    spi_mst_write(0x8F,0x9D5C);//9d50->9d58->9d5c upc gain 6->7
    spi_mst_write(0x90,0xEF24);//0x8F48->cf48->cf24 upc gain 6->7

    spi_mst_write(0x91,0x8428);

    //spi_mst_write(0x92,0x8620);
    spi_mst_write(0x93,0x8200);

    //D_TXDFE_SEL_IN: 0 from DIN; 1 from sram; 2/3 from TDIN
    rf_set_reg_bit(0xA1,11,2,0x1);
    //D_TXDFE_SEL_OUT: 0 from TXDFE filter output; 1 from RAMP GEN; 2/3 from reg_dac_in
    rf_set_reg_bit(0xA1,9,2,0x0);
    //D_TXDFE_DC_IQ_KG
    rf_set_reg_bit(0x9B,6,10,0x100);
    //D_TXDFE_DC_IQ_KP
    rf_set_reg_bit(0x9C,6,10,0x0);

    //close PA,DA in LO FSM
    spi_mst_write(0x10,0xF5BF);
    //FEFC->FEFE.FSM_LO_PD_RSV_9,enable tx dfe,
    spi_mst_write(0x11,0xFEFE);
    //open TX LO in RX FSM
     spi_mst_write(0x12,0x2180);
    //close RF BB test.//0x7A44 for bb test.
    spi_mst_write(0x13,0x784C);
    spi_mst_write(0x14,0x94BF);
    spi_mst_write(0x15,0x87FE);
}

void asr_rfinit4_tmmt()
{
    //Enable AUXADC
    rf_set_reg_bit(0x06,14,1,0x0);
    //Enable TMMT
    rf_set_reg_bit(0x07,4,1,0x0);

    //Enable CLK AUXADC13M; D_XO_CLK_AUXADC13M_EN= 1
    rf_set_reg_bit(0x75,14,1,0x1);
    //Enable XO CLK AUCADC, DFF's RB;D_RST_XO_CLK_AUXADC= 0
    rf_set_reg_bit(0x0F,11,1,0);

    delay(500);

    //Open channel 1
    REG_WR(SYS_REG_BASE_XOCTRL2, ((REG_RD(SYS_REG_BASE_XOCTRL2)&(~(uint32_t)0x7))|(uint32_t)1) &(~(uint32_t)(BIT(9))));

    delay(500);

    //Open channel 0
    REG_WR(SYS_REG_BASE_XOCTRL2, ((REG_RD(SYS_REG_BASE_XOCTRL2)&(~(uint32_t)0x7))|(uint32_t)0) &(~(uint32_t)(BIT(9))));

    delay(500);

    //Close AUXADC
    rf_set_reg_bit(0x06,14,1,0x1);
    //Close TMMT
    rf_set_reg_bit(0x07,4,1,0x1);
    //Close XO CLK AUCADC; D_RST_XO_CLK_AUXADC= 1
    rf_set_reg_bit(0x0F,11,1,0x1);
    //Close CLK AUXADC13M; D_XO_CLK_AUXADC13M_EN= 0
    rf_set_reg_bit(0x75,14,1,0x0);
}

void asr_rfinit5_apll_pu(void)
{
    ECLIC_DisableIRQ(D_APLL_UNLOCK_IRQn);

    //Release apll fcal PD
    spi_mst_write(0x0D,0x047F);
    delay(20);
    spi_mst_write(0x0E,0x4000);

    //Up APLL_VCO_FCAL_EN
    spi_mst_write(0x6E,0x110D);

    check_timeout_nop(300,APLL_FCAL_DONE);
    //Release APLL_PD, except FCAL_PD
    spi_mst_write(0x0D,0x027F);
    delay(20);
    //Release APLL_PDD and RST
    spi_mst_write(0x0D,0x0200);
    delay(20);
    ECLIC_ClearPendingIRQ(D_APLL_UNLOCK_IRQn);
    ECLIC_EnableIRQ(D_APLL_UNLOCK_IRQn);
}

void asr_soc_wifi_clk_enable(void)
{
    //enable all wifi/ble block clock
    REG_WR(0x40000840, 0x1FF<<15);
    delay(50);
}

void asr_soc_wifi_clk_disable(void)
{
    //Disable WiFi core clock
    REG_WR(0x40000848, 0x1FF<<15);
    delay(50);
}

void asr_soc_mac_phy_clock_enable(void)
{
    asr_rfinit0_reset_default_pdval();

    asr_rfinit1_bg_pu();

    asr_rfinit2_xo_pu();

    asr_rfinit3_rcm_pvm();

    asr_rfinit4_tmmt();

    asr_rfinit5_apll_pu();


    //clk_core_sel     0x40000804[0:1]=00: XTAL 52/80MHz clock
    //switch BLE modem clock source and WIFI RW clock source to 480M
    REG_WR(0x40000804, (REG_RD(0x40000804) | (1<<9)));

    //configure WIFI RW clock to 80/40M
    REG_WR(0x40000804, (REG_RD(0x40000804) & (~3)) | 2); //set bit[1:0] to 2'b10
}
#endif // I2S_DEMO
