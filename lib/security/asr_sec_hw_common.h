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
#ifndef _ASR_SEC_HW_COMMON_H_
#define _ASR_SEC_HW_COMMON_H_

#include "asr_rnd.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
    ASR_SILIB_RET_OK = 0,
    ASR_SILIB_RET_EINVAL_CTX_PTR,
    ASR_SILIB_RET_EINVAL_WORK_BUF_PTR,
    ASR_SILIB_RET_HAL,
    ASR_SILIB_RET_PAL,
    ASR_SILIB_RET_RND_INST_ERR,
    ASR_SILIB_RET_EINVAL_HW_VERSION,    /* Invalid HW version */
    ASR_SILIB_RET_EINVAL_HW_SIGNATURE,  /* Invalid HW signature */
    ASR_SILIB_RESERVE32B = 0x7FFFFFFFL
}ASR_libRetCode_t;

#define BITS_IN_32BIT_WORD 32
#define USER_1K_SIZE_IN_BYTES 1024
#define BITS_IN_BYTE 8
#define USER_32BIT_WORD_SIZE (sizeof(uint32_t))
#define ERROR_BASE 0x00F00000UL
#define ERROR_LAYER_RANGE 0x00010000UL
/* The error range number assigned to each module on its specified layer */
#define ERROR_MODULE_RANGE 0x00000100UL
#define LAYER_ERROR_IDX 0x00UL
#define RND_ERROR_IDX 0x0CUL

#define ASR_CALC_FULL_BYTES(numBits)               (((numBits) + (BITS_IN_BYTE -1))/BITS_IN_BYTE)
#define ASR_CALC_FULL_32BIT_WORDS(numBits)         (((numBits) + (BITS_IN_32BIT_WORD -1))/BITS_IN_32BIT_WORD)
#define ASR_CALC_32BIT_WORDS_FROM_BYTES(sizeBytes) (((sizeBytes) + USER_32BIT_WORD_SIZE - 1) / USER_32BIT_WORD_SIZE)
#define ASR_ROUNDUP_BITS_TO_32BIT_WORD(numBits)    (ASR_CALC_FULL_32BIT_WORDS(numBits)*BITS_IN_32BIT_WORD)
#define ASR_ROUNDUP_BITS_TO_BYTES(numBits)         (ASR_CALC_FULL_BYTES(numBits)*BITS_IN_BYTE)
#define ASR_ROUNDUP_BYTES_TO_32BIT_WORD(numBytes)  (USER_32BIT_WORD_SIZE*(((numBytes)+USER_32BIT_WORD_SIZE-1)/USER_32BIT_WORD_SIZE))

/* RND module on the CRYS layer base address - 0x00F00C00 */
#define RND_MODULE_ERROR_BASE   (ERROR_BASE + \
                                (ERROR_LAYER_RANGE * LAYER_ERROR_IDX) + \
                                (ERROR_MODULE_RANGE * RND_ERROR_IDX ) )

extern RND_Context_t *rndContext_ptr;
extern RND_WorkBuff_t *rndWorkBuff_ptr;

/************************ Functions *****************************/
int asr_hw_lib_init(RND_Context_t *rnd_context_p, RND_WorkBuff_t *rnd_workbuff_p);
int asr_hw_lib_deinit(RND_Context_t *rnd_context_p);

int asr_security_engine_init();
int asr_security_engine_deinit();

int asr_security_lock(void);
int asr_security_unlock(void);

#ifdef __cplusplus
}
#endif

#endif //_ASR_SECURITY_HW_COMMON_H_