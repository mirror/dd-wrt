#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	CalmRISC16 core eval board assembler header file
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
## Author(s): 	msalter
## Contributors: msalter
## Date:	2001-02-12
## Purpose:	Core eval board definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the core eval board.
## Usage:
##		#include <cyg/hal/platform.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/calm16.inc>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_io.h>

#define CYGPKG_HAL_RESET_VECTOR_FIRST_CODE
	.macro hal_reset_vector_first_code
	.endm

#define CYGPKG_HAL_EARLY_INIT
	.macro	hal_early_init
	.endm

#------------------------------------------------------------------------------
# Diagnostic macros

#ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

#define CYGPKG_HAL_MIPS_DIAG_DEFINED

#endif // ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED
	
#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
