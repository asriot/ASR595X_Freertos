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
#ifndef __ASR_DMA_H
#define __ASR_DMA_H

#include "asr_port.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  PRI_CHAN 0
#define  ALT_CHAN 1

#define DMA_SRC_DATA_WIDTH_BYTE       (0)
#define DMA_SRC_DATA_WIDTH_HALFWORD   (1)
#define DMA_SRC_DATA_WIDTH_WORD       (2)
#define DMA_DST_DATA_WIDTH_BYTE       (0)
#define DMA_DST_DATA_WIDTH_HALFWORD   (1)
#define DMA_DST_DATA_WIDTH_WORD       (2)

#define DMA_SRC_ADDR_INC_BYTE         (0)
#define DMA_SRC_ADDR_INC_HALFWORD     (1)
#define DMA_SRC_ADDR_INC_WORD         (2)
#define DMA_SRC_ADDR_INC_FIX          (3)

#define DMA_DST_ADDR_INC_BYTE         (0)
#define DMA_DST_ADDR_INC_HALFWORD     (1)
#define DMA_DST_ADDR_INC_WORD         (2)
#define DMA_DST_ADDR_INC_FIX          (3)

#define DMA_OP_MODE_STOP                (0)
#define DMA_OP_MODE_BASIC               (1)
#define DMA_OP_MODE_AUTO_REQ            (2)
#define DMA_OP_MODE_PING_PONG           (3)
#define DMA_OP_MODE_MEM_SCT_GAT_PRI     (4)
#define DMA_OP_MODE_MEM_SCT_GAT_ALT     (5)
#define DMA_OP_MODE_PERI_SCT_GAT_PRI    (6)
#define DMA_OP_MODE_PERI_SCT_GAT_ALT    (7)

#define DMA_R_POWER_1                   (0)   //every dma period   nums of transfer uinit
#define DMA_R_POWER_2                   (1)
#define DMA_R_POWER_4                   (2)
#define DMA_R_POWER_8                   (3)
#define DMA_R_POWER_16                  (4)
#define DMA_R_POWER_32                  (5)
#define DMA_R_POWER_64                  (6)
#define DMA_R_POWER_128                 (7)
#define DMA_R_POWER_256                 (8)
#define DMA_R_POWER_512                 (9)
#define DMA_R_POWER_1024                (10)

#define DMA_N_1_MAX                     (1024)




typedef struct __DMACR
{
    __I  uint32_t STAT;
    __O  uint32_t CFG;
    __IO uint32_t CTL_BASE_PTR;
    __I  uint32_t ALT_CTL_BASE_PTR;
    __I  uint32_t WAIT_ON_REQ_STAT;
    __O  uint32_t CHAN_SW_REQ;
    __IO uint32_t CHAN_USE_BURST_SET;
    __O  uint32_t CHAN_USE_BURST_CLR;
    __IO uint32_t CHAN_REQ_MASK_SET;
    __O  uint32_t CHAN_REQ_MASK_CLR;
    __IO uint32_t CHAN_EN_SET;
    __O  uint32_t CHAN_EN_CLR;
    __IO uint32_t CHAN_PRI_ALT_SET;
    __O  uint32_t CHAN_PRI_ALT_CLR;
    __IO uint32_t CHAN_PRIORITY_SET;
    __O  uint32_t CHNA_PRIORITY_CLR; /* 0x3c */
    __I  uint32_t RESV[3];
    __O  uint32_t ERR_CLR;           /* 0x4c */
    __I  uint32_t RESV1[0x3ec];      /* 0x50 - 0xffc */
} DMA_TypeDef;

#define DMA                 ( (DMA_TypeDef *)DMA_REG_BASE )

typedef void (*asr_dma_callback_func)(uint32_t);
typedef struct
{
    asr_dma_callback_func cb;
    void *arg;
}asr_dma_callback_struct;
//typedef uint32_t (*canon_dma_feed_data_func)(uint32_t isPri,uint8_t **mem_src,uint8_t **mem_dst);

