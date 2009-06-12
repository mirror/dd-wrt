##=============================================================================
##
##      moab.S
##
##      MOAB board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2002, 2003, 2004 Gary Thomas
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
## Author(s):   gthomas
## Contributors:hmt
## Date:        2002-07-22
## Purpose:     MOAB board hardware setup
## Description: This file contains any code needed to initialize the
##              hardware on a TAMS MOAB (PowerPC 405GPr) board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/arch.inc>		/* register symbols et al */
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS        
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */

#------------------------------------------------------------------------------

// No useable LEDs
#define LED(x)

//
// Various setup values - based on board revision &/or xtal speed
//                        
#if defined(CYG_HAL_MOAB_BOARD_REVISION_1_0)
#define MOAB_DCR_SDRAM0_RTR    0x05F00000
#define MOAB_DCR_SDRAM0_B1CR   0x00062001                
#define MOAB_DCR_SDRAM0_B0CR   0x02062001
#define MOAB_DCR_SDRAM0_CFG    0x80800000
#elif defined(CYG_HAL_MOAB_BOARD_REVISION_1_1)
#define MOAB_DCR_SDRAM0_RTR    0x07F00000
#define MOAB_DCR_SDRAM0_B1CR   0x00084001                        
#define MOAB_DCR_SDRAM0_B0CR   0x00000000
#define MOAB_DCR_SDRAM0_CFG    0x80800000
#else
#error "Illegal board revision"
#endif                        
#if CYGHWR_HAL_POWERPC_CPU_SPEED == 250        
#define MOAB_DCR_CPC0_CR0      0x09F8502A	
#define MOAB_DCR_EBC0_B1AP     0x03840000           
#elif CYGHWR_HAL_POWERPC_CPU_SPEED == 333
#define MOAB_DCR_CPC0_CR0      0x09F8503A
#define MOAB_DCR_EBC0_B1AP     0x05840000           
#elif CYGHWR_HAL_POWERPC_CPU_SPEED == 300
#define MOAB_DCR_CPC0_CR0      0x0FF85034        
#define MOAB_DCR_EBC0_B1AP     0x0A840000           
#elif CYGHWR_HAL_POWERPC_CPU_SPEED == 400      
#define MOAB_DCR_CPC0_CR0      0x0FF8503C
#define MOAB_DCR_EBC0_B1AP     0x0C840000           
#else
#error "Illegal CPU speed"
#endif                        
#------------------------------------------------------------------------------       
                
FUNC_START( hal_hardware_init )
        mflr    r30                        // Save return address

#ifndef CYG_HAL_STARTUP_RAM
        
        li      r3,0x00000000              // Make sure nothing is cacheable
        mticcr  r3
        mtdccr  r3
        // Force data caches to be totally clean
        lwi     r3,0
        lwi     r4,0x8000
10:     dcbf    0,r3        
        dccci   0,r3
        addi    r3,r3,16
        cmpw    r3,r4
        bne     10b

        // Basic chip configuration
        lwi     r3,MOAB_DCR_CPC0_CR0
        mtdcr   DCR_CPC0_CR0,r3
        li      r3,0x00000000
        mtdcr   DCR_CPC0_CR1,r3

        lwi     r3,0x60606000            // Edge conditioning register
        mtdcr   DCR_CPC0_ECR,r3

        // GPIO
        //   GPIO 13 - I - media present
        //   GPIO 14 - O - main flash ALE
        //   GPIO 15 - O - main flash CLE
        //   GPIO 16 - I - watchdog ?
        //   GPIO 17 - O - NAND CE
        //   GPIO 24 - O - heartbeat LED
        lwi     r4,GPIO_OR              // Leave NAND not-selected, heartbeat off
        lwi     r3,0x00004080
        stw     r3,0(r4)
        lwi     r4,GPIO_TCR             // Tri-state control - enables outputs
        lwi     r3,0x00034080
        stw     r3,0(r4)
        
        li      r3,DCR_SDRAM0_CFG          // See if SDRAM already configured
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mfdcr   r4,DCR_SDRAM0_CFGDATA
        lwi     r3,MOAB_DCR_SDRAM0_CFG        
        and     r4,r4,r3
        cmpw    r3,r4
        beq     sdram_ok
        
        // DRAM controller
        li      r3,DCR_SDRAM0_CFG            // Turn off controller to allow changes
        lwi     r4,0x00000000
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4
        li      r3,DCR_SDRAM0_TR
        lwi     r4,0x010A800E
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4
        li      r3,DCR_SDRAM0_RTR            // Refresh timing
        lwi     r4,MOAB_DCR_SDRAM0_RTR  
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4
        li      r3,DCR_SDRAM0_B1CR           // Note: non-ascending addresses because
                                             // the low 1/2 of memory fails on some boards        
        lwi     r4,MOAB_DCR_SDRAM0_B1CR
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4
        li      r3,DCR_SDRAM0_B0CR
        lwi     r4,MOAB_DCR_SDRAM0_B0CR
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4
        
        lwi     r4,GPIO_OR                // Turn heartbeat LED on
        lwi     r3,0x00004000
        stw     r3,0(r4)

        lwi     r3,0xA000                 // Pause for at least 200us
        mtctr   r3
