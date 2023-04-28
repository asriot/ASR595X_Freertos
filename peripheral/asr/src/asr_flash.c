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
#ifdef CFG_PLF_RV32
#include "asr_rv32.h"
#endif
#include <stdio.h>
#include <string.h>
#include "errno.h"
#include "asr_flash_alg.h"
#include "asr_flash.h"
#ifndef OS_NOT_SUPPORT
#ifdef ALIOS_SUPPORT
#include "lega_rhino.h"
#else
#include "lega_rtos_api.h"
#endif
#endif
extern const asr_logic_partition_t asr_partitions[];

#define ROUND_DOWN(x, align) ((unsigned long)(x) & ~((unsigned long)align - 1))

/**
 * asr flash init
 *
 * @note this function must be called before flash erase/write operation
 *
 * @param[in]  none
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_init(void)
{
    return asr_flash_alg_init();
}
/**
 * Get the infomation of the specified flash area
 *
 * @param[in]  in_partition  The target flash logical partition
 *
 * @return     HAL_logi_partition struct
 */
asr_logic_partition_t *asr_flash_get_info(asr_partition_t in_partition)
{
    asr_logic_partition_t *logic_partition;

    logic_partition = (asr_logic_partition_t *)&asr_partitions[ in_partition ];

    return logic_partition;
}

