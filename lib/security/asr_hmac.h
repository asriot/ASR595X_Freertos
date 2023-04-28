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
#ifndef _ASR_HMAC_H_
#define _ASR_HMAC_H_

#include "asr_hash.h"

#define HMAC_USER_CTX_SIZE_IN_WORDS 94
/* The HMAC key size after padding for MD5, SHA1, SHA256 */
#define HMAC_KEY_SIZE_IN_BYTES 64

/* The HMAC key size after padding for SHA384, SHA512 */
#define HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES 128

typedef struct HMACUserContext_t
{
    uint32_t buff[HMAC_USER_CTX_SIZE_IN_WORDS];
}HMACUserContext_t;

/************************ Functions *****************************/

/*!
@brief This function initializes the HMAC machine.

It allocates and initializes the HMAC Context. It initiates a HASH session and processes a HASH update on the Key XOR ipad,
then stores it in the context

@return CRYS_OK on success.
@return A non-zero value from crys_hmac_error.h on failure.
*/
int asr_HMAC_Init
(
    HMACUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HMAC context buffer allocated by the user, which is used
                            for the HMAC machine operation. */
    HASH_OperationMode_t OperationMode,          /*!< [in]  One of the supported HASH modes, as defined in CRYS_HASH_OperationMode_t. */
    uint8_t *key_ptr,               /*!< [in]  The pointer to the user's key buffer. */
    uint16_t keySize                /*!< [in]  The key size in bytes. */
);

/*!
@brief This function processes a block of data to be HASHed.

It receives a handle to the HMAC Context, and updates the HASH value with the new data.

@return CRYS_OK on success.
@return A non-zero value from crys_hmac_error.h on failure.
*/

int asr_HMAC_Update
(
    HMACUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HMAC context buffer allocated by the user
                        that is used for the HMAC machine operation. */
    uint8_t *DataIn_ptr,            /*!< [in]  Pointer to the input data to be HASHed.
                                                                The size of the scatter/gather list representing the data buffer is limited to
                        128 entries, and the size of each entry is limited to 64KB
                        (fragments larger than 64KB are broken into fragments <= 64KB). */
    size_t DataInSize             /*!< [in]  Byte size of the input data. Must be > 0.
                                                                If not a multiple of the HASH block size (64 for SHA-1 and SHA-224/256,
                        128 for SHA-384/512), no further calls to ::CRYS_HMAC_Update are allowed in
                        this context, and only ::CRYS_HMAC_Finish can be called to complete the
                        computation. */
);

/*!
@brief This function finalizes the HMAC processing of a data block.

It receives a handle to the HMAC context that was previously initialized by ::CRYS_HMAC_Init, or by ::CRYS_HMAC_Update.
It completes the HASH calculation on the ipad and text, and then executes a new HASH operation with the key XOR opad and the previous
HASH operation result.

@return CRYS_OK on success.
@return A non-zero value from crys_hmac_error.h on failure.
*/

int asr_HMAC_Finish
(
    HMACUserContext_t *ContextID_ptr,         /*!< [in]  Pointer to the HMAC context buffer allocated by the user, which is used
                        for the HMAC machine operation. */
    HASH_Result_t HmacResultBuff         /*!< [out] Pointer to the word-aligned 64 byte buffer. The actual size of the
                        HASH result depends on CRYS_HASH_OperationMode_t. */
);


/*!
@brief This function is a service function that frees the context if the operation has failed.

The function executes the following major steps:
<ol><li> Checks the validity of all of the inputs of the function. </li>
<li> Clears the user's context.</li>
<li> Exits the handler with the OK code.</li></ol>

@return CRYS_OK on success.
@return a non-zero value from crys_hmac_error.h on failure.
*/

int asr_HMAC_Free
(
    HMACUserContext_t *ContextID_ptr         /*!< [in]  Pointer to the HMAC context buffer allocated by the user, which is used for
                        the HMAC machine operation. */
);

/*!
@brief This function processes a single buffer of data, and returns the data buffer's message digest.

@return CRYS_OK on success.
@return A non-zero value from crys_hmac_error.h on failure.
*/
int asr_HMAC(
    HASH_OperationMode_t OperationMode,       /*!< [in]  One of the supported HASH modes, as defined in CRYS_HASH_OperationMode_t. */
    uint8_t *key_ptr,            /*!< [in]  The pointer to the user's key buffer. */
    uint16_t keySize,            /*!< [in]  The key size in bytes. */
    uint8_t *DataIn_ptr,         /*!< [in]  Pointer to the input data to be HASHed.
                                                                The size of the scatter/gather list representing the data buffer is limited to 128
                        entries, and the size of each entry is limited to 64KB (fragments larger than
                        64KB are broken into fragments <= 64KB). */
    size_t DataSize,           /*!< [in]  The size of the data to be hashed (in bytes). */
    HASH_Result_t HmacResultBuff      /*!< [out] Pointer to the word-aligned 64 byte buffer. The actual size of the
                        HMAC result depends on CRYS_HASH_OperationMode_t. */
);

#endif //_ASR_HMAC_H_
