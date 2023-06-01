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

#include "asr_factory_config.h"
#include "asr_aes.h"
#include "asr_efuse.h"
#include "asr_flash.h"
#include "asr_hash.h"
#include "lega_rtos_api.h"
#include "string.h"

#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
static uint8_t asr_factory_config_para[ASR_MATTER_PARTITION_MAX][8] = {
    "iter-cnt", "    salt", "verifier", "discrimi", "dac-cert", "dacPvkey", "dacPbkey", "pai-cert",
    "cert-dcl", "vendorNm", "vendorId", "prodctNm", "prodctId", "rdid-uid", " chip-id", " ftyHash"
};

#define ASR_CONFIG_LEN_SIZE 4
#define ASR_CONFIG_NAME_SIZE 8

static uint32_t asr_factory_dac_private_key_offset = 0;

int32_t asr_factory_config_data_search(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t * pBufLen,
                                       uint32_t * puiOffset)
{
    if ((NULL == buf) || (NULL == puiOffset) || (NULL == pBufLen))
    {
        return -1;
    }

    uint8_t * pFactoryPara = asr_factory_config_para[matter_partition];
    uint32_t usOffset      = 0;
    uint8_t ucParaName[8]  = { 0 };
    int32_t ret            = 0;
    uint32_t uiDataLen     = 0;

    do
    {
        usOffset += uiDataLen;

        ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &usOffset, (void *) ucParaName, ASR_CONFIG_NAME_SIZE);
        if (ret != 0)
        {
            printf("asr_flash_read ASR_CONFIG_NAME_SIZE failed\r\n");
            return ret;
        }

        ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &usOffset, (void *) &uiDataLen, ASR_CONFIG_LEN_SIZE);
        if (ret != 0)
        {
            printf("asr_flash_read ASR_CONFIG_LEN_SIZE failed\r\n");
            return ret;
        }
    } while (memcmp(ucParaName, pFactoryPara, ASR_CONFIG_NAME_SIZE));

    *puiOffset = usOffset - ASR_CONFIG_LEN_SIZE - ASR_CONFIG_NAME_SIZE;

    if (*pBufLen < uiDataLen)
    {
        return -1;
    }

    *pBufLen = uiDataLen;

    ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &usOffset, (void *) buf, uiDataLen);

    if (ASR_FACTORY_HASH_PARTITION == matter_partition)
    {
        asr_factory_dac_private_key_offset = usOffset;
    }

    return ret;
}

int32_t asr_factory_config_offset_search(asr_matter_partition_t matter_partition, uint32_t * puiOffset)
{
    if (NULL == puiOffset)
    {
        return -1;
    }

    uint8_t * pFactoryPara = asr_factory_config_para[matter_partition];
    uint32_t usOffset      = 0;
    uint8_t ucParaName[8]  = { 0 };
    int32_t ret            = 0;
    uint32_t uiDataLen     = 0;

    do
    {
        usOffset += uiDataLen;

        ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &usOffset, (void *) ucParaName, ASR_CONFIG_NAME_SIZE);
        if (ret != 0)
        {
            printf("asr_flash_read ASR_CONFIG_NAME_SIZE failed\r\n");
            return ret;
        }

        ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &usOffset, (void *) &uiDataLen, ASR_CONFIG_LEN_SIZE);
        if (ret != 0)
        {
            printf("asr_flash_read ASR_CONFIG_LEN_SIZE failed\r\n");
            return ret;
        }
    } while (memcmp(ucParaName, pFactoryPara, ASR_CONFIG_NAME_SIZE));

    *puiOffset = usOffset - ASR_CONFIG_LEN_SIZE - ASR_CONFIG_NAME_SIZE;
    return 0;
}

int32_t asr_factory_config_read(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t buf_len, uint32_t * out_len)
{
    int32_t ret        = 0;
    uint32_t uiOffset  = 0;
    uint32_t uiReadLen = buf_len;

    memset(buf, 0, buf_len);

    if (matter_partition == ASR_DAC_KEY_PARTITION)
    {
        ret = asr_factory_dac_prvkey_get(buf, out_len);
        if (ret != 0)
        {
            return ret;
        }
    }
    else
    {
        ret = asr_factory_config_data_search(matter_partition, buf, &uiReadLen, &uiOffset);
        if (ret != 0)
        {
            return ret;
        }

        *out_len = uiReadLen;
    }

    return ret;
}

