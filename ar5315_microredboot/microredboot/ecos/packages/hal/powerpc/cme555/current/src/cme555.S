##=============================================================================
##
##      cme555.S
##
##      CME555 board hardware setup
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
## Author(s):   Bob Koninckx
## Contributors:Bob Koninckx
## Date:        2001-12-15
## Purpose:     cme555 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a cme555 mpc555 board.
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

#if defined(CYGPKG_HAL_POWERPC_CME555) && defined(CYGPKG_HAL_POWERPC_MPC5xx)
        lwi     r3, CYGARC_REG_IMM_BASE             # Base address of control registers
       
        // Configure external interrupt pins as General purpose I/O. They do not
        // have pull-up resistors on the CME555. Add to that that IRQ0 triggers
        // a non-maskable reset of the board ... This one gave me nightmares ...
        lwi     r4, 0x00000800 
        stw     r4, (CYGARC_REG_IMM_SIUMCR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xff88
        stw     r4, (CYGARC_REG_IMM_SYPCR-CYGARC_REG_IMM_BASE)(r3)

        // Memory map
        // CS0 configuration
        // Base address 0x400000, no burst support, 32 bit wide data port
        // Valid addresses till 0x43FFFF, no wait states
        lwi     r4, 0x00400003
        stw     r4, (CYGARC_REG_IMM_BR0-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xfff80002
        stw     r4, (CYGARC_REG_IMM_OR0-CYGARC_REG_IMM_BASE)(r3)

        // CS1 configuration
        // Base address 0x800000, no burst support, 32 bit wide data port
        // Valid adresses till 0x87FFFF, three wait states for access time 70ns
        lwi     r4, 0x00800003
        stw     r4, (CYGARC_REG_IMM_BR1-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xfff80032
        stw     r4, (CYGARC_REG_IMM_OR1-CYGARC_REG_IMM_BASE)(r3)

        // CS2 configuration
        // Base address 0xc00000, no burst support, 32 bit wide data port
        // Valid adresses till 0xc7FFFF, maximum wait states
        lwi     r4, 0x00c00003
        stw     r4, (CYGARC_REG_IMM_BR2-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xfff800f2
        stw     r4, (CYGARC_REG_IMM_OR2-CYGARC_REG_IMM_BASE)(r3)

        // CS3 configuration
        // works for keypad/LCD
        // maximum wait states
        lwi     r4, 0x01000403
        stw     r4, (CYGARC_REG_IMM_BR3-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0xfffe0ff1
        stw     r4, (CYGARC_REG_IMM_OR3-CYGARC_REG_IMM_BASE)(r3)

        // And  some other configuration registers
#if defined(CYGSEM_HAL_POWERPC_MPC5XX_IFLASH_DUAL_MAP)
        lwi     r4, 1
#else
        lwi     r4, 0
#endif
        stw     r4, (CYGARC_REG_IMM_DMBR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0
        stw     r4, (CYGARC_REG_IMM_DMOR-CYGARC_REG_IMM_BASE)(r3)

        // Limb mode enable, TMBCLK = BUS/16, RTCCLK = Crystal / 16
        // generate 40 MHz bus clock
        lwi     r4, 0x3010000
        stw     r4, (CYGARC_REG_IMM_SCCR-CYGARC_REG_IMM_BASE)(r3)
        lwi     r4, 0x900000
        stw     r4, (CYGARC_REG_IMM_PLPRCR-CYGARC_REG_IMM_BASE)(r3)

        // Enable the time base and set the freeze flag
        lwi     r4, 0x03
        sth     r4, (CYGARC_REG_IMM_TBSCR-CYGARC_REG_IMM_BASE)(r3)

        // Unlock the RTC register set
        lwi     r4, 0x55ccaa33
        stw     r4, (CYGARC_REG_IMM_RTCSCK-CYGARC_REG_IMM_BASE)(r3)
        // RTC is clocked by 4MHz crystal, set the freeze flag
        lwi     r4, 0x12
        sth     r4, (CYGARC_REG_IMM_RTCSC-CYGARC_REG_IMM_BASE)(r3)
        // Set the freeze flag for the Periodic interrupt timer
        lwi     r4, 0x02
        sth     r4, (CYGARC_REG_IMM_PISCR-CYGARC_REG_IMM_BASE)(r3)

        lwi     r4, 0x00
        lwi     r5, (CYGARC_REG_IMM_DPTMCR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0xffa0
        lwi     r5, (CYGARC_REG_IMM_RAMBAR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0x00
        lwi     r5, (CYGARC_REG_IMM_PORTQS-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0x00
        lwi     r5, (CYGARC_REG_IMM_PQSPAR_DDRQST-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_MPIOSMDR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_MPIOSMDDR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_MIOS1TPCR-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SRAMMCR_A-CYGARC_REG_IMM_BASE)
        sthx    r4, r3, r5

        lwi     r4, 0x00000000
        lwi     r5, (CYGARC_REG_IMM_SGPIODT1-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SGPIODT2-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_SGPIOCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
        lwi     r5, (CYGARC_REG_IMM_EMCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5

        // Enable 32 interrupt priorities on the IMB3 unit
        lwi     r4, 0x60000000
        lwi     r5, (CYGARC_REG_IMM_UMCR-CYGARC_REG_IMM_BASE)
        stwx    r4, r3, r5
#endif  
                
        sync
        blr

#------------------------------------------------------------------------------
# end of cme555.S
