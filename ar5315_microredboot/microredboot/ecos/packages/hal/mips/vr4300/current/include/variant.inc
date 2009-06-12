#ifndef CYGONCE_HAL_VARIANT_INC
#define CYGONCE_HAL_VARIANT_INC
##=============================================================================
##
##	variant.inc
##
##	VR4300 assembler header file
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
## Purpose:	VR4300 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the VR4300 CPU.
## Usage:
##		#include <cyg/hal/variant.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

        .set    mips3

#include <cyg/hal/mips.inc>

#include <cyg/hal/platform.inc>

#include <pkgconf/hal.h>

#------------------------------------------------------------------------------
# Initial SR value
# Set the KX bit to enable 64bit accesses in kernel mode.

#define INITIAL_SR_VAR	0x00000080

#------------------------------------------------------------------------------
# Set up initial value for config register. Sets endian mode and
# disables the cache on kseg0.

#if defined(CYGPKG_HAL_MIPS_MSBFIRST)
# define	INITIAL_CONFIG0	0x00008002
#elif defined(CYGPKG_HAL_MIPS_LSBFIRST)
# define	INITIAL_CONFIG0	0x00000002
#else
# error MIPS endianness not set by configuration
#endif

#------------------------------------------------------------------------------
# Set up initial value for FPU FCR31 register. We set the FS bit to flush
# denormalized results to zero.

#ifndef CYG_HAL_MIPS_FCSR_INIT
#define CYG_HAL_MIPS_FCSR_INIT 0x01000000
#endif

#------------------------------------------------------------------------------
# Cache macros.
	
#ifndef CYGPKG_HAL_MIPS_CACHE_DEFINED

	.macro	hal_cache_init

#ifndef CYG_HAL_STARTUP_RAM

	mfc0	v0,config0		# disable Kseg0 caching in config0 register
	nop
	nop
	la	v1,0xfffffff8
	and	v0,v0,v1
	ori	v0,v0,2
	mtc0	v0,config0
	nop
	nop
	nop

        .set mips3                      # Set ISA to MIPS 3 to allow cache insns

        # Now ensure the caches are invalidated. The caches are NOT cleared or
	# invalidated on non-power-up resets and may come up in a random state
	# on power-up. Hence they may contain stale or randomly bogus data.
	# Here we use the index-store-tag cache operation to clear all the cache
	# tags and states to zero. This will render them all invalid on the
	# VR4300.

        # D-cache:
        la      t0,0x80000000
        addi    t1,t0,0x2000
1:
	mtc0	zero,$28
	mtc0	zero,$29
        cache   0x09,0(t0)
        addi    t0,t0,0x10
        sub     v0,t1,t0
        bgez    v0,1b
        nop                             # delay slot

	# I-cache:
        la      a0,0x80000000
        addi    a1,a0,0x4000
1:
	mtc0	zero,$28
	mtc0	zero,$29
        cache   0x08,0(a0)
        addi    a0,a0,0x20
        sub     v0,a1,a0
        bgez    v0,1b
        nop                             # delay slot

        .set mips0                      # reset ISA to default
#endif

	.endm

#define CYGPKG_HAL_MIPS_CACHE_DEFINED

#endif	

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VARIANT_INC
# end of variant.inc
