##
#=============================================================================
##	platform.S
##
##	CalmRISC16 CEB platform code
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
## Author(s):	msalter
## Contributors: msalter
## Date:	2001-02-12
## Purpose:	CalmRISC16 platform code
## Description:	Platform specific code for CalmRISC16 core evb
##
##
##
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

#include <cyg/hal/arch.inc>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_arch.h>

##-----------------------------------------------------------------------------

##-----------------------------------------------------------------------------
# Platform Initialization.
# This code performs platform specific initialization.

##-----------------------------------------------------------------------------
## MEMC initialization.
##

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

#endif /* defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) */

##-----------------------------------------------------------------------------
# This code sets up a stack for applications running on redboot
FUNC_START ceb_program_new_stack
#if 0
	push	r4
	push	lr
	ld	r4,sp
	ldw	sp,0xffff0
	jsr	r0
	 nop
	ld	sp,r4
	pop	lr
	jmpd	lr
	 pop	r4
#endif
FUNC_END ceb_program_new_stack

##-----------------------------------------------------------------------------
## end of platform.S
