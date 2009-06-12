##=============================================================================##
##      context.S
##
##      OpenRISC context switch code
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
## Author(s):   Scott Furman
## Contributors:
## Date:        2003-01-21
## Purpose:     OpenRISC context switch code
## Description: This file contains implementations of the thread context 
##              switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>
#include <cyg/hal/openrisc.inc>



#------------------------------------------------------------------------------
# hal_thread_switch_context()
# Switch thread contexts
# R3 = address of sp of next thread to execute
# R4 = address of sp save location of current thread

FUNC_START(hal_thread_switch_context)
        l.addi  sp,sp,-SIZEOF_OR1KREGS  # space for registers

        # Store General Purpose Registers (GPRs).
        l.sw     2 * OR1K_GPRSIZE(sp), r2
        l.sw     9 * OR1K_GPRSIZE(sp), r9
        l.sw    10 * OR1K_GPRSIZE(sp), r10
        l.sw    12 * OR1K_GPRSIZE(sp), r12
        l.sw    14 * OR1K_GPRSIZE(sp), r14
        l.sw    16 * OR1K_GPRSIZE(sp), r16
        l.sw    18 * OR1K_GPRSIZE(sp), r18
        l.sw    20 * OR1K_GPRSIZE(sp), r20
        l.sw    22 * OR1K_GPRSIZE(sp), r22
        l.sw    24 * OR1K_GPRSIZE(sp), r24
        l.sw    26 * OR1K_GPRSIZE(sp), r26
        l.sw    28 * OR1K_GPRSIZE(sp), r28
        l.sw    30 * OR1K_GPRSIZE(sp), r30

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
        
        # R0 is not typically stored because it is always zero-valued,
        # but we store it here for consistency when examining registers
        # in the debugger.
        l.sw     0 * OR1K_GPRSIZE(sp), r0

        # Caller-saved regs don't need to be preserved across
        # context switches, but we do so to make debugging easier.

        l.sw     3 * OR1K_GPRSIZE(sp), r3
        l.sw     4 * OR1K_GPRSIZE(sp), r4
        l.sw     5 * OR1K_GPRSIZE(sp), r5
        l.sw     6 * OR1K_GPRSIZE(sp), r6
        l.sw     7 * OR1K_GPRSIZE(sp), r7
        l.sw     8 * OR1K_GPRSIZE(sp), r8
        l.sw    11 * OR1K_GPRSIZE(sp), r11
        l.sw    13 * OR1K_GPRSIZE(sp), r13
        l.sw    15 * OR1K_GPRSIZE(sp), r15
        l.sw    17 * OR1K_GPRSIZE(sp), r17
        l.sw    19 * OR1K_GPRSIZE(sp), r19
        l.sw    21 * OR1K_GPRSIZE(sp), r21
        l.sw    23 * OR1K_GPRSIZE(sp), r23
        l.sw    25 * OR1K_GPRSIZE(sp), r25
        l.sw    27 * OR1K_GPRSIZE(sp), r27
        l.sw    29 * OR1K_GPRSIZE(sp), r29
        l.sw    31 * OR1K_GPRSIZE(sp), r31

        # save MAC LO and HI regs
        l.mfspr r5,r0,SPR_MACLO
        l.sw    OR1KREG_MACLO(sp),r5
        l.mfspr r5,r0,SPR_MACHI
        l.sw    OR1KREG_MACHI(sp),r5
        
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
        # Make the thread context look like an exception context if thread-
        # aware debugging is required. This state does not need restoring.
        l.sw    OR1KREG_PC(sp),r9
#endif

        l.addi  r5,sp,SIZEOF_OR1KREGS           # save SP in reg dump
        l.sw     1 * OR1K_GPRSIZE(sp), r5

        l.mfspr  r5,r0,SPR_SR                   # save SR in reg dump
        l.sw     OR1KREG_SR(sp), r5

        # Return resulting new SP to caller via second argument
        l.sw     0(r4), sp

        # Now load the destination thread by dropping through
        # to hal_thread_load_context...
FUNC_END(hal_thread_switch_context)

                
#------------------------------------------------------------------------------
# hal_thread_load_context()
# Load thread context
# R3 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context()
# and is simply dropped into from it.
        
