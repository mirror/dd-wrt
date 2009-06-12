##==========================================================================
##
##      hal_var_sp.inc
##
##      SH support code for variants using stack at exception entry
##
##==========================================================================
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    jskov
## Contributors: jskov
## Date:         2002-01-11
## Purpose:      SH support code for variants using stack at exception entry
##
######DESCRIPTIONEND####
##
##==========================================================================

#include <cyg/hal/sh2_offsets.inc>
#include <cyg/hal/hal_intr.h>
	
## This is the address of the reset entry

#if defined(CYG_HAL_STARTUP_RAM) || defined(CYGARC_SH_MOD_CAC_NO_WINDOWS)
	// No translation if RAM startup, or if the variant does not
	// have cached/non-cached windows in the address space.
# define _RESET_ENTRY CYG_LABEL_DEFN(_reset_platform)
#elif defined(CYG_HAL_STARTUP_ROMRAM)
	// Uncached "shadow" address but adjusted for VMA/LMA differences
# define _RESET_ENTRY __reset_platform+0x20000000-CYGMEM_REGION_ram+CYGMEM_REGION_rom
#else
        // Uncached "shadow" address
# define _RESET_ENTRY CYG_LABEL_DEFN(_reset_platform)+0x20000000
#endif

#if !defined(CYG_HAL_STARTUP_RAM) || !defined(CYGSEM_HAL_USE_ROM_MONITOR)

## This is the table of HW_EXC_ENTRY pointers fetched from by the CPU
## on an exception. They point to pre-VSR handlers (below) which prep
## R0 with the vector number (which, AFAICT, there''s no other way
## to find) before calling the VSR.
## Note that this takes up a massive 4KiB so it should only be included
## when really needed (that is, when not relying on RedBoot to provide
## it).

        .align   2
FUNC_START(_HW_EXC_ENTRY_TABLE)
# Power-on reset entry address and stack
	.long _RESET_ENTRY
	.long 0
# Manual reset entry address and stack
	.long _RESET_ENTRY
	.long 0
# All the other vectors
	.set    vecno, 0
        .rept   (256-4)
	.long   $vectors_code+(vecno)*16
	.set    vecno, vecno+1
	.endr

## The piece of code pointed to by the HW_EXC table. Each vector
## has such a piece of code, which saves R0 on the stack, fetches
## the VSR pointer and loads the vector number into R0.
##
## Note the three variants:
##  exception_vector:      maps the HW_EXC vector to the same eCos HAL_VECTOR
##  exception_vector_trap: maps the HW_EXC vector to the eCos HAL_VECTOR_TRAP
##  exception_vector_int:  maps the HW_EXC vector to the eCos HAL_VECTOR_INTERRUPT
##
## With these we translate the SH2 vector style exceptions to something more
## like the SH3/SH4 style:
##
##  HW vectors         eCos vectors
##    00-31                0-31  Various exceptions
##    32-63                32    TRAP
##    64-255               33    Interrupt
##                         34-63 Free for application
## 
## This allows the VSR table to be reduced to 1/4 of the size, and eases
## code sharing with the SH3/SH4 variants without any real loss of power.
##
	.macro exception_vector vec
	mov.l r0,@-r15
        mov.l 1f,r0
	mov.l @r0,r0
	jmp   @r0
	 mov  #\vec,r0
	.align 2
1:	.long CYG_LABEL_DEFN(hal_vsr_table)+4*\vec
	.endm

	.macro exception_vector_trap vec
	mov.l r0,@-r15
        mov.l 1f,r0
	mov.l @r0,r0
	jmp   @r0
	 mov  #\vec,r0
	.align 2
1:	.long CYG_LABEL_DEFN(hal_vsr_table)+4*CYGNUM_HAL_VECTOR_TRAP
	.endm

	# The comment argument is not used for anything. It's just there
	# to provide callers a slot for comments (which is not otherwise
	# possible when doing multiple invocations on the same line).
	.macro exception_vector_int vec,comment
	mov.l r0,@-r15
        mov.l 1f,r0
	mov.l @r0,r0
	jmp   @r0
	 mov  #\vec,r0
	.align 2
