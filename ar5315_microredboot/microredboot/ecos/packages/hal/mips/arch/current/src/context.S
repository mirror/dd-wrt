##=============================================================================##
##	context.S
##
##	MIPS context switch code
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
## Purpose:	MIPS context switch code
## Description:	This file contains implementations of the thread context 
##		switch routines. It also contains the longjmp() and setjmp()
##              routines.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/arch.inc>

#ifdef at
#undef at
#endif

#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# A0 = address of sp of next thread to execute
# A1 = address of sp save location of current thread

	.global	hal_thread_switch_context
hal_thread_switch_context:		

	addi	sp,sp,-mipsreg_size	# space for registers

	# store GPRs
	.set	noat
#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	sgpr	0,sp
	sgpr	1,sp
	sgpr	2,sp
	sgpr	3,sp
	sgpr	4,sp
	sgpr	5,sp
	sgpr	6,sp
	sgpr	7,sp
	sgpr	8,sp
	sgpr	9,sp
	sgpr	10,sp
	sgpr	11,sp
	sgpr	12,sp
	sgpr	13,sp
	sgpr	14,sp
	sgpr	15,sp
	sgpr	24,sp
	sgpr	25,sp
	sgpr	28,sp	# == GP
#endif	
	sgpr	16,sp
	sgpr	17,sp	
	sgpr	18,sp
	sgpr	19,sp
	sgpr	20,sp
	sgpr	21,sp
	sgpr	22,sp
	sgpr	23,sp
#	sgpr	26,sp	# == K0
#	sgpr	27,sp	# == K1
#	sgpr	29,sp	# == SP
	sgpr	30,sp	# == FP
	sgpr	31,sp	# == RA
	spc     $31,sp	# == PC (to help with debugging)
	.set	at
	
#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

	mflo	t0				# save LO and HI regs
	mfhi	t1
	slo	t0,sp
	shi	t1,sp
	
#endif

	hal_fpu_save_callee	sp

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

	hal_fpu_save_caller	sp

#endif
	
	addi	t0,sp,mipsreg_size		# save SP in reg dump
	ssp	t0,sp

	mfc0	t1,status			# Save status register
	sw	t1,mipsreg_sr(sp)
	
	sw	sp,0(a1)			# save sp in save loc
	
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

	lw	sp,0(a0)			# load new SP directly

#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM

	hal_fpu_load_caller sp	
	
#endif
			
	hal_fpu_load_callee sp
	
#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	
	lw	t0,mipsreg_hi(sp)		# load HI and LO regs
	lw	t1,mipsreg_lo(sp)
	mthi	t0
	mtlo	t1

#endif

	# load GPRs
	.set	noat
#	lgpr	0,sp
	lgpr	4,sp	# A0, must load for thread startup
#ifndef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
	lgpr	1,sp
	lgpr	2,sp
	lgpr	3,sp
	lgpr	5,sp
	lgpr	6,sp
	lgpr	7,sp
	lgpr	8,sp
	lgpr	9,sp
	lgpr	10,sp
	lgpr	11,sp
	lgpr	12,sp
	lgpr	13,sp
	lgpr	14,sp
	lgpr	15,sp
	lgpr	24,sp
	lgpr	25,sp
#endif	
	lgpr	16,sp
	lgpr	17,sp
	lgpr	18,sp
	lgpr	19,sp
	lgpr	20,sp
	lgpr	21,sp
	lgpr	22,sp
	lgpr	23,sp
#	lgpr	26,sp	# == K0
#	lgpr	27,sp	# == K1
#	lgpr	28,sp	# == GP
#	lgpr	29,sp	# == SP
	lgpr	30,sp	# == FP
	lgpr	31,sp	# == RA
	.set	at
	
	lw	a2,mipsreg_sr(sp)		# A2 = saved SR
	lsp	sp,sp				# SP = saved SP

	hal_cpu_int_merge a2			# Merge with current SR
	
	jr	ra				# return via ra
	nop					# delay slot - must be nop

