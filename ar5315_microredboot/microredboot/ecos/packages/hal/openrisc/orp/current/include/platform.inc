##==========================================================================
##
##      platform.inc
##
##      OpenRISC Reference Platform (ORP) board-specific defines
##
##==========================================================================
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    sfurman
## Contributors: 
## Date:         2003-01-20
## Purpose:      ORP platform-specific init
## Description:  This file handles the post-reset hardware initialization
##               that is specific to the ORP platform (but not specific to
##               the OpenRISC processor itself).  So far, it only
##               initializes the memory controller so as to map Flash and
##               SDRAM into the memory space.
##
######DESCRIPTIONEND####
##
##==========================================================================

#ifndef _PLATFORM_INC_
#define _PLATFORM_INC_

#include <cyg/hal/mc.h>
#include CYGHWR_MEMORY_LAYOUT_H

/* Memory organization */
#define SDRAM_BASE_ADD  CYGMEM_REGION_ram
#define FLASH_BASE_ADD  CYGMEM_REGION_rom

/* Memory Controller's base address */
#define MC_BASE_ADD     0x93000000

/* Memory controller initialize magic values */
#define MC_CSR_VAL      0x0B000300
#define MC_MASK_VAL     0x000003f0
#define FLASH_TMS_VAL   0x00000103
#define SDRAM_TMS_VAL   0x19220057
#define FLASH_CSC_VAL   (((FLASH_BASE_ADD>>6) & 0x07ff0000) | 0x0025)
#define SDRAM_CSC_VAL   (((SDRAM_BASE_ADD>>6) & 0x07ff0000) | 0x0411)
                

        # Platform-specific, post-reset hardware initialization
        .macro  hal_hardware_init
init_mc:
        load32i r3,MC_BASE_ADD

        # Program Flash chip-select 
        load32i r5,FLASH_CSC_VAL
        l.sw    MC_CSC(0)(r3),r5

        # Init flash timing 
        load32i r5,FLASH_TMS_VAL
        l.sw    MC_TMS(0)(r3),r5

        # Start decoding memory addresses to generate chip-selects
        l.addi  r5,r0,MC_MASK_VAL
        l.sw    MC_BA_MASK(r3),r5

        load32i r5, MC_CSR_VAL
        l.sw    MC_CSR(r3),r5

        # Init DRAM timing 
        load32i r5, SDRAM_TMS_VAL
        l.sw    MC_TMS(1)(r3),r5

        # Program DRAM chip-select 
        load32i r5, SDRAM_CSC_VAL
        l.sw    MC_CSC(1)(r3),r5

        # Wait for SDRAM
        l.addi  r3,r0,0x1000
1:      l.sfeqi r3,0
        l.bnf   1b
        l.addi  r3,r3,-1
        .endm
        
#endif /* ifndef _PLATFORM_INC_ */

#undef CYGIMP_FORCE_INTERRUPT_HANDLING_CODE_IN_RAM

// EOF platform.inc
