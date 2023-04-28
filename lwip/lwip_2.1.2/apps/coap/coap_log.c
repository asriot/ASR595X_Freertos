/*
 * Copyright (c) 2015 Keith Cullen.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  @file coap_log.c
 *
 *  @brief Source file for the FreeCoAP logging module
 */

#include <stdio.h>
#include <stdarg.h>
#include "coap_log.h"

coap_log_level_t coap_log_level = COAP_LOG_DEF_LEVEL;                    /**< Log level used to filter log messages */

void coap_log_set_level(coap_log_level_t level)
{
    switch (level) {
    case COAP_LOG_WARN:                                                         /**< Warning log level */
    case COAP_LOG_NOTICE:                                                       /**< Notice warning level */
    case COAP_LOG_INFO:                                                         /**< Informational warning level */
    case COAP_LOG_DEBUG:                                                        /**< Debug warning level */
        coap_log_level = level;
        break;
    default:
        coap_log_level = COAP_LOG_DEF_LEVEL;
    }
}

coap_log_level_t coap_log_get_level(void)
{
    return coap_log_level;
}

