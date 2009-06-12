#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	STB board assembler header file
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
## Purpose:	STB board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the STB board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>


#------------------------------------------------------------------------------
# Diagnostics macros.

#define CYG_HAL_DIAG_LED_ADDRESS1 0x83f90000
#define CYG_HAL_DIAG_LED_ADDRESS2 0x81f90000
#define CYG_HAL_DIAG_LED_ADDRESS3 hal_diag_led_state

	.macro	hal_diag_data
	.globl	hal_diag_led_state
hal_diag_led_state:	.long	0xffffffff
hal_diag_intr_count:	.long	0
	.endm

	.macro	hal_diag_init
	mov	0xFFFFFFFF,d0
	mov	CYG_HAL_DIAG_LED_ADDRESS1,a0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS2,a0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS3,a0
	mov	d0,(a0)

	mov	0xd4002000,a0	# SR control regs
	mov	0xd4003000,a1	# timer control regs
	mov	99,d0
	movbu	d0,(16,a1)
	mov	0x80,d0
	movbu	d0,(a1)
	clr	d0
	movbu	d0,(4,a0)
	mov	0xc084,d0
	movhu	d0,(a0)

	jmp	1f

hal_diag_digits:
	.byte	0x81	# 0
	.byte	0xf3	# 1
	.byte	0x49	# 2
	.byte	0x61	# 3
	.byte	0x33	# 4
	.byte	0x25	# 5
	.byte	0x05	# 6
	.byte	0xf1	# 7
	.byte	0x01	# 8
	.byte	0x21	# 9
	.byte	0x11	# A
	.byte	0x07	# B
	.byte	0x8d	# C
	.byte	0x43	# D
	.byte	0x0d	# E
	.byte	0x1d	# F

hal_diag_hex_digits:
	.ascii	"0123456789ABCDEF"
1:

#	hal_diag_led 0
#	hal_diag_led 1
#	hal_diag_led 2
#	hal_diag_led 3
#	hal_diag_led 4
#	hal_diag_led 5
#	hal_diag_led 6
#	hal_diag_led 7
#	hal_diag_led 8
#	hal_diag_led 9
#	hal_diag_led 0xa
#	hal_diag_led 0xb
#	hal_diag_led 0xc
#	hal_diag_led 0xd
#	hal_diag_led 0xe
#	hal_diag_led 0xf

	.endm

	.macro	hal_diag_excpt_start
	mov	CYG_HAL_DIAG_LED_ADDRESS3,a0
	mov	(a0),d0
	xor	0x01000000,d0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS1,a0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS2,a0
	mov	d0,(a0)
	.endm

	.macro	hal_diag_intr_start
	mov	(hal_diag_intr_count),d1
	inc	d1
	cmp	100,d1
	bne	x\@
	clr	d1
	mov	CYG_HAL_DIAG_LED_ADDRESS3,a0
	mov	(a0),d0
	xor	0x00010000,d0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS1,a0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS2,a0
	mov	d0,(a0)
x\@:
	mov	d1,(hal_diag_intr_count)
	.endm

#if 0
	.macro	hal_diag_restore
	mov	(hal_diag_intr_count),d0
	and	0x1000,d0
	beq	x\@
	mov	CYG_HAL_DIAG_LED_ADDRESS3,a0
	mov	(a0),d0
	and	0xfffeffff,d0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS1,a0
	mov	d0,(a0)
	mov	CYG_HAL_DIAG_LED_ADDRESS2,a0
	mov	d0,(a0)
x\@:
	.endm
#else
	.macro	hal_diag_restore
	.endm
#endif

#if 0
	.macro	hal_diag_led val
	movm	[d2,d3,a2],(sp)
	mov	hal_diag_digits,a2
	mov	\val,d2
	and	0xf,d2
	add	d2,a2
	movbu	(a2),d3
	mov	CYG_HAL_DIAG_LED_ADDRESS3,a2
	mov	(a2),d2
	asl	8,d2
	or	d3,d2
	mov	d2,(a2)
	mov	CYG_HAL_DIAG_LED_ADDRESS1,a2
	mov	d2,(a2)
	mov	CYG_HAL_DIAG_LED_ADDRESS2,a2
	mov	d2,(a2)

#	mov	\val,d3
#	and	0xf,d3
#	mov	hal_diag_hex_digits,a2
#	add	d3,a2
#	movbu	(a2),d3
#	mov	0xd4002000,a2
#x2:	movhu	(0,a2),d2
#	and	0x20,d2
#	bne	2b
#	movbu	d3,(8,a2)

	mov	200000,d2
1:
	add	-1,d2
	cmp	0,d2
	bne	1b

	movm	(sp),[d2,d3,a2]
	.endm
#else
	.macro	hal_diag_led val
	.endm
#endif

#define CYGPKG_HAL_MN10300_DIAG_DEFINED


#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
