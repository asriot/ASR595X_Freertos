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
#include "asr_dma.h"
#include "asr_port_peripheral.h"
#include "asr_uart.h"
#include "asr_spi.h"
asr_dma_callback_struct   g_asr_dma_callback_struct[DMA_MAX_CHAN_NUM]={0};
Chan_Cfg_TypeDef  *pChan_Cfg_Align;

Chan_Cfg_TypeDef * asr_dma_ctrl_block_init(void)
{
    return (Chan_Cfg_TypeDef *)(DMA_BUFFER_REG);
}

void asr_dma_init(void)
{
    port_dma_init();
}

void asr_dma_finalize(void)
{
    port_dma_finalize();
}


void asr_dma_interrupt_clear(uint32_t chan_idx)
{
    DMA_INT_CLR |= (1<<chan_idx); // write 1 to clear, then reset to 0
    DMA_INT_CLR &= ~(1<<chan_idx);
}

ITstatus asr_dma_get_interrupt_status(uint32_t chan_idx)
{
    if( DMA_INT_STAT & (1<<chan_idx) )
        return SET;
    else
        return RESET;
}

void asr_dma_interrupt_config(uint32_t chan_idx, uint8_t new_state)
{
    if( new_state == ENABLE )
        DMA_INT_MASK |= (1 << chan_idx); // write 1 to unmask
    else
        DMA_INT_MASK &= ~(1 << chan_idx);
}

void asr_dma_channel_cmd(uint32_t chan_idx, uint8_t new_state)
{
    if( new_state == ENABLE )
        DMA->CHAN_EN_SET |= (1 << chan_idx);
    else
        DMA->CHAN_EN_CLR |=  (1 << chan_idx);
}

void asr_dma_alt_channel_cmd(uint32_t chan_idx, uint8_t new_state)
{
    if( new_state == ENABLE )
        DMA->CHAN_PRI_ALT_SET |= (1<<chan_idx);
    else
        DMA->CHAN_PRI_ALT_CLR |= (1<<chan_idx);
}


void asr_dma_generate_sw_req(uint32_t chan_idx)
{
    DMA->CHAN_SW_REQ |= (1<<chan_idx);
}

void asr_dma_generate_perip_req(uint32_t chan_idx)
{
    DMA_WAIT_ON_REQ |=(1<<chan_idx);
}

uint32_t asr_dma_get_src_addr(uint8_t channel)
{
    return (pChan_Cfg_Align + channel)->chan_src_end_ptr;
}

ITstatus asr_dma_get_alt_state(uint8_t channel)
{
    if (DMA->CHAN_PRI_ALT_SET & (1 << channel))
        return RESET;
    else
        return SET;
}


#if 0
static void asr_dma_cfg(DMA_TypeDef *DMAx, DMA_Init_Struct_Type *dma_init_struct)
{
    //uint8_t used_chan;
    //port_dma_cfg();
    pChan_Cfg_Align = asr_dma_ctrl_block_init();//
    //used_chan = dma_init_struct->chan_num;
    //Mem_nomal_used_chan=asr_dma_cfg->dma_init_struct.chan_num ;
    int8_t chan = dma_init_struct->chan_num;
    (pChan_Cfg_Align + chan)->chan_ctr.cycle_ctl     = dma_init_struct->cycle;
    (pChan_Cfg_Align + chan)->chan_ctr.n_minus_1     = dma_init_struct->n_minus1;
    (pChan_Cfg_Align + chan)->chan_ctr.R_pow         = dma_init_struct->R_power;
    (pChan_Cfg_Align + chan)->chan_ctr.src_size      = dma_init_struct->SrcDataWidth;
    (pChan_Cfg_Align + chan)->chan_ctr.dst_size      = dma_init_struct->DstDataWidth;
    (pChan_Cfg_Align + chan)->chan_ctr.src_inc       = dma_init_struct->SrcAddrInc;
    (pChan_Cfg_Align + chan)->chan_ctr.dst_inc       = dma_init_struct->DstAddrInc;
    (pChan_Cfg_Align + chan)->chan_ctr.next_useburst = dma_init_struct->next_useburst;
    (pChan_Cfg_Align + chan)->chan_dst_end_ptr       = (uint32_t) dma_init_struct->DstEndPtr;
    (pChan_Cfg_Align + chan)->chan_src_end_ptr       = (uint32_t) dma_init_struct->SrcEndPtr;
    DMAx->CHAN_PRI_ALT_CLR |= dma_init_struct->use_pri_only << dma_init_struct->chan_num;
    DMAx->CTL_BASE_PTR                              = (uint32_t) pChan_Cfg_Align;
    if (chan >= DMA_MAX_CHAN_NUM)
        chan -= DMA_MAX_CHAN_NUM;
    if (dma_init_struct->use_pri_only)
        DMA->CHAN_PRI_ALT_CLR |= (1 << chan); // use primary data structure
    else
    {
        DMA->CHAN_PRI_ALT_SET |= (1 << chan); // use alternate data structure

    }
    asr_dma_interrupt_config(chan, dma_init_struct->interrupt_en);
    if(dma_init_struct->interrupt_en)
    NVIC_EnableIRQ(DMA_IRQn);

    DMAx->CFG |= 0x1; // dma enable
}

