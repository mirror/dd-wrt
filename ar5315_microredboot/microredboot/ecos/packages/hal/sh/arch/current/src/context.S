##=============================================================================
##
##      context.S
##
##      SH context switch code
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 Nick Garnett 
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
## Author(s):   jskov
## Contributors:jskov, nickg
## Date:        1999-05-01
## Purpose:     SH context switch code
## Description: This file contains implementations of the thread context 
##              switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>
#include <cyg/hal/sh_offsets.inc>
#include <cyg/hal/basetype.h>           // CYG_LABEL_DEFN
                
#------------------------------------------------------------------------------
# Register allocation for the Hitachi calling convention:
#
#       r0		arg return
#	r1..r3          scratch
#	r4..r7		args in
#	r8..r13		call saved
#	r14		frame pointer/call saved
#	r15		stack pointer
#	ap		arg pointer (doesn''t really exist, always eliminated)
#	pr		subroutine return address
#	t               t bit
#	mach		multiply/accumulate result, high part
#	macl		multiply/accumulate result, low part.
#	fpul		fp/int communication register
#	rap		return address pointer register
#	fr0		fp arg return
#	fr1..fr3	scratch floating point registers
#	fr4..fr11	fp args in
#	fr12..fr15	call saved floating point registers
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# R4 = address of sp of next thread to execute
# R5 = address of sp save location of current thread

# Need to save r8-r13, r14 (fp), r15 (sp), pr, mach, macl

FUNC_START(hal_thread_switch_context)
        mov     r15, r0                 ! R0 = saved stack pointer

        sts.l   pr,@-r15                ! save caller in PC slot so it looks
                                        ! sensible when GDB examines it
        stc     sr,r1
        mov.l   r1,@-r15

        sts.l   pr,@-r15                ! return address

#ifdef CYGHWR_HAL_SH_FPU
        sts.l   fpscr,@-r15
        sts.l   fpul,@-r15
#if CYGHWR_HAL_SH_FPU_REGS == 32
        frchg
        fmov.s  fr15,@-r15
        fmov.s  fr14,@-r15
        fmov.s  fr13,@-r15
        fmov.s  fr12,@-r15
        fmov.s  fr11,@-r15
        fmov.s  fr10,@-r15
        fmov.s  fr9,@-r15
        fmov.s  fr8,@-r15
        fmov.s  fr7,@-r15
        fmov.s  fr6,@-r15
        fmov.s  fr5,@-r15
        fmov.s  fr4,@-r15
        fmov.s  fr3,@-r15
        fmov.s  fr2,@-r15
        fmov.s  fr1,@-r15
        fmov.s  fr0,@-r15
        frchg
#endif          
        fmov.s  fr15,@-r15
        fmov.s  fr14,@-r15
        fmov.s  fr13,@-r15
        fmov.s  fr12,@-r15
        fmov.s  fr11,@-r15
        fmov.s  fr10,@-r15
        fmov.s  fr9,@-r15
        fmov.s  fr8,@-r15
        fmov.s  fr7,@-r15
        fmov.s  fr6,@-r15
        fmov.s  fr5,@-r15
        fmov.s  fr4,@-r15
        fmov.s  fr3,@-r15
        fmov.s  fr2,@-r15
        fmov.s  fr1,@-r15
        fmov.s  fr0,@-r15
#endif
        
        sts.l   macl,@-r15              ! macl
        sts.l   mach,@-r15              ! mach
        mov.l   r0,@-r15                ! saved r15 (entry sp)
        mov.l   r14,@-r15               ! r14-r0
        mov.l   r13,@-r15
        mov.l   r12,@-r15
        mov.l   r11,@-r15
        mov.l   r10,@-r15
        mov.l   r9,@-r15
        mov.l   r8,@-r15
        mov.l   r7,@-r15
        mov.l   r6,@-r15
        mov.l   r5,@-r15
        mov.l   r4,@-r15
        mov.l   r3,@-r15
        mov.l   r2,@-r15
        mov.l   r1,@-r15
        mov.l   r0,@-r15

        mov.l   r15,@r5                 ! save SP into save location

        # Now load the destination thread by dropping through
        # to hal_thread_load_context
        
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# R4 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context
# and is simply dropped into from it.

