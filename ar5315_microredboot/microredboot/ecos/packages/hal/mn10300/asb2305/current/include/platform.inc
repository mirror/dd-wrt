#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	ASB2305 board assembler header file
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
## Author(s): 	dhowells
## Contributors:dmoseley
## Date:	2001-05-17
## Purpose:	ASB2305 board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the ASB2305 board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/plf_io.h>

#------------------------------------------------------------------------------
# Diagnostics macros.

#include <cyg/hal/hal_io.h>

	.macro	hal_diag_data
hal_diag_intr_count:	.long	0
	.endm

#define HAL_EARLY_INIT hal_early_init
.macro hal_early_init
	# turn on the sofware status LED
	movbu	(0xDB000008),d0
	or	0x80,d0
	movbu	d0,(0xDB000008)
	movhu	(0xDB000000),d0
	and	0x3FFF,d0
	or	0x4000,d0
	movhu	d0,(0xDB000000)

	# enable system flash memory write
	movbu	(0xDB000008),d0
	and	0xdf,d0
	movbu	d0,(0xDB000008)
	movhu	(0xDB000000),d0
	and	0xF3FF,d0
	or	0x0400,d0
	movhu	d0,(0xDB000000)

.endm

	.macro	hal_diag_init
	mov	0x5f17ff7f,d0			# 'rh -'
	mov	d0,(_hal_led_old_display)
	mov	d0,(HAL_LED_ADDRESS)
	.endm

#define CYGPKG_HAL_MN10300_DIAG_DEFINED

	.macro hal_handle_unknown_int
	mov	_hal_diag_digits,a2
	mov	(0xC0000024),d0	// Get TBR
	mov	0x6D00,d1	// 7-seg LED:	=ABC
	mov	d0,d2		// 0xnnnnnAnn
	lsr	8,d2
	and	0xf,d2
	movbu	(a2,d2),d2
	or	d2,d1
	asl	8,d1
	mov	d0,d2		// 0xnnnnnnBn
	lsr	4,d2
	and	0xf,d2
	movbu	(a2,d2),d2
	or	d2,d1
	asl	8,d1
	and	0xf,d0		// 0xnnnnnnnC
	movbu	(a2,d0),d2
	or	d2,d1
	mov	(_hal_led_old_display),d0
	and	0x00010000,d0
	or	d1,d0
	mov	d0,(_hal_led_old_display)
	mov	d0,(HAL_LED_ADDRESS)
	.endm
	
#define CYG_HAL_HANDLE_UNKNOWN_INT_DEFINED

	.macro	hal_diag_excpt_start
	hal_handle_unknown_int
	.endm

	.macro	hal_diag_excpt_end
	mov	0x5f17ff01,d0
	mov	d0,(_hal_led_old_display)
	mov	d0,(HAL_LED_ADDRESS)
	.endm
#define CYG_HAL_DIAG_EXCPT_END

	.macro	hal_diag_intr_start
	mov	(hal_diag_intr_count),d1
	inc	d1
	cmp	100,d1
	bne	x\@
	clr	d1
	mov	(_hal_led_old_display),d0
	xor	0x00010000,d0
	mov	d0,(_hal_led_old_display)
	mov	d0,(HAL_LED_ADDRESS)
x\@:
	mov	d1,(hal_diag_intr_count)
	.endm

	.macro	hal_diag_restore
	.endm

	.macro	hal_diag_led val
	movm	[d2,d3,a2],(sp)
0:	mov	_hal_diag_digits,a2
	mov	\val,d2
	and	0xf,d2
	add	d2,a2
	movbu	(a2),d3
	mov	(_hal_led_old_display),d2
	asl	8,d2
	or	d3,d2
	mov	d2,(HAL_LED_ADDRESS)
	mov	d2,(_hal_led_old_display)
	movm	(sp),[d2,d3,a2]
	.endm

	.macro hal_cpu_stop
//	mov	0x10,d0
//	movhu	d0,(0xC0000040)	// enter STOP mode (register CPUM)
	.endm
#define CYG_HAL_CPU_STOP_DEFINED

#------------------------------------------------------------------------------
# MEMC macros.

