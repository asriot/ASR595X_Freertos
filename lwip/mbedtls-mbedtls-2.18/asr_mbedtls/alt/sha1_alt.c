/*
 *  FIPS-180-1 compliant SHA-1 implementation
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
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SHA1_C)

#include "mbedtls/sha1.h"
#include "mbedtls/platform_util.h"

#include <string.h>

#if defined(MBEDTLS_SELF_TEST)
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf printf
#endif /* MBEDTLS_PLATFORM_C */
#endif /* MBEDTLS_SELF_TEST */

#define SHA1_VALIDATE_RET(cond)                             \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_SHA1_BAD_INPUT_DATA )

#define SHA1_VALIDATE(cond)  MBEDTLS_INTERNAL_VALIDATE( cond )

#if defined(MBEDTLS_SHA1_ALT)

#include "mbedtls_hw.h"

void mbedtls_sha1_init( mbedtls_sha1_context *ctx )
{
    SHA1_VALIDATE( ctx != NULL );

    memset( ctx, 0, sizeof( mbedtls_sha1_context ) );

    mbedtls_hw_hash_init(ctx, MBEDTLS_HW_HASH_SHA1);
}

void mbedtls_sha1_free( mbedtls_sha1_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_hw_hash_deinit(ctx, MBEDTLS_HW_HASH_SHA1);

    mbedtls_platform_zeroize( ctx, sizeof( mbedtls_sha1_context ) );
}

void mbedtls_sha1_clone( mbedtls_sha1_context *dst,
                         const mbedtls_sha1_context *src )
{
    SHA1_VALIDATE( dst != NULL );
    SHA1_VALIDATE( src != NULL );

    mbedtls_hw_hash_clone(dst, (void *)src, MBEDTLS_HW_HASH_SHA1);
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret( mbedtls_sha1_context *ctx )
{
    SHA1_VALIDATE_RET( ctx != NULL );

    return mbedtls_hw_hash_start(ctx, MBEDTLS_HW_HASH_SHA1);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha1_starts( mbedtls_sha1_context *ctx )
{
    mbedtls_sha1_starts_ret( ctx );
}
#endif

#if !defined(MBEDTLS_SHA1_PROCESS_ALT)
int mbedtls_internal_sha1_process( mbedtls_sha1_context *ctx,
                                   const unsigned char data[64] )
{
    return( 0 );
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha1_process( mbedtls_sha1_context *ctx,
                           const unsigned char data[64] )
{
    mbedtls_internal_sha1_process( ctx, data );
}
#endif
#endif /* !MBEDTLS_SHA1_PROCESS_ALT */

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret( mbedtls_sha1_context *ctx,
                             const unsigned char *input,
                             size_t ilen )
{
    SHA1_VALIDATE_RET( ctx != NULL );
    SHA1_VALIDATE_RET( ilen == 0 || input != NULL );

    if( ilen == 0 )
        return( 0 );

    return mbedtls_hw_hash_update(ctx, MBEDTLS_HW_HASH_SHA1, input, ilen);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha1_update( mbedtls_sha1_context *ctx,
                          const unsigned char *input,
                          size_t ilen )
{
    mbedtls_sha1_update_ret( ctx, input, ilen );
}
#endif

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret( mbedtls_sha1_context *ctx,
                             unsigned char output[20] )
{
    SHA1_VALIDATE_RET( ctx != NULL );
    SHA1_VALIDATE_RET( (unsigned char *)output != NULL );

    return mbedtls_hw_hash_finish(ctx, MBEDTLS_HW_HASH_SHA1, output);
}

#if !defined(MBEDTLS_DEPRECATED_REMOVED)
void mbedtls_sha1_finish( mbedtls_sha1_context *ctx,
                          unsigned char output[20] )
{
    mbedtls_sha1_finish_ret( ctx, output );
}
#endif

#endif /* MBEDTLS_SHA1_ALT */

#endif /* MBEDTLS_SHA1_C */
