/*
 *  TCP/IP or UDP/IP networking functions
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
#ifndef _MBEDTLS_NET_SOCK_H_
#define _MBEDTLS_NET_SOCK_H_


#include "mbedtls/ssl.h"

// #include "mbedtls_private_access.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief          Initialize a context
 *                 Just makes the context ready to be used or freed safely.
 *
 * \param ctx      Context to initialize
 */
void mbedtls_net_init( mbedtls_net_context *ctx );



/**
 * \brief          Read at most 'len' characters. If no error occurs,
 *                 the actual amount read is returned.
 *
 * \param ctx      Socket
 * \param buf      The buffer to write to
 * \param len      Maximum length of the buffer
 *
 * \return         the number of bytes received,
 *                 or a non-zero error code; with a non-blocking socket,
 *                 MBEDTLS_ERR_SSL_WANT_READ indicates read() would block.
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len );

/**
 * \brief          Write at most 'len' characters. If no error occurs,
 *                 the actual amount read is returned.
 *
 * \param ctx      Socket
 * \param buf      The buffer to read from
 * \param len      The length of the buffer
 *
 * \return         the number of bytes sent,
 *                 or a non-zero error code; with a non-blocking socket,
 *                 MBEDTLS_ERR_SSL_WANT_WRITE indicates write() would block.
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len );

/**
 * \brief          Gracefully shutdown the connection and free associated data
 *
 * \param ctx      The context to free
 */
void mbedtls_net_free( mbedtls_net_context *ctx );


#ifdef __cplusplus
}
#endif

#endif /* ! _MBEDTLS_NET_SOCK_H_ */