##=============================================================================
##
##	context.S
##
##	H8/300 context switch code
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
## Author(s): 	yoshinori sato
## Contributors:	yoshinori sato
## Date:	2002-02-17
## Purpose:	H8/300 context switch code
## Description:	This file contains implementations of the thread context 
##		switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>
#include <cyg/hal/basetype.h>


#ifdef CYGPKG_HAL_H8300_H8300H
	.h8300h
#endif
#ifdef CYGPKG_HAL_H8300_H8S
	.h8300s
#endif
	
#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# er0 = address of sp of next thread to execute
# er1 = address of sp save location of current thread

	.global	CYG_LABEL_DEFN(hal_thread_switch_context)
CYG_LABEL_DEFN(hal_thread_switch_context):
	mov.w	@sp,r2		; save ccr
	stc	ccr,r2h
	mov.w	r2,@sp
	hal_cpu_save_context
	mov.l	sp,@er1
	
	# Now load the destination thread by dropping through
	# to hal_thread_load_context
	
#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# D0 = address of sp of next thread to execute
# Note that this function is also the second half of hal_thread_switch_context
# and is simply dropped into from it.
	
	.global	CYG_LABEL_DEFN(hal_thread_load_context)
CYG_LABEL_DEFN(hal_thread_load_context):

	mov.l	@er0,sp
	hal_cpu_load_all
	rte
	
##-----------------------------------------------------------------------------
## HAL longjmp(), setjmp() implementations
## These implementations omit the usual movm [d2,d3,a2,a3],(sp)
## Which is the first instruction of all C compiled functions.	
## Note: These definitions are repeated in hal_arch.h. If changes are required
## remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_ER3       1
#define CYGARC_JMP_BUF_ER4       2
#define CYGARC_JMP_BUF_ER5       3
#define CYGARC_JMP_BUF_ER6       4
#define CYGARC_JMP_BUF_PC        5

#define CYGARC_JMP_BUF_SIZE      6

        
	# This just preserves the callee save registers 
	# namely a2,a3,d2,d3
	# setjmp cannot use movm to do this as we need to keep 
	# the sp underneath all live data at all times.
	.globl CYG_LABEL_DEFN(hal_setjmp)
CYG_LABEL_DEFN(hal_setjmp):				 ; er0=env  
	mov.l	er3,@(CYGARC_JMP_BUF_ER3*4,er0)
	mov.l	er4,@(CYGARC_JMP_BUF_ER4*4,er0)
	mov.l	er5,@(CYGARC_JMP_BUF_ER5*4,er0)
	mov.l	er6,@(CYGARC_JMP_BUF_ER6*4,er0)
	mov.l	@sp,er1
	mov.l	er1,@(CYGARC_JMP_BUF_PC*4,er0)	
	mov	sp,er1
	mov	er1,@(CYGARC_JMP_BUF_SP*4,er0)
	sub.l	er0,er0
	rts

	# longjmp returns to caller of setjmp
	# after restoring callee save registers
	.globl CYG_LABEL_DEFN(hal_longjmp)
CYG_LABEL_DEFN(hal_longjmp):	
	mov.l	@(CYGARC_JMP_BUF_ER3*4,er0),er3
	mov.l	@(CYGARC_JMP_BUF_ER4*4,er0),er4
	mov.l	@(CYGARC_JMP_BUF_ER5*4,er0),er5
	mov.l	@(CYGARC_JMP_BUF_ER6*4,er0),er6
	mov.l	@(CYGARC_JMP_BUF_PC*4,er0),er2
	mov.l	@(CYGARC_JMP_BUF_SP*4,er0),sp
	mov.l	er2,@sp
	mov.l	er1,er0
	rts

	
#------------------------------------------------------------------------------
# end of context.S

	
