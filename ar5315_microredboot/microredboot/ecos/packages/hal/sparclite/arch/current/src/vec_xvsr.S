/*===========================================================================
//
//	vec_xvsr.S
//
//	SPARClite vectors: exception vector service routine
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	 hmt
// Contributors: hmt
// Date:	 1999-02-20
// Purpose:	 SPARClite vector code
// Description:	 see vectors.S; this is the default vector service routine
//               for exceptions.
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

!-----------------------------------------------------------------------------

//	.file	"vec_xvsr.S"

!----------------------------------------------------------------------------

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

!------------------------------------------------------------------------

#include <cyg/hal/vectors.h>

#define DELAYS_AFTER_WRPSR_SAME_WINDOW
#define DELAYS_AFTER_WRWIM

!------------------------------------------------------------------------

	.text

!---------------------------------------------------------------------------
! default exception handler VSR, which calls the appropriate ISR after
! interrupt masking - much the same as the interrupt VSR but does not lock
! scheduler or call interrupt_end().

	.global hal_default_exception_vsr
hal_default_exception_vsr:
	! here,locals have been set up as follows:
	! %l0 = psr (with this CWP/window-level in it)
	! %l1 = pc
	! %l2 = npc
	! %l3 = vector number (16-25 for traps)
	! and we are in our own register window, though it is likely that
	! the next one will need to be saved before we can use it:
	! ie. this one is the invalid register window.

	! must establish a safe stack before re-enabling interrupts + traps
	and	%l0, __WINBITS, %l7	! CWP extracted
	! no inc/dec here, so no need for special measures for not-8-windows
	mov	1, %l6
	sll	%l6, %l7, %l6		! 1 << CWP
	rd	%wim, %l5
	cmp	%l5, %l6		! are they the same?
	bne	1f			! No, so the stack is OK as is.

	! now do by hand an overflow trap, effectively
	mov	%g1, %l7		! (DELAY SLOT)
	srl	%l5, 1, %l5
	sll	%l6, __WINSIZE-1, %l6
	or	%l6, %l5, %g1		! new WIM in %g1 so we can get it
					! within the save:
	save				! Slip into next window
	mov	%g1, %wim		! Install the new wim
					! (invalidates current window!)
#ifdef DELAYS_AFTER_WRWIM
	nop
	nop
	nop
#endif

	std	%l0, [%sp + 0 * 4]	! save L & I registers
	std	%l2, [%sp + 2 * 4]
	std	%l4, [%sp + 4 * 4]
	std	%l6, [%sp + 6 * 4]

	std	%i0, [%sp + 8 * 4]
	std	%i2, [%sp + 10 * 4]
	std	%i4, [%sp + 12 * 4]
	std	%i6, [%sp + 14 * 4]

	restore				! Go back to trap window.
	mov	%l7, %g1		! Restore %g1

1:	! now save away the regs we must preserve
	sub     %fp, 32 * 4, %sp
	! save a maximal context regardless: see also
	! CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
	std	%l0, [%sp + 0 * 4]	! save L & I registers
	std	%l2, [%sp + 2 * 4]
	std	%l4, [%sp + 4 * 4]
	std	%l6, [%sp + 6 * 4]

	std	%i0, [%sp + 8 * 4]
	std	%i2, [%sp + 10 * 4]
	std	%i4, [%sp + 12 * 4]
	std	%i6, [%sp + 14 * 4]

	st	%g1, [%sp + 17 * 4]	! save G registers
	std	%g2, [%sp + 18 * 4]
	std	%g4, [%sp + 20 * 4]
	std	%g6, [%sp + 22 * 4]

	! no point whatsoever in saving O registers

	! and save the CWP in %g0 save place
	st	%l0, [%sp + 16 * 4]

	sub	%sp, 24 * 4, %sp	! fresh frame including
					! arg spill area for callees

	! do not lock the scheduler

	! HELP_GDB_WITH_BACKTRACE
	mov	%i7, %l5		! preserve it JIC
	mov	%l1, %i7		! bogus return link here

	! and we must preserve the Y register (multiply/divide auxiliary)
	! over these calls; we will keep it in %l4 which is otherwise unused.
	rd	%y, %l4

	! Now we can reenable traps (preserving interrupt level)
	or	%l0, 0x0e0, %l7		! set ET (+S,PS), preserve PIL
	wr	%l7, %psr		! and enable!
#ifdef DELAYS_AFTER_WRPSR_SAME_WINDOW
	nop
	nop
	nop
#endif
	! now call the XSR and so on with the appropriate args:
	! ie.
	! isr_retcode = (*(hal_interrupt_handlers[ vector ]))
	!                   ( vector, hal_interrupt_data[ vector ], stackp );

	! from hal_arch.h
	!// ISR tables
	!CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
	!CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
	!CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];        

	mov	%l3, %o0
	sll	%l3, 2, %l3		! %l3 to a word offset
	sethi	%hi(hal_interrupt_data), %l7
	or	%l7, %lo(hal_interrupt_data), %l7
	ld	[ %l7 + %l3 ], %o1	! data
	add	%sp, 24 * 4, %o2	! stackpointer of saved regset

	sethi	%hi(hal_interrupt_handlers), %l7
	or	%l7, %lo(hal_interrupt_handlers), %l7
	ld	[ %l7 + %l3 ], %l6
	call	%l6
	nop

	! do not call _interrupt_end()

	! restore the Y register having done our callouts to C
	wr	%l4, %y

	! We can reinstall the original CWP here; even if interrupt_end()
	! performed a reschedule (ie. yield/resume pair) we will be in the
	! same window.  The window is preserved by reschedule precisely
	! because it is impossible atomically to disable traps here without
	! involving a CWP living in a register for a time when other
	! interrupts may occur.

	! disable traps (using the saved psr is fastest way)
	wr	%l0, %psr	! restores flags, disables traps, same PIL.
#ifdef DELAYS_AFTER_WRPSR_SAME_WINDOW
	nop
	nop
	nop
#endif
	! HELP_GDB_WITH_BACKTRACE
	mov	%l5, %i7	! restore (unused) return link

	! and restore other saved regs
	add	%sp, 24 * 4, %sp	! undo fresh frame
	! restore a maximal context regardless: see also
	! CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
	ldd	[%sp + 0 * 4], %l0	! restore L & I registers
	ldd	[%sp + 2 * 4], %l2	! to support the handler
	ldd	[%sp + 4 * 4], %l4	! having messed with them.
	ldd	[%sp + 6 * 4], %l6

	ldd	[%sp + 8 * 4], %i0
	ldd	[%sp + 10 * 4], %i2
	ldd	[%sp + 12 * 4], %i4
	ldd	[%sp + 14 * 4], %i6

	ld	[%sp + 17 * 4], %g1	! and G registers
	ldd	[%sp + 18 * 4], %g2
	ldd	[%sp + 20 * 4], %g4
	ldd	[%sp + 22 * 4], %g6
	! no point whatsoever in loading back O registers.

	! Now test for window underflow here and fix up if needs be.
	!
	! Why?  interrupt_end() might have yielded us, when only
	! its own frame was restored; its own return to us caused a
	! window underflow trap, as would our return to interruptee
	! unless we deal with it now.

	add	%l0, 1, %l7		! interruptee~s CWP plus noise
	and	%l7, __WINBITS, %l7	! CWP only
#if 8 == __WINSIZE
	! it is in range already
#else   // expect 5 or 6 or 7 windows
	cmp	%l7, __WINSIZE
	bge,a	567f			! taken: do delay slot, handle overflow
	 mov	0, %l7			! only if .ge. above
567:	
#endif
	mov	1, %l6
	sll	%l6, %l7, %l6		! 1 << CWP
	rd	%wim, %l5
	cmp	%l5, %l6		! are they the same?
	bne	2f			! No, so the stack is OK as is.

	! now do by hand an underflow trap, effectively
	sll	%l5, 1, %l5		! Rotate wim left
	srl	%l6, __WINSIZE-1, %l6
	wr	%l5, %l6, %wim
#ifdef DELAYS_AFTER_WRWIM
	nop				! are these delays needed?
	nop				! (following restore uses wim)
	nop
#endif
	restore				! Interruptee~s window
	ldd	[%sp + 0 * 4], %l0	! restore L & I registers
	ldd	[%sp + 2 * 4], %l2
	ldd	[%sp + 4 * 4], %l4
	ldd	[%sp + 6 * 4], %l6

	ldd	[%sp + 8 * 4], %i0
	ldd	[%sp + 10 * 4], %i2
	ldd	[%sp + 12 * 4], %i4
	ldd	[%sp + 14 * 4], %i6
	save				! Back to trap window

2:	! restore the condition codes, PSR and PIL and return from trap.
	wr	%l0, %psr	! restores flags, disables traps, and old PIL
#ifdef DELAYS_AFTER_WRPSR_SAME_WINDOW
	nop
	nop
	nop
#endif
	jmpl	%l1,  %g0
	rett	%l2


! end of vec_xvsr.S