1:	.long CYG_LABEL_DEFN(hal_vsr_table)+4*CYGNUM_HAL_VECTOR_INTERRUPT
	.endm

# And their entry points
$vectors_code:
	exception_vector 4		// general illegal instr
	exception_vector 5		// reserved
	exception_vector 6		// slot illegal instr
	exception_vector 7		// reserved
	exception_vector 8 		// reserved
	exception_vector 9 		// CPU address error
	exception_vector 10		// DMA address error
	exception_vector_int CYGNUM_HAL_INTERRUPT_NMI // NMI 
	exception_vector 12		// User Break
	exception_vector 13		// H-UDI
	.set    vecno, 14
	.rept   (32-14)
	exception_vector vecno		// reserved
	.set    vecno, vecno+1
	.endr
	.set    vecno, 32
	.rept   (64-32)
	exception_vector_trap vecno	// Trap
	.set    vecno, vecno+1
	.endr

#ifdef CYGHWR_HAL_SH_SH2_CUSTOM_INTERRUPT_LAYOUT
	# Some variants may have a very sparsely populated
	# vector table (7044 is an example) which results
	# in many unused entries in various interrupt tables.
	# To reduce bloat, these may define a custom
	# layout of these interrupt decoders - the code
	# is to be found in var_intr.h since it's very
	# tightly coupled with the interrupt vectors.
	CYGHWR_HAL_SH_SH2_CUSTOM_INTERRUPT_LAYOUT
#else
	# Note that there''s an entry for vector 0 (NMI) here again
	# to avoid having to make hal_interrupt_set_vectors()
	# adjust for this off-by-one discrepancy.
	.set    vecno, CYGNUM_HAL_INTERRUPT_NMI
	.rept   (256-64)
	exception_vector_int vecno	// interrupts
	.set    vecno, vecno+1
	.endr
#endif // CYGHWR_HAL_SH_SH2_CUSTOM_INTERRUPT_LAYOUT

#endif // !defined(CYG_HAL_STARTUP_RAM) || !defined(CYGSEM_HAL_USE_ROM_MONITOR)

## For RAM startups, provide a convenient jump to the application start
## at the very start of the image.
FUNC_START(_reset)
        mov.l   1f,r0
        jmp     @r0
         nop
	.align  2
1:	.long _RESET_ENTRY


##-----------------------------------------------------------------------------
## Macros for saving/restoring register state on an exception. These
## are generic for all variants, so be careful to not make assumptions.

## r15 is location stored to/loaded from
## r0  is the available scratch register

## At exit:
## r7  is the vector #
## all other registers (except sp) are available

## Furthermore, stack content at this point is
        #-- original SP address
	# pre-exception SR
	# pre-exception PC
	# pre-exception r0
        #-- present SP address
	
	.macro hal_cpu_save_regs
	mov.l	r0,@-sp                 ! vector number
#ifdef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        add     #-8,sp                  ! Space for gbr, and vbr
#else
        stc     gbr,r0                  ! GBR
        mov.l   r0,@-sp
        stc     vbr,r0                  ! VBR
        mov.l   r0,@-sp
#endif

        add     #-8,sp                  ! Space for entry PC and SR

#if 0 // FIXME
        stc.l   re,@-sp                 ! RE
        stc.l   rs,@-sp                 ! RS
        stc.l   mod,@-sp                ! MOD
