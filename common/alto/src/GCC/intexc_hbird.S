/*
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************
 * \file     intexc_hbird.S
 * \brief    NMSIS Interrupt and Exception Handling Template File
 *  for Nuclei HummingBird evaluation SoC which support Nuclei N/NX class cores
 * \version  V1.00
 * \date     17 Dec 2019
 *
 ******************************************************************************/

#include "riscv_encoding.h"

/**
 * \brief  Global interrupt disabled
 * \details
 *  This function disable global interrupt.
 * \remarks
 *  - All the interrupt requests will be ignored by CPU.
 */
.macro DISABLE_MIE
    csrc CSR_MSTATUS, MSTATUS_MIE
.endm

/**
 * \brief  Macro for context save
 * \details
 * This macro save ABI defined caller saved registers in the stack.
 * \remarks
 * - This Macro could use to save context when you enter to interrupt
 * or exception
*/
/* Save caller registers */
.macro SAVE_CONTEXT
1:
    csrrw sp, CSR_MSCRATCHCSWL, sp
    beqz sp, 1b
    /* Allocate stack space for context saving */
#ifndef __riscv_32e
    addi sp, sp, -40*REGBYTES
#else
    addi sp, sp, -14*REGBYTES
#endif /* __riscv_32e */

    sw ra, 0*REGBYTES(sp)
    sw tp, 1*REGBYTES(sp)
    sw t0, 2*REGBYTES(sp)
    sw t1, 3*REGBYTES(sp)
    sw t2, 4*REGBYTES(sp)
    sw a0, 5*REGBYTES(sp)
    sw a1, 6*REGBYTES(sp)
    sw a2, 7*REGBYTES(sp)
    sw a3, 8*REGBYTES(sp)
    sw a4, 9*REGBYTES(sp)
    sw a5, 10*REGBYTES(sp)
#ifndef __riscv_32e
    sw a6, 14*REGBYTES(sp)
    sw a7, 15*REGBYTES(sp)
    sw t3, 16*REGBYTES(sp)
    sw t4, 17*REGBYTES(sp)
    sw t5, 18*REGBYTES(sp)
    sw t6, 19*REGBYTES(sp)
    fsw ft0, 20*REGBYTES(sp)
    fsw ft1, 21*REGBYTES(sp)
    fsw ft2, 22*REGBYTES(sp)
    fsw ft3, 23*REGBYTES(sp)
    fsw ft4, 24*REGBYTES(sp)
    fsw ft5, 25*REGBYTES(sp)
    fsw ft6, 26*REGBYTES(sp)
    fsw ft7, 27*REGBYTES(sp)
    fsw fa0, 28*REGBYTES(sp)
    fsw fa1, 29*REGBYTES(sp)
    fsw fa2, 30*REGBYTES(sp)
    fsw fa3, 31*REGBYTES(sp)
    fsw fa4, 32*REGBYTES(sp)
    fsw fa5, 33*REGBYTES(sp)
    fsw fa6, 34*REGBYTES(sp)
    fsw fa7, 35*REGBYTES(sp)
    fsw ft8, 36*REGBYTES(sp)
    fsw ft9, 37*REGBYTES(sp)
    fsw ft10, 38*REGBYTES(sp)
    fsw ft11, 39*REGBYTES(sp)
#endif /* __riscv_32e */
.endm

/**
 * \brief  Macro for restore caller registers
 * \details
 * This macro restore ABI defined caller saved registers from stack.
 * \remarks
 * - You could use this macro to restore context before you want return
 * from interrupt or exeception
 */
/* Restore caller registers */
.macro RESTORE_CONTEXT
    lw ra, 0*REGBYTES(sp)
    lw tp, 1*REGBYTES(sp)
    lw t0, 2*REGBYTES(sp)
    lw t1, 3*REGBYTES(sp)
    lw t2, 4*REGBYTES(sp)
    lw a0, 5*REGBYTES(sp)
    lw a1, 6*REGBYTES(sp)
    lw a2, 7*REGBYTES(sp)
    lw a3, 8*REGBYTES(sp)
    lw a4, 9*REGBYTES(sp)
    lw a5, 10*REGBYTES(sp)
