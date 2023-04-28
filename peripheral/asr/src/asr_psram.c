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
#include "asr_psram.h"
#include "asr_pinmux.h"
#include "asr_port_peripheral.h"

int mode_config = -1;

void psram_wait_finish(void)
{
    while(REG_RD(PSRAM_SR) & 0x2);
    while(!(REG_RD(PSRAM_FR) & 0x1));
    REG_WR(PSRAM_FR, 1);
}

void psram_init_lut_ps_read(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_READ));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0x0<<24 | 0x00<<16 | PSRAM_CMD_READ<<10 | 0x0<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_write(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_WRITE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0x0<<24 | 0x00<<16 | PSRAM_CMD_WRITE<<10 | 0x0<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_write_evict(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_WRITE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0<<24 | 0x00<<16 | 21<<10 | 0<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_mode_register_read(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_MODE_REG_READ));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_READ<<26 | 0x0<<24 | 0x2<<16 | PSRAM_CMD_DUMMY<<10 | 0x0<<8 | (8-1)));
    REG_WR((PSRAM_LUT2+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<10 | 0x0<<8 | 0x0));
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_mode_register_write(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_MODE_REG_WRITE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0x0<<24 | 0x00<<16 | PSRAM_CMD_WRITE<<10 | 0x0<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_qmode_enable(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_CMD<<10 | 0<<8 | FLASH_CMD_ENTER_QUAD_MODE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_qmode_exit(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_CMD<<10 | 0x2<<8 | FLASH_CMD_EXIT_QUAD_MODE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_read_4x(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x2<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x2<<8 | FLASH_CMD_FAST_READ_QUAD));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_READ<<26 | 0x2<<24 | 0x20<<16 | PSRAM_CMD_DUMMY<<10 | 0x2<<8 | (6-1)));
    REG_WR((PSRAM_LUT2+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<10 | 0x2<<8 | 0x00) );
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_write_4x(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x2<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x2<<8 | FLASH_CMD_WRITE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0x2<<24 | 0x00<<16 | PSRAM_CMD_WRITE<<10 | 0x2<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_write_evict_4x(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x2<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x2<<8 | FLASH_CMD_WRITE));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_JMP_ON_CS<<26 | 0x2<<24 | 0x00<<16 | 21<<10 | 0x2<<8 | 0x20));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

void psram_init_lut_ps_readid(unsigned int seq_id)
{
    REG_WR((PSRAM_LUT0+0x10*seq_id), (PSRAM_CMD_ADDR<<26 | 0x0<<24 | 0x18<<16 | PSRAM_CMD_CMD<<10 | 0x0<<8 | FLASH_CMD_READ_ID));
    REG_WR((PSRAM_LUT1+0x10*seq_id), (PSRAM_CMD_READ<<10 | 0x0<<8 | 0x9));
    REG_WR((PSRAM_LUT2+0x10*seq_id), 0);
    REG_WR((PSRAM_LUT3+0x10*seq_id), 0);
}

int asr_psram_set_channel(asr_psram_channel channel)
{
    return port_psram_set_channel(channel);
}

