##=============================================================================
##
##      edk.S
##
##      EDK board hardware setup
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
## Author(s):   jskov
## Contributors:jskov
## Date:        1999-05-03
## Purpose:     EDK7708 board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a Hitachi SH3 EDK7708 board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/sh_regs.h>

        .globl  _hal_hardware_init
_hal_hardware_init:

        // Set up the Bus State Controller
        mov.l    $BSC_settings_table,r3
1:      mov.w    @r3+,r0                // Address (or zero)
        cmp/eq   #0,r0
        bt       2f
        mov.w    @r3+,r1                // data
        bra      1b
         mov.w    r1,@r0                // delay slot

2:
        rts
         nop

        .align  2
$BSC_settings_table:
        .long   BSC_settings_table
BSC_settings_table:
        # These are the settings set by the Hitachi ROM Monitor.

        # BCR2: Bus size of areas 1-6 to 32 bits
        .word   CYGARC_REG_BCR2
        .word   0x3ffc
        # BCR1: Areas 2 and 3 are SDRAM
        .word   CYGARC_REG_BCR1
        .word   0x080c
        # BCR2: Bus size of areas 1-6 to 32 bits [note: second write!]
        .word   CYGARC_REG_BCR2
        .word   0x3ffc
        # WCR1: 3 wait-state cycles inserted for all areas
        .word   CYGARC_REG_WCR1
        .word   0x3fff
        # WCR2: extra wait states and full pitch for burst
        .word   CYGARC_REG_WCR2
        .word   0xffd7
        # MCR: RAS/CAS & burst timing area 2/3
        .word   CYGARC_REG_MCR
        .word   0x963c
#if 0
        # DCR: RAS/CAS & burst timing area 2
        .word   CYGARC_REG_DCR
        .word   0x0000
        # PCR: PCMCIA disabled
        .word   CYGARC_REG_PCR
        .word   0x0000
#endif
        # RTCNT: refresh counter (needs a5 in top byte to accept write)
        .word   CYGARC_REG_RTCNT
        .word   (0xa500 | 0x0000)
        # RTCOR: refresh time constant (needs a5 in top byte to accept write)
        .word   CYGARC_REG_RTCOR
        .word   (0xa500 | 0x003b)
        # RFCR:  refresh count register (needs a4 in top byte to accept write)
        .word   CYGARC_REG_RFCR
        .word   (0xa400 | 0x0000)
        # RTCSR: refresh timer control (needs a5 in top byte to accept write)
        .word   CYGARC_REG_RTCSR
        .word   (0xa500 | 0x0008)

        # Set SDMR to 0x220
        .word    0xd880
        .word    0       
        
        # Table end
        .word   0

#------------------------------------------------------------------------------
# end of edk.S