static void asr_mtm_dma_init(asr_dma_dev_t *asr_dma_para_struct)
{
    DMA_Init_Struct_Type asr_dma_cfg_struct;
    uint8_t              temp_chan_alt = 0;
    uint8_t              dma_channel, mode;
    uint16_t             lenth;
    uint32_t             addr;
    uint8_t              datwidth;
    mode        = asr_dma_para_struct->asr_dma_struct_init.dma_mode;
    datwidth    = asr_dma_para_struct->asr_dma_struct_init.type_dat;
    lenth       = asr_dma_para_struct->asr_dma_struct_init.lenth;
    dma_channel = asr_dma_para_struct->asr_dma_struct_init.used_chan;
    addr = (uint32_t) asr_dma_para_struct->asr_dma_struct_init.pbuf;

    if (((mode != DMA_OP_MODE_AUTO_REQ) && (mode != DMA_OP_MODE_PING_PONG)) || (dma_channel > 63))
        return;
    if ((dma_channel >= DMA_MAX_CHAN_NUM) && (dma_channel < DMA_MAX_CHAN_NUM*2))  //ALT CHAN
    {
        temp_chan_alt = dma_channel;
        dma_channel = temp_chan_alt - DMA_MAX_CHAN_NUM;
        //asr_dma_cfg_struct.use_pri_only=DISABLE;
    }
    else
    {
        temp_chan_alt = dma_channel;
        asr_dma_cfg_struct.use_pri_only = ENABLE;
    }

    asr_dma_cfg_struct.chan_num      = temp_chan_alt;
    asr_dma_cfg_struct.cycle         = asr_dma_para_struct->asr_dma_struct_init.dma_mode;
    asr_dma_cfg_struct.SrcEndPtr     = (uint32_t) addr;
    asr_dma_cfg_struct.DstEndPtr     = (uint32_t) asr_dma_para_struct->asr_dma_struct_init.pbuf_bak;
    switch (datwidth)
    {
        case INT8_U :
            asr_dma_cfg_struct.DstAddrInc   = DMA_DST_ADDR_INC_BYTE;
            asr_dma_cfg_struct.DstDataWidth = DMA_DST_DATA_WIDTH_BYTE;
            asr_dma_cfg_struct.SrcAddrInc   = DMA_SRC_DATA_WIDTH_BYTE;
            asr_dma_cfg_struct.SrcDataWidth = DMA_SRC_ADDR_INC_BYTE;
            break;
        case INT16_U :
            asr_dma_cfg_struct.DstAddrInc   = DMA_DST_ADDR_INC_HALFWORD;
            asr_dma_cfg_struct.DstDataWidth = DMA_DST_DATA_WIDTH_HALFWORD;
            asr_dma_cfg_struct.SrcAddrInc   = DMA_SRC_DATA_WIDTH_HALFWORD;
            asr_dma_cfg_struct.SrcDataWidth = DMA_SRC_ADDR_INC_HALFWORD;
            break;
        case INT32_U :
            asr_dma_cfg_struct.DstAddrInc   = DMA_DST_ADDR_INC_WORD;
            asr_dma_cfg_struct.DstDataWidth = DMA_DST_DATA_WIDTH_WORD;
            asr_dma_cfg_struct.SrcAddrInc   = DMA_SRC_DATA_WIDTH_WORD;
            asr_dma_cfg_struct.SrcDataWidth = DMA_SRC_ADDR_INC_WORD;
            break;
        default:
            asr_dma_cfg_struct.DstAddrInc   = DMA_DST_ADDR_INC_BYTE;
            asr_dma_cfg_struct.DstDataWidth = DMA_DST_DATA_WIDTH_BYTE;
            asr_dma_cfg_struct.SrcAddrInc   = DMA_SRC_DATA_WIDTH_BYTE;
            asr_dma_cfg_struct.SrcDataWidth = DMA_SRC_ADDR_INC_BYTE;
            break;
    }
    asr_dma_cfg_struct.DstEndPtr     =(uint32_t) asr_dma_para_struct->asr_dma_struct_init.pbuf_bak + (lenth -1) * datwidth;
    asr_dma_cfg_struct.SrcEndPtr     = (uint32_t) addr + (lenth - 1) * datwidth;
    asr_dma_cfg_struct.n_minus1      = lenth - 1;
    asr_dma_cfg_struct.R_power       = asr_dma_para_struct->asr_dma_struct_init.r_power;
    asr_dma_cfg_struct.next_useburst = 0; //Constant-value
    if(asr_dma_para_struct->priv!=NULL)
    {
        asr_dma_cfg_struct.interrupt_en  = ENABLE;
    }
    else
    {
        asr_dma_cfg_struct.interrupt_en  = DISABLE;
    }
    if (temp_chan_alt < DMA_MAX_CHAN_NUM)
    {
        g_asr_dma_callback_struct[dma_channel].cb  = (asr_dma_callback_func) asr_dma_para_struct->priv;// handlerdummy
        g_asr_dma_callback_struct[dma_channel].arg = asr_dma_para_struct->asr_dma_struct_init.handle_arg;
    }
    //asr_Mem_Dma_Init(&asr_dma_init_struct);
    asr_dma_init(DMA,&asr_dma_cfg_struct);//
}

