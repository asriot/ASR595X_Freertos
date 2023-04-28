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

#include "mbedtls/entropy_poll.h"

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include "stdio.h"
#include "asr_rnd.h"
#include "asr_sec_hw_common.h"

int mbedtls_hardware_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen )
{
    int ret = 0;
    asr_security_lock();
    ret = asr_RND_AddAdditionalInput(rndContext_ptr, "0123", 4);
    if (ret != ASR_SILIB_RET_OK){
        printf("\n asr_RND_AddAdditionalInput failed with 0x%x \n",ret);
        goto rnd_err;
    }
    ret = asr_RND_Reseeding(rndContext_ptr, rndWorkBuff_ptr);
    if (ret != ASR_SILIB_RET_OK){
        printf("\n asr_RND_Reseeding failed with 0x%x \n",ret);
        goto rnd_err;
    }
    ret = asr_RND_GenerateVector(&rndContext_ptr->rndState, len, output);
    if (ret != ASR_SILIB_RET_OK){
        printf("\n asr_RND_GenerateVector for vector 1 failed with 0x%x \n",ret);
        goto rnd_err;
    }

    *olen = len;
    asr_security_unlock();
rnd_err:
    return ret;
}
#endif