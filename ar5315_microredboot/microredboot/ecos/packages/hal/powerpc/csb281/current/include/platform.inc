#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##	platform.inc
##
##	Platform specific assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002, 2003 Gary Thomas
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
## Author(s): 	gthomas
## Contributors:
## Date:	2002-07-31
## Purpose:	Platform specific assembly code
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the platform.
## Usage:       included from <cyg/hal/variant.inc>
##		
##
######DESCRIPTIONEND####
##
##=============================================================================
##-----------------------------------------------------------------------------
## MPC82xx interrupt handling.

#ifndef CYGPKG_HAL_POWERPC_INTC_DEFINED

        # decode the interrupt
	.macro  hal_intc_decode dreg,state
        lwz     \dreg,CYGARC_PPCREG_VECTOR(\state) # retrieve vector number,
        rlwinm. \dreg,\dreg,22,31,31            # isolate bit 21
        beq     0f                              # done if decrementer (vec 0)
        lwi     \dreg,_CSB281_EPIC_IACK         # if external, get vector
        li      r0,0            
        lwbrx   \dreg,r0,\dreg                  # EPIC is little endian!
        cmpi    0,\dreg,0xFF                    # spurious interrupt?
        beq     restore_state
        lwi     r3,_CSB281_EPIC_EOI             # clear interrupt?
        stw     r0,0(r3)
0:      stw     \dreg,CYGARC_PPCREG_VECTOR(\state) # update vector in state frame.
        slwi    \dreg,\dreg,2                   # convert to byte offset.
        .endm                              

#define CYGPKG_HAL_POWERPC_INTC_DEFINED
#endif // CYGPKG_HAL_POWERPC_INTC_DEFINED

#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