FUNC_START(hal_thread_load_context)
        
        mov.l   @r4,r0

        add     #4+4,r0                 ! skip r0 and r1
        !mov.l   @r0+,r0
        !mov.l   @r0+,r1
        mov.l   @r0+,r2
        mov.l   @r0+,r3
        mov.l   @r0+,r4
        mov.l   @r0+,r5
        mov.l   @r0+,r6
        mov.l   @r0+,r7
        mov.l   @r0+,r8
        mov.l   @r0+,r9
        mov.l   @r0+,r10
        mov.l   @r0+,r11
        mov.l   @r0+,r12
        mov.l   @r0+,r13
        mov.l   @r0+,r14
        mov.l   @r0+,r3                 ! New SP.

        lds.l   @r0+,mach               ! mach
        lds.l   @r0+,macl               ! macl

#ifdef CYGHWR_HAL_SH_FPU
        fmov.s  @r0+,fr0
        fmov.s  @r0+,fr1
        fmov.s  @r0+,fr2
        fmov.s  @r0+,fr3
        fmov.s  @r0+,fr4
        fmov.s  @r0+,fr5
        fmov.s  @r0+,fr6
        fmov.s  @r0+,fr7
        fmov.s  @r0+,fr8
        fmov.s  @r0+,fr9
        fmov.s  @r0+,fr10
        fmov.s  @r0+,fr11
        fmov.s  @r0+,fr12
        fmov.s  @r0+,fr13
        fmov.s  @r0+,fr14
        fmov.s  @r0+,fr15
#if CYGHWR_HAL_SH_FPU_REGS == 32
        frchg
        fmov.s  @r0+,fr0
        fmov.s  @r0+,fr1
        fmov.s  @r0+,fr2
        fmov.s  @r0+,fr3
        fmov.s  @r0+,fr4
        fmov.s  @r0+,fr5
        fmov.s  @r0+,fr6
        fmov.s  @r0+,fr7
        fmov.s  @r0+,fr8
        fmov.s  @r0+,fr9
        fmov.s  @r0+,fr10
        fmov.s  @r0+,fr11
        fmov.s  @r0+,fr12
        fmov.s  @r0+,fr13
        fmov.s  @r0+,fr14
        fmov.s  @r0+,fr15
        frchg
#endif                  
        lds.l   @r0+,fpul
        lds.l   @r0+,fpscr
#endif
        
        lds.l   @r0+,pr                 ! pr

        mov     r3,r15                  ! update stack pointer

        mov.l   @r0+,r2                 ! SR
        hal_cpu_int_merge r2,r0,r1      ! restore interrupt state

        rts                             ! and return
         nop

#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only to callee save registers r8-r13, r14(fp), r15(sp)
# and pr into buffer supplied in r4[arg0]

FUNC_START(hal_setjmp)
        mov.l   r15,@(CYGARC_JMPBUF_SP,r4)
        sts     pr,r0
        mov.l   r0,@(CYGARC_JMPBUF_PR,r4)
        mov.l   r8,@(CYGARC_JMPBUF_R8,r4)
        mov.l   r9,@(CYGARC_JMPBUF_R9,r4)
        mov.l   r10,@(CYGARC_JMPBUF_R10,r4)
        mov.l   r11,@(CYGARC_JMPBUF_R11,r4)
        mov.l   r12,@(CYGARC_JMPBUF_R12,r4)
        mov.l   r13,@(CYGARC_JMPBUF_R13,r4)
        mov.l   r14,@(CYGARC_JMPBUF_R14,r4)

        mov    #0,r0            ! return 0
        rts
         nop

# hal_longjmp loads state from r4[arg0] and returns
# argument supplied in r5[arg1]

FUNC_START(hal_longjmp)
        mov.l   @(CYGARC_JMPBUF_SP,r4),r15
        mov.l   @(CYGARC_JMPBUF_PR,r4),r0
        lds     r0,pr
        mov.l   @(CYGARC_JMPBUF_R8,r4),r8
        mov.l   @(CYGARC_JMPBUF_R9,r4),r9
        mov.l   @(CYGARC_JMPBUF_R10,r4),r10
        mov.l   @(CYGARC_JMPBUF_R11,r4),r11
        mov.l   @(CYGARC_JMPBUF_R12,r4),r12
        mov.l   @(CYGARC_JMPBUF_R13,r4),r13
        mov.l   @(CYGARC_JMPBUF_R14,r4),r14

        mov     r5,r0
        rts
         nop

#------------------------------------------------------------------------------
# end of context.S
