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
#ifndef _ASR_AES_H_
#define _ASR_AES_H_

#include <stdint.h>

/*! The size of the user's context prototype (see SaSiAesUserContext_t) in words. */
#define AES_USER_CTX_SIZE_IN_WORDS (4+4+7+4)

/*! The AES block size in words. */
#define AES_BLOCK_SIZE_IN_WORDS 4
/*! The AES block size in bytes. */
#define AES_BLOCK_SIZE_IN_BYTES (AES_BLOCK_SIZE_IN_WORDS * sizeof(uint32_t))

/*! The size of the IV buffer in words. */
#define AES_IV_SIZE_IN_WORDS AES_BLOCK_SIZE_IN_WORDS
/*! The size of the IV buffer in bytes. */
#define AES_IV_SIZE_IN_BYTES (AES_IV_SIZE_IN_WORDS * sizeof(uint32_t))

/*! The maximum size of the AES KEY in words. */
#define AES_KEY_MAX_SIZE_IN_WORDS 4
/*! The maximum size of the AES KEY in bytes. */
#define AES_KEY_MAX_SIZE_IN_BYTES (AES_KEY_MAX_SIZE_IN_WORDS * sizeof(uint32_t))

typedef struct AesUserContext_t
{
    uint32_t buff[AES_USER_CTX_SIZE_IN_WORDS];        /*!< Buffer for the AES context. */
}AesUserContext_t;

/*! AES User Key Data. */
typedef struct AesUserKeyData_t
{
    uint8_t *pKey;        /*!< Pointer to the key. */
    size_t keySize;    /*!< The key size in bytes. Valid values:
                      - For XTS mode - 32 or 64 byte, indicating the full size of the double key (2x128 or 2x256 bit).
                      - For XCBC-MAC mode - 16 byte (limited by the standard).
                      - For all other modes - 16, 24 or 32 byte. */
}AesUserKeyData_t;
/*!
Encrypt or Decrypt operation mode.
*/
typedef enum
{
    AES_ENCRYPT = 0,    /*!< Encrypt mode. */
    AES_DECRYPT = 1,    /*!< Decrypt mode. */

    AES_NUM_OF_ENCRYPT_MODES,
    AES_ENCRYPT_MODE_LAST = 0x7FFFFFFF
}AesEncryptMode_t;

/*!
AES operation mode.
*/
typedef enum
{
    AES_MODE_ECB          = 0,        /*!< ECB mode. */
    AES_MODE_CBC          = 1,        /*!< CBC mode. */
    AES_MODE_CBC_MAC      = 2,        /*!< CBC-MAC mode. */
    AES_MODE_CTR          = 3,        /*!< CTR mode. */
    AES_MODE_XCBC_MAC     = 4,        /*!< XCBC-MAC mode. */
    AES_MODE_CMAC         = 5,        /*!< CMAC mode. */
    AES_MODE_XTS          = 6,        /*!< XTS mode. */
    AES_MODE_CBC_CTS      = 7,        /*!< CBC-CTS mode. */
    AES_MODE_OFB          = 8,        /*!< OFB mode. */

    AES_NUM_OF_OPERATION_MODES,
    AES_OPERATION_MODE_LAST = 0x7FFFFFFF
}AesOperationMode_t;

/*!
AES padding type.
*/
typedef enum
{
    AES_PADDING_NONE  = 0,        /*!< No padding. */
    AES_PADDING_PKCS7 = 1,        /*!< PKCS7 padding. */

    AES_NUM_OF_PADDING_TYPES,
    AES_PADDING_TYPE_LAST = 0x7FFFFFFF
}AesPaddingType_t;

/*!
AES key type.
*/
typedef enum
{
    AES_USER_KEY          = 0,        /*!< user key. */
    AES_PLATFORM_KEY      = 1,        /*!< Kplt hardware key. */
    AES_CUSTOMER_KEY      = 2,        /*!< Kcst hardware key. */

    AES_NUM_OF_KEY_TYPES,
    AES_KEY_TYPE_LAST = 0x7FFFFFFF
}AesKeyType_t;


	/*! The AES block size in words. */
#define AES_BLOCK_SIZE_IN_WORDS 4
#define AES_BLOCK_SIZE_IN_BYTES (AES_BLOCK_SIZE_IN_WORDS * sizeof(uint32_t))

	/*! The size of the IV buffer in words. */
