
/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#include "riscv_encoding.h"

#ifndef __riscv_32e
#define portRegNum          63
#else
#define portRegNum          14
#endif

#define portCONTEXT_SIZE    ( portRegNum * REGBYTES )

.section    .text.entry
.align 8

.extern asr_flash_alg_is_wip
.extern asr_flash_alg_suspend
.extern asr_flash_alg_resume

.extern xPortTaskSwitch
.extern pxCurrentTCB
.global prvPortStartFirstTask

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
Undef_Handler:
default_intexc_handler:
1:
    j 1b

/* Start the first task.  This also clears the bit that indicates the FPU is
    in use in case the FPU was used before the scheduler was started - which
    would otherwise result in the unnecessary leaving of space in the stack
    for lazy saving of FPU registers. */
.align 3
prvPortStartFirstTask:
    /* Setup Interrupt Stack using
       The stack that was used by main()
       before the scheduler is started is
       no longer required after the scheduler is started.
       Interrupt stack pointer is stored in CSR_MSCRATCH */
    la t0, _sp
    csrw CSR_MSCRATCH, t0
    LOAD sp, pxCurrentTCB           /* Load pxCurrentTCB. */
    LOAD sp, 0x0(sp)                /* Read sp from first TCB member */

    /* Pop PC from stack and set MEPC */
    LOAD t0,  0  * REGBYTES(sp)
    csrw CSR_MEPC, t0
    /* Pop mstatus from stack and set it */
    LOAD t0,  (portRegNum - 1)  * REGBYTES(sp)
    csrw CSR_MSTATUS, t0
    /* Interrupt still disable here */
    /* Restore Registers from Stack */
    LOAD x1,  1  * REGBYTES(sp)    /* RA */
    LOAD x4,  2  * REGBYTES(sp)
    LOAD x5,  3  * REGBYTES(sp)
    LOAD x6,  4  * REGBYTES(sp)
    LOAD x7,  5  * REGBYTES(sp)
    LOAD x8,  6  * REGBYTES(sp)
    LOAD x9,  7  * REGBYTES(sp)
    LOAD x10, 8  * REGBYTES(sp)
    LOAD x11, 9  * REGBYTES(sp)
    LOAD x12, 10 * REGBYTES(sp)
    LOAD x13, 11 * REGBYTES(sp)
    LOAD x14, 12 * REGBYTES(sp)
    LOAD x15, 13 * REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 14 * REGBYTES(sp)
    LOAD x17, 15 * REGBYTES(sp)
    LOAD x18, 16 * REGBYTES(sp)
    LOAD x19, 17 * REGBYTES(sp)
    LOAD x20, 18 * REGBYTES(sp)
    LOAD x21, 19 * REGBYTES(sp)
    LOAD x22, 20 * REGBYTES(sp)
    LOAD x23, 21 * REGBYTES(sp)
    LOAD x24, 22 * REGBYTES(sp)
    LOAD x25, 23 * REGBYTES(sp)
    LOAD x26, 24 * REGBYTES(sp)
    LOAD x27, 25 * REGBYTES(sp)
    LOAD x28, 26 * REGBYTES(sp)
    LOAD x29, 27 * REGBYTES(sp)
    LOAD x30, 28 * REGBYTES(sp)
    LOAD x31, 29 * REGBYTES(sp)
    flw f0, 30 * REGBYTES(sp)
    flw f1, 31 * REGBYTES(sp)
    flw f2, 32 * REGBYTES(sp)
    flw f3, 33 * REGBYTES(sp)
    flw f4, 34 * REGBYTES(sp)
    flw f5, 35 * REGBYTES(sp)
    flw f6, 36 * REGBYTES(sp)
    flw f7, 37 * REGBYTES(sp)
    flw f8, 38 * REGBYTES(sp)
    flw f9, 39 * REGBYTES(sp)
    flw f10, 40 * REGBYTES(sp)
    flw f11, 41 * REGBYTES(sp)
    flw f12, 42 * REGBYTES(sp)
    flw f13, 43 * REGBYTES(sp)
    flw f14, 44 * REGBYTES(sp)
    flw f15, 45 * REGBYTES(sp)
    flw f16, 46 * REGBYTES(sp)
    flw f17, 47 * REGBYTES(sp)
    flw f18, 48 * REGBYTES(sp)
    flw f19, 49 * REGBYTES(sp)
    flw f20, 50 * REGBYTES(sp)
    flw f21, 51 * REGBYTES(sp)
    flw f22, 52 * REGBYTES(sp)
    flw f23, 53 * REGBYTES(sp)
    flw f24, 54 * REGBYTES(sp)
    flw f25, 55 * REGBYTES(sp)
    flw f26, 56 * REGBYTES(sp)
    flw f27, 57 * REGBYTES(sp)
    flw f28, 58 * REGBYTES(sp)
    flw f29, 59 * REGBYTES(sp)
    flw f30, 60 * REGBYTES(sp)
    flw f31, 61 * REGBYTES(sp)