static int32_t asr_mtp_dma_init(asr_dma_dev_t *asr_dma_init_struct)
{
    DMA_Init_Struct_Type asr_dma_cfg_struct;
    uint8_t              temp_chan_alt = 0;
    uint8_t              dma_channel;
    uint16_t             lenth;
//    void *addr;
    lenth                               = asr_dma_init_struct->asr_dma_struct_init.lenth;
    dma_channel                         = asr_dma_init_struct->asr_dma_struct_init.used_chan;
//    addr                                = asr_dma_init_struct->asr_dma_struct_init.pbuf;
    if ((asr_dma_init_struct->asr_dma_struct_init.dma_mode > DMA_OP_MODE_PING_PONG)  //for basic    pingpong
        || (dma_channel >= DMA_MAX_CHAN_NUM*2)) //channel is invalid
        return -1;
    if ((dma_channel >= DMA_MAX_CHAN_NUM) && (dma_channel < DMA_MAX_CHAN_NUM*2))  //ALT CHAN
    {
        temp_chan_alt = dma_channel;
        dma_channel = temp_chan_alt - DMA_MAX_CHAN_NUM;
        //asr_dma_cfg_struct.use_pri_only=DISABLE;
    }
    else
    {
        temp_chan_alt = dma_channel;
        asr_dma_cfg_struct.use_pri_only = ENABLE;
    }
    port_dma_peri_init(asr_dma_init_struct,&asr_dma_cfg_struct);
//    asr_dma_cfg_struct.chan_num      = dma_channel;
    asr_dma_cfg_struct.cycle         = asr_dma_init_struct->asr_dma_struct_init.dma_mode;
    asr_dma_cfg_struct.interrupt_en  = ENABLE;
    asr_dma_cfg_struct.next_useburst = 0;
    asr_dma_cfg_struct.n_minus1      = lenth - 1;
    asr_dma_cfg_struct.R_power       = asr_dma_init_struct->asr_dma_struct_init.r_power;
    asr_dma_cfg_struct.use_pri_only  = ENABLE;
    asr_dma_cfg_struct.chan_num = asr_dma_init_struct->asr_dma_struct_init.used_chan;
    if (temp_chan_alt < DMA_MAX_CHAN_NUM)
    {
        g_asr_dma_callback_struct[dma_channel].cb  = (asr_dma_callback_func) asr_dma_init_struct->priv;// handlerdummy
        g_asr_dma_callback_struct[dma_channel].arg = asr_dma_init_struct->asr_dma_struct_init.handle_arg;
    }
    asr_dma_init(DMA,&asr_dma_cfg_struct);// dma init
    return 0;
}

