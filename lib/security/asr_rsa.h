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
#ifndef _ASR_RSA_H_
#define _ASR_RSA_H_

#include "asr_sec_hw_common.h"
#include "asr_hash.h"

#define TST_MAX_PRVT_EXP_SIZE_IN_BYTES 256
typedef enum RSAType_enum
{
    NON_CRT_MODE, CRT_MODE, RSATypeLast = 0x7FFFFFFF,
}RSAType_enum;

typedef enum
{
    RSA_HASH_MD5_mode             = 0,    /*!< MD5 mode for PKCS1 v1.5 only. */
    RSA_HASH_SHA1_mode            = 1,    /*!< HASH SHA1. */
    RSA_HASH_SHA224_mode          = 2,  /*!< HASH SHA224. */
    RSA_HASH_SHA256_mode          = 3,  /*!< HASH SHA256. */
    RSA_HASH_SHA384_mode          = 4,  /*!< HASH SHA384. */
    RSA_HASH_SHA512_mode          = 5,    /*!< HASH SHA512. */
    RSA_After_MD5_mode            = 6,        /*!< For PKCS1 v1.5 only when the data is already hashed with MD5. */
    RSA_After_SHA1_mode           = 7,    /*!< To be used when the data is already hashed with SHA1. */
    RSA_After_SHA224_mode         = 8,    /*!< To be used when the data is already hashed with SHA224. */
    RSA_After_SHA256_mode         = 9,    /*!< To be used when the data is already hashed with SHA256. */
    RSA_After_SHA384_mode         = 10,    /*!< To be used when the data is already hashed with SHA384. */
    RSA_After_SHA512_mode         = 11,    /*!< To be used when the data is already hashed with SHA512. */
    RSA_After_HASH_NOT_KNOWN_mode = 12,    /*!< \internal used only for PKCS#1 Ver 1.5 - possible to perform verify operation without hash mode input,
                        the hash mode is derived from the signature.*/
    RSA_HASH_NO_HASH_mode         = 13,    /*!< Used for PKCS1 v1.5 Encrypt and Decrypt.*/
    RSA_HASH_NumOfModes,

    RSA_HASH_OpModeLast = 0x7FFFFFFF,

}RSA_HASH_OpMode_t;

#define PKA_WORD_SIZE_IN_BITS 64

#define SRP_MAX_MODULUS_SIZE_IN_BITS 3072

#define RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS 2048

#define RSA_MAX_KEY_GENERATION_HW_SIZE_BITS 2048

/* PKA operations maximal count of extra bits: */
#define PKA_EXTRA_BITS 8
#define PKA_MAX_COUNT_OF_PHYS_MEM_REGS 32
#define PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS 5
#define RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS ((RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS + PKA_WORD_SIZE_IN_BITS) / BITS_IN_32BIT_WORD )
#define PKA_PRIV_KEY_BUFF_SIZE_IN_WORDS (2*PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS)
#define RSA_OAEP_MAX_HLEN HASH_SHA512_DIGEST_SIZE_IN_BYTES
#define RSA_OAEP_ENCODE_MAX_SEEDMASK_SIZE RSA_OAEP_MAX_HLEN
#define RSA_PSS_SALT_LENGTH RSA_OAEP_MAX_HLEN
#define RSA_PSS_PAD1_LEN 8