/**
 * Erase an area on a Flash logical partition
 *
 * @note  Erase on an address will erase all data on a sector that the
 *        address is belonged to, this function does not save data that
 *        beyond the address area but in the affected sector, the data
 *        will be lost.
 *
 * @param[in]  in_partition  The target flash logical partition which should be erased
 * @param[in]  off_set       Start address of the erased flash area
 * @param[in]  size          Size of the erased flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_erase(asr_partition_t in_partition, uint32_t off_set, uint32_t size)
{
    uint32_t addr;
    uint32_t start_addr, end_addr;
    int32_t ret = 0;
    asr_logic_partition_t *partition_info;

    partition_info = asr_flash_get_info( in_partition );
    if(size + off_set > partition_info->partition_length)
    {
        return EIO;
    }

    start_addr = ROUND_DOWN((partition_info->partition_start_addr + off_set), SPI_FLASH_SEC_SIZE);
    end_addr = ROUND_DOWN((partition_info->partition_start_addr + off_set + size - 1), SPI_FLASH_SEC_SIZE);

    size = end_addr - start_addr + SPI_FLASH_SEC_SIZE;
    addr = start_addr;
    while(size)
    {
        if(!(addr % SPI_FLASH_BLOCK_64K_SIZE) && (size > (SPI_FLASH_BLOCK_64K_SIZE-SPI_FLASH_SEC_SIZE)))
        {
            ret = asr_flash_alg_erase(BLOCK64_ERASE_CMD, addr); //250ms
            if (ret != 0)
            {
                return ret;
            }
            addr += SPI_FLASH_BLOCK_64K_SIZE;
            if(size > SPI_FLASH_BLOCK_64K_SIZE)
            {
                size -= SPI_FLASH_BLOCK_64K_SIZE;
            }
            else
            {
                size = 0;
            }
        }
        else if(!(addr % SPI_FLASH_BLOCK_32K_SIZE) && (size > (SPI_FLASH_BLOCK_32K_SIZE-SPI_FLASH_SEC_SIZE)))
        {
            ret = asr_flash_alg_erase(BLOCK32_ERASE_CMD, addr); //170ms
            if (ret != 0)
            {
                return ret;
            }
            addr += SPI_FLASH_BLOCK_32K_SIZE;
            if(size > SPI_FLASH_BLOCK_32K_SIZE)
            {
                size -= SPI_FLASH_BLOCK_32K_SIZE;
            }
            else
            {
                size = 0;
            }
        }
        else
        {
            ret = asr_flash_alg_erase(SECTOR_ERASE_CMD, addr); //100ms
            if (ret != 0)
            {
                return ret;
            }
            addr += SPI_FLASH_SEC_SIZE;
            if(size > SPI_FLASH_SEC_SIZE)
            {
                size -= SPI_FLASH_SEC_SIZE;
            }
            else
            {
                size = 0;
            }
        }
    }

    return 0;
}

/**
 * Write data to an area on a flash logical partition without erase
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in]  off_set         Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_write(asr_partition_t in_partition, uint32_t *off_set,
                        const void *in_buf, uint32_t in_buf_len)
{
    uint32_t start_addr, end_addr;
    asr_logic_partition_t *partition_info;
    uint8_t *p_buf = (uint8_t *)(in_buf);
    uint32_t prg_size;
    uint32_t left_buf_len = in_buf_len;

    partition_info = asr_flash_get_info( in_partition );
    if(off_set == NULL || in_buf == NULL || ((*off_set + in_buf_len) > partition_info->partition_length))
    {
        // ptr and size over range check
        return EIO;
    }
    start_addr = partition_info->partition_start_addr + *off_set;
    end_addr = start_addr + left_buf_len;

    while(left_buf_len > 0)
    {
        if((end_addr / SPI_FLASH_PAGE_SIZE) > (start_addr / SPI_FLASH_PAGE_SIZE))
        {
            prg_size = SPI_FLASH_PAGE_SIZE - (start_addr % SPI_FLASH_PAGE_SIZE);
        }
        else
        {
            prg_size = left_buf_len;
        }
        asr_flash_alg_programpage(start_addr, prg_size, p_buf);
        p_buf += prg_size;
        start_addr += prg_size;
        left_buf_len -= prg_size;
    }

    *off_set += in_buf_len;
    return 0;
}

/**
 * Write data to an area on a flash logical partition with erase first
 *
 * @param[in]  in_partition    The target flash logical partition which should be read which should be written
 * @param[in]  off_set         Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  inBuffer        point to the data buffer that will be written to flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_erase_write(asr_partition_t in_partition, uint32_t *off_set,
                              const void *in_buf, uint32_t in_buf_len)
{
    int32_t ret = 0;
    ret = asr_flash_erase(in_partition, *off_set, in_buf_len);
    if (ret != 0)
    {
        return ret;
    }
    return asr_flash_write(in_partition, off_set, in_buf, in_buf_len);
}

/**
 * Read data from an area on a Flash to data buffer in RAM
 *
 * @param[in]  in_partition    The target flash logical partition which should be read
 * @param[in]  off_set         Point to the start address that the data is read, and
 *                             point to the last unread address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  outBuffer       Point to the data buffer that stores the data read from flash
 * @param[in]  inBufferLength  The length of the buffer
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_read(asr_partition_t in_partition, uint32_t *off_set,
                       void *out_buf, uint32_t in_buf_len)
{
    int32_t ret = 0;
    uint32_t start_addr;
    asr_logic_partition_t *partition_info;

    partition_info = asr_flash_get_info( in_partition );

    if(off_set == NULL || out_buf == NULL || *off_set + in_buf_len > partition_info->partition_length)
    {
        return EIO;
    }
    start_addr = partition_info->partition_start_addr + *off_set;
    memcpy(out_buf, (void *)(start_addr), in_buf_len);
    *off_set += in_buf_len;
    return ret;
}

/**
* Read flash id
*
* @param[in]  none
*
* @return  the flash id
*/
int asr_flash_read_id(void)
{
    return asr_flash_alg_read_id();
}

/**
* Read flash uuid
*
* @param[in]  uuid_buf        Point to the buffer that stores the uuid read from flash
* @param[in]  buf_len         The length of the buffer, no less than 16 bytes
*
* @return  0 : On success, EIO : If an error occurred with any step
*/
int asr_flash_read_uuid(void *uuid_buf, uint32_t buf_len)
{
    return asr_flash_alg_read_uuid(uuid_buf, buf_len);
}

/**
 * Set security options on a logical partition
 *
 * @param[in]  partition  The target flash logical partition
 * @param[in]  offset     Point to the start address that the data is read, and
 *                        point to the last unread address after this function is
 *                        returned, so you can call this function serval times without
 *                        update this start address.
 * @param[in]  size       Size of enabled flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_enable_secure(asr_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

/**
 * Disable security options on a logical partition
 *
 * @param[in]  partition  The target flash logical partition
 * @param[in]  offset     Point to the start address that the data is read, and
 *                        point to the last unread address after this function is
 *                        returned, so you can call this function serval times without
 *                        update this start address.
 * @param[in]  size       Size of disabled flash area
 *
 * @return  0 : On success, EIO : If an error occurred with any step
 */
int32_t asr_flash_dis_secure(asr_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

