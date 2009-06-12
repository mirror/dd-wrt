##=============================================================================
##
##	variant.S
##
##	POWERPC MPC8xx variant code
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 2003 Bart Veer
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
## Author(s): 	jskov
## Contributors:jskov, gthomas
## Date:	2000-02-04
## Purpose:	PowerPC MPC8xx variant code
## Description:	Variant specific code for PowerPC MPC8xx CPUs.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/arch.inc>
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */
					/* registers, IMM layout...         */

#ifdef CYGPKG_HAL_QUICC                
#include <cyg/hal/quicc/ppc8xx.h>       /* more of the same */
#endif        
        
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

        
#---------------------------------------------------------------------------
# Hard reset support
#

        .text                
        .globl  _mpc8xx_reset
_mpc8xx_reset:
#ifdef CYGPKG_HAL_QUICC                
	lwi     r4,CYGARC_REG_IMM_BASE  # base address of control registers

        // Enable checkstop reset
        lwz     r3,PLPRCR(r4)
        ori     r3,r3,0x0080  // Bit 24
	stw	r3,PLPRCR(r4)
        mfmsr   r3
        lwi     r5,~CYGARC_REG_MSR_ME
        and     r3,r3,r5
        mtmsr   r3

        // Pull the plug by disabling CS0 & CS1.  This will cause a checkstop.
        li      r0,0
        stw     r0,BR0(r4)
        stw     r0,BR1(r4)
        lwi     r3,_end
10:     nop
        lwzu    r3,4(r2)        // Force a memory access
        b       10b
#else
10:     nop
        b       10b
#endif                        

##-----------------------------------------------------------------------------
## end of variant.S
