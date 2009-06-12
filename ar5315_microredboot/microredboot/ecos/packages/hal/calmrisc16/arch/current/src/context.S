##=============================================================================##
##	context.S
##
##	CalmRISC16 context switch code
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
## Purpose:	CalmRISC16 context switch code
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

#define CYGARC_JMP_BUF_R4        0
#define CYGARC_JMP_BUF_R5        2
#define CYGARC_JMP_BUF_A12       4
#define CYGARC_JMP_BUF_A13       8
#define CYGARC_JMP_BUF_A14      12
#define CYGARC_JMP_BUF_A15      16

#define CYGARC_JMP_BUF_SIZE     20

// FIXME: The follwing restricts us to using only 32 bit registers
// in jump buffers. If/when we move to a full 64 bit architecture,
// this will need to change, as will the instructions that we use to
// save and restore them.

#define jmpbuf_regsize 4
	
	.globl	hal_setjmp
hal_setjmp:
	ldw	a8,@[sp+2]	// jmpbuf
	ldw	@[a8+0],r4
	ldw	@[a8+2],r5
	ldw	@[a8+4],a12
	ldw	@[a8+8],a13
	ldw	@[a8+12],a14
	ldw	@[a8+16],a15
	retd
	 ld	r0,#0
	.end hal_setjmp

	.globl	hal_longjmp
hal_longjmp:	
	ldw	a8,@[sp+2]
	ldw	r0,@[sp+6]
	ldw	r4,@[a8+0]
	ldw	r5,@[a8+2]
	ldw	a12,@[a8+4]
	ldw	a13,@[a8+8]
	ldw	a14,@[a8+12]
	ldw	a15,@[a8+16]
	and	r0,r0
	retd
	incc	r0
	.end hal_longjmp
	
	
#------------------------------------------------------------------------------
# end of context.S
