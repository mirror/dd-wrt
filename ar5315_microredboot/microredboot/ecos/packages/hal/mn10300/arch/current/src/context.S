##=============================================================================
##
##	context.S
##
##	MN10300 context switch code
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
## Author(s): 	nickg
## Contributors:	nickg
## Date:	1998-04-27
## Purpose:	MN10300 context switch code
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
# D0 = address of sp of next thread to execute
# D1 = address of sp save location of current thread

	.global	_hal_thread_switch_context
_hal_thread_switch_context:
	add	-8,sp		# space for SP and PSW
	mov	d2,a0		# move d2->a0 since PSW can only be
				# saved to a data register
	hal_cpu_get_psw	d2	# save PSW to d2
	mov	d2,(4,sp)	# store in stack loc
	mov	a0,d2		# retrieve old d2 value
	hal_cpu_save_all	# save entire register set
	mov	d1,a2		# move save loc addr to A2
	mov	sp,(0,a2)	# store SP in save location

	# Now load the destination thread by dropping through
	# to hal_thread_load_context
	
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# D0 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context
# and is simply dropped into from it.
	
	.global	_hal_thread_load_context
_hal_thread_load_context:

	mov	d0,a2		# move next sp loc to a2
	mov	(0,a2),sp	# load SP with new value
	hal_cpu_load_all	# load whole register set
	mov	(4,sp),d1	# retrieve saved PSW
	hal_cpu_set_psw d1	# and restore it
	add	8,sp		# skip saved SP and PSW
	ret	[],0		# and return
	
##-----------------------------------------------------------------------------
## HAL longjmp(), setjmp() implementations
## These implementations omit the usual movm [d2,d3,a2,a3],(sp)
## Which is the first instruction of all C compiled functions.	
## Note: These definitions are repeated in hal_arch.h. If changes are required
## remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_D2        1
#define CYGARC_JMP_BUF_D3        2
#define CYGARC_JMP_BUF_A2        3
#define CYGARC_JMP_BUF_A3        4
#define CYGARC_JMP_BUF_LR        5

#define CYGARC_JMP_BUF_SIZE      6

        
	# This just preserves the callee save registers 
	# namely a2,a3,d2,d3
	# setjmp cannot use movm to do this as we need to keep 
	# the sp underneath all live data at all times.
	.globl _hal_setjmp
_hal_setjmp:			# d0=env
	mov	d0,a0		# env
	mov	d2,(CYGARC_JMP_BUF_D2*4,a0)
	mov	d3,(CYGARC_JMP_BUF_D3*4,a0)
	mov	a2,(CYGARC_JMP_BUF_A2*4,a0)
	mov	a3,(CYGARC_JMP_BUF_A3*4,a0)
	mov	mdr,d1		# link saved by call (also in (0,sp))
	mov	d1,(CYGARC_JMP_BUF_LR*4,a0)	
	mov	sp,a1
	mov	a1,(CYGARC_JMP_BUF_SP*4,a0) # a1==(caller''s sp)
	clr	d0		# return 0
	ret	[],0

	# longjmp returns to caller of setjmp
	# after restoring callee save registers
	.globl _hal_longjmp
_hal_longjmp:
	mov	d0,a0
	mov	(CYGARC_JMP_BUF_D2*4,a0),d2
	mov	(CYGARC_JMP_BUF_D3*4,a0),d3
	mov	(CYGARC_JMP_BUF_A2*4,a0),a2
	mov	(CYGARC_JMP_BUF_A3*4,a0),a3
	mov	(CYGARC_JMP_BUF_LR*4,a0),d0
##	mov	d0,mdr
	mov	(CYGARC_JMP_BUF_SP*4,a0),a1 # stashed (caller''s sp)
	mov	a1,sp
	mov	d0,(0,sp)	# put link on stack
	mov	d1,d0		# return arg1
	ret	[],0		# this ignores (0,sp) and does pc=mdr

	
#------------------------------------------------------------------------------
# end of context.S

	