#endif
        sts.l   pr,@-sp                 ! PR
        sts.l   mach,@-sp               ! mach
        sts.l   macl,@-sp               ! macl

        add     #-4,sp                  ! Space for entry sp
        mov.l   r14,@-sp                ! r14-r0
        mov.l   r13,@-sp
        mov.l   r12,@-sp
        mov.l   r11,@-sp
        mov.l   r10,@-sp
        mov.l   r9,@-sp
        mov.l   r8,@-sp
        mov.l   r7,@-sp
        mov.l   r6,@-sp
        mov.l   r5,@-sp
        mov.l   r4,@-sp
        mov.l   r3,@-sp
        mov.l   r2,@-sp
        mov.l   r1,@-sp
        add     #-4,sp                  ! space for r0

	! Compute location of pre-exception r0 and move
	! data from above structure into the structure
	mov     sp,r1
	add     #CYGARC_SH_EXCEPTION_SIZE,r1
	mov.l   @r1+,r0			! pre-exception R0
	mov.l	r0,@(CYGARC_SHREG_REGS,sp)
	mov.l   @r1+,r2			! pre-exception PC
	mov     #CYGARC_SHREG_PC,r0
	mov.l	r2,@(r0,sp)
	mov.l   @r1+,r2			! pre-exception SR
	mov     #CYGARC_SHREG_SR,r0
	mov.l	r2,@(r0,sp)
	mov     #CYGARC_SHREG_SP,r0
	mov.l   r1,@(r0,sp)             ! pre-exception SP
	! Load up the vector number
	mov     #CYGARC_SHREG_EVENT,r0
	mov.l   @(r0,sp),r7
	.endm

	.macro hal_exception_entry_extras
        # Disable interrupts before anything else
        mov.l   1f,r0
        ldc     r0,sr
	bra	2f
	 nop
        .align  2        
1:	.long   CYG_SR
2:
	.endm

	# No additional magic needed. hal_cpu_save_regs does it all.
	.macro hal_interrupt_entry_extras
	.endm

#---------------------------------------------------------------------------
## Restore registers after exception:
## At entry:
## r15 is location to be loaded from
## all other registers (except sp) are available

## At exit:
##  Returns to interrupted code

	.macro hal_cpu_restore_regs_return
        ! Disable interrupts during the restore operation
        mov.l   1f,r1
        ldc     r1,sr

        ! Move R0/PC/SR values from register structure onto the stack where
        ! they''ll be popped from on return. Note that this is the SP as
	! set in the register frame! (otherwise GDB would be unable to change
	! SP).
	mov.l	@(CYGARC_SHREG_SP,sp),r0
	mov     r0,r1
	add     #-12,r1
	mov.l	@(CYGARC_SHREG_REGS,sp),r0
	mov.l   r0,@r1			! pre-exception R0
	mov     #CYGARC_SHREG_PC,r0
	mov.l	@(r0,sp),r2
	mov.l   r2,@(4,r1)		! pre-exception PC
	mov     #CYGARC_SHREG_SR,r0
	mov.l	@(r0,sp),r2
	mov.l   r2,@(8,r1)		! pre-exception SR

	! Load up registers
        add     #4,sp			! skip r0
        mov.l   @sp+,r1
        mov.l   @sp+,r2
        mov.l   @sp+,r3
        mov.l   @sp+,r4
        mov.l   @sp+,r5
        mov.l   @sp+,r6
        mov.l   @sp+,r7
        mov.l   @sp+,r8
        mov.l   @sp+,r9
        mov.l   @sp+,r10
        mov.l   @sp+,r11
        mov.l   @sp+,r12
        mov.l   @sp+,r13
        mov.l   @sp+,r14
        add     #4,sp                   ! skip SP

        lds.l   @sp+,macl               ! macl
        lds.l   @sp+,mach               ! mach
        lds.l   @sp+,pr                 ! PR

#if 0 // FIXME - also change SP adjustment below!
        ldc.l   @sp+,re                 ! RE
        ldc.l   @sp+,rs                 ! RS
        ldc.l   @sp+,mod                ! MOD
#endif
	add     #8,sp			! skip sr and pc

#ifndef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
        mov.l   @sp+,r0
        ldc     r0,vbr                  ! return VBR
        mov.l   @sp+,r0
        ldc     r0,gbr                  ! return GBR
#else
	add     #8,sp                   ! skip VBR+GBR
#endif

	add     #-8*4,sp                ! get to new SP
	mov.l   @sp,sp
	add     #-12,sp                 ! get to exception state + saved r0

	mov.l	@sp+,r0			! pre-exception r0
	rte
	 nop
	.align  2
1:	.long   CYG_SR
	.endm

#---------------------------------------------------------------------------
# Translate cause of exception to a vector number
	.macro hal_exception_translate
	.endm

#---------------------------------------------------------------------------
# end of hal_var_sp.inc