#endif

    addi sp, sp, portCONTEXT_SIZE

    mret

.align 2
.global eclic_msip_handler
eclic_msip_handler:
    addi sp, sp, -portCONTEXT_SIZE
    STORE x1,  1  * REGBYTES(sp)    /* RA */
    STORE x4,  2  * REGBYTES(sp)
    STORE x5,  3  * REGBYTES(sp)
    STORE x6,  4  * REGBYTES(sp)
    STORE x7,  5  * REGBYTES(sp)
    STORE x8,  6  * REGBYTES(sp)
    STORE x9,  7  * REGBYTES(sp)
    STORE x10, 8  * REGBYTES(sp)
    STORE x11, 9  * REGBYTES(sp)
    STORE x12, 10 * REGBYTES(sp)
    STORE x13, 11 * REGBYTES(sp)
    STORE x14, 12 * REGBYTES(sp)
    STORE x15, 13 * REGBYTES(sp)
#ifndef __riscv_32e
    STORE x16, 14 * REGBYTES(sp)
    STORE x17, 15 * REGBYTES(sp)
    STORE x18, 16 * REGBYTES(sp)
    STORE x19, 17 * REGBYTES(sp)
    STORE x20, 18 * REGBYTES(sp)
    STORE x21, 19 * REGBYTES(sp)
    STORE x22, 20 * REGBYTES(sp)
    STORE x23, 21 * REGBYTES(sp)
    STORE x24, 22 * REGBYTES(sp)
    STORE x25, 23 * REGBYTES(sp)
    STORE x26, 24 * REGBYTES(sp)
    STORE x27, 25 * REGBYTES(sp)
    STORE x28, 26 * REGBYTES(sp)
    STORE x29, 27 * REGBYTES(sp)
    STORE x30, 28 * REGBYTES(sp)
    STORE x31, 29 * REGBYTES(sp)
    fsw f0, 30 * REGBYTES(sp)
    fsw f1, 31 * REGBYTES(sp)
    fsw f2, 32 * REGBYTES(sp)
    fsw f3, 33 * REGBYTES(sp)
    fsw f4, 34 * REGBYTES(sp)
    fsw f5, 35 * REGBYTES(sp)
    fsw f6, 36 * REGBYTES(sp)
    fsw f7, 37 * REGBYTES(sp)
    fsw f8, 38 * REGBYTES(sp)
    fsw f9, 39 * REGBYTES(sp)
    fsw f10, 40 * REGBYTES(sp)
    fsw f11, 41 * REGBYTES(sp)
    fsw f12, 42 * REGBYTES(sp)
    fsw f13, 43 * REGBYTES(sp)
    fsw f14, 44 * REGBYTES(sp)
    fsw f15, 45 * REGBYTES(sp)
    fsw f16, 46 * REGBYTES(sp)
    fsw f17, 47 * REGBYTES(sp)
    fsw f18, 48 * REGBYTES(sp)
    fsw f19, 49 * REGBYTES(sp)
    fsw f20, 40 * REGBYTES(sp)
    fsw f21, 51 * REGBYTES(sp)
    fsw f22, 52 * REGBYTES(sp)
    fsw f23, 53 * REGBYTES(sp)
    fsw f24, 54 * REGBYTES(sp)
    fsw f25, 55 * REGBYTES(sp)
    fsw f26, 56 * REGBYTES(sp)
    fsw f27, 57 * REGBYTES(sp)
    fsw f28, 58 * REGBYTES(sp)
    fsw f29, 59 * REGBYTES(sp)
    fsw f30, 60 * REGBYTES(sp)
    fsw f31, 61 * REGBYTES(sp)
