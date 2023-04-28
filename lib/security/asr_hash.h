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
#ifndef _ASR_HASH_H_
#define _ASR_HASH_H_

#include <stdint.h>
#include <stddef.h>

#define HASH_RESULT_SIZE_IN_WORDS 16
#define HASH_USER_CTX_SIZE_IN_WORDS 60
/* The SHA-256 digest result size in bytes */
#define HASH_SHA224_DIGEST_SIZE_IN_BYTES 28
#define HASH_MD5_DIGEST_SIZE_IN_BYTES 16
/* The SHA-256 digest result size in bytes */
#define HASH_SHA256_DIGEST_SIZE_IN_BYTES 32

/* The SHA-384 digest result size in bytes */
#define HASH_SHA384_DIGEST_SIZE_IN_BYTES 48

/* The SHA-512 digest result size in bytes */
#define HASH_SHA512_DIGEST_SIZE_IN_BYTES 64
#define HASH_SHA1_DIGEST_SIZE_IN_BYTES 20

typedef enum
{
    HASH_SHA1_mode          = 0,    /*!< SHA1 */
    HASH_SHA224_mode        = 1,    /*!< SHA224 */
    HASH_SHA256_mode        = 2,    /*!< SHA256 */
    HASH_SHA384_mode        = 3,    /*!< SHA384 */
    HASH_SHA512_mode        = 4,    /*!< SHA512 */
    HASH_MD5_mode           = 5,    /*!< MD5 */

    HASH_NumOfModes,

    HASH_OperationModeLast  = 0x7FFFFFFF,

}HASH_OperationMode_t;

typedef uint32_t HASH_Result_t[HASH_RESULT_SIZE_IN_WORDS];

typedef struct HASHUserContext_t
{
    uint32_t buff[HASH_USER_CTX_SIZE_IN_WORDS];
}HASHUserContext_t;

/************************ Functions *****************************/

int asr_hash
(
    HASH_OperationMode_t OperationMode,       /*!< [in]  One of the supported HASH modes, as defined in CRYS_HASH_OperationMode_t. */
    uint8_t *DataIn_ptr,          /*!< [in]  Pointer to the input data to be HASHed.
                                                                The size of the scatter/gather list representing the data buffer is limited
                        to 128 entries, and the size of each entry is limited to 64KB
                        (fragments larger than 64KB are broken into fragments <= 64KB). */
    size_t DataSize,            /*!< [in]  The size of the data to be hashed in bytes. */
    HASH_Result_t HashResultBuff       /*!< [out] Pointer to a word-aligned 64 byte buffer. The actual size of the HASH
                        result depends on CRYS_HASH_OperationMode_t. */
);

int asr_hash_init
(
    HASHUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HASH context buffer allocated by the user that is used
                    for the HASH machine operation. */
    HASH_OperationMode_t OperationMode           /*!< [in]  One of the supported HASH modes, as defined in CRYS_HASH_OperationMode_t. */
);

int asr_hash_update
(
    HASHUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HASH context buffer allocated by the user, which is used for the
                        HASH machine operation. */
    uint8_t *DataIn_ptr,            /*!< [in]  Pointer to the input data to be HASHed.
                                                                The size of the scatter/gather list representing the data buffer is limited to
                        128 entries, and the size of each entry is limited to 64KB
                        (fragments larger than 64KB are broken into fragments <= 64KB). */
    size_t DataInSize             /*!< [in]  Byte size of the input data. Must be > 0.
                                                                If not a multiple of the HASH block size (64 for MD5, SHA-1 and SHA-224/256,
                        128 for SHA-384/512), no further calls
                                                                to CRYS_HASH_Update are allowed in this context, and only CRYS_HASH_Finish
                        can be called to complete the computation. */
);

int asr_hash_finish
(
    HASHUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HASH context buffer allocated by the user that is used for
                        the HASH machine operation. */
    HASH_Result_t HashResultBuff         /*!< [in]  Pointer to the word-aligned 64 byte buffer. The actual size of the HASH
                        result depends on CRYS_HASH_OperationMode_t. */
);


#endif //_ASR_HASH_H_