#define AES_IV_SIZE_IN_WORDS AES_BLOCK_SIZE_IN_WORDS
	/*! The size of the IV buffer in bytes. */
#define AES_IV_SIZE_IN_BYTES (AES_IV_SIZE_IN_WORDS * sizeof(uint32_t))

/*! Defines the IV buffer  - 16 bytes array. */
typedef uint8_t AesIv_t[AES_IV_SIZE_IN_BYTES];

/************************ Functions *****************************/

/*!
@brief This function is used to initialize an AES operation context.
       To operate the AES machine, this must be the first API called.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesInit
(
    AesUserContext_t *pContext,            /*!< [in]  Pointer to the AES context buffer that is allocated by the caller and initialized by this API.
                                Should be used in all subsequent calls that are part of the same operation. */
    AesEncryptMode_t encryptDecryptFlag,  /*!< [in]  A flag specifying whether an AES Encrypt (SASI_AES_Encrypt) or Decrypt (SASI_AES_Decrypt) operation should be performed.
                                Must be set to CRYS_AES_Encrypt in CBC-MAC, XCBC-MAC and CMAC modes. */
    AesOperationMode_t operationMode,       /*!< [in]  The operation cipher/mode. */
    AesPaddingType_t paddingType          /*!< [in]  The padding type for AES operation:
                            - NONE  - supported for all operation modes.
                            - PKCS7 - supported for ECB, CBC, CBC-MAC operation modes. */
);


/*!
@brief This function sets the key information for the AES operation, in the context that was initialized by SaSi_AesInit.
\note When FIPS certification mode is set to ON, and the mode is AES-XTS, weak keys are not allowed (128/256 lsb bits must be
different than 128/256 msb bits, according to the key size).
@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesSetKey
(
    AesUserContext_t *pContext,        /*!< [in]  Pointer to the AES context, after it was initialized by SaSi_AesInit. */
    AesKeyType_t keyType,         /*!< [in]  The type of key to be used for the AES operation.
                            Currently only SASI_AES_USER_KEY is supported - the key is plaintext and provided in the pKeyData parameter. */
    void *pKeyData,        /*!< [in]  Pointer to the key data structure (to be casted to the relevant struct type). */
    size_t keyDataSize      /*!< [in]  The size of data passed in pKeyData in bytes. */
);


/*!
@brief This function sets the IV, counter or tweak data for the following AES operation on the same context.
       The context must be first initialized by SaSi_AesInit.
       It must be called at least once prior to the first SaSi_AesBlock operation on the same context - for those ciphers that require it.
       If needed, it can also be called to override the IV in the middle of a sequence of SaSi_AesBlock operations.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesSetIv
(
    AesUserContext_t *pContext,    /*!< [in]  Pointer to the AES context. */
    AesIv_t pIV          /*!< [in]  Pointer to the buffer of the IV, counter or tweak.
                        <ul><li> For CBC, CBC-CTS, OFB and CBC-MAC modes - the IV value.</li>
                        <li> For CTR mode - the counter.</li>
                        <li> For XTS mode - the tweak value.</li>
                        <li> For all other modes - N/A. </li></ul>*/
);


/*!
@brief This function retrieves the current IV, counter or tweak from the AES context.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesGetIv
(
    AesUserContext_t *pContext,    /*!< [in]  Pointer to the AES context. */
    AesIv_t pIV          /*!< [out] Pointer to the buffer of the IV, counter or tweak.
                        <ul><li> For CBC, CBC-CTS, OFB and CBC-MAC modes - the IV value.</li>
                        <li> For CTR mode - the counter.</li>
                        <li> For XTS mode - the tweak value.</li>
                        <li> For all other modes - N/A. </li></ul> */
);