#define RSA_OAEP_ENCODE_MAX_MASKDB_SIZE (RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*sizeof(uint32_t)) /*!< \internal For OAEP Encode; the max size is emLen */
#define RSA_OAEP_DECODE_MAX_DBMASK_SIZE (RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*sizeof(uint32_t)) /*!< \internal For OAEP Decode; the max size is emLen */
#define PKA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS
#define PKA_KGDATA_BUFF_SIZE_IN_WORDS (3*PKA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS)
#define PKA_RSA_HASH_CTX_SIZE_IN_WORDS HASH_USER_CTX_SIZE_IN_WORDS
#define RSA_TMP_BUFF_SIZE (RSA_OAEP_ENCODE_MAX_MASKDB_SIZE + RSA_OAEP_ENCODE_MAX_SEEDMASK_SIZE + PKA_RSA_HASH_CTX_SIZE_IN_WORDS*sizeof(uint32_t) + sizeof(HASH_Result_t))
#define RSA_MIN_VALID_KEY_SIZE_VALUE_IN_BITS 512
#define RSA_FIPS_KEY_SIZE_1024_BITS 1024
#define RSA_FIPS_KEY_SIZE_2048_BITS 2048
#define RSA_FIPS_KEY_SIZE_3072_BITS 3072
#define RSA_FIPS_MODULUS_SIZE_BITS RSA_FIPS_KEY_SIZE_2048_BITS
/*! Make given data item aligned (alignment in bytes). */

#define PAL_COMPILER_ALIGN(alignement) __attribute__((aligned(alignement)))
#define RSA_SIZE_OF_T_STRING_BYTES (RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*sizeof(uint32_t))

typedef enum
{
    RSA_NoCrt = 10,

    RSA_Crt = 11,

    RSADecryptionNumOfOptions,

    RSA_DecryptionModeLast = 0x7FFFFFFF,
}RSA_DecryptionMode_t;

/* the Key source enum */
typedef enum
{
    RSA_ExternalKey = 1,

    RSA_InternalKey = 2,

    RSA_KeySourceLast = 0x7FFFFFFF,

}RSA_KeySource_t;

typedef enum
{
    PKCS1_MGF1 = 0,

    PKCS1_NO_MGF = 1,

    RSA_NumOfMGFFunctions,

    PKCS1_MGFLast = 0x7FFFFFFF,
}PKCS1_MGF_t;

typedef enum
{
    PKCS1_VER15 = 0,

    PKCS1_VER21 = 1,

    RSA_NumOf_PKCS1_versions,

    PKCS1_versionLast = 0x7FFFFFFF,
}PKCS1_version;

