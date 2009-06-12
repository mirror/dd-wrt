##=============================================================================##
##	context.S
##
##	CalmRISC32 context switch code
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
## Author(s): 	msalter
## Contributors: msalter
## Date:	2001-02-12
## Purpose:	CalmRISC32 context switch code
## Description:	This file contains implementations of the thread context 
##		switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/arch.inc>

#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# A0 = address of sp of next thread to execute
# A1 = address of sp save location of current thread

	.global	hal_thread_switch_context
hal_thread_switch_context:		
	// FIXME	
	# Now load the destination thread by dropping through
	# to hal_thread_load_context
	
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# A0 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context
# and is simply dropped into from it.
	
	.global	hal_thread_load_context
hal_thread_load_context:
	// FIXME

#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only callee save registers into given buffer
# Note: These definitions are repeated in hal_arch.h. If changes are required
# remember to update both sets.

#define CYGARC_JMP_BUF_R0        0
#define CYGARC_JMP_BUF_R1        1
#define CYGARC_JMP_BUF_R2        2
#define CYGARC_JMP_BUF_R3        3
#define CYGARC_JMP_BUF_R4        4
#define CYGARC_JMP_BUF_R5        5
#define CYGARC_JMP_BUF_R6        6
#define CYGARC_JMP_BUF_R7        7
#define CYGARC_JMP_BUF_R12       8
#define CYGARC_JMP_BUF_R13       9
#define CYGARC_JMP_BUF_R14      10
#define CYGARC_JMP_BUF_R15      11

#define CYGARC_JMP_BUF_SIZE     48

// FIXME: The follwing restricts us to using only 32 bit registers
// in jump buffers. If/when we move to a full 64 bit architecture,
// this will need to change, as will the instructions that we use to
// save and restore them.

#define jmpbuf_regsize 4
	
	.globl	hal_setjmp
hal_setjmp:
	ld	r8, r0
	ldw	@[r8+0], r0
	ldw	@[r8+4], r1
	ldw	@[r8+8], r2
	ldw	@[r8+12], r3
	ldw	@[r8+16], r4
	ldw	@[r8+20], r5
	ldw	@[r8+24], r6
	ldw	@[r8+28], r7
	ldw	@[r8+32], r12
	ldw	@[r8+36], r13
	ldw	@[r8+40], r14
	ldw	@[r8+44], sp

	jmpd	r14
	 ldw	r0, #0

	.globl	hal_longjmp
hal_longjmp:	
	ld	r9, r0
	ld	r8, r1
	ldw	sp, @[r9+44]
	ldw	r14, @[r9+40]
	ldw	r13, @[r9+36]
	ldw	r12, @[r9+32]
	ldw	r7, @[r9+28]
	ldw	r6, @[r9+24]
	ldw	r5, @[r9+20]
	ldw	r4, @[r9+16]
	ldw	r3, @[r9+12]
	ldw	r2, @[r9+8]
	ldw	r1, @[r9+4]
        # Value to return is in r8.  If zero, return 1
	ld	r0, r8
	cmp eq	r0, #0
	brf 	0f
	ldw	r0, #1
    0:
	jmpd	r14
	 nop
	
	
#------------------------------------------------------------------------------
# end of context.S
