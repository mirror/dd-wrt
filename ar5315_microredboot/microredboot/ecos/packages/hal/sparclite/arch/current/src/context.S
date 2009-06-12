/*=============================================================================
//
//	context.S
//
//	SPARClite context switch code
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
// Author(s): 	nickg, gthomas, hmt
// Contributors:	nickg, gthomas, hmt
// Date:	1998-12-15
// Purpose:	SPARClite context switch code
// Description:	This file contains implementations of the thread context 
//		switch routines. It also contains the longjmp() and setjmp()
//              routines.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/vectors.h>
	
#define DELAYS_AFTER_WRPSR_SAME_WINDOW
#define DELAYS_AFTER_WRWIM

	.text

! ------------------------------------------------------------------------------
!  hal_thread_switch_context
!  Switch thread contexts
!  %o0 = address of sp of next thread to execute
!  %o1 = address of sp save location of current thread

	.global	hal_thread_switch_context
hal_thread_switch_context:	

	! First take the stack down to make room for the saved register
	! state, including a window save area at the base.  Leave the
	! current window save area undisturbed.  It is unused within the
	! save but will become current again when we continue in this
	! context.  This lets us do this whole piece of work without
	! diabling interrupts for too long, since, for example, we can
	! lower the stack atomically with one instruction:
	sub	%sp, SAVE_REGS_SIZE, %sp
	st	%sp, [ %o1 ]		! return SP for this thread
			
	std	%l0, [%sp + 0 * 4]	! save L & I registers
	std	%l2, [%sp + 2 * 4]
	std	%l4, [%sp + 4 * 4]
	std	%l6, [%sp + 6 * 4]

	std	%i0, [%sp + 8 * 4]
	std	%i2, [%sp + 10 * 4]
	std	%i4, [%sp + 12 * 4]
	std	%i6, [%sp + 14 * 4]
		
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	st	%o7, [%sp + 31 * 4]	! save only the return address
	! and no need to preserve %o0 even though it is restored
#else // save a maximal context
	st	%g1, [%sp + 17 * 4]	! save G & O registers
	std	%g2, [%sp + 18 * 4]
	std	%g4, [%sp + 20 * 4]
	std	%g6, [%sp + 22 * 4]

	std	%o0, [%sp + 24 * 4]
	std	%o2, [%sp + 26 * 4]
	std	%o4, [%sp + 28 * 4]
	std	%o6, [%sp + 30 * 4]
#endif // !CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	! and save the CWP in %g0 save place
	rd	%psr, %g7
	st	%g7, [%sp + 16 * 4]
	
	! Now load the destination thread by dropping through
	! to hal_thread_load_context
	
! ------------------------------------------------------------------------------
!  hal_thread_load_context
!  Load thread context
!  %o0 = address of sp of next thread to execute
!  Note that this function is also the second half of hal_thread_switch_context
!  and is simply dropped into from it.

	.global hal_thread_load_context
hal_thread_load_context:

	! Here, we are a leaf routine but with slightly odd properties.
	! The stack is still the callers at this point but the register
	! set is up for grabs.  So we can use globals:
	
	ld	[ %o0 ], %g7		! Get the next saved SP
	
	! DISABLE INTERRUPTS *ONLY* NOT TRAPS
	rd	%psr, %g6
	or	%g6, 0xfe0, %g5		! PIL up to 15 leave traps enabled
	wr	%g5, %psr
	
	! force out all our callers register sets onto the stack
	! if necessary:	the system will handily take care of this for
	! us as follows:
	save	%sp, -16 * 4, %sp	! need all these to preserve
	save	%sp, -16 * 4, %sp	! the linked list property...
	save	%sp, -16 * 4, %sp
	save	%sp, -16 * 4, %sp
#if 6 < __WINSIZE
	save	%sp, -16 * 4, %sp
#if 7 < __WINSIZE
	save	%sp, -16 * 4, %sp
#endif
#endif
	! Fewer saves if fewer register windows.  For 8 register windows,
	! six of these is correct; a seventh would force out the current
	! set that was already saved manually above.  Note that minimal
	! space is allowed on stack for locals and ins in case this
	! sequence itself gets interrupted and recurses too deep.
	
	! now select the new window with traps disabled...

	! get the new PSR and CWP that we will ultimately restore
	! from the %g0 save place...
	ld	[%g7 + 16 * 4], %g6	! %g7 holds the new stack pointer
	andn	%g6, 0x20, %g5		! clear ET into %g5
	and	%g6, __WINBITS, %g4	! CWP bits only in %g4
		
	! calculate a new WIM...
	add	%g4, 1, %g3		! required invalid window number
#if 8 == __WINSIZE
	and	%g3, __WINBITS, %g3	! modulo 8
#else   // expect 5 or 6 or 7 windows
	cmp	%g3, __WINSIZE
	bge,a	567f			! taken: do delay slot, handle overflow
	 mov	0, %g3			! only if .ge. above
567:	
#endif
	mov	1, %g2
	sll	%g2, %g3, %g2		! converted to a mask for the WIM

	! DISABLE INTERRUPTS (TRAPS)
	wr	%g5, %psr		! set CWP to new window, disable traps
	wr	%g2, %wim		! and WIM to new value
	nop
	nop
	nop
		
	! Must do this atomically so that the registers match the stack.
	! After locals and ins are loaded, we are conformant to the PCS
	! so can re-enable interrupts.
	mov	%g7, %sp		! target sp in situ (%sp = %o6)

	ldd	[%sp + 0 * 4], %l0	! restore L & I registers
	ldd	[%sp + 2 * 4], %l2
	ldd	[%sp + 4 * 4], %l4
	ldd	[%sp + 6 * 4], %l6

	ldd	[%sp + 8 * 4], %i0
	ldd	[%sp + 10 * 4], %i2
	ldd	[%sp + 12 * 4], %i4
	ldd	[%sp + 14 * 4], %i6

	! RESTORE INTERRUPTS to saved state
	wr	%g6, %psr		! set new CWP and old ET and PIL
	nop
	nop
	nop
	
	! now load the rest of the context; we can be interrupted here
	! (if the saved context was a voluntary yield or threadstart)
	! but that is OK, other state will be preserved in that case...

#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	ld	[%sp + 24 * 4], %o0	! must load the initial argument
#else // restore a maximal context
	ld	[%sp + 17 * 4], %g1
	ldd	[%sp + 18 * 4], %g2
	ldd	[%sp + 20 * 4], %g4
	ldd	[%sp + 22 * 4], %g6

	ldd	[%sp + 24 * 4], %o0
	ldd	[%sp + 26 * 4], %o2
	ldd	[%sp + 28 * 4], %o4
#endif // !CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
					! %o6 = %sp, already set up
	ld	[%sp + 31 * 4], %o7	! "return" address

	retl
	add	%sp, SAVE_REGS_SIZE, %sp ! and set the stack back
					! to its entrant value

! ------------------------------------------------------------------------------
!  HAL longjmp, setjmp implementations

!FUNC_START(hal_setjmp)
	.global	hal_setjmp
hal_setjmp:
	! Treat this as a leaf routine, may as well.
	! %o0 is the address of the buffer.

	std	%l0, [%o0 + 0 * 4]	! save L & I registers
	std	%l2, [%o0 + 2 * 4]
	std	%l4, [%o0 + 4 * 4]
	std	%l6, [%o0 + 6 * 4]

	std	%i0, [%o0 + 8 * 4]
	std	%i2, [%o0 + 10 * 4]
	std	%i4, [%o0 + 12 * 4]
	std	%i6, [%o0 + 14 * 4]
		
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	std	%o6, [%o0 + 30 * 4]	! just save %sp and return address
#else // save a maximal context
	st	%g1, [%o0 + 17 * 4]	! save G & O registers
	std	%g2, [%o0 + 18 * 4]
	std	%g4, [%o0 + 20 * 4]
	std	%g6, [%o0 + 22 * 4]

	std	%o0, [%o0 + 24 * 4]
	std	%o2, [%o0 + 26 * 4]
	std	%o4, [%o0 + 28 * 4]
	std	%o6, [%o0 + 30 * 4]
#endif // !CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	
	! and save the CWP in %g0 save place
	rd	%psr, %g7
	st	%g7, [%o0 + 16 * 4]

	! DISABLE INTERRUPTS *ONLY* NOT TRAPS
	or	%g7, 0xfe0, %g6		! PIL up to 15 leave traps enabled
	wr	%g6, %psr
	
	! force out all our callers register sets onto the stack
	! if necessary:	the system will handily take care of this for
	! us as follows:
	save	%sp, -16 * 4, %sp	! need all these to preserve
	save	%sp, -16 * 4, %sp	! the linked list property...
	save	%sp, -16 * 4, %sp
	save	%sp, -16 * 4, %sp
#if 6 < __WINSIZE
	save	%sp, -16 * 4, %sp
#if 7 < __WINSIZE
	save	%sp, -16 * 4, %sp
#endif
#endif
	! Fewer saves if fewer register windows.  For 8 register windows,
	! six of these is correct; a seventh would force out the current
	! set that was already saved manually above.  Note that minimal
	! space is allowed on stack for locals and ins in case this
	! sequence itself gets interrupted and recurses too deep.

	! (after all, we are about to call deeper not shallower, otherwise
	!  using setjmp is inappropriate)

	! ENABLE INTERRUPTS
	wr	%g7, %psr		! set CWP back to as-was
	nop
	nop
	nop
	
#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	ldd	[%o0 + 22 * 4], %g6	! preserve %g7 and %g6
#endif
	
	retl				! ret and return zero to indicate
	mov	0, %o0			! not longjumped-to
	
!  hal_longjmp loads state from arg0 and returns arg1
!FUNC_START(hal_longjmp)
	.global	hal_longjmp
hal_longjmp:
	! This is kind of a leaf routine, it returns elsewhere
	! %o0 is the address of the buffer.
	! %o1 is the value to return in %o0 (since we are a leaf)

	mov	%o0, %g7		! keep the pointer handy
	mov	%o1, %g1		! and the return value
	! now select the new window with traps disabled...
	rd	%psr, %g6
					! preserve ET, clear CWP
	andn	%g6, __WINBITS_MAXIMAL, %g6
	andn	%g6, 0x20, %g5		! clear ET also into %g5

	! get new CWP from %g0 save place...
	ld	[%g7 + 16 * 4], %g4	! %g7 holds the new stack pointer
	and	%g4, __WINBITS, %g4	! preserve CWP bits
		
	! calculate a new WIM...
	add	%g4, 1, %g3		! required invalid window number
#if 8 == __WINSIZE
	and	%g3, __WINBITS, %g3	! modulo 8
#else   // expect 5 or 6 or 7 windows
	cmp	%g3, __WINSIZE
	bge,a	567f			! taken: do delay slot, handle overflow
	 mov	0, %g3			! only if .ge. above
567:	
#endif
	mov	1, %g2
	sll	%g2, %g3, %g2		! converted to a mask for the WIM

	! DISABLE INTERRUPTS
	wr	%g5, %g4, %psr		! set CWP to new window, disable traps
	wr	%g2, %wim		! and WIM to new value
	nop
	nop
	nop
		
	! Must do this atomically so that the registers match the stack.
	! After locals and ins are loaded, we are conformant to the PCS
	! so can re-enable interrupts.

	ldd	[%g7 + 0 * 4], %l0	! restore L & I registers
	ldd	[%g7 + 2 * 4], %l2
	ldd	[%g7 + 4 * 4], %l4
	ldd	[%g7 + 6 * 4], %l6

	ldd	[%g7 + 8 * 4], %i0
	ldd	[%g7 + 10 * 4], %i2
	ldd	[%g7 + 12 * 4], %i4
	ldd	[%g7 + 14 * 4], %i6

	ld	[%g7 + 30 * 4], %sp	! %o6 = %sp, set up now so as to conform
					! to PCS and so be interruptible
	! ENABLE INTERRUPTS
	wr	%g6, %g4, %psr		! set new CWP and old ET
	nop
	nop
	nop
	
	! now load the rest of the context; we can be interrupted here, but
	! that is OK, other state will be preserved in that case...
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	! we are not preserving globals...
					! %o6 = %sp, already set up
	ld	[%g7 + 31 * 4], %o7	! "return" address
	retl				! %g1 still holds the return value
	mov	%g1, %o0

#else // load a maximal context
	mov	%g7, %o0		! original pointer was in %o0 anyway

	ldd	[%o0 + 18 * 4], %g2
	ldd	[%o0 + 20 * 4], %g4
	ldd	[%o0 + 22 * 4], %g6

	ld	[%o0 + 25 * 4], %o1	! %o0 = original pointer
	ldd	[%o0 + 26 * 4], %o2
	ldd	[%o0 + 28 * 4], %o4
					! %o6 = %sp, already set up
	ld	[%o0 + 31 * 4], %o7	! "return" address

	! %g1 still holds the return value; want to get this into %o0
	! and restore %g1 from the saved state;  %o0 is the state pointer:
					! g1 = R,   o0 = P
	xor	%o0, %g1, %g1		! g1 = R^P, o0 = P
	xor	%o0, %g1, %o0		! g1 = R^P, o0 = R
	xor	%o0, %g1, %g1		! g1 = P,   o0 = R all done
		
	retl
	ld	[%g1 + 17 * 4], %g1	! and finally restore %g1
#endif // !CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	
! ------------------------------------------------------------------------------
! end of context.S



