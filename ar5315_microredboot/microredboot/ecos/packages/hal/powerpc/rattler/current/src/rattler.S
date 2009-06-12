#==============================================================================
##
##      rattler.S
##
##      MPC8250 RATTLER board hardware setup
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
## Copyright (C) 2003 Gary Thomas
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
## Author(s):    nickg
## Contributors: wpd, gthomas
## Date:         2003-06-30
## Purpose:      RATTLER MPC8250 board hardware setup
## Description:  This file contains any code needed to initialize the
##               hardware on the Analogue & Micro Rattler (MPC8250) board.
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>
        
#include <cyg/hal/arch.inc>
#include <cyg/hal/ppc_regs.h>		/* on-chip resource layout, special */
#------------------------------------------------------------------------------

        .globl  hal_hardware_init
hal_hardware_init:
#if defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_ROM)
   #----------------------------------------------
   # Load the IMMR register with the base address
   #----------------------------------------------

   addis    r30,0,0xFF01   # IMMR base addr = 0xFF000000+10000. We add 
                          # 0x10000 because using relative addressing
                          # in load and store instructions only allow a
                          # offset from the base of +/-32767.
        
        // Setup for LEDS, using PA[0..2]
        lwi     r3,0xE0000000
        stw     r3,CYGARC_REG_IMM_PDIRA(r30)
        li      r3,0    // All LEDs on
        stw     r3,CYGARC_REG_IMM_PDATA(r30)

        // Force BR0 to map everything
        lwi     r3,0x00000E76
        stw     r3,CYGARC_REG_IMM_OR0(r30)
	
// Need to set the PC into the FLASH (ROM) before the address map changes
	lwi	r3,10f
        lwi     r5,CYGMEM_REGION_rom
	or	r3,r3,r5
	mtctr	r3
	bctr
10:	

        // Running in FLASH address space
        lwi     r3,0xC0000000
        stw     r3,CYGARC_REG_IMM_PDATA(r30)

   #*******************************************
   # Main System Clock Configuration Registers 
   #*******************************************


// SC  SIUMCR           fF010000  0E24c000 SIU              
// SC  SYPCR            fF010004  FFFFFFC3 SIU              
// SC  SWSR             fF01000E  0000     SIU              
        
        lwi     r3,0xFFFFFFC3
        stw     r3,CYGARC_REG_IMM_SYPCR(r30)
        lwz     r3,CYGARC_REG_IMM_SCCR(r30)
        ori     r3,r3,0x00000001        // Force baud rate clock divisor = 16
        stw     r3,CYGARC_REG_IMM_SCCR(r30)
#ifdef CYGHWR_HAL_POWERPC_RATTLER_PCI        
        lwz     r3,CYGARC_REG_IMM_SIUMCR(r30)
        lwi     r4,0x0E040000           // Set configuration bits
        or      r3,r3,r4
#else
        lwi     r3,0x0E24C000           // Set configuration bits
#endif                
        stw     r3,CYGARC_REG_IMM_SIUMCR(r30)
        li      r3,0x0000
        sth     r3,CYGARC_REG_IMM_SWSR(r30)

