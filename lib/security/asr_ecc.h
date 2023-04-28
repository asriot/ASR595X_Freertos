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
#ifndef _ASR_ECC_H_
#define _ASR_ECC_H_

#include <stdint.h>
#include "asr_hash.h"
#include "asr_rnd.h"

typedef enum
{
    /* For prime field */
    ECPKI_DomainID_secp160k1,   /*!< EC secp160r1 */
    ECPKI_DomainID_secp160r1,   /*!< EC secp160k1 */
    ECPKI_DomainID_secp160r2,   /*!< EC secp160r2 */
    ECPKI_DomainID_secp192k1,   /*!< EC secp192k1 */
    ECPKI_DomainID_secp192r1,   /*!< EC secp192r1 */
    ECPKI_DomainID_secp224k1,   /*!< EC secp224k1 */
    ECPKI_DomainID_secp224r1,   /*!< EC secp224r1 */
    ECPKI_DomainID_secp256k1,   /*!< EC secp256k1 */
    ECPKI_DomainID_secp256r1,   /*!< EC secp256r1 */
    ECPKI_DomainID_secp384r1,   /*!< EC secp384r1 */
    ECPKI_DomainID_secp521r1,   /*!< EC secp521r1 */

    ECPKI_DomainID_Builded,     /*!< User given, not identified. */
    ECPKI_DomainID_OffMode,

    ECPKI_DomainIDLast      = 0x7FFFFFFF,
}ECPKI_DomainID_t;

#define ECPKI_MODUL_MAX_LENGTH_IN_WORDS 18

#define PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS 5
#define CRYS_PKA_DOMAIN_BUFF_SIZE_IN_WORDS (2*CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)

#define PKA_DOMAIN_LLF_BUFF_SIZE_IN_WORDS (10 + 3*ECPKI_MODUL_MAX_LENGTH_IN_WORDS)
#define ECPKI_MODUL_MAX_LENGTH_IN_WORDS 18

#define ECPKI_ORDER_MAX_LENGTH_IN_WORDS (ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1)

#define CRYS_PKA_DOMAIN_BUFF_SIZE_IN_WORDS (2*CRYS_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)

#define CRYS_PKA_EL_GAMAL_BUFF_MAX_LENGTH_IN_WORDS (4*CRYS_ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 4)

/* ECC NAF buffer definitions */
#define COUNT_NAF_WORDS_PER_KEY_WORD 8  /*!< \internal Change according to NAF representation (? 2)*/
#define PKA_ECDSA_NAF_BUFF_MAX_LENGTH_IN_WORDS (COUNT_NAF_WORDS_PER_KEY_WORD*ECPKI_ORDER_MAX_LENGTH_IN_WORDS + 1)

/* on fast SCA non protected mode required additional buffers for NAF key */
#define PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS (PKA_ECDSA_NAF_BUFF_MAX_LENGTH_IN_WORDS+ECPKI_MODUL_MAX_LENGTH_IN_WORDS+2)
#define PKA_KG_BUFF_MAX_LENGTH_IN_WORDS (2*ECPKI_ORDER_MAX_LENGTH_IN_WORDS + PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)
#define PKA_ECPKI_BUILD_TMP_BUFF_MAX_LENGTH_IN_WORDS  \
(3*ECPKI_MODUL_MAX_LENGTH_IN_WORDS+PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)
#define PKA_ECDSA_SIGN_BUFF_MAX_LENGTH_IN_WORDS (6*ECPKI_MODUL_MAX_LENGTH_IN_WORDS+PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define PKA_ECDH_BUFF_MAX_LENGTH_IN_WORDS (2*ECPKI_ORDER_MAX_LENGTH_IN_WORDS + PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)

#define PKA_KG_BUFF_MAX_LENGTH_IN_WORDS (2*ECPKI_ORDER_MAX_LENGTH_IN_WORDS + PKA_ECPKI_SCALAR_MUL_BUFF_MAX_LENGTH_IN_WORDS)
#define PKA_ECDSA_VERIFY_BUFF_MAX_LENGTH_IN_WORDS (3*ECPKI_MODUL_MAX_LENGTH_IN_WORDS)

