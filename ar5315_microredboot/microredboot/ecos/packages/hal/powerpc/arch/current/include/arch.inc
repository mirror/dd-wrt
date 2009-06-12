##=============================================================================
##
##	arch.inc
##
##	PowerPC architecture assembler header file
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
## Contributors:nickg
## Date:	1997-10-16
## Purpose:	PowerPC definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the PowerPC
##              It also includes the variant assembly header file.
## Usage:
##		#include <cyg/hal/arch.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/variant.inc>
#include <cyg/hal/ppc_offsets.inc>
        
#------------------------------------------------------------------------------
# Easier to read names for the registers

	.equ	r0, 0
	.equ	r1, 1
	.equ	r2, 2
	.equ	r3, 3
	.equ	r4, 4
	.equ	r5, 5
	.equ	r6, 6
	.equ	r7, 7
	.equ	r8, 8
	.equ	r9, 9
	.equ	r10, 10
	.equ	r11, 11
	.equ	r12, 12
	.equ	r13, 13
	.equ	r14, 14
	.equ	r15, 15
	.equ	r16, 16
	.equ	r17, 17
	.equ	r18, 18
	.equ	r19, 19
	.equ	r20, 20
	.equ	r21, 21
	.equ	r22, 22
	.equ	r23, 23
	.equ	r24, 24
	.equ	r25, 25
	.equ	r26, 26
	.equ	r27, 27
	.equ	r28, 28
	.equ	r29, 29
	.equ	r30, 30
	.equ	r31, 31

	.equ	sp, r1

#ifdef CYGHWR_HAL_POWERPC_FPU
        .equ    f0, 0
        .equ    f1, 1
        .equ    f2, 2
        .equ    f3, 3
        .equ    f4, 4
        .equ    f5, 5
        .equ    f6, 6
        .equ    f7, 7
        .equ    f8, 8
        .equ    f9, 9
        .equ    f10, 10
        .equ    f11, 11
        .equ    f12, 12
        .equ    f13, 13
        .equ    f14, 14
        .equ    f15, 15
        .equ    f16, 16
        .equ    f17, 17
        .equ    f18, 18
        .equ    f19, 19
        .equ    f20, 20
        .equ    f21, 21
        .equ    f22, 22
        .equ    f23, 23
        .equ    f24, 24
        .equ    f25, 25
        .equ    f26, 26
        .equ    f27, 27
        .equ    f28, 28
        .equ    f29, 29
        .equ    f30, 30
        .equ    f31, 31
#endif


#------------------------------------------------------------------------------
# Some useful coding macros

	# Load immediate word, has to be done with 2 instructions
	.macro	lwi	reg,val
	lis	\reg,\val@H
	ori	\reg,\reg,\val@L
	.endm

#ifdef CYGPKG_HAL_POWERPC_PPC603
	# Move from HID0 hw control register
	.macro	mfhid0 reg
	mfspr	\reg,HID0
	.endm
#endif

#define FUNC_START(name)        \
        .type name,@function;   \
        .globl name;            \
name:   

#define FUNC_END(name)          \
	/* nothing for now */

##-----------------------------------------------------------------------------
## PowerPC FPU state handling
## The PowerPC ABI defines f14-f31 as callee saved. There is thus no need to 
## save them when calling C functions

#ifdef CYGDBG_HAL_COMMON_INTERRUPTS_SAVE_MINIMUM_CONTEXT
# define MAX_SAVE_FREG    13
#else
# define MAX_SAVE_FREG    31
#endif                  

        .macro	hal_fpu_save regs
#ifdef CYGHWR_HAL_POWERPC_FPU
        .set _freg, 0
        .rept MAX_SAVE_FREG+1
        stfd _freg,(CYGARC_PPCREG_FREGS+_freg*8)(\regs)
        .set _freg, _freg+1
        .endr
#endif
        .endm

        .macro	hal_fpu_load regs
#ifdef CYGHWR_HAL_POWERPC_FPU
        .set _freg, 0
        .rept MAX_SAVE_FREG+1
        lfd _freg,(CYGARC_PPCREG_FREGS+_freg*8)(\regs)
        .set _freg, _freg+1
        .endr
#endif
        .endm

##-----------------------------------------------------------------------------
## CPU specific macros. These provide a common assembler interface to
## operations that may have CPU specific implementations on different
## variants of the architecture.		

	# Enable interrupts
	.macro hal_cpu_int_enable
        mfmsr   r0
        ori     r3,r3,0x8000
        rlwimi  r0,r3,0,16,16
        sync
        mtmsr   r0
        sync
	.endm		

	# Disable interrupts
	.macro hal_cpu_int_disable
        mfmsr   r0
        li      r3,0
        rlwimi  r0,r3,0,16,16
        sync
        mtmsr   r0
        sync
	.endm		

	# Merge the interrupt enable state of the status register in
	# \sr with the current sr.
	.macro	hal_cpu_int_merge sr
        mfmsr   r0
        rlwimi  r0,\sr,0,16,16
        sync
        mtmsr   r0
        sync
	.endm

#------------------------------------------------------------------------------
# end of arch.inc
