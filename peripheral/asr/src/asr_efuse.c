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
#include "asr_efuse.h"
#include "asr_rf_spi.h"
#include "asr_port_peripheral.h"

/*
    efuse init, must be called before read operation
    input parameter write_en is used for compatible with old version,
    and won't take effect for efuse should not be write via SDK,
    eufse should be changed only on MP process
*/
void asr_efuse_init(uint8_t write_en)
{
    EFUSE->RD_CNT = EFUSE_READ_OP_WAIT_CYCLE;
    EFUSE->WR_CNT = EFUSE_WRITE_OP_WAIT_CYCLE;
    EFUSE->DIV_CNT = EFUSE_DIV_OP_WAIT_CYCLE;
}


static uint8_t get_blk_idex(uint16_t addr)
{
    uint8_t index ;
    uint16_t blk_addr;
    uint16_t blk_size;
    index=0;
    blk_addr=0;
    blk_size=0;
    for(index=0;index<EFUSE_MAX_BLK;index++)
    {
        blk_addr=efuse_blk[index].vir_hadr;
        blk_size=efuse_blk[index].size;
        if((addr>=blk_addr) &&(addr<blk_addr+blk_size) )
        {
            return index;
        }

    }
    return  EFUSE_MAX_BLK;
}

static uint16_t get_efuse_addr(uint16_t addr)
{
    uint8_t idex;
    uint16_t fuse_addr;
    idex=get_blk_idex(addr);
    if(idex <EFUSE_MAX_BLK)
    {
        fuse_addr= efuse_blk[idex].efuse_adr+(addr-efuse_blk[idex].vir_hadr);

    }
    else
    {
        fuse_addr= EFUSE_INVALID_ADDR;
    }
    return  fuse_addr;


}

/*
    read one efuse byte
    param-addr: efuse addr, from 0x000 to 0x25F
    user DATA storage area  0xF0~0x1EF &  0x200~0x25F
*/
uint8_t asr_efuse_byte_read(uint16_t addr)
{
    uint16_t addr_tmp;

    if(addr <=USER_BLK_END_ADDR1)
    {
        addr_tmp= get_efuse_addr(addr) ;
        if(addr_tmp ==EFUSE_INVALID_ADDR)
        {
            return 0xFF;
        }
    }
    else //invalid address
    {
        return 0xFF;
    }
    EFUSE->B_ADDR = addr_tmp;
    EFUSE->CFG_TYPE = 0x0;//read type
    EFUSE->START = 0x1;
    while(EFUSE->START & 0x1);
    return EFUSE->RDBK_DATA;
}

/*
    read one efuse word
    param-addr: efuse addr, from 0x000 to 0x1FC
*/
uint32_t asr_efuse_word_read(uint16_t addr)
{
    uint32_t rd_word_data = 0;
    uint8_t rd_byte_data = 0;
    for(int i = 0; i < 4; i++)
    {
        rd_byte_data = asr_efuse_byte_read(addr+i);
        rd_word_data |= rd_byte_data << (i<<3);
    }
    return rd_word_data;
}

/*
    read multiple efuse bytes
    param-start_addr: efuse addr, from 0x000 to 0x1FF
    param-size_in_bytes: how many bytes to be read
    param-pData: where efuse data is stored
*/
void asr_efuse_multi_read(uint16_t start_addr, uint16_t size_in_bytes, uint8_t *pData)
{
    uint16_t i;
    //efuse init
    asr_efuse_init(DISABLE);

    //efuse byte read
    for(i = 0; i < size_in_bytes; i++)
    {
        *(pData+i) = asr_efuse_byte_read(start_addr+i);
    }
}