/*!
@brief This function performs an AES operation on an input data buffer, according to the configuration defined in the context parameter.
       It can be called as many times as needed, until all the input data is processed.
       SaSi_AesInit, SaSi_AesSetKey, and for some ciphers SaSi_AesSetIv, must be called before
       the first call to this API with the same context.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesBlock
(
    AesUserContext_t *pContext,    /*!< [in]  Pointer to the AES context. */
    uint8_t *pDataIn,     /*!< [in]  Pointer to the buffer of the input data to the AES. The pointer does not need to be aligned.
                            For TZ, the size of the scatter/gather list representing the data buffer is limited to 128 entries,
                            and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB).
                            For ARM CryptoCell 3xx, The buffer must be contiguous and limited to 64KB. */
    size_t dataInSize,  /*!< [in]  Size of the input data in bytes.
                        <ul><li> For all modes except XTS, must be multiple of 16 bytes.</li>
                        <li> For XTS mode, only the following data sizes are supported: 64, 512, 520, 521, 1024 and 4096 bytes.
                                The data passed in a single SaSi_AesBlock call is considered to be a single XTS unit.
                                All subsequent calls to this API with the same context must use the same data size. </li></ul>*/
    uint8_t *pDataOut     /*!< [out] Pointer to the output buffer. The pointer does not need to be aligned.
                            For CBC-MAC, XCBC-MAC, CMAC modes it may be NULL.
                            For TZ, the size of the scatter/gather list representing the data buffer is limited to 128 entries,
                            and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB).
                            For ARM CryptoCell 3xx, The buffer must be contiguous and limited to 64KB. */
);


/*!
@brief This function is used to finish AES operation.

       It processes the last data block if needed, finalizes the AES operation (cipher-specific),
       and produces operation results (for MAC operations).
       \note In case AES padding is used (PKCS#7) Din and Dout user's buffers must include extra space for
       the padding scheme.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesFinish
(
    AesUserContext_t *pContext,       /*!< [in]  Pointer to the AES context. */
    size_t dataSize,       /*!< [in]  The size of the input data in bytes.
                            <ul><li> For CBC-CTS mode, must be > 16. Can be <=16 only if this is the only data (no previous calls were
                                    made to SaSi_AesBlock with the same context).</li>
                            <li> For XTS mode, the data size must conform to the dataInSize rules as listed for XTS under the
                            SaSi_AesBlock API, and match the data size passed in the previous calls to SaSi_AesBlock with the
                            same context.</li>
                            <li> For all other modes, zero is a valid size.</li>
                            <li> For ECB, CBC, CBC-MAC modes: </li>
                                <ul><li> Must be >= 0, if direction is SASI_AES_ENCRYPT and padding type is SASI_AES_PADDING_PKCS7.</li>
                                <li> Must be >= 16 and a multiple of 16 bytes, if direction is SASI_AES_DECRYPT and padding type
                                is SASI_AES_PADDING_PKCS7.</li>
                                <li> Must be a multiple of 16 bytes, otherwise. </li></ul></ul>*/
    uint8_t *pDataIn,        /*!< [in]  Pointer of the input data buffer.
                            For TZ, the size of the scatter/gather list representing the data buffer is limited to 128 entries,
                            and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB).
                            For ARM CryptoCell 3xx, The buffer must be contiguous and limited to 64KB. */
    size_t dataInBuffSize, /*!< [in]  Size of pDataIn buffer in bytes.
                            <ul><li> Must be >= dataSize. </li>
                            <li> According to padding type, must be >= dataSize + padding. For PKCS7, padding size is
                            maximum SASI_AES_BLOCK_SIZE_IN_BYTES. </li></ul>*/
    uint8_t *pDataOut,       /*!< [out] Pointer to the output buffer.
                            For TZ, the size of the scatter/gather list representing the data buffer is limited to 128 entries,
                            and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB).
                            For ARM CryptoCell 3xx, The buffer must be contiguous and limited to 64KB. */
    size_t *dataOutBuffSize /*!< [in,out]  In - Size of pDataOut buffer in bytes.
                            The output buffer size must be no less than:
                            <ul><li> For CBC-MAC, XCBC-MAC, CMAC modes - 16 bytes (for MAC result).</li>
                            <li> For non-MAC modes - dataInBuffSize.</li></ul>
                            Out - The size in bytes of the actual output data:
                            <ul><li> If direction is SASI_AES_ENCRYPT and padding type is SASI_AES_PADDING_PKCS7, it is the actual size
                                with the padding.</li>
                            <li> If direction is SASI_AES_DECRYPT and padding type is SASI_AES_PADDING_PKCS7, it is the size without
                                the padding. </li>
                            <li> For CBC-MAC, XCBC-MAC, CMAC modes - always 16 bytes. </li></ul>*/
);


/*!
@brief This function releases and crears resources after AES operations.

@return SASI_OK on success,
@return A non-zero value from ssi_aes_error.h on failure.
*/
int asr_AesFree(AesUserContext_t *pContext);     /*!< [in] Pointer to the AES context. */


#endif //_ASR_AES_H_