##=============================================================================
##
##      context.S
##
##      PowerPC context switch code
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):   nickg
## Contributors:        nickg
## Date:        1998-04-27
## Purpose:     PowerPC context switch code
## Description: This file contains implementations of the thread context 
##              switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/arch.inc>
#include <cyg/hal/ppc_offsets.inc>

#------------------------------------------------------------------------------
# Configure to use either a minimal or maximal thread state
        
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

#define MIN_SAVE_REG    13
#define MAX_SAVE_REG    31

        .macro  save_special
        .endm

        .macro  load_special
        .endm
#else

#define MIN_SAVE_REG     6
#define MAX_SAVE_REG    31

        .macro  save_special
        mfxer   r6
        mfctr   r7
        stw     r6,CYGARC_PPCREG_XER(sp)
        stw     r7,CYGARC_PPCREG_CTR(sp)
        .endm

        .macro  load_special
        lwz     r6,CYGARC_PPCREG_XER(sp)
        lwz     r7,CYGARC_PPCREG_CTR(sp)
        mtxer   r6
        mtctr   r7
        .endm
                
#endif                  

#ifdef CYGHWR_HAL_POWERPC_FPU
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

#define MIN_CONTEXT_SAVE_FREG   14
#define MAX_CONTEXT_SAVE_FREG   31

#else

#define MIN_CONTEXT_SAVE_FREG    0
#define MAX_CONTEXT_SAVE_FREG   31

#endif
#endif
                
#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# R3 = address of sp of next thread to execute
# R4 = address of sp save location of current thread

        
FUNC_START(hal_thread_switch_context)
        mr      r5,sp                   # R5 = saved stack pointer
        subi    sp,sp,CYGARC_PPC_CONTEXT_SIZE  # space for state

        # Save registers MIN..MAX
        .set    _reg,MIN_SAVE_REG
        .rept   MAX_SAVE_REG+1-MIN_SAVE_REG
        stw     _reg,(CYGARC_PPCREG_REGS+_reg*4)(sp)
        .set    _reg,_reg+1
        .endr

#ifdef CYGHWR_HAL_POWERPC_FPU
        # Save floating point MIN..MAX
        .set    _freg, MIN_CONTEXT_SAVE_FREG
        .rept   MAX_CONTEXT_SAVE_FREG+1-MIN_CONTEXT_SAVE_FREG
        stfd    _freg,(CYGARC_PPCREG_FREGS+_freg*8)(sp)
        .set    _freg, _freg+1
        .endr
#endif

        stw     r0,CYGARC_PPCREG_REGS+0*4(sp)  # R0
        stw     r5,CYGARC_PPCREG_REGS+1*4(sp)  # R5 = real SP, save in R1 slot
        stw     r2,CYGARC_PPCREG_REGS+2*4(sp)  # R2 = TOC

        # CR cannot be treated as a special register since GCC will only
        # do partial restore of CR at function exit, depending on which
        # of CR2, CR3, and CR4 have been used in the given function.
        mfcr    r5                      # save CR.
        stw     r5,CYGARC_PPCREG_CR(sp)

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
        # Make the thread context look like an exception context if thread-
        # aware debugging is required. This state does not need restoring.
        mflr    r5
        stw     r5,CYGARC_PPCREG_PC(sp) # pc of caller
#endif

        mfmsr   r5
        stw     r5,CYGARC_PPCREG_MSR(sp)# msr (or PS)

        save_special                    # save special regs
        
        mflr    r5                      # save LR == return link for this fn
        stw     r5,CYGARC_PPCREG_LR(sp)
        
        stw     sp,0(r4)                # save SP into save location

        # Now load the destination thread by dropping through
        # to hal_thread_load_context
        
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# R3 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context
# and is simply dropped into from it.
        