#ifndef CYGPKG_HAL_MN10300_MEMC_DEFINED

// These settings follow the recommended settings in the
// "MN103E010 Evaluation Board User's Guide"

#define BCCR                    0xC0002000
#define BCCR_INIT               0x12040580

#define SBBASE0                 0xD8C00100
#define SBBASE0_INIT_SYSFL			0x8000FE01
#define SBBASE0_INIT_BPROM			0x8400FE01
#define SBBASE1                 0xD8C00110
#define SBBASE1_INIT_SYSFL			0x8400FE01
#define SBBASE1_INIT_BPROM			0x8000FE01
#define SBBASE2                 0xD8C00120
#define SBBASE2_INIT				0x8600FF81
#define SBBASE3                 0xD8C00130
#define SBBASE3_INIT				0x8680FF81
#define SBBASE4                 0xD8C00140
#define SBBASE4_INIT				0x9800F801
#define SBBASE5                 0xD8C00150
#define SBBASE5_INIT				0x00000000
#define SBBASE6                 0xD8C00160
#define SBBASE6_INIT				0x00000000
#define SBBASE7                 0xD8C00170
#define SBBASE7_INIT				0x00000000

#define SBCTRL00                0xD8C00200
#define SBCTRL00_INIT				0x21111000
#define SBCTRL10                0xD8C00210
#define SBCTRL10_INIT				0x21111000
#define SBCTRL20                0xD8C00220
#define SBCTRL20_INIT				0x21111000
#define SBCTRL30                0xD8C00230
#define SBCTRL30_INIT				0x11111000
#define SBCTRL40                0xD8C00240
#define SBCTRL40_INIT				0x00140000
#define SBCTRL50                0xD8C00250
#define SBCTRL50_INIT				0x22100000
#define SBCTRL60		0xD8C00260
#define SBCTRL60_INIT				0x22100000
#define SBCTRL70		0xD8C00270
#define SBCTRL70_INIT				0x22100000

#define SBCTRL01		0xD8C00204
#define SBCTRL01_INIT				0x00100200
#define SBCTRL11		0xD8C00214
#define SBCTRL11_INIT				0x00100200
#define SBCTRL21		0xD8C00224
#define SBCTRL21_INIT				0x00100200
#define SBCTRL31		0xD8C00234
#define SBCTRL31_INIT				0x00100100
#define SBCTRL41		0xD8C00244
#define SBCTRL41_INIT				0x11011100
#define SBCTRL51		0xD8C00254
#define SBCTRL51_INIT				0x00001100
#define SBCTRL61		0xD8C00264
#define SBCTRL61_INIT				0x00001100
#define SBCTRL71		0xD8C00274
#define SBCTRL71_INIT				0x00001100

#define SBCTRL02		0xD8C00208
#define SBCTRL02_INIT				0x00000004
#define SBCTRL12		0xD8C00218
#define SBCTRL12_INIT				0x04000004
#define SBCTRL22		0xD8C00228
#define SBCTRL22_INIT				0x00000004
#define SBCTRL32		0xD8C00238
#define SBCTRL32_INIT				0x00000002
#define SBCTRL42		0xD8C00248
#define SBCTRL42_INIT				0x01000001
#define SBCTRL52		0xD8C00258
#define SBCTRL52_INIT				0x0000000F
#define SBCTRL62		0xD8C00268
#define SBCTRL62_INIT				0x0000000F
#define SBCTRL72		0xD8C00278
#define SBCTRL72_INIT				0x0000000F

#define SDBASE0			0xDA000008
#define SDBASE1			0xDA00000C
#define SDRAMBUS		0xDA000000

// 16MB SDRAM
#define SDBASE0_8M_INIT         0x9000FF81
#define SDBASE1_8M_INIT         0x9080FF81
#define SDRAMBUS_8M_INIT        0xA8990654

// 32MB SDRAM
#define SDBASE0_16M_INIT        0x9000FF01
#define SDBASE1_16M_INIT        0x9100FF01
#define SDRAMBUS_16M_INIT       0xA89a0654

