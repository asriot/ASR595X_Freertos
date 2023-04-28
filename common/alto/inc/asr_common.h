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
 * @file asr_common.h
 *
 * @brief adapt xxx_common.h
 *
 *
 ****************************************************************************************
 */
#ifndef _ASR_COMMON_H_
#define _ASR_COMMON_H_

#ifdef CFG_PLF_RV32
#include "alto_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define asr_intrpt_enter() lega_intrpt_enter()
#define asr_intrpt_exit()  lega_intrpt_exit()

#if defined(CFG_PLF_RV32)
/**
 ****************************************************************************************
 * @brief Enable interrupts globally in the system.
 * This macro must be used when the initialization phase is over and the interrupts
 * can start being handled by the system.
 *
 * **************************************************************************************
 */
#define GLOBAL_INT_START()                                 \
    do {                                                   \
          __enable_irq();                                  \
    } while(0)


/**
 *****************************************************************************************
 * @brief Disable interrupts globally in the system.
 * This macro must be used when the system wants to disable all the interrupts
 * it could handle.
 *
 *****************************************************************************************
 */

#define GLOBAL_INT_STOP()                                      \
    do {                                                       \
              __disable_irq();                                  \
    } while(0)

#else
/**
 ****************************************************************************************
 * @brief Enable interrupts globally in the system.
 * This macro must be used when the initialization phase is over and the interrupts
 * can start being handled by the system.
 *
 * **************************************************************************************
 */
#define GLOBAL_INT_START()                                 \
    do {                                                   \
        __asm volatile( "cpsie i" : : : "memory" );        \
    } while(0)


/**
 *****************************************************************************************
 * @brief Disable interrupts globally in the system.
 * This macro must be used when the system wants to disable all the interrupts
 * it could handle.
 *
 *****************************************************************************************
 */

#define GLOBAL_INT_STOP()                                      \
    do {                                                       \
       __asm volatile( "cpsid i" : : : "memory");              \
    } while(0)

#endif // defined(CFG_PLF_RV32)

extern void (*pf_global_int_disable)(void);
extern void (*pf_global_int_restore)(void);
#define GLOBAL_INT_DISABLE() pf_global_int_disable()
#define GLOBAL_INT_RESTORE() pf_global_int_restore()

/**
 *****************************************************************************************
 * @brief Invoke the wait for interrupt procedure of the processor.
 *
 * @warning It is suggested that this macro is called while the interrupts are disabled
 * to have performed the checks necessary to decide to move to sleep mode.
 *****************************************************************************************
 */
#define WFI()                                                                       \
    /* The stop instruction signals external hardware that it should stop     */    \
    /* the processor clock so that the processor can enter a low power mode.  */    \
    __asm volatile( "wfi" );

#ifdef __cplusplus
}
#endif

#endif //_ASR_COMMON_H_


