#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	AM31 assembler header file
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
## Date:	1999-04-06
## Purpose:	AM31 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AM31 CPU variant.
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


#------------------------------------------------------------------------------
# Register definitions

#define NMICR	0x34000100	// NMI control register
#define DCR	0x20000030	// Debug control register
#define ISR	0x20000034	// Interrupt control register

#define DCR_DE	0x0010		// DE bit in DCR

#------------------------------------------------------------------------------
# CPU state save and restore macros

	.macro	hal_cpu_save_all
	movm	[d2,d3,a2,a3,other],(sp)		# push all registers
	.endm

	.macro	hal_cpu_load_all
	movm	(sp),[d2,d3,a2,a3,other]		# pop regs
	.endm

	.macro	hal_cpu_get_psw reg
	mov	psw,\reg
	.endm

	.macro	hal_cpu_set_psw reg
	mov	\reg,psw
	.endm

# Location of PC in saved register context (HAL_SavedRegisters)
#define SAVED_CONTEXT_PC_OFFSET          56

##-----------------------------------------------------------------------------
# It appears that there is an undocumented extra bit
# in the PSW that masks the delivery of NMIs. It is in
# the upper 16 bits of the register that we cannot access
# directly. So, to clear it, we must set up a fake interrupt
# state and do an RTI to load the PSW. We need to do this if
# we are going to be able to set breakpoints in NMI handlers.
# Note that the AM33 has a documented NMID at bit 17 of the
# extended PSW (along with instructions to access it).

	.macro	hal_cpu_clear_nmid
	mov	PSW,d3				# D3 = PSW
	and	0xFFFF,d3
	mov	nmid\@,d2			# D2 = Next PC
	movm	[d2,d3],(sp)			# Push
	rti					# and load into CPU
nmid\@:	
	.endm


##-----------------------------------------------------------------------------
## Register addresses and initialization values
	
	.equ	IOBCTR	,0x32000010
	.equ	MEMCTR0	,0x32000020
	.equ	MEMCTR1	,0x32000022
	.equ	MEMCTR2	,0x32000024
	.equ	MEMCTR3	,0x32000026
	.equ	MEMCTR4	,0x32000028
	.equ	MEMCTR5	,0x3200002a
	.equ	MEMCTR6	,0x3200002c
	.equ	MEMCTR7	,0x3200002e
	.equ	DRAMCTR	,0x32000040
	.equ	REFCNT	,0x32000042

	.equ	INIT_MEMCTR0,0x1200	# 2wait
	.equ	INIT_MEMCTR1,0x0120	# 1wait-32bit-
	.equ	INIT_MEMCTR2,0x0065
	.equ	INIT_DRAMCTR,0x0287
	.equ	INIT_REFCNT,0x00eb	

	.equ	P2OUT	,0x36008004
	.equ	P2MD	,0x36008024
	.equ	P2SS	,0x36008044
	.equ	P2DIR	,0x36008064

	.equ	INIT_P2MD,0xf0
	.equ	INIT_P2DIR,0xff

#------------------------------------------------------------------------------
# MEMC macros.
	
#ifndef CYGPKG_HAL_MN10300_MEMC_DEFINED

	.macro	hal_memc_init
	mov	INIT_MEMCTR0,d2
	movhu	d2,(MEMCTR0)

	mov	INIT_MEMCTR1,d2
	movhu	d2,(MEMCTR1)

	mov	INIT_MEMCTR2,d2
	movhu	d2,(MEMCTR2)

	mov	INIT_REFCNT,d2
	movhu	d2,(REFCNT)

	mov	INIT_DRAMCTR,d2
	movhu	d2,(DRAMCTR)
	.endm

#define CYGPKG_HAL_MN10300_MEMC_DEFINED

#endif	

##-----------------------------------------------------------------------------
# Default interrupt decoding macros.

#ifndef CYGPKG_HAL_MN10300_INTC_DEFINED


	
	# initialize all interrupts to disabled
	.macro	hal_intc_init
	.endm

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

#define CYG_ISR_TABLE_SIZE	10

	.macro	hal_intc_decode dreg,areg,dreg1
	mov	_mn10300_interrupt_control,\areg
	movhu	(0x100,\areg),\dreg1		# dreg1 = IAGR
	mov	(0,a2),\dreg			# dreg = vector priority
	mov	\dreg1,(0,a2)			# store real vector in saved state
	asl	2,\dreg				# dreg = byte index of isr
	add	12,\dreg			# skip NMI vectors
	.endm

