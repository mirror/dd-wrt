#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	AM33 assembler header file
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
## Contributors:	nickg, dmoseley
## Date:	1999-04-06
## Purpose:	AM33 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the AM33 CPU variant.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

	.am33		# Enable AM33 instruction set

#include <cyg/hal/platform.inc>

#include CYGHWR_MEMORY_LAYOUT_H

#------------------------------------------------------------------------------
# Register definitions

#define NMICR	0xD4000000	// NMI control register
#define DCR	0xC0000030	// Debug control register
#define ISR	0xC0000034	// Interrupt control register
#define TBR	0xC0000024	// Trap Base Register
#define CHCTR	0xC0000070	// Cache Control Register

#define DCR_DE	0x0001		// DE bit in DCR

##-----------------------------------------------------------------------------
## Register addresses and initialization values

#define SDRAMCONFIG		0xDA000000

#define INIT_SDRAMCONFIG	0x0000002f

#define SC0ICR			0xD4002004
#define SC1ICR			0xD4002014
#define SC2ICR			0xD4002024
#define TM0MD			0xD4003000
#define TM1MD			0xD4003001
#define TM2MD			0xD4003002
#define TM3MD			0xD4003003
#define TM4MD			0xD4003080
#define TM5MD			0xD4003082
#define TM6MD			0xD4003084
#define TM7MD			0xD4003086
#define TM8MD			0xD4003088
#define TM9MD			0xD400308A
#define TM10MD			0xD400308C
#define TM11MD			0xD400308E
#define DM0CTR			0xD2000000
#define DM1CTR			0xD2000100
#define DM2CTR			0xD2000200
#define DM3CTR			0xD2000300
#define WDCTR			0xC0001002
#define RTCRB			0xD860000B
	
#------------------------------------------------------------------------------
# AM33 specific CPU initialization:

	# Initialize CPU
	.macro	hal_cpu_init
	# Set up the PSW
#if CYGHWR_HAL_MN10300_AM33_REVISION == 2
	mov	0x00100700,d0		# Enable the FPU
#else
	mov	0x00000700,d0
#endif
	mov	d0,epsw
#ifndef CYGSEM_HAL_USE_ROM_MONITOR
// Don't change TBR for RAM startup if RAM base is not aligned for TBR.
// In that case, the only option is to rely on ROM startup TBR and patch
// into vectors through the hal_vsr_table.
//
#if !(defined(CYG_HAL_STARTUP_RAM) && (CYGMEM_REGION_ram & 0xffffff))
	mov	reset_vector,a0		# set TBR to vector table
	mov	a0,(TBR)
#endif
#endif

	# stop as many internal interrupt sources as possible
	mov	0,d0
	movhu	d0,(SC0ICR)
	movhu	d0,(SC1ICR)
	movhu	d0,(SC2ICR)
	movbu	d0,(TM0MD)
	movbu	d0,(TM1MD)
	movbu	d0,(TM2MD)
	movbu	d0,(TM3MD)
	movbu	d0,(TM4MD)
	movbu	d0,(TM5MD)
	movhu	d0,(TM6MD)
	movbu	d0,(TM7MD)
	movbu	d0,(TM8MD)
	movbu	d0,(TM9MD)
	movbu	d0,(TM10MD)
	movbu	d0,(TM11MD)
	movhu	d0,(WDCTR)

	movbu	(RTCRB),d0
	and	0x8F,d0
	movbu	d0,(RTCRB)

	mov	0x80000000,d0
	mov	d0,(DM0CTR)
	mov	d0,(DM1CTR)
	mov	d0,(DM2CTR)
	mov	d0,(DM3CTR)
	
	# disable all interrupts
	mov	_mn10300_interrupt_control,a0
	mov	_mn10300_interrupt_control+(41*4),a1
	mov	0x0010,d0
1:	movhu	d0,(a0)
	add	4,a0
	cmp	a0,a1
	bcc	1b
	.endm

#define CYGPKG_HAL_MN10300_CPU_INIT_DEFINED

#------------------------------------------------------------------------------
# CPU state save and restore macros

	.macro	hal_cpu_save_all
	movm	[all],(sp)				# push all registers
	.endm

	.macro	hal_cpu_load_all
	movm	(sp),[all]				# pop regs
	.endm

	.macro	hal_cpu_get_psw reg
	mov	epsw,\reg
	.endm

	.macro	hal_cpu_set_psw reg
	mov	\reg,epsw
	.endm

# Location of PC in saved context (HAL_SavedRegisters)
#define SAVED_CONTEXT_PC_OFFSET          104

#-----------------------------------------------------------------------------
# Clear the NMID bit in the epsw to allow NMIs to be delivered again.

	.macro	hal_cpu_clear_nmid
	and	0xFFFDFFFF,epsw			# clear NMID bit
	.endm


#------------------------------------------------------------------------------
# MEMC macros.
	
#ifndef CYGPKG_HAL_MN10300_MEMC_DEFINED

	.macro	hal_memc_init
	mov	INIT_SDRAMCONFIG,d0
	mov	d0,(SDRAMCONFIG)
	.endm

#define CYGPKG_HAL_MN10300_MEMC_DEFINED

#endif	
	
#------------------------------------------------------------------------------
# Cache macros.
	
#ifndef CYGPKG_HAL_MN10300_CACHE_DEFINED

	.macro	hal_cache_init