FUNC_START(hal_thread_load_context)

        # Copy R3 to SP         
        l.lwz   sp, 0(r3)

        # Restore General Purpose Registers (GPRs).
        # R0 is not restored because it is always zero-valued.
        l.lwz   r2,   2 * OR1K_GPRSIZE(sp)
        l.lwz   r9,   9 * OR1K_GPRSIZE(sp)
        l.lwz   r10, 10 * OR1K_GPRSIZE(sp)
        l.lwz   r12, 12 * OR1K_GPRSIZE(sp)
        l.lwz   r14, 14 * OR1K_GPRSIZE(sp)
        l.lwz   r16, 16 * OR1K_GPRSIZE(sp)
        l.lwz   r18, 18 * OR1K_GPRSIZE(sp)
        l.lwz   r20, 20 * OR1K_GPRSIZE(sp)
        l.lwz   r22, 22 * OR1K_GPRSIZE(sp)
        l.lwz   r24, 24 * OR1K_GPRSIZE(sp)
        l.lwz   r26, 26 * OR1K_GPRSIZE(sp)
        l.lwz   r28, 28 * OR1K_GPRSIZE(sp)
        l.lwz   r30, 30 * OR1K_GPRSIZE(sp)

        # Merge interrupt-enable state of new thread into
        # current SR
        load32i r5,~(SPR_SR_TEE|SPR_SR_IEE)
        l.mfspr r6, r0, SPR_SR
        l.and   r6, r5, r6
        l.lwz   r5,  OR1KREG_SR(sp)
        l.andi  r5, r5, (SPR_SR_TEE|SPR_SR_IEE)
        l.or    r5, r5, r6
        l.mtspr r0, r5, SPR_SR

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

        # Caller-saved regs don't need to be preserved across
        # context switches, but we do so here to make debugging
        # easier.

        # Restore MAC LO and HI regs
        l.lwz   r5, OR1KREG_MACLO(sp)
        l.mtspr r0,r5,SPR_MACLO
        l.lwz   r5, OR1KREG_MACHI(sp)
        l.mtspr r0,r5,SPR_MACHI

        l.lwz    r4,  4 * OR1K_GPRSIZE(sp)
        l.lwz    r5,  5 * OR1K_GPRSIZE(sp)
        l.lwz    r6,  6 * OR1K_GPRSIZE(sp)
        l.lwz    r7,  7 * OR1K_GPRSIZE(sp)
        l.lwz    r8,  8 * OR1K_GPRSIZE(sp)
        l.lwz   r11, 11 * OR1K_GPRSIZE(sp)
        l.lwz   r13, 13 * OR1K_GPRSIZE(sp)
        l.lwz   r15, 15 * OR1K_GPRSIZE(sp)
        l.lwz   r17, 17 * OR1K_GPRSIZE(sp)
        l.lwz   r19, 19 * OR1K_GPRSIZE(sp)
        l.lwz   r21, 21 * OR1K_GPRSIZE(sp)
        l.lwz   r23, 23 * OR1K_GPRSIZE(sp)
        l.lwz   r25, 25 * OR1K_GPRSIZE(sp)
        l.lwz   r27, 27 * OR1K_GPRSIZE(sp)
        l.lwz   r29, 29 * OR1K_GPRSIZE(sp)
        l.lwz   r31, 31 * OR1K_GPRSIZE(sp)

#endif

	# If this is the first time we're running a thread, R3
	# contains the argument to the thread entry point function,
	# So we always have to restore it even though it's a callee-saved
	# register.
        l.lwz    r3,  3 * OR1K_GPRSIZE(sp)

        # Finally, restore target thread's true SP
        l.lwz   sp,   1 * OR1K_GPRSIZE(sp)

        l.jr    lr
        l.nop                                   # delay slot - must be nop

FUNC_END(hal_thread_load_context)
        
#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only callee-saved registers into buffer supplied in r3:
#       1,2,9,10,13,15,17,19,21,23,25,27,29,31
# Note: These definitions are repeated in hal_arch.h. If changes are required
# remember to update both sets.

