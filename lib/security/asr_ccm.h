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
#ifndef _ASR_CCM_H_
#define _ASR_CCM_H_

#include <stdint.h>
#include "asr_aes.h"

/* key and key buffer sizes definitions */
#define AESCCM_KEY_SIZE_WORDS           8
#define AESCCM_MODE_CCM      0
#define AESCCM_MODE_STAR     1

/************************ Typedefs  ****************************/

typedef enum
{
    AES_Key128BitSize   = 0,
    AES_Key192BitSize   = 1,
    AES_Key256BitSize   = 2,
    AES_Key512BitSize   = 3,

    AES_KeySizeNumOfOptions,

    AES_KeySizeLast     = 0x7FFFFFFF,
}AESCCM_KeySize_t;

/* Defines the AES_CCM key buffer */
typedef uint8_t AESCCM_Key_t[AESCCM_KEY_SIZE_WORDS * sizeof(uint32_t)];
typedef uint8_t AESCCM_Mac_Res_t[AES_BLOCK_SIZE_IN_BYTES];

/************************ Functions *****************************/

int asr_CC_AESCCM
(
    AesEncryptMode_t EncrDecrMode,     /*!< [in] A flag specifying whether an AES Encrypt (::SASI_AES_ENCRYPT) or Decrypt
                                (::SASI_AES_DECRYPT) operation should be performed. */
    AESCCM_Key_t CCM_Key,          /*!< [in] Pointer to AES-CCM key. */
    AESCCM_KeySize_t KeySizeId,        /*!< [in] Enumerator defining the key size (only 128 bit is valid). */
    uint8_t *N_ptr,            /*!< [in] Pointer to the Nonce. */
    uint8_t SizeOfN,          /*!< [in] Nonce byte size. The valid values depend on the ccm mode:
                                                                            <ul><li>CCM:  valid values = [7 .. 13].</li>
                                                                            <li>CCM*: valid values = [13].</li></ul> */
    uint8_t *ADataIn_ptr,      /*!< [in] Pointer to the additional input data. The buffer must be contiguous. */
    uint32_t ADataInSize,      /*!< [in] Byte size of the additional data. */
    uint8_t *TextDataIn_ptr,   /*!< [in] Pointer to the plain-text data for encryption or cipher-text data for decryption.
                                The buffer must be contiguous. */
    uint32_t TextDataInSize,   /*!< [in] Byte size of the full text data. */
    uint8_t *TextDataOut_ptr,  /*!< [out] Pointer to the output (cipher or plain text data according to encrypt-decrypt mode)
                                data. The buffer must be contiguous. */
    uint8_t SizeOfT,          /*!< [in] AES-CCM MAC (tag) byte size. The valid values depend on the ccm mode:
                                                                            <ul><li>CCM:  valid values = [4, 6, 8, 10, 12, 14, 16].</li>
                                                                            <li>CCM*: valid values = [0, 4, 8, 16].</li></ul>*/
    AESCCM_Mac_Res_t Mac_Res,           /*!< [in/out] Pointer to the MAC result buffer. */
    uint32_t ccmMode                             /*!< [in] Flag specifying whether AES-CCM or AES-CCM* should be performed. */
);

#define asr_AESCCM(EncrDecrMode, CCM_Key, KeySizeId, N_ptr, SizeOfN, ADataIn_ptr, ADataInSize, TextDataIn_ptr, TextDataInSize, TextDataOut_ptr, SizeOfT, Mac_Res) \
        asr_CC_AESCCM(EncrDecrMode, CCM_Key, KeySizeId, N_ptr, SizeOfN, ADataIn_ptr, ADataInSize, TextDataIn_ptr, TextDataInSize, TextDataOut_ptr, SizeOfT, Mac_Res, AESCCM_MODE_CCM)

#define asr_AESCCMStar(EncrDecrMode, CCM_Key, KeySizeId, N_ptr, SizeOfN, ADataIn_ptr, ADataInSize, TextDataIn_ptr, TextDataInSize, TextDataOut_ptr, SizeOfT, Mac_Res) \
        asr_CC_AESCCM(EncrDecrMode, CCM_Key, KeySizeId, N_ptr, SizeOfN, ADataIn_ptr, ADataInSize, TextDataIn_ptr, TextDataInSize, TextDataOut_ptr, SizeOfT, Mac_Res, AESCCM_MODE_STAR)

#endif //_ASR_CCM_H_