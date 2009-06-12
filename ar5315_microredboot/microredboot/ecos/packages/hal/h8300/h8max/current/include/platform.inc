#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	H8MAX "board" assembler header file
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
## Author(s): 	 Yoshinori Sato
## Contributors: Yoshinori Sato,Masahiro Ochiai
## Date:	 2002-08-10
## Purpose:	 H8MAX "board" definitions.
## Description:	 This file contains various definitions and macros that are
##               required for writing assembly code for the H8MAX board
##               Currently there are none
## Usage:
##		 #include <cyg/hal/platform.inc>
##		 ...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/mod_regs_bsc.h>
#include <cyg/hal/mod_regs_pio.h>

##-----------------------------------------------------------------------------

#define CYGPKG_HAL_H8300_MEMC_DEFINED
	.macro	hal_memc_init
#if defined(CYG_HAL_STARTUP_ROM)
	mov.l	#init_regs,er0
1:
	mov.w	@er0+,r2
	beq	1f
	mov.w	#0x00ff,e2	; internal I/O regs (2) high address
	cmp.w	#0xf000,r2
	bhi	2f
	dec.w	#1,e2		; internal I/O regs (1) high address
2:
	mov.w	@er0+,r1
	mov.b	r1l,@er2
	bra	1b

init_regs:
#define INIT_REGS_DATA(REGS,DATA) \
	.word	(REGS & 0xffff),DATA
init_regs:
INIT_REGS_DATA(CYGARC_RTCOR,195)
INIT_REGS_DATA(CYGARC_RTMCSR,0x17)
INIT_REGS_DATA(CYGARC_DRCRB,0xb8)
INIT_REGS_DATA(CYGARC_DRCRA,0x38)
INIT_REGS_DATA(CYGARC_ASTCR,0xfa)
INIT_REGS_DATA(CYGARC_ABWCR,0xe0)
INIT_REGS_DATA(CYGARC_BCR  ,0xc7)
INIT_REGS_DATA(CYGARC_CSCR ,0x1f)
INIT_REGS_DATA(CYGARC_WCRH ,0xfd)
INIT_REGS_DATA(CYGARC_WCRL ,0x44)
INIT_REGS_DATA(CYGARC_P1DDR,0xff)
INIT_REGS_DATA(CYGARC_P2DDR,0xff)
INIT_REGS_DATA(CYGARC_P6DDR,0xf6)
INIT_REGS_DATA(CYGARC_P8DDR,0xee)
	.word	0
;; Special Thanks Masahiro Ochiai

;; External peripheral setup delay
1:
	mov.l	#420000,er0	;wait 50ms
2:
	dec.l	#1,er0
	bpl	2b
#endif
	.endm

#define CYGPKG_HAL_H8300_DIAG_DEFINED

	.macro	hal_diag_init
	mov.b	#0x83,r0l
	mov.b	r0l,@CYGARC_PADDR
	mov.b	#0,r0l
	mov.b	r0l,@CYGARC_PADR
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

	.macro	hal_diag_data
	.endm

#if defined(CYGPKG_IO_ETH_DRIVERS)
#define CYGPKG_HAL_H8300_SAVED_VECTORS
	.macro	h8300_save_vectors
	.byte	16
	.endm
#endif

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