void asr_mtm_scagat_dma_init(asr_dma_dev_t *asr_dma_para_struct)
{
    DMA_Init_Struct_Type asr_dma_cfg_struct;
    uint8_t              dma_channel;
    uint16_t             cycles;
    Chan_Cfg_TypeDef     *pChan_Cfg_Align_Alt;
    pChan_Cfg_Align = asr_dma_ctrl_block_init();//
    cycles      = asr_dma_para_struct->asr_dma_struct_init.lenth;
    dma_channel = asr_dma_para_struct->asr_dma_struct_init.used_chan;
    pChan_Cfg_Align_Alt = (Chan_Cfg_TypeDef *) asr_dma_para_struct->asr_dma_struct_init.pbuf;
    asr_dma_cfg_struct.SrcEndPtr     = (uint32_t) &(pChan_Cfg_Align_Alt[cycles - 1].resv);
    asr_dma_cfg_struct.DstEndPtr     = (uint32_t) &(pChan_Cfg_Align[dma_channel + DMA_MAX_CHAN_NUM].resv);
    asr_dma_cfg_struct.DstAddrInc    = DMA_DST_ADDR_INC_WORD;  //Constant-value
    asr_dma_cfg_struct.DstDataWidth  = DMA_DST_DATA_WIDTH_WORD;//Constant-value
    asr_dma_cfg_struct.SrcAddrInc    = DMA_SRC_ADDR_INC_WORD;//Constant-value
    asr_dma_cfg_struct.SrcDataWidth  = DMA_SRC_DATA_WIDTH_WORD;//Constant-value
    asr_dma_cfg_struct.cycle         = DMA_OP_MODE_MEM_SCT_GAT_PRI;
    asr_dma_cfg_struct.R_power       = DMA_R_POWER_4; //Constant-value
    asr_dma_cfg_struct.n_minus1      = 4 * cycles - 1;
    //4*n-1 primary transfer  cycle n=sizeof(pChan_Cfg_Align_Alt)/sizeof(uint32_t)
    asr_dma_cfg_struct.use_pri_only  = ENABLE;//Constant-value
    asr_dma_cfg_struct.interrupt_en  = ENABLE;//Constant-value
    asr_dma_cfg_struct.next_useburst = 0; //Constant-value
    asr_dma_cfg_struct.chan_num = dma_channel;
    g_asr_dma_callback_struct[dma_channel].cb  = (asr_dma_callback_func) asr_dma_para_struct->priv;// handlerdummy
    g_asr_dma_callback_struct[dma_channel].arg = asr_dma_para_struct->asr_dma_struct_init.handle_arg;
    asr_dma_init(DMA,&asr_dma_cfg_struct);//primary dma cfg
}