FUNC_START(hal_thread_load_context)
        
        lwz     sp,0(r3)                # load state directly into SP

        load_special                    # load special registers

        lwz     r5,CYGARC_PPCREG_CR(sp) # restore CR
        mtcr    r5

        lwz     r5,CYGARC_PPCREG_LR(sp) # get LR as set as return link
        mtlr    r5
                
        # Load registers MIN..MAX
        .set    _reg,MIN_SAVE_REG
        .rept   MAX_SAVE_REG+1-MIN_SAVE_REG
        lwz     _reg,(CYGARC_PPCREG_REGS+_reg*4)(sp)
        .set    _reg,_reg+1
        .endr

#ifdef CYGHWR_HAL_POWERPC_FPU
        # Load floating registers MIN..MAX
        .set    _freg, MIN_CONTEXT_SAVE_FREG
        .rept   MAX_CONTEXT_SAVE_FREG+1-MIN_CONTEXT_SAVE_FREG
        lfd     _freg,(CYGARC_PPCREG_FREGS+_freg*8)(sp)
        .set    _freg,_freg+1
        .endr
#endif

        lwz     r3,CYGARC_PPCREG_MSR(sp)       # merge interrupt state
        hal_cpu_int_merge r3

        lwz     r0,CYGARC_PPCREG_REGS+0*4(sp)  # R0
        lwz     r2,CYGARC_PPCREG_REGS+2*4(sp)  # R2 = TOC
        lwz     r3,CYGARC_PPCREG_REGS+3*4(sp)  # load r3
        
        lwz     sp,CYGARC_PPCREG_REGS+1*4(sp)  # finally restore true SP
        
        blr                             # jump to LR
                        
#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only to callee save registers 13-31, r1[sp],r2, cr[2-4]
# and lr into buffer supplied in r3[arg0]

FUNC_START(hal_setjmp)
        mfcr    r5
        stw     r5, CYGARC_JMPBUF_CR(r3)
        mflr    r5
        stw     r5, CYGARC_JMPBUF_LR(r3)
        stw     r31,CYGARC_JMPBUF_R31(r3)
        stw     r30,CYGARC_JMPBUF_R30(r3)
        stw     r29,CYGARC_JMPBUF_R29(r3)
        stw     r28,CYGARC_JMPBUF_R28(r3)
        stw     r27,CYGARC_JMPBUF_R27(r3)
        stw     r26,CYGARC_JMPBUF_R26(r3)
        stw     r25,CYGARC_JMPBUF_R25(r3)
        stw     r24,CYGARC_JMPBUF_R24(r3)
        stw     r23,CYGARC_JMPBUF_R23(r3)
        stw     r22,CYGARC_JMPBUF_R22(r3)
        stw     r21,CYGARC_JMPBUF_R21(r3)
        stw     r20,CYGARC_JMPBUF_R20(r3)
        stw     r19,CYGARC_JMPBUF_R19(r3)
        stw     r18,CYGARC_JMPBUF_R18(r3)
        stw     r17,CYGARC_JMPBUF_R17(r3)
        stw     r16,CYGARC_JMPBUF_R16(r3)
        stw     r15,CYGARC_JMPBUF_R15(r3)
        stw     r14,CYGARC_JMPBUF_R14(r3)
        stw     r13,CYGARC_JMPBUF_R13(r3)
#ifdef CYGHWR_HAL_POWERPC_FPU
        stfd    f31,CYGARC_JMPBUF_F31(r3)
        stfd    f30,CYGARC_JMPBUF_F30(r3)
        stfd    f29,CYGARC_JMPBUF_F29(r3)
        stfd    f28,CYGARC_JMPBUF_F28(r3)
        stfd    f27,CYGARC_JMPBUF_F27(r3)
        stfd    f26,CYGARC_JMPBUF_F26(r3)
        stfd    f25,CYGARC_JMPBUF_F25(r3)
        stfd    f24,CYGARC_JMPBUF_F24(r3)
        stfd    f23,CYGARC_JMPBUF_F23(r3)
        stfd    f22,CYGARC_JMPBUF_F22(r3)
        stfd    f21,CYGARC_JMPBUF_F21(r3)
        stfd    f20,CYGARC_JMPBUF_F20(r3)
        stfd    f19,CYGARC_JMPBUF_F19(r3)
        stfd    f18,CYGARC_JMPBUF_F18(r3)
        stfd    f17,CYGARC_JMPBUF_F17(r3)
        stfd    f16,CYGARC_JMPBUF_F16(r3)
        stfd    f15,CYGARC_JMPBUF_F15(r3)
        stfd    f14,CYGARC_JMPBUF_F14(r3)
