##=============================================================================
##
##	arch.inc
##
##	i386 assembler header file for the synthetic target
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
## Author(s): 	jskov
## Contributors:jskov, nickg, bartv
## Date:	1999-01-20
## Purpose:	i386 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the i386
## Usage:
##		#include <cyg/hal/arch.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================

#------------------------------------------------------------------------------
# Exception, interrupt and thread context save area layout
# The layout of this structure is also defined in "hal_arch.h", for C
# code. Do not change this without changing that (or vice versa).

	# See SYSV ABI4, i386 supplement (page 37-38)
        # http://www.sco.com/products/layered/develop/devspecs/abi386-4.pdf

	# Callee save registers (eax, ecx, and edx are scratch registers)
	.equ	i386reg_esp,		0
	.equ	i386reg_next_context,	4 # only used when dropping through...
	.equ	i386reg_ebp,		8 # ...from switch_ to load_context.
	.equ	i386reg_ebx,		12
	.equ	i386reg_esi,		16
	.equ	i386reg_edi,		20
	.equ	i386reg_interrupts,	24
	.equ	i386reg_context_size, 	28

#------------------------------------------------------------------------------
# end of arch.inc
