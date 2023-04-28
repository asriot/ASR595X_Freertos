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

#ifndef MBEDTLS_HW_H
#define MBEDTLS_HW_H

#include <string.h>

// #ifdef MBEDTLS_HW

// #ifdef MBEDTLS_HW_RSA

#include "mbedtls/pk.h"

#define HW_RSA_OK 0
#define HW_RSA_ERROR -1
#define HW_RSA_NOT_SUPPORT_THE_HASH -2
#define HW_RSA_NOT_SUPPORT_THE_KEYSIZE -3

int mbedtls_hw_rsa_pkcs1_v15_verify(mbedtls_rsa_context *ctx,
                            mbedtls_md_type_t md_alg,
                            unsigned int hashlen,
                            const unsigned char *hash,
                            const unsigned char *sig);

int mbedtls_hw_rsa_pkcs1_v15_sign(mbedtls_rsa_context *ctx,
                            mbedtls_md_type_t md_alg,
                            unsigned int hashlen,
                            const unsigned char *hash,
                            unsigned char *sig);

int mbedtls_hw_rsa_pkcs1_v15_decrypt(mbedtls_rsa_context *ctx,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t osize);

int mbedtls_hw_rsa_pkcs1_v15_encrypt(mbedtls_rsa_context *ctx,
                            size_t ilen,
                            const unsigned char *input,
                            unsigned char *output);

int mbedtls_hw_rsa_oaep_verify(mbedtls_rsa_context *ctx,
                            mbedtls_md_type_t md_alg,
                            unsigned int hashlen,
                            const unsigned char *hash,
                            const unsigned char *sig);

int mbedtls_hw_rsa_oaep_sign(mbedtls_rsa_context *ctx,
                            mbedtls_md_type_t md_alg,
                            unsigned int hashlen,
                            const unsigned char *hash,
                            unsigned char *sig);

int mbedtls_hw_rsa_oaep_decrypt(mbedtls_rsa_context *ctx,
                            size_t *olen,
                            const unsigned char *input,
                            unsigned char *output,
                            size_t osize);

int mbedtls_hw_rsa_oaep_encrypt(mbedtls_rsa_context *ctx,
                            size_t ilen,
                            const unsigned char *input,
                            unsigned char *output);
// #endif /*MBEDTLS_HW_RSA*/
// #ifdef MBEDTLS_HW_ECC

#include "mbedtls/ecp.h"
#include "mbedtls/ecdsa.h"

#define CRYPTO_ECC_SECP192K1_RAW_PRIVATE_KEY_SIZE  (192 / 8)      //< @brief Raw private key size for secp192k1 (Koblitz 192-bit).
#define CRYPTO_ECC_SECP192R1_RAW_PRIVATE_KEY_SIZE  (192 / 8)      //< @brief Raw private key size for secp192r1 (NIST 192-bit).

#define CRYPTO_ECC_SECP224K1_RAW_PRIVATE_KEY_SIZE  (224 / 8)      //< @brief Raw private key size for secp224k1 (Koblitz 224-bit).
#define CRYPTO_ECC_SECP224R1_RAW_PRIVATE_KEY_SIZE  (224 / 8)      //< @brief Raw private key size for secp224r1 (NIST 224-bit).

#define CRYPTO_ECC_SECP256K1_RAW_PRIVATE_KEY_SIZE  (256 / 8)      //< @brief Raw private key size for secp256k1 (Koblitz 256-bit)
#define CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE  (256 / 8)      //< @brief Raw private key size for secp256r1 (NIST 256-bit).

#define CRYPTO_ECC_SECP384R1_RAW_PRIVATE_KEY_SIZE  (384 / 8)      //< @brief Raw private key size for secp384r1 (NIST 384-bit).
#define CRYPTO_ECC_SECP521R1_RAW_PRIVATE_KEY_SIZE  (528 / 8)      //< @brief Raw private key size for secp521r1 (NIST 521-bit).

#define CRYPTO_ECC_SECP192K1_RAW_PUBLIC_KEY_SIZE   (2 * 192 / 8)  //< @brief Raw public key size for curve secp192k1 (Koblitz 192-bit).
#define CRYPTO_ECC_SECP192R1_RAW_PUBLIC_KEY_SIZE   (2 * 192 / 8)  //< @brief Raw public key size for curve secp192r1 (NIST 192-bit).