typedef struct
{
    uint32_t cycle_ctl:3;
    uint32_t next_useburst:1;
    uint32_t n_minus_1:10;
    uint32_t R_pow:4;
    uint32_t src_prot:3;
    uint32_t dest_prot:3;
    uint32_t src_size:2;
    uint32_t src_inc:2;
    uint32_t dst_size:2;
    uint32_t dst_inc:2;
}Chan_Ctl_Data_TypeDef;

typedef struct
{
    uint32_t SrcEndPtr;
    uint32_t DstEndPtr;
    uint32_t n_minus1;
    uint8_t chan_num;
    uint8_t next_useburst;
    uint32_t cycle;
    uint32_t R_power;
    uint32_t SrcDataWidth;
    uint32_t DstDataWidth;
    uint32_t SrcAddrInc;
    uint32_t DstAddrInc;
    uint8_t interrupt_en;
    uint8_t use_pri_only;
}DMA_Init_Struct_Type;

typedef enum
{
    INT8_U=1,
    INT16_U=2,
    INT32_U=4,
} DAT_TYPE;

typedef struct
{
    uint32_t chan_src_end_ptr;
    uint32_t chan_dst_end_ptr;
    Chan_Ctl_Data_TypeDef chan_ctr;
    uint32_t resv;
}Chan_Cfg_TypeDef;

typedef struct
{
    DMA_CHANNEL             used_chan;
    void                    *pbuf;
    void                    *pbuf_bak;
    uint16_t                lenth;
    uint8_t                 dma_mode; //
    uint16_t                r_power;
    void                    *handle_arg;
    DAT_TYPE                type_dat;
} asr_dma_init_struct;

typedef struct
{
    uint8_t             port;
    asr_dma_init_struct asr_dma_struct_init;
    void                *priv;

}asr_dma_dev_t;
extern Chan_Cfg_TypeDef  *pChan_Cfg_Align;
extern asr_dma_callback_struct   g_asr_dma_callback_struct[32];
//extern Chan_Cfg_TypeDef *pChan_Cfg_Align;
Chan_Cfg_TypeDef * asr_dma_ctrl_block_init(void);
void asr_dma_init(void);
void asr_dma_finalize(void);
void asr_dma_mem2mem(uint8_t chan_num,uint8_t *mem_src,uint8_t *mem_dst,uint16_t len);
void asr_dma_uart_rx(uint8_t uart_idx,uint8_t *data,uint16_t len);
void asr_dma_uart_tx(uint8_t uart_idx,uint8_t *data,uint16_t len);
void asr_dma_spi_rx(uint8_t ssp_idx,uint8_t *data,uint16_t len);
void asr_dma_spi_tx(uint8_t ssp_idx,uint8_t *data,uint16_t len);
void asr_dma_callback_register(uint8_t chn_idx,asr_dma_callback_func func);


void asr_dma_channel_cmd(uint32_t chan_idx, uint8_t new_state);
void asr_dma_generate_sw_req(uint32_t chan_idx);
void asr_dma_generate_perip_req(uint32_t chan_idx);
ITstatus asr_dma_get_alt_state(uint8_t channel);
#if 0
void asr_mtm_scagat_dma_init(asr_dma_dev_t *canon_dma_para_struct) ;
void asr_mem_perip_dma_init(asr_dma_dev_t *canon_dma_para_struct);
void asr_mem_mem_dma_init(asr_dma_dev_t *canon_dma_para_struct) ;
void asr_dma_tx_reload(uint8_t channel, uint8_t dma_mode, void *buf, uint16_t lenth) ;
void asr_dma_rx_reload(uint8_t channel, uint8_t dma_mode, void *buf, uint16_t lenth) ;
uint32_t asr_dma_get_src_addr(uint8_t channel);
uint32_t asr_dma_get_dst_addr(uint8_t channel);
void asr_dma_interrupt_config(uint32_t chan_idx, uint8_t new_state);
uint32_t asr_get_dma_sta(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //__ASR_DMA_H