#endif
    /* Push mstatus to stack */
    csrr t0, CSR_MSTATUS
    STORE t0,  (portRegNum - 1)  * REGBYTES(sp)

    /* Push additional registers */

    /* Store sp to task stack */
    LOAD t0, pxCurrentTCB
    STORE sp, 0(t0)

    csrr t0, CSR_MEPC
    STORE t0, 0(sp)

    call asr_flash_alg_is_wip
    beqz a0, 1f

    /* Clear SWIRQ, skip task switch */
    li     t0, 0x00120FFC
    sw     zero,(t0)
    j 2f

1:
    jal xPortTaskSwitch

2:
    /* Switch task context */
    LOAD t0, pxCurrentTCB           /* Load pxCurrentTCB. */
    LOAD sp, 0x0(t0)                /* Read sp from first TCB member */

    /* Pop PC from stack and set MEPC */
    LOAD t0,  0  * REGBYTES(sp)
    csrw CSR_MEPC, t0
    /* Pop additional registers */

    /* Pop mstatus from stack and set it */
    LOAD t0,  (portRegNum - 1)  * REGBYTES(sp)
    csrw CSR_MSTATUS, t0
    /* Interrupt still disable here */
    /* Restore Registers from Stack */
    LOAD x1,  1  * REGBYTES(sp)    /* RA */
    LOAD x4,  2  * REGBYTES(sp)
    LOAD x5,  3  * REGBYTES(sp)
    LOAD x6,  4  * REGBYTES(sp)
    LOAD x7,  5  * REGBYTES(sp)
    LOAD x8,  6  * REGBYTES(sp)
    LOAD x9,  7  * REGBYTES(sp)
    LOAD x10, 8  * REGBYTES(sp)
    LOAD x11, 9  * REGBYTES(sp)
    LOAD x12, 10 * REGBYTES(sp)
    LOAD x13, 11 * REGBYTES(sp)
    LOAD x14, 12 * REGBYTES(sp)
    LOAD x15, 13 * REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 14 * REGBYTES(sp)
    LOAD x17, 15 * REGBYTES(sp)
    LOAD x18, 16 * REGBYTES(sp)
    LOAD x19, 17 * REGBYTES(sp)
    LOAD x20, 18 * REGBYTES(sp)
    LOAD x21, 19 * REGBYTES(sp)
    LOAD x22, 20 * REGBYTES(sp)
    LOAD x23, 21 * REGBYTES(sp)
    LOAD x24, 22 * REGBYTES(sp)
    LOAD x25, 23 * REGBYTES(sp)
    LOAD x26, 24 * REGBYTES(sp)
    LOAD x27, 25 * REGBYTES(sp)
    LOAD x28, 26 * REGBYTES(sp)
    LOAD x29, 27 * REGBYTES(sp)
    LOAD x30, 28 * REGBYTES(sp)
    LOAD x31, 29 * REGBYTES(sp)
    flw f0, 30 * REGBYTES(sp)
    flw f1, 31 * REGBYTES(sp)
    flw f2, 32 * REGBYTES(sp)
    flw f3, 33 * REGBYTES(sp)
    flw f4, 34 * REGBYTES(sp)
    flw f5, 35 * REGBYTES(sp)
    flw f6, 36 * REGBYTES(sp)
    flw f7, 37 * REGBYTES(sp)
    flw f8, 38 * REGBYTES(sp)
    flw f9, 39 * REGBYTES(sp)
    flw f10, 40 * REGBYTES(sp)
    flw f11, 41 * REGBYTES(sp)
    flw f12, 42 * REGBYTES(sp)
    flw f13, 43 * REGBYTES(sp)
    flw f14, 44 * REGBYTES(sp)
    flw f15, 45 * REGBYTES(sp)
    flw f16, 46 * REGBYTES(sp)
    flw f17, 47 * REGBYTES(sp)
    flw f18, 48 * REGBYTES(sp)
    flw f19, 49 * REGBYTES(sp)
    flw f20, 50 * REGBYTES(sp)
    flw f21, 51 * REGBYTES(sp)
    flw f22, 52 * REGBYTES(sp)
    flw f23, 53 * REGBYTES(sp)
    flw f24, 54 * REGBYTES(sp)
    flw f25, 55 * REGBYTES(sp)
    flw f26, 56 * REGBYTES(sp)
    flw f27, 57 * REGBYTES(sp)
    flw f28, 58 * REGBYTES(sp)
    flw f29, 59 * REGBYTES(sp)
    flw f30, 60 * REGBYTES(sp)
    flw f31, 61 * REGBYTES(sp)
#endif

    addi sp, sp, portCONTEXT_SIZE
    mret