// 64MB SDRAM
#define SDBASE0_32M_INIT        0x9000fe01
#define SDBASE1_32M_INIT        0x9200fe01
#define SDRAMBUS_32M_INIT       0xa89b0654

	.macro	hal_memc_init
	mov	BCCR,a0
	mov	BCCR_INIT,d0
	mov	d0,(a0)

	// reduce the span of the ROM banks first
	mov	(SBBASE0),d0
	btst	1,d0
	beq	0f
	mov	0x8000FE01,d0
	mov	d0,(SBBASE0)
0:
	mov	(SBBASE1),d0
	btst	1,d0
	beq	0f
	mov	0x8000FE01,d0
	mov	d0,(SBBASE1)
0:
	
	// memory bank 2
	mov	SBBASE2_INIT,d0
	mov	d0,(SBBASE2)
	mov	SBCTRL20_INIT,d0
	mov	d0,(SBCTRL20)
	mov	SBCTRL21_INIT,d0
	mov	d0,(SBCTRL21)
	mov	SBCTRL22_INIT,d0
	mov	d0,(SBCTRL22)

	// memory bank 3
	mov	SBBASE3_INIT,d0
	mov	d0,(SBBASE3)
	mov	SBCTRL30_INIT,d0
	mov	d0,(SBCTRL30)
	mov	SBCTRL31_INIT,d0
	mov	d0,(SBCTRL31)
	mov	SBCTRL32_INIT,d0
	mov	d0,(SBCTRL32)

	// memory bank 4
	mov	SBBASE4_INIT,d0
	mov	d0,(SBBASE4)
	mov	SBCTRL40_INIT,d0
	mov	d0,(SBCTRL40)
	mov	SBCTRL41_INIT,d0
	mov	d0,(SBCTRL41)
	mov	SBCTRL42_INIT,d0
	mov	d0,(SBCTRL42)

	// memory bank 5
#if SBBASE5_INIT != 0
	mov	SBBASE5_INIT,d0
	mov	d0,(SBBASE5)
	mov	SBCTRL50_INIT,d0
	mov	d0,(SBCTRL50)
	mov	SBCTRL51_INIT,d0
	mov	d0,(SBCTRL51)
	mov	SBCTRL52_INIT,d0
	mov	d0,(SBCTRL52)
#endif

	// memory bank 6
#if SBBASE6_INIT != 0
	mov	SBBASE6_INIT,d0
	mov	d0,(SBBASE6)
	mov	SBCTRL60_INIT,d0
	mov	d0,(SBCTRL60)
	mov	SBCTRL61_INIT,d0
	mov	d0,(SBCTRL61)
	mov	SBCTRL62_INIT,d0
	mov	d0,(SBCTRL62)
#endif

	// memory bank 7
#if SBBASE7_INIT != 0
	mov	SBBASE7_INIT,d0
	mov	d0,(SBBASE7)
	mov	SBCTRL70_INIT,d0
	mov	d0,(SBCTRL70)
	mov	SBCTRL71_INIT,d0
	mov	d0,(SBCTRL71)
	mov	SBCTRL72_INIT,d0
	mov	d0,(SBCTRL72)
#endif

#ifndef CYG_HAL_STARTUP_RAM

        // Setup for 64MB initially and determine final mem config below.
        mov     SDRAMBUS,a0
        mov     (a0),d0
        and     0xfffffffb,d0  // disable refresh
        mov     d0,(a0)

        mov     SDBASE0,a0
        mov     SDBASE0_32M_INIT,d0
        mov     d0,(a0)
        mov     SDBASE1,a0
        mov     SDBASE1_32M_INIT,d0
        mov     d0,(a0)
        mov     SDRAMBUS,a0
        mov     SDRAMBUS_32M_INIT,d0
        mov     d0,(a0)

        mov     0x1000,d0