// SC  BCR              fF010024  00000000 SIU              
// SC  PPC_ACR          fF010028  02       SIU              
// SC  PPC_ALRH         fF01002C  01234567 SIU              
// SC  PPC_ALRL         fF010030  89ABCDEF SIU              
// SC  LCL_ACR          fF010034  02       SIU              
// SC  LCL_ALRH         fF010038  01234567 SIU              
// SC  LCL_ALRL         fF01003C  89ABCDEF SIU              
// SC  TESCR1           fF010040  00000000 SIU              
// SC  TESCR2           fF010044  00000000 SIU              
// SC  LTESCR1          fF010048  00000000 SIU              
// SC  LTESCR2          fF01004C  00000000 SIU              
// SC  PDTEA            fF010050  00000000 SIU              
// SC  PDTEM            fF010054  00       SIU              
// SC  LDTEA            fF010058  00000000 SIU              
// SC  LDTEM            fF01005C  00       SIU  
 
        li      r3,0x00000000
        stw     r3,CYGARC_REG_IMM_BCR(r30)
        li      r3,0x02
        stb     r3,CYGARC_REG_IMM_PPC_ACR(r30)
        lwi     r3,0x01234567
        stw     r3,CYGARC_REG_IMM_PPC_ALRH(r30)
        lwi     r3,0x89ABCDEF
        stw     r3,CYGARC_REG_IMM_PPC_ALRL(r30)
        li      r3,0x02
        stb     r3,CYGARC_REG_IMM_LCL_ACR(r30)
        lwi     r3,0x01234567
        stw     r3,CYGARC_REG_IMM_LCL_ALRH(r30)
        lwi     r3,0x89ABCDEF
        stw     r3,CYGARC_REG_IMM_LCL_ALRL(r30)
        li      r3,0x00000000
        stw     r3,CYGARC_REG_IMM_TESCR1(r30)
        li      r3,0x00000000
        stw     r3,CYGARC_REG_IMM_TESCR2(r30)
        li      r3,0x00000000
        stw     r3,CYGARC_REG_IMM_PDTEA(r30)
        li      r3,0x00
        stw     r3,CYGARC_REG_IMM_PDTEM(r30)
        li      r3,0x00000000
        stw     r3,CYGARC_REG_IMM_LDTEA(r30)
        li      r3,0x00
        stw     r3,CYGARC_REG_IMM_LDTEM(r30)


   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   # Initialize the memory controller and SDRAM
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 
        // Memory layout

        // CS0 - 8MB FLASH
        lwi     r3,0xFE001001
        stw     r3,CYGARC_REG_IMM_BR0(r30)
        lwi     r3,0xFF800E76
        stw     r3,CYGARC_REG_IMM_OR0(r30)

        // CS1 - 16MB DRAM
        lwi     r3,0xFF003080
        stw     r3,CYGARC_REG_IMM_OR1(r30)
        lwi     r3,0x00000041        
        stw     r3,CYGARC_REG_IMM_BR1(r30)
 
        // CS4 - 1MB external
        lwi     r3,0xFFF00000
        stw     r3,CYGARC_REG_IMM_OR4(r30)
        lwi     r3,0x80001801
        stw     r3,CYGARC_REG_IMM_BR4(r30)

        // SDRAM controller
        li      r5,0
        li      r6,0xFF
        lwi     r3,0x2000
        sth     r3,CYGARC_REG_IMM_MPTPR(r30)
        li      r3,0x1F
        stb     r3,CYGARC_REG_IMM_PSRT(r30)
#define SDRAM_MUX_SETUP 0x0269A452
        lwi     r3,0xE8000000|SDRAM_MUX_SETUP
        stw     r3,CYGARC_REG_IMM_PSDMR(r30)
        stb     r6,0(r5)
        lwi     r3,0xC8000000|SDRAM_MUX_SETUP
        stw     r3,CYGARC_REG_IMM_PSDMR(r30)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        stb     r6,0(r5)
        lwi     r3,0xD8000000|SDRAM_MUX_SETUP
        stw     r3,CYGARC_REG_IMM_PSDMR(r30)
        stb     r6,0(r5)
        lwi     r3,0xC0000000|SDRAM_MUX_SETUP
        stw     r3,CYGARC_REG_IMM_PSDMR(r30)

        // Registers programmed, starting up DRAM
        lwi     r3,0xA0000000
        stw     r3,CYGARC_REG_IMM_PDATA(r30)
        
        lwi     r3,0x10000
        mtctr   r3
10:     nop     
        bdnz    10b        

        // Ready to return
        lwi     r3,0x80000000
        stw     r3,CYGARC_REG_IMM_PDATA(r30)

#ifdef CYG_HAL_STARTUP_ROM
	# move return address to where the ROM is
	mflr	r3
        lwi     r4,0x00FFFFFF        // CAUTION!! Assumes only low 16M for ROM
        and     r3,r3,r4
	oris	r3,r3,CYGMEM_REGION_rom>>16
	mtlr	r3
#endif

#ifdef CYG_HAL_STARTUP_ROMRAM
        // Copy image from ROM to RAM
        mflr    r3              
        lwi     r4,(CYGMEM_REGION_rom&0xFE000000)
        lwi     r5,0x01FFFFFF   // ROM/FLASH base
        and     r3,r3,r5        // segment relative
        lwi     r6,_hal_hardware_init_done
        mtlr    r6
        sub     r6,r3,r6        // Absolute address
        add     r6,r6,r4        // FLASH address
        lwi     r7,0            // where to copy to
        lwi     r8,__ram_data_end
10:     lwz     r5,0(r6)
        stw     r5,0(r7)
        addi    r6,r6,4
        addi    r7,r7,4
        cmplw   r7,r8
        bne     10b
#endif                

        // Ready to return
        lwi     r3,0xE0000000
        stw     r3,CYGARC_REG_IMM_PDATA(r30)

#endif // ROM/ROMRAM
                
hardware_init_done:
        blr

#------------------------------------------------------------------------------
# end of rattler.S