#define CYGARC_JMP_BUF_R1        0
#define CYGARC_JMP_BUF_R2        1
#define CYGARC_JMP_BUF_R9        2
#define CYGARC_JMP_BUF_R10       3
#define CYGARC_JMP_BUF_R12       4
#define CYGARC_JMP_BUF_R14       5
#define CYGARC_JMP_BUF_R16       6
#define CYGARC_JMP_BUF_R18       7
#define CYGARC_JMP_BUF_R20       8
#define CYGARC_JMP_BUF_R22       9
#define CYGARC_JMP_BUF_R24      10
#define CYGARC_JMP_BUF_R26      11
#define CYGARC_JMP_BUF_R28      12
#define CYGARC_JMP_BUF_R30      13

#define CYGARC_JMP_BUF_SIZE     14

#define jmpbuf_regsize 4

FUNC_START(hal_setjmp)
        # Store General Purpose Registers (GPRs).
        # R0 is not stored because it is always zero-valued.
        # Caller-saved registers are not stored
        l.sw    CYGARC_JMP_BUF_R1  * OR1K_GPRSIZE(r3), r1
        l.sw    CYGARC_JMP_BUF_R2  * OR1K_GPRSIZE(r3), r2
        l.sw    CYGARC_JMP_BUF_R9  * OR1K_GPRSIZE(r3), r9
        l.sw    CYGARC_JMP_BUF_R10 * OR1K_GPRSIZE(r3), r10
        l.sw    CYGARC_JMP_BUF_R12 * OR1K_GPRSIZE(r3), r12
        l.sw    CYGARC_JMP_BUF_R14 * OR1K_GPRSIZE(r3), r14
        l.sw    CYGARC_JMP_BUF_R16 * OR1K_GPRSIZE(r3), r16
        l.sw    CYGARC_JMP_BUF_R18 * OR1K_GPRSIZE(r3), r18
        l.sw    CYGARC_JMP_BUF_R20 * OR1K_GPRSIZE(r3), r20
        l.sw    CYGARC_JMP_BUF_R22 * OR1K_GPRSIZE(r3), r22
        l.sw    CYGARC_JMP_BUF_R24 * OR1K_GPRSIZE(r3), r24
        l.sw    CYGARC_JMP_BUF_R26 * OR1K_GPRSIZE(r3), r26
        l.sw    CYGARC_JMP_BUF_R28 * OR1K_GPRSIZE(r3), r28
        l.sw    CYGARC_JMP_BUF_R30 * OR1K_GPRSIZE(r3), r30
        l.jr    lr
        l.nop                   # delay slot
FUNC_END(hal_setjmp)


FUNC_START(hal_longjmp)
        l.lwz    r1, CYGARC_JMP_BUF_R1  * OR1K_GPRSIZE(r3)
        l.lwz    r2, CYGARC_JMP_BUF_R2  * OR1K_GPRSIZE(r3)
        l.lwz    r9, CYGARC_JMP_BUF_R9  * OR1K_GPRSIZE(r3)
        l.lwz   r10, CYGARC_JMP_BUF_R10 * OR1K_GPRSIZE(r3)
        l.lwz   r12, CYGARC_JMP_BUF_R12 * OR1K_GPRSIZE(r3)
        l.lwz   r14, CYGARC_JMP_BUF_R14 * OR1K_GPRSIZE(r3)
        l.lwz   r16, CYGARC_JMP_BUF_R16 * OR1K_GPRSIZE(r3)
        l.lwz   r18, CYGARC_JMP_BUF_R18 * OR1K_GPRSIZE(r3)
        l.lwz   r20, CYGARC_JMP_BUF_R20 * OR1K_GPRSIZE(r3)
        l.lwz   r22, CYGARC_JMP_BUF_R22 * OR1K_GPRSIZE(r3)
        l.lwz   r24, CYGARC_JMP_BUF_R24 * OR1K_GPRSIZE(r3)
        l.lwz   r26, CYGARC_JMP_BUF_R26 * OR1K_GPRSIZE(r3)
        l.lwz   r28, CYGARC_JMP_BUF_R28 * OR1K_GPRSIZE(r3)
        l.lwz   r30, CYGARC_JMP_BUF_R30 * OR1K_GPRSIZE(r3)

        l.jr    lr
        l.nop                   # delay slot
FUNC_END(hal_longjmp)
        

#------------------------------------------------------------------------------
# end of context.S
