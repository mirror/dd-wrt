##=============================================================================
##
##	variant.S
##
##	PowerPC MPC8xxx variant code
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
## Author(s): 	pfine
## Contributors:jskov
## Date:	2001-12-12
## Purpose:	PowerPC MPC8xxx variant code
## Description:	Variant specific code for PowerPC MPC8xxx CPUs.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/variant.inc>

#---------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.

        .data

        .extern hal_default_decrementer_isr
        .extern hal_default_isr

        .globl  hal_interrupt_handlers
hal_interrupt_handlers:
        .long   hal_default_decrementer_isr
        .rept   CYGNUM_HAL_ISR_COUNT-1
        .long   hal_default_isr
        .endr

        .globl  hal_interrupt_data
hal_interrupt_data:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

        .globl  hal_interrupt_objects
hal_interrupt_objects:
        .rept   CYGNUM_HAL_ISR_COUNT
        .long   0
        .endr

##-----------------------------------------------------------------------------
## end of variant.S
	
