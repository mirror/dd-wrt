#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	H8/300H assembler header file
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
## Author(s): 	ysato
## Contributors:	ysato
## Date:	2002-02-28
## Purpose:	H8/300H definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the H8/300H CPU variant.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/platform.inc>
#include <cyg/hal/basetype.h>	


#------------------------------------------------------------------------------
# Register definitions

#define SYSCR 0xfee012

#------------------------------------------------------------------------------
# CPU state save and restore macros

	.macro	hal_cpu_save_all
	subs	#4,sp
	mov.l	sp,@-sp
	mov.l	er6,@-sp			; push all registers
	mov.l	er5,@-sp
	mov.l	er4,@-sp
	mov.l	er3,@-sp
	mov.l	er2,@-sp
	mov.l	er1,@-sp
	mov.l	er0,@-sp
	mov.w	@(4*9:16,sp),r0
	sub.l	er1,er1
	mov.b	r0h,r1l
	mov.l	er1,@(4*8:16,sp)		; copy ccr
	mov.l	@sp,er0
	mov.l	@(4:16,sp),er1
	.endm

	.macro	hal_cpu_load_all
	mov.w	@(4*8+2,sp),r0
	mov.w	@(4*9,sp),r1
	mov.b	r0l,r1h
	mov.w	r1,@(4*9,sp)			; set ccr to exception frame
	mov.l	@sp+,er0			; pop regs
	mov.l	@sp+,er1
	mov.l	@sp+,er2
	mov.l	@sp+,er3
	mov.l	@sp+,er4
	mov.l	@sp+,er5
	mov.l	@sp+,er6
	adds	#4,sp
	adds	#4,sp
	.endm

        .macro hal_cpu_save_context
        hal_cpu_save_all
        .endm

        .macro	h8300_var_interrupt_entry
	mov.l	@CYG_LABEL_DEFN(_intvector),er0
	shll.l	er0
	shll.l	er0
	mov.l	@(CYG_LABEL_DEFN(hal_vsr_table),er0),er0
	.endm

	.macro	h8300_var_call_pending_DSR
	mov.l	er5,@-sp			; save some work regs
	mov.l	er6,@-sp
	mov.l	sp,er6				; save current SP
	mov.l	#__interrupt_stack,sp		; interrupt stack pointer
	stc	ccr,r5l				; save PSW
	hal_cpu_int_enable			; enable interrupts

	; Call kernel code to invoke DSRs.
	jsr	@CYG_LABEL_DEFN(cyg_interrupt_call_pending_DSRs)

	; On return the old SP in a2 and the old PSW in d2 will 
	; have been preserved by the calling conventions.
	
	hal_cpu_int_merge r5l			; Restore previous PSW
	mov.l	er6,sp				; restore old SP
	mov.l	@sp+,er6			; Retrieve old work regs
	mov.l	@sp+,er5
	rts					; and return
	.endm
		
# Location of PC in saved register context (HAL_SavedRegisters)
#define SAVED_CONTEXT_PC_OFFSET (4*9)          

	# Enable interrupts
	.macro hal_cpu_int_enable
	andc	#0x3f,ccr
	.endm		

	# Disable interrupts
	.macro hal_cpu_int_disable
	orc	#0xc0,ccr
	.endm	

	# Merge the interrupt enable state of the status register in
	# \sr with the current sr.
	.macro	hal_cpu_int_merge sr wk=r0l
	and.b	#0xc0,\sr
	stc	ccr,\wk
	and.b	#0x3f,\wk
	or	\sr,\wk
	ldc	\wk,ccr
	.endm

	.macro	hal_cpu_set_int_level
	ldc	ccr,r0h
	or.b	#0xc0,r0h
	and.b	r0l,r0h
	mov.b	r0h,ccr
	.endm

	.macro	h8300_trap_adjust_pc
	mov.l	@(36:16,sp),er0
	subs	#2,er0
	mov.l	er0,@(36:16,sp)
	.endm

##-----------------------------------------------------------------------------
# Default interrupt decoding macros.

#ifndef CYGPKG_HAL_H8300_INTC_DEFINED
	
#define CYG_ISR_TABLE_SIZE	 64

	.macro  hal_intc_init
	mov.b	@SYSCR,r0l
	bclr	#3,r0l
	mov.b	r0l,@SYSCR
	.endm

#define CYGPKG_HAL_H8300_INTC_DEFINED

#endif


	
#ifndef CYGPKG_HAL_H8300_MON_DEFINED

	.macro	hal_mon_init
	.endm

#if !(defined(CYG_HAL_STARTUP_ROM)	 	|| \
	defined(CYGPKG_HAL_H8300_H8300H_SIM)	|| \
	!defined(CYGSEM_HAL_USE_ROM_MONITOR))

#define CYG_HAL_H8300_VSR_TABLE_DEFINED

#endif



#define CYGPKG_HAL_H8300_MON_DEFINED

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