#if defined(CYG_HAL_STARTUP_ROM)
    # Note that the hardware seems to come up with the
    # caches containing random data. Hence they must be
    # invalidated before being enabled.
    # However, we only do this if we are in ROM. If we are
    # in RAM, then we leave the caches in the state chosen
    # by the ROM monitor. If we enable them when the monitor
    # is not expecting it, we can end up breaking things if the
    # monitor is not doing cache flushes.
	mov	CHCTR,a0
	mov	0x0030,d0
	mov	d0,(a0)				# Invalidate the ICACHE and DCACHE
	mov	0x0003,d0
	mov	d0,(a0)				# Enable both caches
#else
	nop
#endif
	.endm

#define CYGPKG_HAL_MN10300_CACHE_DEFINED

#endif	

##-----------------------------------------------------------------------------
# Default interrupt decoding macros.

#ifndef CYGPKG_HAL_MN10300_INTC_DEFINED


	
	# initialize all interrupts to disabled
	.macro	hal_intc_init
	.endm

#define CYGPKG_HAL_MN10300_INTC_DEFINED

#endif

#ifndef CYGPKG_HAL_MN10300_INTC_DECODE_DEFINED

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

#else

#define CYG_ISR_TABLE_SIZE	 57

	# decode the interrupt
	.macro  hal_intc_decode dreg,areg,dreg1
	mov	_mn10300_interrupt_control,\areg
	movhu	(0x100,\areg),\dreg		# dreg = IAGR
	mov	\dreg,(0,a2)			# store real vector in saved state
	add	12,\dreg			# skip NMI vectors
	.endm
		
#endif

#define CYGPKG_HAL_MN10300_INTC_DECODE_DEFINED

#endif


#------------------------------------------------------------------------------
# Diagnostics macros.
	
#ifndef CYGPKG_HAL_MN10300_DIAG_DEFINED

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

	.macro	hal_diag_led val
	movm	[d2,a2],(sp)
	mov	\val,d2
	movm	(sp),[d2,a2]
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

#if defined(CYG_HAL_STARTUP_ROM) || !defined(CYGSEM_HAL_USE_ROM_MONITOR)
	.macro	hal_mon_init_vectors
	# direct external interrupts
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

	.extern nmi_vsr_trampoline
	.extern nmi_sysef_trampoline

	# init vsr table in SRAM where the ROM
	# vectors the interrupts.

#if defined(CYG_HAL_STARTUP_ROM)
	.macro	hal_mon_init_vsr
	mov	_hal_vsr_table,a0
	mov	__default_interrupt_vsr,d0
	mov	d0,(0,a0)	
	mov	d0,(4,a0)
	mov	d0,(8,a0)
	mov	d0,(12,a0)	
	mov	d0,(16,a0)	
	mov	d0,(20,a0)	
	mov	d0,(24,a0)
	mov	nmi_vsr_trampoline,d0
	mov	d0,(28,a0)
	mov	__default_trap_vsr,d0
	mov	d0,(32,a0)
	mov	nmi_sysef_trampoline,d0
	mov	d0,(44,a0)
	mov	__default_nmi_vsr,d0
	mov	d0,(36,a0)
	mov	d0,(40,a0)
	mov	d0,(48,a0)
	mov	d0,(52,a0)
	mov	d0,(56,a0)
	mov	d0,(60,a0)
	mov	d0,(64,a0)
	mov	d0,(68,a0)
	mov	d0,(72,a0)
	mov	d0,(76,a0)
	mov	d0,(80,a0)
	mov	d0,(84,a0)
	mov	d0,(88,a0)
	mov	d0,(92,a0)
	mov	d0,(96,a0)
	mov	d0,(100,a0)
	mov	d0,(104,a0)
	mov	d0,(108,a0)
	.endm
#elif defined(CYG_HAL_STARTUP_RAM)
	.macro	hal_mon_init_vsr
	mov	_hal_vsr_table,a0
	mov	__default_interrupt_vsr,d0
	mov	d0,(0,a0)	
	mov	d0,(4,a0)
	mov	d0,(8,a0)
	mov	d0,(12,a0)	
	mov	d0,(16,a0)	
	mov	d0,(20,a0)	
	mov	d0,(24,a0)
#if !(defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs))
	mov	nmi_vsr_trampoline,d0
	mov	d0,(28,a0)
	mov	nmi_sysef_trampoline,d0
	mov	d0,(44,a0)
#endif
	mov	__default_trap_vsr,d0
	mov	d0,(32,a0)
	mov	__default_nmi_vsr,d0
	mov	d0,(36,a0)
	mov	d0,(40,a0)
	mov	d0,(48,a0)
	mov	d0,(52,a0)
	mov	d0,(56,a0)
	mov	d0,(60,a0)
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
	mov	d0,(64,a0)	# breakpoint VSR only if stubs included
#endif
	mov	d0,(68,a0)
	mov	d0,(72,a0)
	mov	d0,(76,a0)
	mov	d0,(80,a0)
	mov	d0,(84,a0)
	mov	d0,(88,a0)
	mov	d0,(92,a0)
	mov	d0,(96,a0)
	mov	d0,(100,a0)
	mov	d0,(104,a0)
	mov	d0,(108,a0)
	.endm
#else
	.macro	hal_mon_init_vsr
	.endif
#endif

#define CYGPKG_HAL_MN10300_MON_DEFINED

#define CYG_HAL_MN10300_VSR_TABLE_DEFINED

#endif	



#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