13:     nop
        bdnz    13b
        
        li      r3,DCR_SDRAM0_CFG         // Enable controller
        lwi     r4,MOAB_DCR_SDRAM0_CFG        
        mtdcr   DCR_SDRAM0_CFGADDR,r3        
        mtdcr   DCR_SDRAM0_CFGDATA,r4

sdram_ok:               
        // Bus controller
        li      r3,DCR_EBC0_B0AP          // BOOT FLASH at 0xFFExxxxx, R/W
        lwi     r4,0x7F8FFE80
        mtdcr   DCR_EBC0_CFGADDR,r3
        mtdcr   DCR_EBC0_CFGDATA,r4
        li      r3,DCR_EBC0_B0CR
        lwi     r4,0xFFE38000
        mtdcr   DCR_EBC0_CFGADDR,r3
        mtdcr   DCR_EBC0_CFGDATA,r4
        li      r3,DCR_EBC0_B1AP          // MAIN FLASH at 0xCxxxxxxx, R/W
        lwi     r4,MOAB_DCR_EBC0_B1AP
        mtdcr   DCR_EBC0_CFGADDR,r3
        mtdcr   DCR_EBC0_CFGDATA,r4
        li      r3,DCR_EBC0_B1CR
        lwi     r4,0xC0018000             // 1MB, 8bits [actually only 1 byte!]
        mtdcr   DCR_EBC0_CFGADDR,r3
        mtdcr   DCR_EBC0_CFGDATA,r4

        // On-chip memory
        lwi     r4,0xD0000000            // Instruction/Data at 0xD0XXXXXX..0xD7XXXXXX
        lwi     r3,0x80000000
        mtdcr   DCR_OCM0_ISARC,r4
        mtdcr   DCR_OCM0_DSARC,r4
        mtdcr   DCR_OCM0_ISCNTL,r3
        mtdcr   DCR_OCM0_DSCNTL,r3
        
        lwi     r4,GPIO_OR              // Turn heartbeat LED off
        lwi     r3,0x00004080
        stw     r3,0(r4)
        
#endif  // ROM or ROMRAM startup
        
        lwi     r3,0x80000001              // DRAM can be cached - instructions only
        iccci   0,r3
        mticcr  r3
        lwi     r3,0x00000000
        mtdccr  r3

#ifdef CYG_HAL_STARTUP_ROMRAM
        // Copy image from ROM to RAM
        mr      r6,r30
        lwi     r7,0xFFF80000
        and     r6,r6,r7
        subi    r6,r6,4
        lwi     r7,0-4            // where to copy to
        lwi     r8,__ram_data_end
10:     lwzu    r5,4(r6)
        stwu    r5,4(r7)
        cmplw   r7,r8
        bne     10b
        lwi     r30,_hal_hardware_init_done
#endif
        
        lwi     r4,GPIO_OR              // Turn heartbeat LED on
        lwi     r3,0x00004000
        stw     r3,0(r4)

        mtlr    r30             // Restore return address
	blr
FUNC_END( hal_hardware_init )

        .globl  _hang
_hang:   nop
        b       _hang
        blr

#if 0        
        .text
        .globl  _get_cache_contents
_get_cache_contents:
        subi    r3,r3,4
        li      r4,256
        mtctr   r4
        li      r4,0        // Address
        lwi     r7,0xFFFFFFE0
        mfspr   r5,SPR_CCR0
10:     and     r5,r5,r7        // CIS=0,CWS=0 =>  DATA, WAY A
        mtspr   SPR_CCR0,r5
        sync
        isync
        dcread  r6,0,r4
        stwu    r6,4(r3)
        and     r5,r5,r7        // CIS=0,CWS=1 =>  DATA, WAY B
        ori     r5,r5,0x01
        mtspr   SPR_CCR0,r5
        sync
        isync
        mfspr   r5,SPR_CCR0
        dcread  r6,0,r4
        stwu    r6,4(r3)
        and     r5,r5,r7        // CIS=1,CWS=0 =>  TAG, WAY A
        ori     r5,r5,0x10
        mtspr   SPR_CCR0,r5
        sync
        isync
        dcread  r6,0,r4
        stwu    r6,4(r3)
        and     r5,r5,r7        // CIS=1,CWS=1 =>  TAG, WAY B
        ori     r5,r5,0x11
        mtspr   SPR_CCR0,r5
        sync
        isync
        dcread  r6,0,r4
        stwu    r6,4(r3)
        addi    r4,r4,32
        bdnz    10b
        blr
#endif        
        
#ifndef CYG_HAL_STARTUP_RAM
        .section        ".reset_vector","ax"
        ba      0xFFF80100
        .previous
#endif

#------------------------------------------------------------------------------
# end of moab.S
