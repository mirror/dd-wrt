#==============================================================================
##
##      sim.S
##
##      Simulator hardware setup
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
## Purpose:     Simulator hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on the PowerPC simulator.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/arch.inc>

#------------------------------------------------------------------------------
                
        .globl  hal_hardware_init
hal_hardware_init:

#if 0   
        # Set up the BAT registers to cover same address spaces

        # ibat0
        # base addr 0xfff00000
        # length 1M
        # WIMG = 0000
        # covers ROM
        lwi     r4,0xfff0001f
        mtibatu 0,r4
        lwi     r4,0xfff00002
        mtibatl 0,r4
        
        # ibat1
        # base addr 0x00000000
        # length 256M
        # WIMG = 0000
        # covers DRAM and slots
        lwi     r4,0x00001fff
        mtibatu 1,r4
        lwi     r4,0x00000002
        mtibatl 1,r4
                
        # dbat0
        # base addr 0xfff00000
        # length 256M
        # WIMG = 0101
        # covers ROM for data access
        lwi     r4,0xfff01fff
        mtdbatu 0,r4
        lwi     r4,0xfff0002a
        mtdbatl 0,r4

        # dbat1
        # base addr 0x00000000
        # length 256M
        # WIMG = 0000
        # covers DRAM and slots
        lwi     r4,0x00001fff
        mtdbatu 1,r4
        lwi     r4,0x00000002
        mtdbatl 1,r4
                
        # Enable translation in the MSR
        mfmsr   r3
        lwi     r4,0x00000030
        or      r3,r3,r4
        mtmsr   r3
#endif          
        blr

#------------------------------------------------------------------------------
# end of sim.S          
