#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	EDOSK-2764 "board" assembler header file
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
## Author(s): 	 yoshinori sato
## Contributors: yoshinori sato
## Date:	 2003-02-23
## Purpose:	 EDOSK-2674 "board" definitions.
## Description:	 This file contains various definitions and macros that are
##               required for writing assembly code for the am31 simulator.
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
#include <cyg/hal/mod_regs_intc.h>
#include <cyg/hal/mod_regs_sys.h>

##-----------------------------------------------------------------------------

#define CYGPKG_HAL_H8300_MEMC_DEFINED
	.macro	hal_memc_init
#if defined(CYG_HAL_STARTUP_ROM)
;BSC/GPIO setup
	mov.l	#init_regs,er0
	mov.w	#0xffff,e2
1:
	mov.w	@er0+,r2
	beq	2f
	mov.w	@er0+,r1
	mov.b	r1l,@er2
	bra	1b

#define INIT_REGS_DATA(REGS,DATA) \
	.word	((REGS) & 0xffff),DATA

init_regs:
INIT_REGS_DATA(CYGARC_ASTCR,0xfb)
INIT_REGS_DATA(CYGARC_RDNCR,0x00)
INIT_REGS_DATA(CYGARC_ABWCR,0x80)
INIT_REGS_DATA(CYGARC_WTCRAH,0x27)
INIT_REGS_DATA(CYGARC_WTCRAL,0x77)
INIT_REGS_DATA(CYGARC_WTCRBH,0x71)
INIT_REGS_DATA(CYGARC_WTCRBL,0x22)
INIT_REGS_DATA(CYGARC_CSACRH,0x80)
INIT_REGS_DATA(CYGARC_CSACRL,0x80)
INIT_REGS_DATA(CYGARC_BROMCRH,0xa0)
INIT_REGS_DATA(CYGARC_BROMCRL,0xa0)
INIT_REGS_DATA(CYGARC_P3DDR,0x3a)
INIT_REGS_DATA(CYGARC_P3ODR,0x06)
INIT_REGS_DATA(CYGARC_PADDR,0xff)
INIT_REGS_DATA(CYGARC_PFDDR,0xfe)
INIT_REGS_DATA(CYGARC_PGDDR,0x0f)
INIT_REGS_DATA(CYGARC_PHDDR,0x0f)
INIT_REGS_DATA(CYGARC_PFCR0,0xff)
INIT_REGS_DATA(CYGARC_PFCR2,0x0d)
INIT_REGS_DATA(CYGARC_INTCR,0x28)
INIT_REGS_DATA(CYGARC_ITSR, 0xff)
INIT_REGS_DATA(CYGARC_ITSR+1,0x3f)
INIT_REGS_DATA(CYGARC_ISCRL, 0x55)
INIT_REGS_DATA(CYGARC_ISCRL+1,0x55)
INIT_REGS_DATA(CYGARC_MSTPCRL,0xf7)

	.word	0
2:
;SDRAM setup
#define SDRAM_SMR 0x400040

	mov.w	#0,r0
	mov.w	r0,@CYGARC_DRACCR:16
	mov.w	#0x188,r0
	mov.w	r0,@CYGARC_REFCR:16
	mov.w	#0x85b4,r0
	mov.w	r0,@CYGARC_DRAMCR:16
	mov.b	#0,r1l
	mov.b	r1l,@SDRAM_SMR
	mov.w	#0x84b4,r0
	mov.w	r0,@CYGARC_DRAMCR:16
	
;special thanks to Arizona Cooperative Power
#endif
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
