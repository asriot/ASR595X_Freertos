/*
 *  FIPS-180-2 compliant SHA-256 implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
/*
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SHA256_C)

#include "mbedtls/sha256.h"
#include "mbedtls/platform_util.h"

#include <string.h>

#if defined(MBEDTLS_SELF_TEST)
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_calloc    calloc
#define mbedtls_free       free
#endif /* MBEDTLS_PLATFORM_C */
#endif /* MBEDTLS_SELF_TEST */

#define SHA256_VALIDATE_RET(cond)                           \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_SHA256_BAD_INPUT_DATA )
#define SHA256_VALIDATE(cond)  MBEDTLS_INTERNAL_VALIDATE( cond )

#if defined(MBEDTLS_SHA256_ALT)

#include "mbedtls_hw.h"

void mbedtls_sha256_init( mbedtls_sha256_context *ctx )
{
    SHA256_VALIDATE( ctx != NULL );

    memset( ctx, 0, sizeof( mbedtls_sha256_context ) );

    mbedtls_hw_hash_init(ctx, MBEDTLS_HW_HASH_SHA256);
}

void mbedtls_sha256_free( mbedtls_sha256_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_hw_hash_deinit(ctx, MBEDTLS_HW_HASH_SHA256);

    mbedtls_platform_zeroize( ctx, sizeof( mbedtls_sha256_context ) );
}

void mbedtls_sha256_clone( mbedtls_sha256_context *dst,
                           const mbedtls_sha256_context *src )
{
    SHA256_VALIDATE( dst != NULL );
    SHA256_VALIDATE( src != NULL );

    mbedtls_hw_hash_clone(dst, (void *)src, MBEDTLS_HW_HASH_SHA256);
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret( mbedtls_sha256_context *ctx, int is224 )
{
    SHA256_VALIDATE_RET( ctx != NULL );
    SHA256_VALIDATE_RET( is224 == 0 || is224 == 1 );

    ctx->is224 = is224;

    if(is224)
        return mbedtls_hw_hash_start(ctx, MBEDTLS_HW_HASH_SHA224);
    else
        return mbedtls_hw_hash_start(ctx, MBEDTLS_HW_HASH_SHA256);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha256_starts( mbedtls_sha256_context *ctx,
                            int is224 )
{
    mbedtls_sha256_starts_ret( ctx, is224 );
}
#endif

#if !defined(MBEDTLS_SHA256_PROCESS_ALT)

int mbedtls_internal_sha256_process( mbedtls_sha256_context *ctx,
                                const unsigned char data[64] )
{
    return( 0 );
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha256_process( mbedtls_sha256_context *ctx,
                             const unsigned char data[64] )
{
    mbedtls_internal_sha256_process( ctx, data );
}
#endif
#endif /* !MBEDTLS_SHA256_PROCESS_ALT */

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret( mbedtls_sha256_context *ctx,
                               const unsigned char *input,
                               size_t ilen )
{
    SHA256_VALIDATE_RET( ctx != NULL );
    SHA256_VALIDATE_RET( ilen == 0 || input != NULL );

    if( ilen == 0 )
        return( 0 );

    return mbedtls_hw_hash_update(ctx, MBEDTLS_HW_HASH_SHA256, input, ilen);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha256_update( mbedtls_sha256_context *ctx,
                            const unsigned char *input,
                            size_t ilen )
{
    mbedtls_sha256_update_ret( ctx, input, ilen );
}
#endif

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret( mbedtls_sha256_context *ctx,
                               unsigned char output[32] )
{
    SHA256_VALIDATE_RET( ctx != NULL );
    SHA256_VALIDATE_RET( (unsigned char *)output != NULL );

    return mbedtls_hw_hash_finish(ctx, MBEDTLS_HW_HASH_SHA256, output);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha256_finish( mbedtls_sha256_context *ctx,
                            unsigned char output[32] )
{
    mbedtls_sha256_finish_ret( ctx, output );
}
#endif

#endif /* MBEDTLS_SHA256_ALT */

#endif /* MBEDTLS_SHA256_C */