void asr_mem_perip_dma_init(asr_dma_dev_t *asr_dma_para_struct)
{
    {
        if (((asr_dma_para_struct + PRI_CHAN)->asr_dma_struct_init.dma_mode == DMA_OP_MODE_BASIC) ||
            ((asr_dma_para_struct + PRI_CHAN)->asr_dma_struct_init.dma_mode == DMA_OP_MODE_AUTO_REQ))
        {
            asr_mtp_dma_init(asr_dma_para_struct + PRI_CHAN);
        }
        else if ((asr_dma_para_struct + PRI_CHAN)->asr_dma_struct_init.dma_mode == DMA_OP_MODE_PING_PONG)
        {
            asr_mtp_dma_init(asr_dma_para_struct + PRI_CHAN);
            asr_mtp_dma_init(asr_dma_para_struct + ALT_CHAN);
        }
    }
}

void asr_mem_mem_dma_init(asr_dma_dev_t *asr_dma_para_struct)
{
    if ((asr_dma_para_struct + PRI_CHAN)->asr_dma_struct_init.dma_mode == DMA_OP_MODE_AUTO_REQ)
    {
        asr_mtm_dma_init(asr_dma_para_struct + PRI_CHAN);
    }
    else if ((asr_dma_para_struct + PRI_CHAN)->asr_dma_struct_init.dma_mode == DMA_OP_MODE_PING_PONG)
    {
        asr_mtm_dma_init(asr_dma_para_struct + PRI_CHAN);
        asr_mtm_dma_init(asr_dma_para_struct + ALT_CHAN);
    }
}

void asr_dma_tx_reload(uint8_t channel, uint8_t dma_mode, void *buf, uint16_t lenth)
{
    uint8_t data_size, data_width;
    data_size = (pChan_Cfg_Align + channel)->chan_ctr.src_size;
    switch (data_size)
    {
        case DMA_SRC_DATA_WIDTH_BYTE :
            data_width = 1;
            break;
        case DMA_SRC_DATA_WIDTH_HALFWORD:
            data_width = 2;
            break;
        case DMA_SRC_DATA_WIDTH_WORD:
            data_width = 4;
            break;
        default:
            data_width = 1;
            break;
    }
    (pChan_Cfg_Align + channel)->chan_src_end_ptr   = (uint32_t) buf + (lenth - 1) * data_width;
    (pChan_Cfg_Align + channel)->chan_ctr.cycle_ctl = dma_mode;
    (pChan_Cfg_Align + channel)->chan_ctr.n_minus_1 = lenth - 1;
}

void asr_dma_rx_reload(uint8_t channel, uint8_t dma_mode, void *buf, uint16_t lenth)
{

    uint8_t data_size, data_width;
    data_size = (pChan_Cfg_Align + channel)->chan_ctr.dst_size;
    switch (data_size)
    {
        case DMA_DST_DATA_WIDTH_BYTE :
            data_width = 1;
            break;
        case DMA_DST_DATA_WIDTH_HALFWORD:
            data_width = 2;
            break;
        case DMA_DST_ADDR_INC_WORD:
            data_width = 4;
            break;
        default:
            data_width = 1;
            break;
    }
    (pChan_Cfg_Align + channel)->chan_dst_end_ptr   = (uint32_t) buf + (lenth - 1) * data_width;
    (pChan_Cfg_Align + channel)->chan_ctr.cycle_ctl = dma_mode;
    (pChan_Cfg_Align + channel)->chan_ctr.n_minus_1 = lenth - 1;
}
#endif