int asr_psram_config(asr_psram_mode mode)
{
    if(mode != PSRAM_MODE_SPI && mode != PSRAM_MODE_QSPI)
        return -1;

    port_psram_init_clk();

    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) & (~(1<<14))); //MCR,MDIS = 0

    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) | ((3))); //MCR , AHB and SF domain reset

    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) & 0xfffffffc); //clear AHB and SF reset

    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) | 1<<14); //MCR , MDIS=1

    if(mode == PSRAM_MODE_SPI)//the simple logic has been modified. set bit7 means to use the original logic
        REG_WR(PSRAM_SMPR, 0x00000080); // sampled by sfif_clk_b; half cycle delay
    else
        REG_WR(PSRAM_SMPR, 0x00000000); // sampled by sfif_clk_b; half cycle delay

    REG_WR(PSRAM_FLSHCR, 0x00000000);  //set setup and hold time for psram

    //Give the default source address:
    REG_WR(PSRAM_SFAR, PSRAM_FLASH_A1_BASE);  //set IDATSIZ

    // lijin: 2018_01_11-11:07 total buffer size is 64*64b = 512B = 0x200B
    // config the ahb buffer
    REG_WR(PSRAM_BUF0IND, 0x00000080);
    REG_WR(PSRAM_BUF1IND, 0x00000100);
    REG_WR(PSRAM_BUF2IND, 0x00000180);

    // lijin: 2018_01_11-11:07 each buffer is 8B * 16 = 128B = 0x80B

    //kzheng: TO DO, now hmaster[3:0] tie0, so need modified here
    //mst id =0 is CPU
    //PSRAM0_BUF0CR = 0x00001001; // CPU A9
    REG_WR(PSRAM_BUF0CR, 0x00001000); // CPU A9
    REG_WR(PSRAM_BUF1CR, 0x00001006); // CPU M4
    REG_WR(PSRAM_BUF2CR, 0x00001003); // SDMA
    REG_WR(PSRAM_BUF3CR, 0x80001002); // other masters

    // config the flash mmap
    REG_WR(PSRAM_SFA1AD, PSRAM_FLASH_A1_TOP & 0xfffffc00);
    REG_WR(PSRAM_SFA2AD, PSRAM_FLASH_A2_TOP & 0xfffffc00);
    REG_WR(PSRAM_SFB1AD, PSRAM_FLASH_B1_TOP & 0xfffffc00);
    REG_WR(PSRAM_SFB2AD, PSRAM_FLASH_B2_TOP & 0xfffffc00);

    // ISD3FB, ISD2FB, ISD3FA, ISD2FA = 1; ENDIAN = 'h3; END_CFG='h3
    // DELAY_CLK4X_EN = 1
    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) | 0x000F000C);

    //ddr_en   data aligned with 2x serial flash half clock
    //REG_WR(PSRAM_FLSHCR, REG_RD(PSRAM_FLSHCR) | 0x00010000);

    //MCR SCLKCFG 0, dqs en =1
    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) & 0xfbffffff );

    //dqs_loopback_en = 1, dqs_loopback_from_pad = 1
    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) | 3<<24 );

    //ddr_en = 1, enable 2x and 4x clock
    ///REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) | 1<<7 );

    //MDIS = 0, enable psram clocks,must clear MDIS to enable clock for transfer.
    REG_WR(PSRAM_MCR, REG_RD(PSRAM_MCR) & 0xffffbfff );

    //printf("PSRAM initialize done. \n");
    //printf("begin initialize LUT tables. \n");

    if(mode == PSRAM_MODE_SPI) {
        psram_init_lut_ps_read(0);
        psram_init_lut_ps_read(PSRAM_SEQ_ID_READ);
        psram_init_lut_ps_write_evict(PSRAM_SEQ_ID_WRITE_EVICT);
        psram_init_lut_ps_write(PSRAM_SEQ_ID_WRITE);
        psram_init_lut_ps_readid(PSRAM_SEQ_ID_READ_ID);
        psram_init_lut_ps_mode_register_read(PSRAM_SEQ_ID_MODE_REG_READ);
        psram_init_lut_ps_mode_register_write(PSRAM_SEQ_ID_MODE_REG_WRITE);
    }
    else {
        psram_init_lut_ps_read_4x(0);
        psram_init_lut_ps_read_4x(PSRAM_SEQ_ID_READ);
        psram_init_lut_ps_write_evict_4x(PSRAM_SEQ_ID_WRITE_EVICT);
        psram_init_lut_ps_write_4x(PSRAM_SEQ_ID_WRITE);
    }
    psram_init_lut_ps_qmode_enable(PSRAM_SEQ_ID_QUAD_ENABLE);
    psram_init_lut_ps_qmode_exit(PSRAM_SEQ_ID_QUAD_EXIT);

    //printf(" initialize LUT tables done. \n");

    // set read miss cmd, evict is in next lut
    REG_WR(PSRAM_BFGENCR, PSRAM_SEQ_ID_READ<<12);

    if(mode_config != mode) {
        if(mode == PSRAM_MODE_SPI) {
            REG_WR(PSRAM_SFAR, PSRAM_AMBA_BASE);
            REG_WR(PSRAM_IPCR, PSRAM_SEQ_ID_QUAD_EXIT<<24);
            psram_wait_finish();
        }
        else {
            REG_WR(PSRAM_SFAR, PSRAM_AMBA_BASE);
            REG_WR(PSRAM_IPCR, PSRAM_SEQ_ID_QUAD_ENABLE<<24);
            psram_wait_finish();
        }
    }
    mode_config = mode;

    return 0;
}
