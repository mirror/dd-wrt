/*=============================================================================
//
//	hal_cygm.S
//
//	SPARClite and CygMon play nice: vector service routines
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
// Date:	1998-02-24
// Purpose:	SPARClite vectors for interworking with CygMon
// Description:	see vectors.S and hal_priv.c; these VSRs are installed to
//		interwork with CygMon.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

!-----------------------------------------------------------------------------

//	.file	"hal_cygm.S"

!-----------------------------------------------------------------------------

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

!------------------------------------------------------------------------

#include <cyg/hal/vectors.h>	// eCos vector number et al
#include <cyg/hal/hal_cygm.h>	// CygMon vector number et al
	
#define DELAYS_AFTER_WRPSR_SAME_WINDOW
#define DELAYS_AFTER_WRWIM

!------------------------------------------------------------------------

	.text

!---------------------------------------------------------------------------

#ifdef CYG_HAL_USE_ROM_MONITOR_CYGMON

	.global hal_user_trap_to_cygmon_vsr
hal_user_trap_to_cygmon_vsr:
	! here,locals have been set up as follows:
	! %l0 = psr (with this CWP/window-level in it)
	! %l1 = pc
	! %l2 = npc
	! %l3 = vector number (1-15 for interrupts)
	! and we are in our own register window, though it is likely that
	! the next one will need to be saved before we can use it:
	! ie. this one is the invalid register window.

	rd	%tbr, %l4		! Get the trap type
	srl	%l4, 4, %l4		! into the bottom byte
	and	%l4, 0xff, %l4		! actual trap number

	! we deal with traps 1,3,8(+128) and 255 here.
	! ta 1	=> tt 129	:	breakpoint
	! ta 3	=> tt 131	:	flush windows (unneccessary, JIC)
	! ta 8	=> tt 136	:	syscall (apparently)
	! ta 127 -OR- Debug Support Unit trap
	!	=> tt 255	:	debug trap
	! others bounce through to eCos handler, of course.

	cmp	%l4, 129
	beq	bounce_to_cygmon
	mov	BSP_EXC_BREAK, %l5

	! ta 2 is a "skipped" breakpoint, we increment the PC, NPC
	! ourselves to permit continuation.
	cmp	%l4, 130
	bne	1f
	nop
	mov	%l2, %l1
	add	%l2, 4, %l2
	b	bounce_to_cygmon
	mov	BSP_EXC_BREAK, %l5

1:	
	cmp	%l4, 131
	beq	bounce_to_cygmon
	mov	BSP_EXC_WINFLUSH, %l5
	
	cmp	%l4, 136
	beq	bounce_to_cygmon
	mov	BSP_EXC_SYSCALL, %l5
	
	cmp	%l4, 255
	beq	bounce_to_cygmon
	mov	BSP_EXC_DEBUG, %l5

	! if we are here, it~s not a recognized trap to feed to CygMon, so
	! call the eCos default trap VSR:
	.extern hal_default_exception_vsr
	ba	hal_default_exception_vsr	! should be in range
	nop					! does not return


	.global hal_nofpcp_trap_to_cygmon_vsr
hal_nofpcp_trap_to_cygmon_vsr:
	rd	%tbr, %l4		! Get the trap type
	srl	%l4, 4, %l4		! into the bottom byte
	and	%l4, 0xff, %l4		! actual trap number
	! we deal with trap types 4 and 36 here.

	cmp	%l4, 4
	beq	bounce_to_cygmon
	mov	BSP_EXC_FPDIS, %l5
	
	cmp	%l4, 36
	beq	bounce_to_cygmon
	mov	BSP_EXC_CPDIS, %l5

	! we really should not be here otherwise, but for best debugging...
	b	bounce_to_cygmon	
	mov	BSP_EXC_TRAP, %l5
	
bounce_to_cygmon:
	! here we have decided to jump over CygMon, the CygMon vector
	! number to use is in %l5.
	set	CYGMON_VECTOR_TABLE_BASE, %l6
	sll	%l5, 2, %l5		! to a word offset
	! our calling convention is the same, so...
	ld	[ %l6 + %l5 ], %l6
	jmp	%l6
	nop


// Now a handler for INT15 (NMI) to assist in debugging; we have to ack and
// clear the interrupt, after waiting for the source to go away, I guess...
	.global hal_nmi_handler
hal_nmi_handler:
	! here,locals have been set up as follows:
	! %l0 = psr (with this CWP/window-level in it)
	! %l1 = pc
	! %l2 = npc
	! %l3 = vector number (1-15 for interrupts)
	! and we are in our own register window, though it is likely that
	! the next one will need to be saved before we can use it:
	! ie. this one is the invalid register window.
	
#define TRGM0	0
#define TRGM1	4	    
#define REQSNS  8
#define REQCLR 12
#define IMASK  16
#define IRLAT  20
#define IMODE  24

#define NMIBIT 0x80000000

	sethi	%hi( 0x10000000 ), %l4	! base address of the 86940 companion

10:	
	add	%l4, REQCLR, %l5	! Request clear register
	set	NMIBIT, %l6
	sta	%l6, [ %l5 ] 4		! Clear INT15

	add	%l4, REQSNS, %l5	! Request sense register
	lda	[ %l5 ] 4, %l7
	andcc	%l6, %l7, %g0		! test the sense bit

	sethi	%hi( 0x02000000 ), %l6	! Pump it out to the LED
	srl	%l7, 24, %l7
	stb	%l7, [ %l6 + 3 ]

	bne	10b			! poll until the request is gone
	nop
	nop
	nop

	set	0x00100000, %l6		! clear the latch
	add	%l4, IRLAT, %l5
	sta	%l6, [ %l5 ] 4

	nop
	nop
	nop

	b	bounce_to_cygmon
	mov	BSP_EXC_INT15, %l5
	

	
	
	
#endif // CYG_HAL_USE_ROM_MONITOR_CYGMON
		
! end of hal_cygm.S