#endif
        stw     r2, CYGARC_JMPBUF_R2(r3)        # TOC, optimize out?
        stw     sp, CYGARC_JMPBUF_SP(r3)
        li      r3,0            # return 0
        blr

# hal_longjmp loads state from r3[arg0] and returns
# to the setjmp caller with r4[arg1] as return value 

FUNC_START(hal_longjmp)
        lwz     r5,CYGARC_JMPBUF_CR(r3)
        mtcr    r5
        lwz     r5,CYGARC_JMPBUF_LR(r3)
        mtlr    r5
        lwz     r31,CYGARC_JMPBUF_R31(r3)
        lwz     r30,CYGARC_JMPBUF_R30(r3)
        lwz     r29,CYGARC_JMPBUF_R29(r3)
        lwz     r28,CYGARC_JMPBUF_R28(r3)
        lwz     r27,CYGARC_JMPBUF_R27(r3)
        lwz     r26,CYGARC_JMPBUF_R26(r3)
        lwz     r25,CYGARC_JMPBUF_R25(r3)
        lwz     r24,CYGARC_JMPBUF_R24(r3)
        lwz     r23,CYGARC_JMPBUF_R23(r3)
        lwz     r22,CYGARC_JMPBUF_R22(r3)
        lwz     r21,CYGARC_JMPBUF_R21(r3)
        lwz     r20,CYGARC_JMPBUF_R20(r3)
        lwz     r19,CYGARC_JMPBUF_R19(r3)
        lwz     r18,CYGARC_JMPBUF_R18(r3)
        lwz     r17,CYGARC_JMPBUF_R17(r3)
        lwz     r16,CYGARC_JMPBUF_R16(r3)
        lwz     r15,CYGARC_JMPBUF_R15(r3)
        lwz     r14,CYGARC_JMPBUF_R14(r3)
        lwz     r13,CYGARC_JMPBUF_R13(r3)
#ifdef CYGHWR_HAL_POWERPC_FPU
        lfd     f31,CYGARC_JMPBUF_F31(r3)
        lfd     f30,CYGARC_JMPBUF_F30(r3)
        lfd     f29,CYGARC_JMPBUF_F29(r3)
        lfd     f28,CYGARC_JMPBUF_F28(r3)
        lfd     f27,CYGARC_JMPBUF_F27(r3)
        lfd     f26,CYGARC_JMPBUF_F26(r3)
        lfd     f25,CYGARC_JMPBUF_F25(r3)
        lfd     f24,CYGARC_JMPBUF_F24(r3)
        lfd     f23,CYGARC_JMPBUF_F23(r3)
        lfd     f22,CYGARC_JMPBUF_F22(r3)
        lfd     f21,CYGARC_JMPBUF_F21(r3)
        lfd     f20,CYGARC_JMPBUF_F20(r3)
        lfd     f19,CYGARC_JMPBUF_F19(r3)
        lfd     f18,CYGARC_JMPBUF_F18(r3)
        lfd     f17,CYGARC_JMPBUF_F17(r3)
        lfd     f16,CYGARC_JMPBUF_F16(r3)
        lfd     f15,CYGARC_JMPBUF_F15(r3)
        lfd     f14,CYGARC_JMPBUF_F14(r3)
#endif
        lwz     r2, CYGARC_JMPBUF_R2(r3)
        lwz     sp, CYGARC_JMPBUF_SP(r3)
        mr      r3,r4           # return r4[arg1]
        blr

#------------------------------------------------------------------------------
# end of context.S