#------------------------------------------------------------------------------
# HAL longjmp, setjmp implementations
# hal_setjmp saves only to callee save registers 16-23, 28, 30, 31[ra], 29[sp]
# into buffer supplied in a0[arg0]
# Note: These definitions are repeated in hal_arch.h. If changes are required
# remember to update both sets.

#define CYGARC_JMP_BUF_SP        0
#define CYGARC_JMP_BUF_R16       1
#define CYGARC_JMP_BUF_R17       2
#define CYGARC_JMP_BUF_R18       3
#define CYGARC_JMP_BUF_R19       4
#define CYGARC_JMP_BUF_R20       5
#define CYGARC_JMP_BUF_R21       6
#define CYGARC_JMP_BUF_R22       7
#define CYGARC_JMP_BUF_R23       8
#define CYGARC_JMP_BUF_R28       9
#define CYGARC_JMP_BUF_R30      10
#define CYGARC_JMP_BUF_R31      11

#define CYGARC_JMP_BUF_SIZE     12

// FIXME: The follwing restricts us to using only 32 bit registers
// in jump buffers. If/when we move to a full 64 bit architecture,
// this will need to change, as will the instructions that we use to
// save and restore them.

#define jmpbuf_regsize 4
	
	.globl	hal_setjmp
	.ent	hal_setjmp
hal_setjmp:
	sw	$31,CYGARC_JMP_BUF_R31*jmpbuf_regsize(a0)	# ra (link)
	sw	$30,CYGARC_JMP_BUF_R30*jmpbuf_regsize(a0)
	sw	$28,CYGARC_JMP_BUF_R28*jmpbuf_regsize(a0)	# gp, optimize out?
	sw	$23,CYGARC_JMP_BUF_R23*jmpbuf_regsize(a0)
	sw	$22,CYGARC_JMP_BUF_R22*jmpbuf_regsize(a0)
	sw	$21,CYGARC_JMP_BUF_R21*jmpbuf_regsize(a0)
	sw	$20,CYGARC_JMP_BUF_R20*jmpbuf_regsize(a0)
	sw	$19,CYGARC_JMP_BUF_R19*jmpbuf_regsize(a0)
	sw	$18,CYGARC_JMP_BUF_R18*jmpbuf_regsize(a0)
	sw	$17,CYGARC_JMP_BUF_R17*jmpbuf_regsize(a0)
	sw	$16,CYGARC_JMP_BUF_R16*jmpbuf_regsize(a0)
	sw	sp,CYGARC_JMP_BUF_SP*jmpbuf_regsize(a0)	# $29
	li	v0,0
	jr	ra
	nop			# delay slot
	.end hal_setjmp

	.globl	hal_longjmp
	.ent	hal_longjmp
hal_longjmp:	
	lw	$31,CYGARC_JMP_BUF_R31*jmpbuf_regsize(a0)	# ra (link)
	lw	$30,CYGARC_JMP_BUF_R30*jmpbuf_regsize(a0)
	lw	$28,CYGARC_JMP_BUF_R28*jmpbuf_regsize(a0)	# gp, optimize out?
	lw	$23,CYGARC_JMP_BUF_R23*jmpbuf_regsize(a0)
	lw	$22,CYGARC_JMP_BUF_R22*jmpbuf_regsize(a0)
	lw	$21,CYGARC_JMP_BUF_R21*jmpbuf_regsize(a0)
	lw	$20,CYGARC_JMP_BUF_R20*jmpbuf_regsize(a0)
	lw	$19,CYGARC_JMP_BUF_R19*jmpbuf_regsize(a0)
	lw	$18,CYGARC_JMP_BUF_R18*jmpbuf_regsize(a0)
	lw	$17,CYGARC_JMP_BUF_R17*jmpbuf_regsize(a0)
	lw	$16,CYGARC_JMP_BUF_R16*jmpbuf_regsize(a0)
	lw	sp,CYGARC_JMP_BUF_SP*jmpbuf_regsize(a0)	# $29
	move	v0,a1
	jr	ra
	nop			# delay slot
	.end hal_longjmp
	
	
#------------------------------------------------------------------------------
# end of context.S
