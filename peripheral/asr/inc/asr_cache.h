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
#ifndef _ASR_CACHE_H_
#define _ASR_CACHE_H_

#ifdef CFG_PLF_RV32
#include "core_feature_cache.h"

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1)
#define asr_enable_icache EnableICache
#define asr_disable_icache DisableICache
#define asr_is_enable_icache IsEnabledICache
#define asr_flush_icache FlushICache
#endif

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1)
#define asr_enable_dcache EnableDCache
#define asr_disable_dcache DisableDCache
#define asr_is_enable_dcache IsEnabledDCache
#define asr_flush_dcache FlushWbDCache
#endif

#else // CFG_PLF_RV32
#include "core_cm4.h"

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1)
#define asr_enable_icache SCB_EnableICache
#define asr_disable_icache SCB_DisableICache
#define asr_is_enable_icache SCB_IsEnabledICache
#define asr_flush_icache SCB_InvalidateICache
#endif

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1)
#define asr_enable_dcache SCB_EnableDCache
#define asr_disable_dcache SCB_DisableDCache
#define asr_is_enable_dcache SCB_IsEnabledDCache
#define asr_flush_dcache SCB_InvalidateDCache
#endif

#endif // CFG_PLF_RV32

#endif // _ASR_CACHE_H_

