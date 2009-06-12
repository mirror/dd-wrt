##=============================================================================
##
##      cogent.S
##
##      Cogent board hardware setup
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
## Author(s):   nickg
## Contributors:        nickg
## Date:        1997-11-11
## Purpose:     Cogent board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a Cogent PowerPC board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/arch.inc>

#------------------------------------------------------------------------------
                
        .globl  hal_hardware_init
hal_hardware_init:

#if defined(CYGPKG_HAL_POWERPC_COGENT) && defined(CYGPKG_HAL_POWERPC_MPC8xx)

        # Set up MPC8xx mapping registers for cogent

        lwi     r3,CYGARC_REG_IMM_BASE  # base address of control registers
	mtspr	CYGARC_REG_IMMR,r3

        # Set timebase divisor to busclock/16, pitrtc divisor to 512.
        lwz     r4,(CYGARC_REG_IMM_SCCR - CYGARC_REG_IMM_BASE)(r3)
        lwi     r5,(CYGARC_REG_IMM_SCCR_RTDIV | CYGARC_REG_IMM_SCCR_RTSEL | CYGARC_REG_IMM_SCCR_TBS)
        or      r4,r4,r5
        stw     r4,(CYGARC_REG_IMM_SCCR - CYGARC_REG_IMM_BASE)(r3)

        # Enable TimeBase
        li      r4,CYGARC_REG_IMM_TBSCR_TBE
        sth     r4,(CYGARC_REG_IMM_TBSCR - CYGARC_REG_IMM_BASE)(r3)

        # Note:  I do not know if the timeout is correct/optimal. 
        #        Works with the above memory mappings/wait states and 
        #        the PromICE.
        # Disable watchdog, enable bus monitor, timeout after 255*8 cycles
        lwi     r4,(CYGARC_REG_IMM_SYPCR_BMT_MASK | CYGARC_REG_IMM_SYPCR_BME)
        stw     r4,(CYGARC_REG_IMM_SYPCR - CYGARC_REG_IMM_BASE)(r3)

        # Set CS0 to
        # base addr 0xfff00000
        # wait states 4
        # size 1M
        # Covers ROM
        # Note: After a reset code is currently executing via the default
	# mapping defined in bank 0. This bank must remain valid while
	# it is being configured.
        lwi     r4,(0xfff00000 | CYGARC_REG_IMM_BR_PS_16 | CYGARC_REG_IMM_BR_V)
        stw     r4,(CYGARC_REG_IMM_BR0 - CYGARC_REG_IMM_BASE)(r3)
        lwi     r4,(0xfff00000 | CYGARC_REG_IMM_OR_BI | 4 << CYGARC_REG_IMM_OR_SCY_SHIFT)
        stw     r4,(CYGARC_REG_IMM_OR0 - CYGARC_REG_IMM_BASE)(r3)

        # Set CS1 to
        # base addr 0x00000000
        # external ACK
        # size 64M
        # Covers DRAM and slot0
        lwi     r4,(0xfc000000 | CYGARC_REG_IMM_OR_SETA)
        stw     r4,(CYGARC_REG_IMM_OR1 - CYGARC_REG_IMM_BASE)(r3)
        lwi     r4,(0x00000000 | CYGARC_REG_IMM_BR_PS_32 | CYGARC_REG_IMM_BR_V)
        stw     r4,(CYGARC_REG_IMM_BR1 - CYGARC_REG_IMM_BASE)(r3)

        # Set CS2 to
        # base addr 0x04000000
        # external ACK
        # size 64M
        # slot1 and slot2
        lwi     r4,(0xfc000000 | CYGARC_REG_IMM_OR_SETA)
        stw     r4,(CYGARC_REG_IMM_OR2 - CYGARC_REG_IMM_BASE)(r3)
        lwi     r4,(0x04000000 | CYGARC_REG_IMM_BR_PS_32 | CYGARC_REG_IMM_BR_V)
        stw     r4,(CYGARC_REG_IMM_BR2 - CYGARC_REG_IMM_BASE)(r3)

        # Set CS3 to
        # base addr 0x0e000000
        # exernal ack
        # size 32M
        # covers IO registers
        lwi     r4,(0xfe000000 | CYGARC_REG_IMM_OR_BI | CYGARC_REG_IMM_OR_SETA)
        stw     r4,(CYGARC_REG_IMM_OR3 - CYGARC_REG_IMM_BASE)(r3)
        lwi     r4,(0x0e000000 | CYGARC_REG_IMM_BR_PS_32 | CYGARC_REG_IMM_BR_V)
        stw     r4,(CYGARC_REG_IMM_BR3 - CYGARC_REG_IMM_BASE)(r3)

#endif  
                
        sync
        blr

#------------------------------------------------------------------------------
# end of cogent.S