#ifndef __riscv_32e
    lw x16, 14*REGBYTES(sp)
    lw x17, 15*REGBYTES(sp)
    lw x28, 16*REGBYTES(sp)
    lw x29, 17*REGBYTES(sp)
    lw x30, 18*REGBYTES(sp)
    lw x31, 19*REGBYTES(sp)
    flw ft0, 20*REGBYTES(sp)
    flw ft1, 21*REGBYTES(sp)
    flw ft2, 22*REGBYTES(sp)
    flw ft3, 23*REGBYTES(sp)
    flw ft4, 24*REGBYTES(sp)
    flw ft5, 25*REGBYTES(sp)
    flw ft6, 26*REGBYTES(sp)
    flw ft7, 27*REGBYTES(sp)
    flw fa0, 28*REGBYTES(sp)
    flw fa1, 29*REGBYTES(sp)
    flw fa2, 30*REGBYTES(sp)
    flw fa3, 31*REGBYTES(sp)
    flw fa4, 32*REGBYTES(sp)
    flw fa5, 33*REGBYTES(sp)
    flw fa6, 34*REGBYTES(sp)
    flw fa7, 35*REGBYTES(sp)
    flw ft8, 36*REGBYTES(sp)
    flw ft9, 37*REGBYTES(sp)
    flw ft10, 38*REGBYTES(sp)
    flw ft11, 39*REGBYTES(sp)

    /* De-allocate the stack space */
    addi sp, sp, 40*REGBYTES
#else
    /* De-allocate the stack space */
    addi sp, sp, 14*REGBYTES
#endif /* __riscv_32e */
1:
    csrrw sp, CSR_MSCRATCHCSWL, sp
    beqz sp, 1b
.endm

/**
 * \brief  Macro for save necessary CSRs to stack
 * \details
 * This macro store MCAUSE, MEPC, MSUBM to stack.
 */
.macro SAVE_CSR_CONTEXT
    /* Store CSR mcause to stack using pushmcause */
    csrrwi  x0, CSR_PUSHMCAUSE, 11
    /* Store CSR mepc to stack using pushmepc */
    csrrwi  x0, CSR_PUSHMEPC, 12
    /* Store CSR msub to stack using pushmsub */
    csrrwi  x0, CSR_PUSHMSUBM, 13
.endm

/**
 * \brief  Macro for restore necessary CSRs from stack
 * \details
 * This macro restore MSUBM, MEPC, MCAUSE from stack.
 */
.macro RESTORE_CSR_CONTEXT
    LOAD x5,  13*REGBYTES(sp)
    csrw CSR_MSUBM, x5
    LOAD x5,  12*REGBYTES(sp)
    csrw CSR_MEPC, x5
    LOAD x5,  11*REGBYTES(sp)
    csrw CSR_MCAUSE, x5
.endm

/**
 * \brief  Exception/NMI Entry
 * \details
 * This function provide common entry functions for exception/nmi.
 * \remarks
 * This function provide a default exception/nmi entry.
 * ABI defined caller save register and some CSR registers
 * to be saved before enter interrupt handler and be restored before return.
 */
.section .text.trap
/* In CLIC mode, the exeception entry must be 64bytes aligned */
.align 6
.global exc_entry
.weak exc_entry
exc_entry:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    /* Save the necessary CSR registers */
    SAVE_CSR_CONTEXT

    /*
     * Set the exception handler function arguments
     * argument 1: mcause value
     * argument 2: current stack point(SP) value
     */
    csrr a0, mcause
    mv a1, sp
    /*
     * TODO: Call the exception handler function
     * By default, the function template is provided in
     * system_Device.c, you can adjust it as you want
     */
    call core_exception_handler

    /* Restore the necessary CSR registers */
    RESTORE_CSR_CONTEXT
    /* Restore the caller saving registers (context) */
    RESTORE_CONTEXT

    /* Return to regular code */
    mret

/**
 * \brief  Non-Vector Interrupt Entry
 * \details
 * This function provide common entry functions for handling
 * non-vector interrupts
 * \remarks
 * This function provide a default non-vector interrupt entry.
 * ABI defined caller save register and some CSR registers need
 * to be saved before enter interrupt handler and be restored before return.
 */
.section      .text.irq
/* In CLIC mode, the interrupt entry must be 4bytes aligned */
.align 2
.global irq_entry
.weak irq_entry
/* This label will be set to MTVT2 register */
irq_entry:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    /* Save the necessary CSR registers */
    SAVE_CSR_CONTEXT

    call asr_flash_alg_suspend

    /* This special CSR read/write operation, which is actually
     * claim the CLIC to find its pending highest ID, if the ID
     * is not 0, then automatically enable the mstatus.MIE, and
     * jump to its vector-entry-label, and update the link register
     */
    csrrw ra, CSR_JALMNXTI, ra

    /* Critical section with interrupts disabled */
    DISABLE_MIE

    call asr_flash_alg_resume

    /* Restore the necessary CSR registers */
    RESTORE_CSR_CONTEXT
    /* Restore the caller saving registers (context) */
    RESTORE_CONTEXT

    /* Return to regular code */
    mret

/* Default Handler for Exceptions / Interrupts */
.global default_intexc_handler
.weak default_intexc_handler
Undef_Handler:
default_intexc_handler:
1:
    j 1b