void asr_dma_mem2mem(uint8_t chan_num,uint8_t *mem_src,uint8_t *mem_dst,uint16_t len)
{
    uint8_t dma_chan = chan_num;
    Chan_Cfg_TypeDef * pChan_Cfg_Align = asr_dma_ctrl_block_init();
    Chan_Ctl_Data_TypeDef ch_ctl_data;
    Chan_Cfg_TypeDef ch_cfg;

    ch_ctl_data.cycle_ctl = DMA_OP_MODE_AUTO_REQ;
    ch_ctl_data.n_minus_1 = len-1;
    ch_ctl_data.R_pow= 1;
    ch_ctl_data.src_inc = DMA_SRC_ADDR_INC_BYTE;
    ch_ctl_data.dst_inc = DMA_DST_ADDR_INC_BYTE;
    ch_ctl_data.src_size= DMA_SRC_DATA_WIDTH_BYTE;
    ch_ctl_data.dst_size= DMA_DST_DATA_WIDTH_BYTE;

    ch_cfg.chan_ctr = ch_ctl_data;
    ch_cfg.chan_src_end_ptr = (uint32_t)&mem_src[len-1];
    ch_cfg.chan_dst_end_ptr = (uint32_t)&mem_dst[len-1];

    (pChan_Cfg_Align + dma_chan)->chan_ctr = ch_cfg.chan_ctr;
    (pChan_Cfg_Align + dma_chan)->chan_src_end_ptr = ch_cfg.chan_src_end_ptr;
    (pChan_Cfg_Align + dma_chan)->chan_dst_end_ptr = ch_cfg.chan_dst_end_ptr;
    DMA->CFG |= 0x1; // dma enable
    DMA->CHAN_PRI_ALT_CLR |= (1<<dma_chan);
    DMA->CTL_BASE_PTR = (uint32_t)pChan_Cfg_Align;
//    DMA->CHAN_EN_CLR |= ~(1<<dma_chan); // disable other channels
    DMA->CHAN_EN_SET |= (1<<dma_chan); // enbale channel 0

    DMA_INT_MASK |= (1<<dma_chan); // dma interrupt unmask, write 1
    //manually generate software request for channel 0 for mem2mem transfer
    DMA->CHAN_SW_REQ |= (1<<dma_chan);
}

void asr_dma_uart_rx(uint8_t uart_idx,uint8_t *data,uint16_t len)
{
    uint8_t dma_chan = 0;
    UART_TypeDef *UARTx = uart_get_uartx_via_idx(uart_idx);
    if(uart_idx>=UART_NUM)
        return;
    dma_chan = port_dma_get_uart_ch(uart_idx,0);

    Chan_Cfg_TypeDef * pChan_Cfg_Align = asr_dma_ctrl_block_init();
    Chan_Ctl_Data_TypeDef ch_ctl_data;
    Chan_Cfg_TypeDef ch_cfg;

    ch_ctl_data.cycle_ctl = DMA_OP_MODE_BASIC;
    ch_ctl_data.n_minus_1 = len -1;
    ch_ctl_data.R_pow= 1;
    ch_ctl_data.src_inc = DMA_SRC_ADDR_INC_FIX;
    ch_ctl_data.dst_inc = DMA_DST_ADDR_INC_BYTE;
    ch_ctl_data.src_size= DMA_SRC_DATA_WIDTH_BYTE;
    ch_ctl_data.dst_size= DMA_DST_DATA_WIDTH_BYTE;

    ch_cfg.chan_ctr = ch_ctl_data;
    ch_cfg.chan_src_end_ptr = (uint32_t)&(UARTx->DR);
    ch_cfg.chan_dst_end_ptr = (uint32_t)(data+len-1);

    (pChan_Cfg_Align + dma_chan)->chan_ctr = ch_cfg.chan_ctr;
    (pChan_Cfg_Align + dma_chan)->chan_src_end_ptr = ch_cfg.chan_src_end_ptr;
    (pChan_Cfg_Align + dma_chan)->chan_dst_end_ptr = ch_cfg.chan_dst_end_ptr;

    DMA_WAIT_ON_REQ |= (1<<dma_chan);
    DMA->CFG |= 0x1; // dma enable
    DMA_INT_MASK |= (1<<dma_chan); // dma interrupt unmask, write 1
    DMA->CHAN_PRI_ALT_CLR |= (1<<dma_chan);
    DMA->CTL_BASE_PTR = (uint32_t)pChan_Cfg_Align;
//    DMA->CHAN_EN_CLR |= ~(1<<dma_chan); // disable other channels
    DMA->CHAN_EN_SET |= (1<<dma_chan); // enable channel
}