typedef struct
{
    /*! EC modulus: P. */
    uint32_t ecP [ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    /*! EC equation parameters a, b. */
    uint32_t ecA [ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    uint32_t ecB [ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    /*! Order of generator. */
    uint32_t ecR [ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1];
    /*! EC cofactor EC_Cofactor_K
        Generator (EC base point) coordinates in projective form. */
    uint32_t ecGx [ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    uint32_t ecGy [ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    uint32_t ecH;
    /*! include the specific fields that are used by the low level.*/
    uint32_t llfBuff[PKA_DOMAIN_LLF_BUFF_SIZE_IN_WORDS];
    /*! Size of fields in bits. */
    uint32_t modSizeInBits;
    uint32_t ordSizeInBits;
    /*! Size of each inserted Barret tag in words; 0 - if not inserted.*/
    uint32_t barrTagSizeInWords;
    /*! EC Domain identifier.*/
    ECPKI_DomainID_t DomainID;
    int8_t name[20];
}ECPKI_Domain_t;

#define SEED_MAX_SIZE_WORDS 12
#define BLOCK_SIZE_IN_WORDS 4

typedef struct ECPKI_BUILD_TempData_t
{
    uint32_t crysBuildTmpIntBuff[PKA_ECPKI_BUILD_TMP_BUFF_MAX_LENGTH_IN_WORDS];
}ECPKI_BUILD_TempData_t;

typedef struct ECDH_TempData_t
{
    uint32_t crysEcdhIntBuff[PKA_ECDH_BUFF_MAX_LENGTH_IN_WORDS];
}ECDH_TempData_t;

typedef uint32_t (*RndGenerateVectWorkFunc_t)(  \
    RND_State_t *rndState_ptr,  /*context*/  \
    uint16_t outSizeBytes,  /*in*/ \
    uint8_t *out_ptr    /*out*/);

typedef struct
{
    /*! Public Key coordinates. */
    uint32_t x[ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    uint32_t y[ECPKI_MODUL_MAX_LENGTH_IN_WORDS];
    ECPKI_Domain_t domain;
    uint32_t pointType;
}ECPKI_PublKey_t;

typedef struct ECPKI_UserPublKey_t
{
    uint32_t valid_tag;
    uint32_t PublKeyDbBuff[(sizeof(ECPKI_PublKey_t)+3)/4];
}ECPKI_UserPublKey_t;

typedef enum
{
    USER_SCAP_Inactive,
    USER_SCAP_Active,
    USER_SCAP_OFF_MODE,
    USER_SCAP_LAST = 0x7FFFFFFF
}ECPKI_ScaProtection_t;

typedef struct
{
    /*! Private Key data. */
    uint32_t PrivKey[ECPKI_MODUL_MAX_LENGTH_IN_WORDS + 1];
    ECPKI_Domain_t domain;
    ECPKI_ScaProtection_t scaProtection;
}ECPKI_PrivKey_t;

typedef struct ECPKI_UserPrivKey_t
{
    uint32_t valid_tag;
    uint32_t PrivKeyDbBuff[(sizeof(ECPKI_PrivKey_t)+3)/4];
}ECPKI_UserPrivKey_t;

typedef struct ECPKI_KG_TempData_t
{
    uint32_t crysKGIntBuff[PKA_KG_BUFF_MAX_LENGTH_IN_WORDS];
}ECPKI_KG_TempData_t;

typedef enum
{
    ECPKI_HASH_SHA1_mode    = 0,        /*!< HASH SHA1 mode. */
    ECPKI_HASH_SHA224_mode  = 1,        /*!< HASH SHA224 mode. */
    ECPKI_HASH_SHA256_mode  = 2,        /*!< HASH SHA256 mode. */
    ECPKI_HASH_SHA384_mode  = 3,        /*!< HASH SHA384 mode. */
    ECPKI_HASH_SHA512_mode  = 4,        /*!< HASH SHA512 mode. */

    ECPKI_AFTER_HASH_SHA1_mode    = 5,     /*!< After HASH SHA1 mode (message was already hashed). */
    ECPKI_AFTER_HASH_SHA224_mode  = 6,    /*!< After HASH SHA224 mode (message was already hashed). */
    ECPKI_AFTER_HASH_SHA256_mode  = 7,    /*!< After HASH SHA256 mode (message was already hashed). */
    ECPKI_AFTER_HASH_SHA384_mode  = 8,    /*!< After HASH SHA384 mode (message was already hashed). */
    ECPKI_AFTER_HASH_SHA512_mode  = 9,    /*!< After HASH SHA512 mode (message was already hashed). */

    ECPKI_HASH_NumOfModes,
    ECPKI_HASH_OpModeLast        = 0x7FFFFFFF,
}ECPKI_HASH_OpMode_t;

typedef uint32_t ECDSA_SignIntBuff[PKA_ECDSA_SIGN_BUFF_MAX_LENGTH_IN_WORDS];

typedef struct
{
    /* A user's buffer for the Private Key Object -*/
    ECPKI_UserPrivKey_t ECDSA_SignerPrivKey;

    /*HASH specific data and buffers */
    uint32_t hashUserCtxBuff[sizeof(HASHUserContext_t)];
    HASH_Result_t hashResult;
    uint32_t hashResultSizeWords;
    ECPKI_HASH_OpMode_t hashMode;

    ECDSA_SignIntBuff crysEcdsaSignIntBuff;
}USER_ECDSA_SignContext_t;

typedef enum
{
    USER_CheckPointersAndSizesOnly = 0,   /* Only preliminary input parameters checking */
    USER_ECpublKeyPartlyCheck      = 1,   /* In addition check that EC PubKey is point on curve */
    USER_ECpublKeyFullCheck        = 2,   /* In addition check that EC_GeneratorOrder*PubKey = O */

    USER_PublKeyChecingOffMode,
    USER_EC_PublKeyCheckModeLast  = 0x7FFFFFFF,
}USER_EC_PublKeyCheckMode_t;

/*---------------------------------------------------*/
/*! Enumerator for the EC point compression idetifier. */
typedef enum
{
    EC_PointCompressed     = 2,
    EC_PointUncompressed   = 4,
    EC_PointContWrong      = 5, /* wrong Point Control value */
    EC_PointHybrid         = 6,

    EC_PointCompresOffMode = 8,

    ECPKI_PointCompressionLast = 0x7FFFFFFF,
}ECPKI_PointCompression_t;

typedef struct ECDSA_SignUserContext_t
{
    uint32_t context_buff[(sizeof(USER_ECDSA_SignContext_t)+3)/4];
    uint32_t valid_tag;
}ECDSA_SignUserContext_t;

typedef uint32_t ECDSA_VerifyIntBuff[PKA_ECDSA_VERIFY_BUFF_MAX_LENGTH_IN_WORDS];

typedef struct
{
    /* A user's buffer for the Private Key Object -*/
    ECPKI_UserPublKey_t ECDSA_SignerPublKey;

    /*HASH specific data and buffers */
    uint32_t hashUserCtxBuff[sizeof(HASHUserContext_t)];
    HASH_Result_t hashResult;
    uint32_t hashResultSizeWords;
    ECPKI_HASH_OpMode_t hashMode;
    ECDSA_VerifyIntBuff crysEcdsaVerIntBuff;
}USER_ECDSA_VerifyContext_t;

typedef struct ECDSA_VerifyUserContext_t
{
    uint32_t context_buff[(sizeof(USER_ECDSA_VerifyContext_t)+3)/4];
    uint32_t valid_tag;
}ECDSA_VerifyUserContext_t;

typedef struct ECPKI_KG_FipsContext_t
{
    union
    {
        ECDSA_SignUserContext_t signCtx;
        ECDSA_VerifyUserContext_t verifyCtx;
    }operationCtx;
    uint32_t signBuff[2*ECPKI_ORDER_MAX_LENGTH_IN_WORDS];
}ECPKI_KG_FipsContext_t;

/************************ Functions *****************************/

const ECPKI_Domain_t *asr_ecpki_getEcDomain(ECPKI_DomainID_t domainId/*!< [in] Index of one of the domain Id (must be
 * one of the supported domains). */);

const ECPKI_Domain_t *asr_ecpki_getsecp160k1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp160r1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp160r2DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp192k1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp192r1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp224k1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp224r1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp256k1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp256r1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp384r1DomainP(void);

const ECPKI_Domain_t *asr_ecpki_getsecp521r1DomainP(void);

int asr_ecpki_genkeypair
(
    RND_Context_t *pRndContext,      /*!< [in/out] Pointer to the RND context buffer. */
    const ECPKI_Domain_t *pDomain,          /*!< [in]  Pointer to EC domain (curve). */
    ECPKI_UserPrivKey_t *pUserPrivKey,     /*!< [out] Pointer to the private key structure. This structure is used as input to the ECPKI cryptographic primitives. */
    ECPKI_UserPublKey_t *pUserPublKey,     /*!< [out] Pointer to the public key structure. This structure is used as input to the CPKI cryptographic primitives. */
    ECPKI_KG_TempData_t *pTempData,        /*!< [in] Temporary buffers for internal use, defined in ::CRYS_ECPKI_KG_TempData_t. */
    ECPKI_KG_FipsContext_t *pFipsCtx       /*!< [in] Pointer to temporary buffer used in case FIPS certification if required. */
);

int asr_ecpki_buildprivkey
(
    const ECPKI_Domain_t *pDomain,            /*!< [in] The EC domain (curve). */
    const uint8_t *pPrivKeyIn,         /*!< [in] Pointer to private key data. */
    uint32_t PrivKeySizeInBytes, /*!< [in] Size of private key data (in bytes). */
    ECPKI_UserPrivKey_t *pUserPrivKey        /*!< [out] Pointer to the private key structure. This structure is used as input to the ECPKI cryptographic primitives. */
);

int asr_ecpki_buildpubkey
(
    const ECPKI_Domain_t *pDomain,               /*!< [in]  The EC domain (curve). */
    uint8_t *PublKeyIn_ptr,         /*!< [in]  Pointer to the input public key data, in compressed or uncompressed or hybrid form:
                            [PC||X||Y] Big-Endian representation, structured according to [IEEE1363], where: 
                            <ul><li>X and Y are the public key's EC point coordinates. In compressed form, Y is omitted.</li>
                            <li> The sizes of X and Y are equal to the size of the EC modulus.</li>
                            <li> PC is a one-byte point control that defines the type of point compression. </li></ul>*/
    uint32_t PublKeySizeInBytes,    /*!< [in]  The size of public key data (in bytes).* */
    USER_EC_PublKeyCheckMode_t CheckMode,             /*!< [in]  The required level of public key
    * verification (higher verification level means longer verification time):
                            <ul><li> 0 = preliminary validation. </li>
                            <li> 1 = partial validation. </li>
                            <li> 2 = full validation. </li></ul>*/
    ECPKI_UserPublKey_t *pUserPublKey,          /*!< [out] Pointer to the output public key structure. This structure is used as input to the ECPKI cryptographic primitives. */
    ECPKI_BUILD_TempData_t *pTempBuff              /*!< [in]  Pointer for a temporary buffer required for the build function. */
);

#define asr_ecpki_build_pubkey_fullcheck(pDomain, PublKeyIn_ptr, PublKeySizeInBytes, pUserPublKey,  pTempBuff) \
        asr_ecpki_buildpubkey((pDomain), (PublKeyIn_ptr), (PublKeySizeInBytes), (USER_ECpublKeyFullCheck), (pUserPublKey),  (pTempBuff))

int asr_ecpki_exportpubkey
(
    ECPKI_UserPublKey_t *pUserPublKey,       /*in*/
    ECPKI_PointCompression_t compression,        /*in*/
    uint8_t *pExportPublKey,     /*in*/
    uint32_t *pPublKeySizeBytes   /*in/out*/
);

int asr_ecpki_exportprikey
(
    ECPKI_UserPrivKey_t *pUserPrivKey,       /*in*/
    uint8_t *pExportPrivKey,     /*in*/
    uint32_t *pPrivKeySizeBytes   /*in/out*/
);

int asr_ecdh_svdp_dh
(
    ECPKI_UserPublKey_t *PartnerPublKey_ptr,       /*!< [in]  Pointer to a partner public key. */
    ECPKI_UserPrivKey_t *UserPrivKey_ptr,          /*!< [in]  Pointer to a user private key. */
    uint8_t *SharedSecretValue_ptr,    /*!< [out] Pointer to an output buffer that will contain the shared secret value. */
    uint32_t *SharedSecrValSize_ptr,    /*!< [in/out] Pointer to the size of user-passed buffer (in) and actual size of output of calculated shared secret value(out). */
    ECDH_TempData_t *TempBuff_ptr              /*!< [in]  Pointer to a temporary buffer. */
);

int asr_ecdsa_sign
(
    RND_Context_t *pRndContext,        /*!< [in/out] Pointer to the RND context buffer. */
    ECDSA_SignUserContext_t *pSignUserContext,   /*!< [in/out] Pointer to the user buffer for signing the database. */
    ECPKI_UserPrivKey_t *pSignerPrivKey,    /*!< [in]  A pointer to a user private key structure. */
    ECPKI_HASH_OpMode_t hashMode,           /*!< [in]  One of the supported SHA-x HASH modes, as defined in ::CRYS_ECPKI_HASH_OpMode_t.\note MD5 is not supported. */
    uint8_t *pMessageDataIn,    /*!< [in] Pointer to the input data to be signed.The size of the scatter/gather list representing the data buffer
                                                        is limited to 128 entries, and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB). */
    uint32_t messageSizeInBytes, /*!< [in]  Size of message data in bytes. */
    uint8_t *pSignatureOut,      /*!< [in]  Pointer to a buffer for output of signature. */
    uint32_t *pSignatureOutSize   /*!< [in/out] Pointer to the signature size. Used to pass the size of the SignatureOut buffer (in), which must be >= 2
                                                        * OrderSizeInBytes. When the API returns, it is replaced with the size of the actual signature (out). */
);

int asr_ecdsa_verify
(
    ECDSA_VerifyUserContext_t *pVerifyUserContext, /*!< [in] Pointer to the user buffer for signing the database. */
    ECPKI_UserPublKey_t *pUserPublKey,       /*!< [in] Pointer to a user public key structure. */
    ECPKI_HASH_OpMode_t hashMode,           /*!< [in] One of the supported SHA-x HASH modes, as defined in ::CRYS_ECPKI_HASH_OpMode_t.\note MD5 is not supported. */
    uint8_t *pSignatureIn,       /*!< [in] Pointer to the signature to be verified. */
    uint32_t SignatureSizeBytes, /*!< [in] Size of the signature (in bytes).  */
    uint8_t *pMessageDataIn,     /*!< [in] Pointer to the input data that was signed (same as given to the signing function). The size of the scatter/gather list representing
                                                            the data buffer is limited to 128 entries, and the size of each entry is limited to 64KB (fragments larger than 64KB are broken into fragments <= 64KB). */
    uint32_t messageSizeInBytes  /*!< [in] Size of the input data (in bytes). */
);

int asr_PkaEcWrstScalarMult
(
    const ECPKI_Domain_t *pDomain,
    const uint32_t *scalar,
    uint32_t scalSizeInWords,
    uint32_t *inPointX,
    uint32_t *inPointY,
    uint32_t *outPointX,
    uint32_t *outPointY,
    uint32_t *tmpBuff
);

#endif // _ASR_ECC_H_
