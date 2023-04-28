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
/**
 ****************************************************************************************
 *
 * @file compiler.h
 *
 * @brief Definitions of compiler specific directives.
 *
 *
 *
 ****************************************************************************************
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

#define __STATIC static

#define __INLINE static __attribute__((__always_inline__)) inline

#define __ARRAY_EMPTY

#define __FIQ __attribute__((__interrupt__("FIQ")))

#define __BLEIRQ

#define __IRQ __attribute__((__interrupt__("IRQ")))

#define __PACKED __attribute__ ((__packed__))

#endif // _COMPILER_H_