void asr_dma_uart_tx(uint8_t uart_idx,uint8_t *data,uint16_t len)
{
    uint8_t dma_chan = 0;
    UART_TypeDef *UARTx = uart_get_uartx_via_idx(uart_idx);
    if(uart_idx>=UART_NUM)
        return;
    dma_chan = port_dma_get_uart_ch(uart_idx,1);
    // malloc for channel descriptor
    Chan_Cfg_TypeDef * pChan_Cfg_Align = asr_dma_ctrl_block_init();

    Chan_Ctl_Data_TypeDef ch_ctl_data;
    Chan_Cfg_TypeDef ch_cfg;

    ch_ctl_data.cycle_ctl = DMA_OP_MODE_BASIC;
    ch_ctl_data.n_minus_1 = len-1;
    ch_ctl_data.R_pow= 2;
    ch_ctl_data.src_inc = DMA_SRC_ADDR_INC_BYTE;
    ch_ctl_data.dst_inc = DMA_DST_ADDR_INC_FIX;
    ch_ctl_data.src_size= DMA_SRC_DATA_WIDTH_BYTE;
    ch_ctl_data.dst_size= DMA_DST_DATA_WIDTH_BYTE;
    ch_cfg.chan_ctr = ch_ctl_data;
    ch_cfg.chan_src_end_ptr = (uint32_t)(data+len-1);
    ch_cfg.chan_dst_end_ptr = (uint32_t)&(UARTx->DR);

    (pChan_Cfg_Align + dma_chan)->chan_ctr = ch_cfg.chan_ctr;
    (pChan_Cfg_Align + dma_chan)->chan_src_end_ptr = ch_cfg.chan_src_end_ptr;
    (pChan_Cfg_Align + dma_chan)->chan_dst_end_ptr = ch_cfg.chan_dst_end_ptr;

    DMA->CFG |= 0x1; // dma enable
    DMA_INT_MASK |= (1<<dma_chan); // dma interrupt unmask, write 1
    DMA->CHAN_PRI_ALT_CLR |= (1<<dma_chan);
    DMA->CTL_BASE_PTR = (uint32_t)pChan_Cfg_Align;
//    DMA->CHAN_EN_CLR |= ~(1<<dma_chan); // disable other channels
    DMA->CHAN_EN_SET |= (1<<dma_chan); // enable channel
}


void asr_dma_spi_tx(uint8_t spi_idx,uint8_t *data,uint16_t len)
{
    uint8_t dma_chan = 0;
    if(spi_idx>=SPI_NUM)
        return;

    SPI_TypeDef *SPIx = getSpixViaIdx(spi_idx);
    dma_chan = port_dma_get_spi_ch(spi_idx,1);
   // malloc for channel descriptor
    Chan_Cfg_TypeDef * pChan_Cfg_Align = asr_dma_ctrl_block_init();
    Chan_Ctl_Data_TypeDef ch_ctl_data;
    Chan_Cfg_TypeDef ch_cfg;

    ch_ctl_data.cycle_ctl = DMA_OP_MODE_BASIC;
    ch_ctl_data.n_minus_1 = len-1;
    ch_ctl_data.R_pow= 1;
    ch_ctl_data.src_inc = DMA_SRC_ADDR_INC_BYTE;
    ch_ctl_data.dst_inc = DMA_DST_ADDR_INC_FIX;
    ch_ctl_data.src_size= DMA_SRC_DATA_WIDTH_BYTE;
    ch_ctl_data.dst_size= DMA_DST_DATA_WIDTH_BYTE;

    ch_cfg.chan_ctr = ch_ctl_data;
    ch_cfg.chan_src_end_ptr = (uint32_t)(data+len-1);
    ch_cfg.chan_dst_end_ptr = (uint32_t)&(SPIx->DR);

    (pChan_Cfg_Align + dma_chan)->chan_ctr = ch_cfg.chan_ctr;
    (pChan_Cfg_Align + dma_chan)->chan_src_end_ptr = ch_cfg.chan_src_end_ptr;
    (pChan_Cfg_Align + dma_chan)->chan_dst_end_ptr = ch_cfg.chan_dst_end_ptr;

    DMA->CFG |= 0x1; // dma enable
    DMA_INT_MASK |= (1<<dma_chan); // dma interrupt unmask, write 1
    DMA->CHAN_PRI_ALT_CLR |= (1<<dma_chan);
    DMA->CTL_BASE_PTR = (uint32_t)pChan_Cfg_Align;
    // set channel useburst bit to diasble sreq from generating dma request
    DMA->CHAN_USE_BURST_SET |= (1<<dma_chan);
//    DMA->CHAN_EN_CLR |= ~(1<<dma_chan); // disable other channels
    DMA->CHAN_EN_SET |= (1<<dma_chan); // enable channel
}