#define CRYPTO_ECC_SECP224K1_RAW_PUBLIC_KEY_SIZE   (2 * 224 / 8)  //< @brief Raw public key size for curve secp224k1 (Koblitz 224-bit).
#define CRYPTO_ECC_SECP224R1_RAW_PUBLIC_KEY_SIZE   (2 * 224 / 8)  //< @brief Raw public key size for curve secp224r1 (NIST 224-bit).

#define CRYPTO_ECC_SECP256K1_RAW_PUBLIC_KEY_SIZE   (2 * 256 / 8)  //< @brief Raw public key size for curve secp256k1 (Koblitz 256-bit).
#define CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE   (2 * 256 / 8)  //< @brief Raw public key size for curve secp256r1 (NIST 256-bit).

#define CRYPTO_ECC_SECP384R1_RAW_PUBLIC_KEY_SIZE   (2 * 384 / 8)  //< @brief Raw public key size for curve secp384r1 (NIST 384-bit).
#define CRYPTO_ECC_SECP521R1_RAW_PUBLIC_KEY_SIZE   (2 * 528 / 8)  //< @brief Raw public key size for curve secp521r1 (NIST 521-bit).

int mbedtls_hw_ecdsa_sign(mbedtls_ecp_group *grp, mbedtls_mpi *r, mbedtls_mpi *s,
                            const mbedtls_mpi *d, const unsigned char *hash, size_t hlen);

int mbedtls_hw_ecdsa_verify(mbedtls_ecp_group *grp,
                            const unsigned char *hash, size_t hlen,
                            const mbedtls_ecp_point *Q, const mbedtls_mpi *r, const mbedtls_mpi *s);

int mbedtls_hw_ecdh_compute_shared(mbedtls_ecp_group *grp, mbedtls_mpi *z,
                            const mbedtls_ecp_point *Q, const mbedtls_mpi *d);

int mbedtls_hw_ecp_gen_keypair(mbedtls_ecp_group *grp,
                            mbedtls_mpi *d, mbedtls_ecp_point *Q);

int mbedtls_ecp_hw_mul(mbedtls_ecp_group *grp, mbedtls_ecp_point *R,
                            const mbedtls_mpi *m, const mbedtls_ecp_point *P);

// #endif /*MBEDTLS_HW_ECC*/

// #ifdef MBEDTLS_HW_AES

#include "mbedtls/aes.h"

#define MBEDTLS_HW_AES_ECB_MODE 1
#define MBEDTLS_HW_AES_CBC_MODE 2

int mbedtls_hw_aes_init(mbedtls_aes_context *ctx);

int mbedtls_hw_aes_deinit(mbedtls_aes_context *ctx);

int mbedtls_hw_aes_setkey(mbedtls_aes_context *ctx, const unsigned char *key,
                            unsigned int keybits);

int mbedtls_hw_aes_crypt(mbedtls_aes_context *ctx,
                            int mode,
                            int AES_MODE,
                            size_t length,
                            unsigned char iv[16],
                            const unsigned char *input,
                            unsigned char *output);

// #endif /*MBEDTLS_HW_AES*/

// #ifdef MBEDTLS_HW_HASH

#define MBEDTLS_HW_HASH_SHA1 1
#define MBEDTLS_HW_HASH_SHA224 2
#define MBEDTLS_HW_HASH_SHA256 3

int mbedtls_hw_hash_init(void *ctx, int hash_type);

int mbedtls_hw_hash_deinit(void *ctx, int hash_type);

int mbedtls_hw_hash_clone(void *dst, void *src, int hash_type);

int mbedtls_hw_hash_start(void *ctx, int hash_type);

int mbedtls_hw_hash_update(void *ctx, int hash_type, const unsigned char *input, size_t ilen);

int mbedtls_hw_hash_finish(void *ctx, int hash_type, unsigned char *output);

int mbedtls_hw_hash_ret(const unsigned char *input, size_t ilen, unsigned char *output, int hash_type);

// #endif /*MBEDTLS_HW_HASH*/

// #endif /*MBEDTLS_HW*/

#endif /*MBEDTLS_HW_H*/