0:
        sub     1,d0
        bne     0b

        // Check for 16MB and 32MB shadowing to determine actual amount of
        // memory installed. This assumes 8M, 16M, or 32M configs.
        mov     0,d0
        mov     d0,(0x90800000)
        mov     d0,(0x91000000)
        mov     0xaaaaaaaa,d0
        mov     d0,(0x90000000)
        mov     (0x90800000),d1
        cmp     d0,d1
        bne     1f

        // 16MB installed
        mov     SDRAMBUS,a0
        mov     (a0),d0
        and     0xfffffffb,d0  // disable refresh
        mov     d0,(a0)
        mov     SDBASE0,a1
        mov     SDBASE0_8M_INIT,d1
        mov     d1,(a1)
        mov     SDBASE1,a1
        mov     SDBASE1_8M_INIT,d1
        mov     d1,(a1)
        mov     SDRAMBUS_8M_INIT,d0
        mov     d0,(a0)
        jmp     2f
1:
        mov     (0x91000000),d1
        cmp     d0,d1
        bne     2f

        // 32MB installed
        mov     SDRAMBUS,a0
        mov     (a0),d0
        and     0xfffffffb,d0  // disable refresh
        mov     d0,(a0)
        mov     SDBASE0,a1
        mov     SDBASE0_16M_INIT,d1
        mov     d1,(a1)
        mov     SDBASE1,a1
        mov     SDBASE1_16M_INIT,d1
        mov     d1,(a1)
        mov     SDRAMBUS_16M_INIT,d0
        mov     d0,(a0)
2:
#endif // ! CYG_HAL_STARTUP_RAM

	// now the ROMs need putting into the right place
	// - this is tricky because when we're booting from the system flash,
	//   it has had its base address pre-swapped by the CPU
	// - we need to copy a small piece of code to the SRAM and execute it
	//   there

	// display on the LEDs
	mov	0x7e7e7e7e,d0			# '-.-.-.-.'
	mov	d0,(HAL_LED_ADDRESS)

	// copy the ROM address adjustor to the SRAM
	add	-4,sp
	call	__hal_plf_base_ref,[],0		// note we can't address ourselves directly yet
__hal_plf_base_ref:
	movm	(sp),[a3]
	mov	a3,a0
	add	__hal_plf_rom_swap_start-__hal_plf_base_ref,a0
	add	__hal_plf_rom_swap_end-__hal_plf_base_ref,a3
	mov	0x8C000000,a1
0:	movbu	(a0),d0
	movbu	d0,(a1)
	inc	a0
	inc	a1
	cmp	a0,a3
	bcc	0b
	mov	0x8C000000,a1
	jmp	(a1)	

	////////////////////////////////////////////////////////////////
__hal_plf_rom_swap_start:
	// put boot PROM at 0x80000000, and system flash at 0x84000000
	mov	SBBASE0_INIT_BPROM,d0
	mov	d0,(SBBASE0)

	mov	SBBASE1_INIT_BPROM,d0
	mov	d0,(SBBASE1)

	// memory bank 0
	mov	SBCTRL00_INIT,d0
	mov	d0,(SBCTRL00)
	mov	SBCTRL01_INIT,d0
	mov	d0,(SBCTRL01)
	mov	SBCTRL02_INIT,d0
	mov	d0,(SBCTRL02)

	// memory bank 1
	mov	SBCTRL10_INIT,d0
	mov	d0,(SBCTRL10)
	mov	SBCTRL11_INIT,d0
	mov	d0,(SBCTRL11)
	mov	SBCTRL12_INIT,d0
	mov	d0,(SBCTRL12)

	// jump forward so we start running from the 80000000/84000000 base address
        mov     __hal_plf_rom_swap_reentry,a0
	jmp	(a0)
__hal_plf_rom_swap_end:
	////////////////////////////////////////////////////////////////

__hal_plf_rom_swap_reentry:
	// clear the on-CPU 16Kb SRAM
	clr	d0
	mov	16384/4-4,d1
	mov	0x8C000000,a0
0:
	mov	d0,(a0,d1)
	sub	4,d1
	bnc	0b

	.endm

#define CYGPKG_HAL_MN10300_MEMC_DEFINED

#endif


//-----------------------------------------------------------------------------
// Syscall support.
#if defined(CYGPKG_CYGMON) || defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
// Cygmon provides syscall handling for this board
// These must be kept in sync with the rest of the tree.
#define SIGSYS     12
#define SIGSYSCALL SIGSYS
#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