void asr_dma_spi_rx(uint8_t spi_idx,uint8_t *data,uint16_t len)
{
    uint8_t dma_chan = 0;
    if(spi_idx>=SPI_NUM)
        return;

    SPI_TypeDef *SPIx = getSpixViaIdx(spi_idx);
    dma_chan = port_dma_get_spi_ch(spi_idx,0);
   // malloc for channel descriptor
    Chan_Cfg_TypeDef * pChan_Cfg_Align = asr_dma_ctrl_block_init();
    Chan_Ctl_Data_TypeDef ch_ctl_data;
    Chan_Cfg_TypeDef ch_cfg;

    ch_ctl_data.cycle_ctl = DMA_OP_MODE_BASIC;
    ch_ctl_data.n_minus_1 = len-1;
    ch_ctl_data.R_pow= 1;
    ch_ctl_data.src_inc = DMA_DST_ADDR_INC_FIX;
    ch_ctl_data.dst_inc = DMA_SRC_ADDR_INC_BYTE;
    ch_ctl_data.src_size= DMA_SRC_DATA_WIDTH_BYTE;
    ch_ctl_data.dst_size= DMA_DST_DATA_WIDTH_BYTE;

    ch_cfg.chan_ctr = ch_ctl_data;
    ch_cfg.chan_src_end_ptr = (uint32_t)&(SPIx->DR);
    ch_cfg.chan_dst_end_ptr = (uint32_t)(data+len-1);

    (pChan_Cfg_Align + dma_chan)->chan_ctr = ch_cfg.chan_ctr;
    (pChan_Cfg_Align + dma_chan)->chan_src_end_ptr = ch_cfg.chan_src_end_ptr;
    (pChan_Cfg_Align + dma_chan)->chan_dst_end_ptr = ch_cfg.chan_dst_end_ptr;

    DMA->CFG |= 0x1; // dma enable
    DMA_WAIT_ON_REQ |= (1<<dma_chan);
    DMA_INT_MASK |= (1<<dma_chan); // dma interrupt unmask, write 1
    DMA->CHAN_PRI_ALT_CLR |= (1<<dma_chan);
    DMA->CTL_BASE_PTR = (uint32_t)pChan_Cfg_Align;

    DMA->CHAN_USE_BURST_CLR |= (1<<dma_chan);
//    DMA->CHAN_EN_CLR |= ~(1<<dma_chan); // disable other channels
    DMA->CHAN_EN_SET |= (1<<dma_chan); // enable channel
}

void asr_dma_callback_register(uint8_t chn_idx,asr_dma_callback_func func)
{
    g_asr_dma_callback_struct[chn_idx].cb = func;
}

void DMA_IRQHandler(void)
{
    uint8_t  i;
    uint32_t chan_used = DMA_INT_STAT; // get all enabled channels
    for (i = 0; i < DMA_MAX_CHAN_NUM; i++)
    {
        if (chan_used & (1 << i))
        {
            asr_dma_interrupt_clear(i);
            if (g_asr_dma_callback_struct[i].cb != NULL)
            {
                g_asr_dma_callback_struct[i].cb((i));
                break;
            }
        }
    }
}



