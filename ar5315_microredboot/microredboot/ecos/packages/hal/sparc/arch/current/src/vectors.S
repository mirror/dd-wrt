/*=============================================================================
//
//	vectors.S
//
//	SPARC vectors and bootup code
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	hmt
// Contributors:hmt
// Date:	1998-12-15
// Purpose:	SPARC vector code
// Description:	This file contains the code which hangs off SPARC vectors
//              including reset; it handles register under/overflow as well
//              as bootup, anything else is deferred to the default interrupt
//              or exception vsrs respectively.  See vec_[ix]vsr.S ...
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

!-----------------------------------------------------------------------------
	
//	.file	"vectors.S"

!-----------------------------------------------------------------------------
		
#include <pkgconf/system.h>	
#include <pkgconf/hal.h>

#include <pkgconf/hal_sparc.h>
#include CYGBLD_HAL_PLATFORM_H		// Platform config file

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#else
# undef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
# undef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
#endif
			
#if defined( CYGPKG_HAL_SPARCLITE_SIM ) || \
    defined( CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK ) || \
    defined( CYGPKG_HAL_SPARC_LEON ) || \
    defined( CYGPKG_HAL_SPARC_ERC32 )
#define BOOTUPSTACK_IS_INTERRUPTSTACK
#endif
	
//#define CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING

#ifndef CYGHWR_HAL_SPARC_HAS_ASR17
#ifndef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
#error Single Vector Trapping (SVT) demands ASR17 
#endif
#endif
	
#ifdef CYG_HAL_STARTUP_ROM
# ifndef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING

! ROM startup and Single Vector Trapping demands a copy to RAM.
! Otherwise it may be configured in, but it is not required.
	
#  ifndef CYGIMP_HAL_SPARC_COPY_VECTORS_TO_RAM
#   define CYGIMP_HAL_SPARC_COPY_VECTORS_TO_RAM
#  endif

# endif
#endif

!------------------------------------------------------------------------

#include <cyg/hal/vectors.h>

#define DELAYS_AFTER_WRPSR_SAME_WINDOW
#define DELAYS_AFTER_WRWIM

!------------------------------------------------------------------------

#ifdef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING

	.macro	VECTOR_EXCEPTION
	.p2align 4
	rd	%tbr, %l3
	rd	%psr, %l0
	ba	__entry_exception
	 and	%l3, TT_MASK, %l4
	.endm
	
	.macro	VECTOR_INTERRUPT level
	.p2align 4
	rd	%psr, %l0
	mov	\level << 2, %l5
	ba	__entry_interrupt
	 mov	\level << 4, %l4
	.endm
	
	.macro	VECTOR_CODE_WIM name
	.p2align 4
	ba	__entry_\name
	 rd	%wim, %l0
	.endm
	
#endif // CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING

!------------------------------------------------------------------------
! The start of the code; this is the entry point:
	
	.section ".vectors","ax"

	.global rom_vectors
rom_vectors:
	.global	reset_vector
reset_vector:	
	! this code goes to the real reset handler, it will be
	! overwritten by the start of vectoring handler...
	b	genuine_reset
	nop
	! usually drop through to:
#ifdef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
	ba	__entry_exception	! reset becomes an exception
	 and	%l3, TT_MASK, %l4	! once we are running
	
	VECTOR_EXCEPTION	 	!   1
	VECTOR_EXCEPTION	 	!   2
	VECTOR_EXCEPTION	 	!   3
	VECTOR_EXCEPTION	 	!   4
	
	VECTOR_CODE_WIM wover	 	!   5 window overflow
	VECTOR_CODE_WIM wunder		!   6 window underflow
	VECTOR_EXCEPTION		!   7
	VECTOR_EXCEPTION		!   8
	VECTOR_EXCEPTION		!   9

	VECTOR_EXCEPTION		!  10
	VECTOR_EXCEPTION		!  11
	VECTOR_EXCEPTION		!  12
	VECTOR_EXCEPTION		!  13
	VECTOR_EXCEPTION		!  14
	
	VECTOR_EXCEPTION		!  15
	VECTOR_EXCEPTION		!  16
	VECTOR_INTERRUPT  1		!  17 interrupt_level_1
	VECTOR_INTERRUPT  2		!  18 interrupt_level_2
	VECTOR_INTERRUPT  3		!  19 interrupt_level_3
	
	VECTOR_INTERRUPT  4		!  20 interrupt_level_4
	VECTOR_INTERRUPT  5		!  21 interrupt_level_5
	VECTOR_INTERRUPT  6		!  22 interrupt_level_6
	VECTOR_INTERRUPT  7		!  23 interrupt_level_7
	VECTOR_INTERRUPT  8		!  24 interrupt_level_8
	
	VECTOR_INTERRUPT  9		!  25 interrupt_level_9
	VECTOR_INTERRUPT 10		!  26 interrupt_level_10
	VECTOR_INTERRUPT 11		!  27 interrupt_level_11
	VECTOR_INTERRUPT 12		!  28 interrupt_level_12
	VECTOR_INTERRUPT 13		!  29 interrupt_level_13
	
	VECTOR_INTERRUPT 14		!  30 interrupt_level_14
	VECTOR_INTERRUPT 15		!  31 interrupt_level_15
	VECTOR_EXCEPTION		!  32
	VECTOR_EXCEPTION		!  33
	VECTOR_EXCEPTION		!  34

	VECTOR_EXCEPTION		!  35
	VECTOR_EXCEPTION		!  36
	VECTOR_EXCEPTION		!  37
	VECTOR_EXCEPTION		!  38
	VECTOR_EXCEPTION		!  39
	
	.rept 216			! 40-255 is 216 of them
	VECTOR_EXCEPTION		! whichever
	.endr
#endif // CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING

#ifndef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
real_vector:
	! here,locals have been set up as follows:
	! %l0 = psr
	! %l1 = pc	
	! %l2 = npc
	! %l3 = tbr
	and	%l3, TT_IS_INTR_MASK, %l4
	cmp	%l4, TT_IS_INTR_VALUE
	bne	not_an_interrupt	! delay slot does not matter
	! here be the pre-vector interrupt handler
interrupt:
	and	%l3, 0x0f0, %l4		! get an interrupt number out
	srl	%l4, 2, %l5		! to a word address offset
#endif // !CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
__entry_interrupt:
	sethi	%hi(hal_vsr_table), %l6
	or	%l6, %lo(hal_vsr_table), %l6
	ld	[ %l6 + %l5 ], %l6	! get vector in %l6
	jmp	%l6			! and go there
	srl	%l4, 4, %l3		! vector number into %l3: so that
					! interrupts and exceptions/traps
					! have the same API to VSRs

#ifndef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
not_an_interrupt:
	and	%l3, TT_MASK, %l4
	cmp	%l4, (TRAP_WUNDER << TT_SHL)

	bne,a	not_window_underflow
	cmp	%l4, (TRAP_WOVER << TT_SHL)	! (if taken)

	! here be the window underflow handler:
window_underflow:
	! CWP is trap handler
	! CWP + 1 is trapped RESTORE instruction
	! CWP + 2 is invalid context which must be restored
	! CWP + 3 is next invalid context

	wr	%l0, %psr		! restore the condition flags
					! (CWP is unchanged)
	! the following instructions delay enough; no need for NOPs
	rd	%wim, %l0		! get the wim
#endif // !CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
__entry_wunder:	
	sll	%l0, 1, %l3		! Rotate wim left
	srl	%l0, __WINSIZE-1, %l0
	wr	%l0, %l3, %wim		! Install the new wim

#ifdef DELAYS_AFTER_WRWIM
	nop				! are these delays needed?
	nop				! (following restore uses wim)
	nop
#endif
	restore				! Users window
	restore				! Her callers window (now valid)
	
	ldd	[%sp + 0 * 4], %l0	! restore L & I registers
	ldd	[%sp + 2 * 4], %l2
	ldd	[%sp + 4 * 4], %l4
	ldd	[%sp + 6 * 4], %l6

	ldd	[%sp + 8 * 4], %i0
	ldd	[%sp + 10 * 4], %i2
	ldd	[%sp + 12 * 4], %i4
	ldd	[%sp + 14 * 4], %i6

	save				! Back to trap window
	save	

	jmp	%l1
	rett	%l2

#ifndef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
not_window_underflow:
	bne,a	not_window_overflow
	srl	%l4, 4, %l4		! (if taken)

	! here be the window overflow handler:
window_overflow:
	! CWP + 1 is caller whose SAVE bounced
	! CWP is trap handler = invalid window
	! CWP - 1 is next invalid window which needs to be saved

	wr	%l0, %psr		! restore the condition flags
        				! (CWP is unchanged)
	! the following instructions delay enough; no need for NOPs
	rd	%wim, %l0
#endif // !CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
__entry_wover:	
	mov	%g1, %l3		! Save g1, we use it to hold the wim
	srl	%l0, 1, %g1		! Rotate wim right
	sll	%l0, __WINSIZE-1, %l0
	or	%l0, %g1, %g1

	save				! Slip into next window
	mov	%g1, %wim		! Install the new wim
					! (invalidates current window!)

	std	%l0, [%sp + 0 * 4]	! save L & I registers
	std	%l2, [%sp + 2 * 4]
	std	%l4, [%sp + 4 * 4]
	std	%l6, [%sp + 6 * 4]

	std	%i0, [%sp + 8 * 4]
	std	%i2, [%sp + 10 * 4]
	std	%i4, [%sp + 12 * 4]
	std	%i6, [%sp + 14 * 4]

	restore				! Go back to trap window.
	mov	%l3, %g1		! Restore %g1

	jmpl	%l1,  %g0
	rett	%l2

#ifdef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
	// ADDITIONAL code to provide an entry point:
__entry_exception:
	srl	%l4, 4, %l4
#endif // CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
not_window_overflow:
	! from here on in, %l4 is the trap number in clear
	cmp	%l4, 128 
	bge	1f
	mov	0, %l5			! offset 0 for user traps

	cmp	%l4, 36			! coprocessor special case
	beq	1f
	mov	4, %l5			! ...treated as FP, code 4

	cmp	%l4, 10
	bge	1f
	mov	10, %l5			! offset 10 for "others"

	! if we are here, the trap number is 1-9 inclusive
	! so put it in %l5 and drop through...
	mov	%l4, %l5
1:	
	or	%l5, 16, %l5		! offset into table is 16... for traps.
	sll	%l5,  2, %l5		! to a word address offset
	sethi	%hi(hal_vsr_table), %l6
	or	%l6, %lo(hal_vsr_table), %l6
	ld	[ %l6 + %l5 ], %l6	! get vector in %l6
	jmp	%l6			! and go there
	srl	%l5, 2, %l3		! vector number into %l3: so that
					! interrupts and exceptions/traps
					! have the same API to VSRs
	! NB that~s eCos vector number not TRAP number above.
	
	! and that is the end of the pre-vector trap handler	

	.global rom_vectors_end
rom_vectors_end:
		
	! these instructions are copied into the reset vector
	! after startup to _not_ branch to the genuine_reset code below
real_vector_instructions:
	rd	%tbr, %l3
	rd	%psr, %l0



	! genuine reset code called from time zero:
genuine_reset:				! set psr, mask interrupts & traps
	sethi	%hi(reset_vector), %g1
	andn	%g1, 0xfff, %g1		! should not be needed
	wr	%g1, %tbr		! Traps are at reset_vector
	wr	%g0, 0xfc0 + __WIN_INIT, %psr ! mode = prevMode = S, CWP=7
	wr	%g0, 0, %wim		! No invalid windows (yet)
#ifdef CYGHWR_HAL_SPARC_HAS_ASR17
#ifdef CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
	wr	%g0, 0, %asr17		! Multiple vector trapping
#else
	wr	%g0, 1, %asr17		! Single vector trapping
#endif // !CYGHWR_HAL_SPARC_MULTIPLE_VECTOR_TRAPPING
#endif // CYGHWR_HAL_SPARC_HAS_ASR17
	nop
	nop
	nop

// INCLUDE PLATFORM BOOT

// This should set up RAM and caches, and calm down any external interrupt
// sources.  Also copy two instructions from real_vector_instructions
// into reset_vector, then invalidate the instruction cache.
	
#include <cyg/hal/halboot.si>
	
// halboot.si returns with %sp all set, in sleb versions.
// (though we override if there is an interrupt stack)

	led	0x80
		
	! now set up a stack and initial frame linkage
	! so as to be able to make C function calls:
	! current window is 7, the highest, so we store a
	! saved frame thingy that refers to itself in the stack,
	! then another which is valid and drop into main from there.

#ifdef BOOTUPSTACK_IS_INTERRUPTSTACK
	sethi	%hi(cyg_interrupt_stack), %i6
	or	%i6, %lo(cyg_interrupt_stack), %i6
#endif	
	andn	%i6, 7, %i6		! round fp down to double alignment
	mov	0, %i7			! null return address
	sethi	%hi(0xb51ac000), %i0	! "BStac" pattern
	or	%i0, 24, %i0	
	or	%i0, 1, %i1
	or	%i0, 2, %i2
	or	%i0, 3, %i3
	or	%i0, 4, %i4
	or	%i0, 5, %i5

	sethi	%hi(0xb51ac000), %l0	! "BStac" pattern
	or	%l0, 16, %l0	
	or	%l0, 1, %l1
	or	%l0, 2, %l2
	or	%l0, 3, %l3
	or	%l0, 4, %l4
	or	%l0, 5, %l5
	or	%l0, 6, %l6
	or	%l0, 7, %l7
	
	sub	%fp, 16 * 4, %sp	! Stack pointer

	led	0x90
	
	std	%l0, [%sp + 0 * 4]	! save L & I registers
	std	%l2, [%sp + 2 * 4]	! into new stack frame
	std	%l4, [%sp + 4 * 4]
	std	%l6, [%sp + 6 * 4]

	led	0x91
	
	std	%i0, [%sp + 8 * 4]
	std	%i2, [%sp + 10 * 4]
	std	%i4, [%sp + 12 * 4]
	std	%i6, [%sp + 14 * 4]
	
	led	0x92
	
	sethi	%hi(0xb0010000), %o0	! "Boot" pattern
	or	%o0, 8, %o0	
	or	%o0, 1, %o1
	or	%o0, 2, %o2
	or	%o0, 3, %o3
	or	%o0, 4, %o4
	or	%o0, 5, %o5

	led	0x98
		
	wr	%g0, __WIM_INIT, %wim	! Window 7 (current) is invalid
	nop
	nop
	nop
	
	led	0x99
	
	sethi	%hi(0xb0010000), %g1	! "Boot" pattern
	or	%g1, 2, %g2
	or	%g1, 3, %g3
	or	%g1, 4, %g4
	or	%g1, 5, %g5
	or	%g1, 6, %g6
	or	%g1, 7, %g7
	or	%g1, 1, %g1

	led	0xa0
	
	wr	%g0, 0xfe0 + __WIN_INIT, %psr
	nop				! Enable traps:             
	nop				! set psr, _do_ mask interrupts
	nop				! mode = prevMode = S, CWP=7

	led	0xb0

	! now we can start calling out and running C code!
	.extern cyg_hal_start	
	call	cyg_hal_start		! puts return address in %o7
	or	%g1, 1, %g1
	
loop_forever:
	ta	1
	b	loop_forever		! if it returns
	nop

	
!---------------------------------------------------------------------------
! hal_vsr_table...

	.section ".data"
	.balign 4
	.global hal_vsr_table
hal_vsr_table:
	.rept 16
	.word hal_default_interrupt_vsr
	.endr
	.rept 11
	.word hal_default_exception_vsr
	.endr

!---------------------------------------------------------------------------
! Bootup stack (only needed explicitly in sim)
	
#ifdef BOOTUPSTACK_IS_INTERRUPTSTACK
	.section ".bss"

#ifndef CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#define CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE 4096
#endif
	.balign 16
	.global cyg_interrupt_stack_base
cyg_interrupt_stack_base:
	.rept CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
	.byte 0
	.endr
	.balign 16
	.global cyg_interrupt_stack
cyg_interrupt_stack:
	.long	0,0,0,0,0,0,0,0		! here be secret state stored
#endif

!------------------------------------------------------------------------
! Define a section that reserves space at the start of RAM for the
! vectors to be copied into, for ROM start only.
	
	.section ".ram_vectors","awx",@nobits
#ifdef CYGIMP_HAL_SPARC_COPY_VECTORS_TO_RAM
	! need a space at base of RAM for copied vector/trampoline code
	.align 0x1000
	.space 8			! for fencepost errors
	.space (rom_vectors_end - rom_vectors)
#endif // CYGIMP_HAL_SPARC_COPY_VECTORS_TO_RAM

!------------------------------------------------------------------------
! end of vectors.S