/* The private key on non-CRT mode data structure */
typedef struct
{
    /* The RSA private exponent buffer and its size in bits */
    uint32_t d[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t dSizeInBits;

    /* The RSA public exponent buffer and its size in bits */
    uint32_t e[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t eSizeInBits;

}RSAPrivNonCRTKey_t;

typedef struct
{
    /* The first factor buffer and size in bits */
    uint32_t P[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
    uint32_t PSizeInBits;

    /* The second factor buffer and its size in bits */
    uint32_t Q[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
    uint32_t QSizeInBits;

    /* The first CRT exponent buffer and its size in bits */
    uint32_t dP[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
    uint32_t dPSizeInBits;

    /* The second CRT exponent buffer and its size in bits */
    uint32_t dQ[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
    uint32_t dQSizeInBits;

    /* The first CRT coefficient buffer and its size in bits */
    uint32_t qInv[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
    uint32_t qInvSizeInBits;
}RSAPrivCRTKey_t;

typedef struct
{
    /* The RSA modulus buffer and its size in bits */
    uint32_t n[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t nSizeInBits;

    /* The decryption operation mode */
    RSA_DecryptionMode_t OperationMode;

    /* the source ( Import or Keygen ) */
    RSA_KeySource_t KeySource;

    /* The union between the CRT and non-CRT data structures */
    union
    {
        RSAPrivNonCRTKey_t NonCrt;
        RSAPrivCRTKey_t Crt;
    }PriveKeyDb;

    /* #include specific fields that are used by the low level */
    uint32_t RSAPrivKeyIntBuff[PKA_PRIV_KEY_BUFF_SIZE_IN_WORDS];
}RSAPrivKey_t;

typedef struct RSAUserPrivKey_t
{
    uint32_t valid_tag;
    uint32_t PrivateKeyDbBuff[sizeof(RSAPrivKey_t) / sizeof(uint32_t) + 1];
}RSAUserPrivKey_t;

typedef struct
{
    /* The RSA modulus buffer and its size in bits */
    uint32_t n[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t nSizeInBits;

    /* The RSA public exponent buffer and its size in bits */
    uint32_t e[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t eSizeInBits;

    /* #include the specific fields that are used by the low level */
    uint32_t RSAIntBuff[PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS];
}RSAPubKey_t;

typedef struct RSAUserPubKey_t
{
    uint32_t valid_tag;
    uint32_t PublicKeyDbBuff[sizeof(RSAPubKey_t) / sizeof(uint32_t) + 1];
}RSAUserPubKey_t;
/*! Temporary buffers for RSA usage. */

typedef struct RSAPrimeData_t
{
    /* The aligned input and output data buffers */
    uint32_t DataIn[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t DataOut[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];

    uint8_t InternalBuff[RSA_TMP_BUFF_SIZE] PAL_COMPILER_ALIGN (4);
}RSAPrimeData_t;

/* the KG data type */
typedef union RSAKGData_t
{
    struct
    {
        /* The aligned input and output data buffers */
        uint32_t p[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
        uint32_t q[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS / 2];
        union
        {
            /* #include specific fields that are used by the low level */
            uint32_t RSAKGDataIntBuff[PKA_KGDATA_BUFF_SIZE_IN_WORDS];
        }kg_buf;
    }KGData;

    union
    {
        struct
        {
            RSAPrimeData_t PrimData;
        }primExt;
    }prim;
}RSAKGData_t;

/*! Required for internal FIPS verification for RSA key generation. */
typedef struct RSAKGFipsContext_t
{
    RSAPrimeData_t primData;
    uint8_t decBuff[((RSA_MIN_VALID_KEY_SIZE_VALUE_IN_BITS / BITS_IN_BYTE) -
                                 2 * (HASH_SHA1_DIGEST_SIZE_IN_BYTES) - 2)];
    uint8_t encBuff[RSA_FIPS_MODULUS_SIZE_BITS / BITS_IN_BYTE];
}RSAKGFipsContext_t;

typedef struct
{
    /* A union for the Key Object - there is no need for the Private
    key and the Public key to be in the memory at the same time */
    RSAUserPrivKey_t PrivUserKey;

    /* RSA PKCS#1 Version 1.5/2.1 */
    uint8_t PKCS1_Version;

    /* MGF 2 use for the PKCS1 Ver 2.1 Sign/Verify operation */
    uint8_t MGF_2use;

    /*The Salt random intended length for PKCS#1 PSS Ver 2.1*/
    uint16_t SaltLen;

    /*Struct for the Exp evaluation*/
    RSAPrimeData_t PrimeData;

    /* User Context of the Hash Context - Hash functions get as input a CRYS_HASHUserContext_t */
    //CRYS_HASHUserContext_t  HashUserContext;
    uint32_t CRYSPKAHashCtxBuff[PKA_RSA_HASH_CTX_SIZE_IN_WORDS];
    HASH_Result_t HASH_Result;
    uint16_t HASH_Result_Size;     /*in words*/
    RSA_HASH_OpMode_t RsaHashOperationMode; /*RSA HASH enum. */
    HASH_OperationMode_t HashOperationMode;    /*CRYS HASH enum. */
    uint16_t HashBlockSize;        /*in words*/
    uint8_t doHash;

    /* Used for sensitive data manipulation in the context space, which is safer and which saves stack space */
    uint32_t EBD[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t EBDSizeInBits;

    /* Used for sensitive data manipulation in the context space, which is safer and which saves stack space */
    uint8_t T_Buf[RSA_SIZE_OF_T_STRING_BYTES];
    uint16_t T_BufSize;

    /*Buffer for the use of the Ber encoder in the case of PKCS#1 Ver 1.5 - in Private context only*/
    uint32_t BER[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint16_t BERSize;

    /* This Buffer is added for encrypting the context. Note: This block must be
    at the end of the context. */
    uint8_t DummyBufAESBlockSize[16];
}USER_RSAPrivContext_t;

typedef struct RSAPrivUserContext_t
{
    uint32_t valid_tag;
    uint32_t AES_iv; /* For the use of the AES CBC mode of Encryption and Decryption of the context in CCM */
    uint8_t context_buff[sizeof(USER_RSAPrivContext_t) + sizeof(uint32_t)] PAL_COMPILER_ALIGN (4);  /* must be aligned to 4 */
}RSAPrivUserContext_t;

typedef struct
{
    /* A union for the Key Object - there is no need for the Private
    key and the Public key to be in the memory in the same time */
    RSAUserPubKey_t PubUserKey;

    /* public key size in bytes */
    uint32_t nSizeInBytes;

    /* RSA PKCS#1 Version 1.5/2.1 */
    uint8_t PKCS1_Version;

    /* MGF 2 use for the PKCS1 Ver 2.1 Sign/Verify operation */
    uint8_t MGF_2use;

    /*The Salt random intended length for PKCS#1 PSS Ver 2.1*/
    uint16_t SaltLen;

    /*Struct for the Exp evaluation*/
    RSAPrimeData_t PrimeData;

    /* User Context of the Hash Context - Hash functions get as input a _HASHUserContext_t */
    uint32_t CRYSPKAHashCtxBuff[PKA_RSA_HASH_CTX_SIZE_IN_WORDS];

    HASH_Result_t HASH_Result;
    uint16_t HASH_Result_Size; /* denotes the length, in words, of the hash function output */
    RSA_HASH_OpMode_t RsaHashOperationMode; /*RSA HASH enum. */
    HASH_OperationMode_t HashOperationMode;    /*CRYS HASH enum. */
    uint16_t HashBlockSize; /*in words*/
    uint8_t doHash;

    /* Used for sensitive data manipulation in the context space, which is safer and which saves stack space */
    uint32_t EBD[RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
    uint32_t EBDSizeInBits;

    /* Used for sensitive data manipulation in the context space, which is safer and which saves stack space */
    uint8_t T_Buf[RSA_SIZE_OF_T_STRING_BYTES];
    uint16_t T_BufSize;

    /* This Buffer is added for encrypting the context ( encrypted part's size must be 0 modulo 16).
    * Note: This block must be at the end of the context.
    */
    uint8_t DummyBufAESBlockSize[16];
}USER_RSAPubContext_t;

/*! Temporary buffers for the RSA usage. */
typedef struct RSAPubUserContext_t
{
    uint32_t valid_tag;
    uint32_t AES_iv; /* For the use of the AES CBC mode of Encryption and Decryption of the context in CCM */
    uint32_t context_buff[sizeof(USER_RSAPubContext_t)/sizeof(uint32_t) + 1] ;
}RSAPubUserContext_t;

/************************ Functions *****************************/

int asr_rsa_keypair_gen
(
    RND_Context_t *rndContext_ptr, uint8_t *pubExp_ptr, uint16_t pubExpSizeInBytes,
    uint32_t keySize, RSAUserPrivKey_t *userPrivKey_ptr, RSAUserPubKey_t *userPubKey_ptr,
    RSAKGData_t *keyGenData_ptr, RSAKGFipsContext_t *pFipsCtx
);

int asr_rsa_keypair_crt_gen
(
    RND_Context_t *rndContext_ptr,        /*!< [in/out] Pointer to the RND context
                                            * buffer. */
    uint8_t *pubExp_ptr,           /*!< [in]  The pointer to the public exponent (public key). */
    uint16_t pubExpSizeInBytes,    /*!< [in]  The public exponent size in bytes. */
    uint32_t keySize,              /*!< [in]  The size of the key, in bits. Supported sizes are
                                                                        512, 1024, 2048, 3072 and 4096 bit. */
    RSAUserPrivKey_t *userPrivKey_ptr,    /*!< [out] Pointer to the private key structure. */
    RSAUserPubKey_t *userPubKey_ptr,     /*!< [out] Pointer to the public key structure. */
    RSAKGData_t *keyGenData_ptr,     /*!< [in] Pointer to a temporary structure required for the KeyGen operation. */
    RSAKGFipsContext_t *pFipsCtx       /*!< [in] Pointer to temporary buffer used in case FIPS certification if required. */
);

int asr_rsa_build_pubkey
(
    RSAUserPubKey_t *UserPubKey_ptr,   /*!< [out] Pointer to the public key structure. */
    uint8_t *Exponent_ptr,                  /*!< [in]  Pointer to the exponent stream of bytes (Big-Endian format). */
    uint16_t ExponentSize,                  /*!< [in]  The size of the exponent (in bytes). */
    uint8_t *Modulus_ptr,                   /*!< [in]  Pointer to the modulus stream of bytes (Big-Endian format).
                        The most significant bit (MSB) must be set to '1'. */
    uint16_t ModulusSize                    /*!< [in]  The modulus size in bytes. Supported sizes are 64, 128, 256, 384 and 512. */
);

int asr_rsa_build_prikey
(
    RSAUserPrivKey_t *UserPrivKey_ptr,   /*!< [out] Pointer to the public key structure.*/
    uint8_t *PrivExponent_ptr,    /*!< [in]  Pointer to the private exponent stream of bytes (Big-Endian format). */
    uint16_t PrivExponentSize,    /*!< [in]  The size of the private exponent (in bytes). */
    uint8_t *PubExponent_ptr,     /*!< [in]  Pointer to the public exponent stream of bytes (Big-Endian format). */
    uint16_t PubExponentSize,     /*!< [in]  The size of the public exponent (in bytes). */
    uint8_t *Modulus_ptr,         /*!< [in]  Pointer to the modulus stream of bytes (Big-Endian format).
                    The most significant bit must be set to '1'. */
    uint16_t ModulusSize          /*!< [in]  The modulus size in bytes. Supported sizes are 64, 128, 256, 384 and 512. */
);

int asr_rsa_build_prikey_crt
(
    RSAUserPrivKey_t *UserPrivKey_ptr,   /*!< [out] Pointer to the public key structure. */
    uint8_t *P_ptr,                           /*!< [in]  Pointer to the first factor stream of bytes (Big-Endian format). */
    uint16_t PSize,                           /*!< [in]  The size of the first factor (in bytes). */
    uint8_t *Q_ptr,                           /*!< [in]  Pointer to the second factor stream of bytes (Big-Endian format). */
    uint16_t QSize,                           /*!< [in]  The size of the second factor (in bytes). */
    uint8_t *dP_ptr,                          /*!< [in]  Pointer to the first factor's CRT exponent stream of bytes
                        (Big-Endian format). */
    uint16_t dPSize,                          /*!< [in]  The size of the first factor's CRT exponent (in bytes). */
    uint8_t *dQ_ptr,                          /*!< [in]  Pointer to the second factor's CRT exponent stream of bytes
                        (Big-Endian format). */
    uint16_t dQSize,                          /*!< [in]  The size of the second factor's CRT exponent (in bytes). */
    uint8_t *qInv_ptr,                        /*!< [in]  Pointer to the first CRT coefficient stream of bytes (Big-Endian format). */
    uint16_t qInvSize                         /*!< [in]  The size of the first CRT coefficient (in bytes). */
);

int asr_rsa_get_pubkey
(
    RSAUserPubKey_t *UserPubKey_ptr,   /*!< [in] A pointer to the public key structure. */
    uint8_t *Exponent_ptr,                 /*!< [out] A pointer to the exponent stream of bytes (Big-Endian format). */
    uint16_t *ExponentSize_ptr,             /*!< [in/out] the size of the exponent buffer in bytes,
                    it is updated to the actual size of the exponent, in bytes. */
    uint8_t *Modulus_ptr,                  /*!< [out] A pointer to the modulus stream of bytes (Big-Endian format).
                The MS (most significant) bit must be set to '1'. */
    uint16_t *ModulusSize_ptr               /*!< [in/out] the size of the modulus buffer in bytes, it is updated to the actual
                    size of the modulus, in bytes. */
);

int asr_rsa_sign
(
    RND_Context_t *rndContext_ptr,              /*!< [in/out] Pointer to the RND context buffer. */
    RSAPrivUserContext_t *UserContext_ptr,      /*!< [in]  Pointer to a temporary context for internal use. */
    RSAUserPrivKey_t *UserPrivKey_ptr,          /*!< [in]  Pointer to the private key data structure of the user.
                                                                The representation (pair or quintuple) and hence the algorithm (CRT or not CRT)
                            is determined by the Private Key build function -
                                                                ::CRYS_RSA_Build_PrivKey or ::CRYS_RSA_Build_PrivKeyCRT. */
    RSA_HASH_OpMode_t rsaHashMode,              /*!< [in]  One of the supported SHA-x HASH modes, as defined in ::CRYS_RSA_HASH_OpMode_t.
                            (MD5 is not supported). */
    PKCS1_MGF_t MGF,                            /*!< [in]  The mask generation function. [PKCS1_2.1] defines only MGF1, so the only value
                            allowed for [PKCS1_2.1] is CRYS_PKCS1_MGF1. */
    uint16_t SaltLen,                            /*!< [in]  The Length of the Salt buffer (relevant for PKCS#1 Ver 2.1 only, typically lengths is 0 or hash Len).
                                                                FIPS 186-4 requires, that SaltLen <= hash len. If SaltLen > KeySize - hash Len - 2, the function
                                                                returns an error.  */
    uint8_t *DataIn_ptr,                         /*!< [in]  Pointer to the input data to be signed.
                                                                The size of the scatter/gather list representing the data buffer is limited to 128
                            entries, and the size of each entry is limited to 64KB (fragments larger than
                            64KB are broken into fragments <= 64KB). */
    uint32_t DataInSize,                         /*!< [in]  The size (in bytes) of the data to sign. */
    uint8_t *Output_ptr,                         /*!< [out] Pointer to the signature. The buffer must be at least PrivKey_ptr->N.len bytes
                            long (i.e. the modulus size in bytes). */
    uint16_t *OutputSize_ptr,                     /*!< [in/out] Pointer to the signature size value - the input value is the signature
                            buffer size allocated, the output value is the signature size used.
                                                                    he buffer must be equal to PrivKey_ptr->N.len bytes long
                            (i.e. the modulus size in bytes). */
    PKCS1_version PKCS1_ver                     /*!< [in]  [PKCS1_1.5] or [PKCS1_2.1], according to the functionality required. */
);

#define asr_rsa_pss_sign(rndContext_ptr, UserContext_ptr,UserPrivKey_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
        asr_rsa_sign(rndContext_ptr, UserContext_ptr,UserPrivKey_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr,PKCS1_VER21)
#define asr_rsa_pkcs1v15_sign(rndContext_ptr, UserContext_ptr,UserPrivKey_ptr,hashFunc,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
        asr_rsa_sign(rndContext_ptr, (UserContext_ptr),(UserPrivKey_ptr),(hashFunc),(PKCS1_NO_MGF),0,(DataIn_ptr),(DataInSize),(Output_ptr),(OutputSize_ptr),PKCS1_VER15)

int asr_rsa_verify
(
    RSAPubUserContext_t *UserContext_ptr,      /*!< [in]  Pointer to a temporary context for internal use. */
    RSAUserPubKey_t *UserPubKey_ptr,           /*!< [in]  Pointer to the public key data structure of the user. */
    RSA_HASH_OpMode_t rsaHashMode,             /*!< [in]  One of the supported SHA-x HASH modes, as defined in ::CRYS_RSA_HASH_OpMode_t.
                                            (MD5 is not supported). */
    PKCS1_MGF_t MGF,                           /*!< [in]  The mask generation function. [PKCS1_2.1] defines only MGF1, so the only
                                            value allowed for [PKCS_2.1] is CRYS_PKCS1_MGF1. */
    uint16_t SaltLen,                               /*!< [in]  The Length of the Salt buffer. Relevant only for [PKCS1_2.1].
                                            Typical lengths are 0 or hash Len (20 for SHA-1).
                                                                                    The maximum length allowed is [modulus size - hash Len - 2]. */
    uint8_t *DataIn_ptr,                        /*!< [in]  Pointer to the input data to be verified.
                                                                                    The size of the scatter/gather list representing the data buffer is
                                        limited to 128 entries, and the size of each entry is limited to 64KB
                                        (fragments larger than 64KB are broken into fragments <= 64KB). */
    uint32_t DataInSize,                        /*!< [in]  The size (in bytes) of the data whose signature is to be verified. */
    uint8_t *Sig_ptr,                           /*!< [in]  Pointer to the signature to be verified.
                                                                                    The length of the signature is PubKey_ptr->N.len bytes
                                            (i.e. the modulus size in bytes). */
    PKCS1_version PKCS1_ver                    /*!< [in]  [PKCS1_1.5] or [PKCS1_2.1], according to the functionality required. */
);

#define asr_rsa_pss_verify(UserContext_ptr,UserPubKey_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Sig_ptr)\
        asr_rsa_verify(UserContext_ptr,UserPubKey_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Sig_ptr,PKCS1_VER21)
#define asr_rsa_pkcs1v15_verify(UserContext_ptr,UserPubKey_ptr,hashFunc,DataIn_ptr,DataInSize,Sig_ptr)\
        asr_rsa_verify(UserContext_ptr,UserPubKey_ptr,hashFunc,PKCS1_NO_MGF,0,DataIn_ptr,DataInSize,Sig_ptr,PKCS1_VER15)

int asr_rsa_encrypt
(
    RND_Context_t *rndContext_ptr,         /*!< [in/out] Pointer to the RND context buffer. */
    RSAUserPubKey_t *UserPubKey_ptr,      /*!< [in]  Pointer to the public key data structure. */
    RSAPrimeData_t *PrimeData_ptr,       /*!< [in]  Pointer to a temporary structure that is internally used as workspace for the
                        Encryption operation. */
    RSA_HASH_OpMode_t hashFunc,            /*!< [in]  The HASH function to be used. One of the supported SHA-x HASH modes, as defined
                        in ::CRYS_RSA_HASH_OpMode_t (MD5 is not supported).*/
    uint8_t *L,                                /*!< [in]  The label input pointer. Relevant for [PKCS1_2.1] only. NULL by default.
                        NULL for [PKCS1_1.5]. */
    uint16_t Llen,                             /*!< [in]  The label length. Relevant for [PKCS1_2.1] only. Zero by default.
                        Must be <=2048. Zero for [PKCS1_1.5]. */
    PKCS1_MGF_t MGF,                       /*!< [in]  The mask generation function. [PKCS1_2.1] defines MGF1, so the only value
                        allowed here is CRYS_PKCS1_MGF1. */
    uint8_t *DataIn_ptr,                      /*!< [in]  Pointer to the data to encrypt. */
    uint16_t DataInSize,                      /*!< [in]  The size (in bytes) of the data to encrypt. The data size must be:
                                                            - For [PKCS1_2.1], DataSize <= modulus size - 2*HashLen - 2.
                                                            - For [PKCS1_1.5], DataSize <= modulus size - 11. */
    uint8_t *Output_ptr,                      /*!< [out] Pointer to the encrypted data. The buffer must be at least modulus size bytes long. */
    PKCS1_version PKCS1_ver               /*!< [in]  [PKCS1_1.5] or [PKCS1_2.1], according to the functionality required. */
);

#define asr_rsa_oaep_encrypt(rndContext_ptr, UserPubKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr)\
        asr_rsa_encrypt(rndContext_ptr, UserPubKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,PKCS1_VER21)

#define asr_rsa_pkcs1v15_encrypt(rndContext_ptr, UserPubKey_ptr,PrimeData_ptr,DataIn_ptr,DataInSize,Output_ptr)\
        asr_rsa_encrypt(rndContext_ptr, UserPubKey_ptr,PrimeData_ptr,RSA_HASH_NO_HASH_mode,NULL,0,PKCS1_NO_MGF,DataIn_ptr, DataInSize, Output_ptr, PKCS1_VER15)

int asr_rsa_decrypt
(
    RSAUserPrivKey_t *UserPrivKey_ptr,    /*!< [in]  Pointer to the private key data structure of the user. */
    RSAPrimeData_t *PrimeData_ptr,      /*!< [in]  Pointer to a temporary structure that is internally used as workspace
                                            for the decryption operation. */
    RSA_HASH_OpMode_t hashFunc,           /*!< [in]  The HASH function to be used. One of the supported SHA-x HASH modes,
                                            as defined in ::CRYS_RSA_HASH_OpMode_t (MD5 is not supported). */
    uint8_t *L,                 /*!< [in]  The label input pointer. Relevant for [PKCS1_2.1] only. NULL by default.
                                            NULL for [PKCS1_1.5]. */
    uint16_t Llen,                 /*!< [in]  The label length. Relevant for [PKCS1_2.1] only. Zero by default.
                                            Zero for [PKCS1_1.5]. */
    PKCS1_MGF_t MGF,                /*!< [in]  The mask generation function. [PKCS1_2.1] defines MGF1, so the only
                                            value allowed here is CRYS_PKCS1_MGF1. */
    uint8_t *DataIn_ptr,           /*!< [in]  Pointer to the data to decrypt. */
    uint16_t DataInSize,           /*!< [in]  The size (in bytes) of the data to decrypt. DataSize must be &le;
                                            the modulus size. */
    uint8_t *Output_ptr,           /*!< [in]  Pointer to the decrypted data. The buffer must be at least
                                            PrivKey_ptr->N.len bytes long (i.e. the modulus size in bytes). */
    uint16_t *OutputSize_ptr,       /*!< [in]  Pointer to the byte size of the buffer pointed to by Output_buffer.
                                            The size must be:
                                                                                    <ul><li> For PKCS #1 v2.1: Modulus size > OutputSize >=
                                            (modulus size - 2*HashLen - 2).</li>
                                                                                    <li> For PKCS #1 v1.5: Modulus size > OutputSize >= (modulus size - 11).
                                                                                    The value pointed by OutputSize_ptr is updated after decryption with
                                        the actual number of bytes that are loaded to Output_ptr.</li></ul> */
    PKCS1_version PKCS1_ver           /*!< [in]  [PKCS1_1.5] or [PKCS1_2.1], according to the functionality required. */
);

#define asr_rsa_oaep_decrypt(UserPrivKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,OutputSize_ptr)\
        asr_rsa_decrypt(UserPrivKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,OutputSize_ptr,PKCS1_VER21)

#define asr_rsa_pkcs1v15_decrypt(UserPrivKey_ptr,PrimeData_ptr,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
        asr_rsa_decrypt(UserPrivKey_ptr,PrimeData_ptr,RSA_HASH_NO_HASH_mode,NULL,0,PKCS1_NO_MGF,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr,PKCS1_VER15)

#endif //_ASR_RSA_H_