#define CYGPKG_HAL_MN10300_INTC_DECODE_DEFINED

#else

#define CYG_ISR_TABLE_SIZE	 34

	# decode the interrupt
	.macro  hal_intc_decode dreg,areg,dreg1
	mov	_mn10300_interrupt_control,\areg
	movhu	(0x100,\areg),\dreg		# dreg = IAGR
	mov	\dreg,(0,a2)			# store real vector in saved state
	add	12,\dreg			# skip NMI vectors
	.endm

#define CYGPKG_HAL_MN10300_INTC_DECODE_DEFINED

#endif

#define CYGPKG_HAL_MN10300_INTC_DEFINED

#endif


#------------------------------------------------------------------------------
# Diagnostics macros.
	
#ifndef CYGPKG_HAL_MN10300_DIAG_DEFINED

	.macro	hal_diag_init
	# Set up LED
	mov	INIT_P2MD,d2
	movbu	d2,(P2MD)
	mov	INIT_P2DIR,d2
	movbu	d2,(P2DIR)
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

	.macro	hal_diag_led val
	movm	[d2],(sp)
	mov	\val,d2
	asl	4,d2
	movbu	d2,(P2OUT)
	movm	(sp),[d2]
	.endm


	.macro	hal_diag_data
led_count:	.long 0
led_value:	.byte 0x40
led_foo1:	.byte 0x00 # Keep alignment to work around compiler/linker bug
led_foo2:	.byte 0x00
led_foo3:	.byte 0x00
	.endm

#define CYGPKG_HAL_MN10300_DIAG_DEFINED

#endif	

#------------------------------------------------------------------------------
# Monitor initialization.
	
#ifndef CYGPKG_HAL_MN10300_MON_DEFINED

	.macro	hal_mon_init
	hal_mon_init_vectors
	hal_mon_init_vsr
	.endm

#if defined(CYG_HAL_STARTUP_ROM) ||		\
    defined(CYGPKG_HAL_MN10300_AM31_SIM)
	.macro	hal_mon_init_vectors
	mov	_mn10300_interrupt_vectors,a0
	mov	__hardware_vector_0,d0
	movhu	d0,(0,a0)
	mov	__hardware_vector_1,d0
	movhu	d0,(4,a0)
	mov	__hardware_vector_2,d0
	movhu	d0,(8,a0)
	mov	__hardware_vector_3,d0
	movhu	d0,(12,a0)
	mov	__hardware_vector_4,d0
	movhu	d0,(16,a0)
	mov	__hardware_vector_5,d0
	movhu	d0,(20,a0)
	mov	__hardware_vector_6,d0
	movhu	d0,(24,a0)
	.endm
#else
	.macro	hal_mon_init_vectors
	.endm
#endif
	
#if defined(CYG_HAL_STARTUP_RAM)

	# init vsr table in SRAM where the ROM
	# vectors the interrupts.

	.macro	hal_mon_init_vsr
	mov	_hal_vsr_table,a0
	mov	__default_interrupt_vsr,d0
	mov	d0,(0,a0)	
	mov	d0,(4,a0)
	mov	d0,(8,a0)
#ifndef CYGSEM_HAL_USE_ROM_MONITOR_CygMon
	# When using Cygmon, leave level 3 for GDB
	# Ctrl-C interrupts.
	mov	d0,(12,a0)	
#endif	
	mov	d0,(16,a0)	
	mov	d0,(20,a0)	
	mov	d0,(24,a0)
	mov	__default_nmi_vsr,d0
	mov	d0,(28,a0)
	mov	__default_trap_vsr,d0
	mov	d0,(32,a0)
	.endm

#else
	.macro	hal_mon_init_vsr
	.endm
#endif

#if !(defined(CYG_HAL_STARTUP_ROM)	 	|| \
	defined(CYGPKG_HAL_MN10300_AM31_SIM)	|| \
	!defined(CYGSEM_HAL_USE_ROM_MONITOR))

#define CYG_HAL_MN10300_VSR_TABLE_DEFINED

#endif



#define CYGPKG_HAL_MN10300_MON_DEFINED

#endif	



#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