int32_t asr_factory_config_write(uint8_t * configbuffer, uint32_t buf_len)
{
    if (buf_len > asr_factory_dac_private_key_offset)
    {
        printf("asr matter: write buffer too large.\n");
        return -2;
    }
    uint32_t off_set = 0;

    return asr_flash_erase_write(ASR_CONFIG_BASE, (uint32_t *) &off_set, (void *) configbuffer, buf_len);
}

int32_t asr_factory_config_buffer_write(asr_matter_partition_t matter_partition, const void * buf, uint32_t buf_len)
{
    int ret            = 0;
    uint32_t uiOffset  = 0;
    uint32_t uiDataLen = 0;

    ret = asr_factory_config_offset_search(matter_partition, &uiOffset);
    if (ret != 0)
    {
        return ret;
    }

    uiOffset += ASR_CONFIG_NAME_SIZE;
    ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &uiOffset, (void *) &uiDataLen, ASR_CONFIG_LEN_SIZE);
    if (ret != 0)
    {
        printf("read data len failed\r\n");
        return ret;
    }

    if (buf_len != uiDataLen)
    {
        return -1;
    }

    uiOffset -= uiDataLen;

    return asr_flash_erase_write(ASR_CONFIG_BASE, (uint32_t *) &uiOffset, (void *) buf, buf_len);
}
extern int32_t asr_factory_decrypt_dac_prvkey(uint8_t * pDacCipher, uint8_t ucDacKeyLen, uint8_t * pRdBuf, uint32_t * pOutLen);
int32_t asr_factory_dac_prvkey_get(uint8_t * pRdBuf, uint32_t * pOutLen)
{
    uint32_t off_set = asr_factory_dac_private_key_offset;
    uint8_t ucCipher[32];
    int ret = 0;
    uint8_t ucTmp[32];
    uint32_t uiCipherLen = sizeof(ucCipher);

    memset(ucTmp, 0, 32);

    if (NULL == pRdBuf || NULL == pOutLen)
    {
        return -1;
    }

    // get DAC cipher
    ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &off_set, (void *) ucCipher, 32);
    if (ret != 0)
        return ret;

    ret = memcmp(ucCipher, ucTmp, 32);
    if (ret != 0)
    {
        ret = asr_factory_decrypt_dac_prvkey(ucCipher, 32, pRdBuf, pOutLen);
        if (ret != 0)
        {
            printf("duet_factory_decrypt_dac_prvkey fail\r\n");
            return ret;
        }
    }
    else
    {
        ret = asr_factory_config_data_search(ASR_DAC_KEY_PARTITION, pRdBuf, &uiCipherLen, &off_set);
        if (ret != 0)
        {
            return -1;
        }

        *pOutLen = uiCipherLen;
    }

    return ret;
}

uint8_t asr_factory_check()
{
    int ret                    = -1;
    uint32_t uiOffset          = 0;
    uint32_t uiRdLen           = 0;
    HASH_Result_t hashOutBuff  = { 0 };
    uint8_t ExceptHashBuff[32] = { 0 };
    uint32_t uiBufLen          = sizeof(ExceptHashBuff);

    ret = asr_factory_config_data_search(ASR_FACTORY_HASH_PARTITION, ExceptHashBuff, &uiBufLen, &uiRdLen);
    if (ret != 0)
    {
        printf("asr_factory_config_data_search fail\r\n");
        return -1;
    }

    uint8_t * ucRdBuf = lega_rtos_malloc(uiRdLen);
    if (ucRdBuf == NULL)
    {
        return -1;
    }

    memset(ucRdBuf, 0x00, uiRdLen);

    // read the factory bin.
    ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &uiOffset, ucRdBuf, uiRdLen);
    if (ret != 0)
    {
        printf("read the factory bin. fail \r\n");
        goto free_and_return;
    }

    // calculate the hash result of the factory bin.
    ret = asr_hash(HASH_SHA256_mode, ucRdBuf, uiRdLen, hashOutBuff);
    if (ret != 0)
    {
        goto free_and_return;
    }

    // check the factory bin's hash value.
    ret = memcmp(ExceptHashBuff, hashOutBuff, 32);

free_and_return:
    lega_rtos_free(ucRdBuf);
    return ret;
}

#endif