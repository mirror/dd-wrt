#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H
/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-10-27
// Purpose:      ARM9/AAED2000 platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/aaed2000.h>           // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

// We need this here - can't rely on a translation table until MMU has
// been initialized
	.macro RAW_LED_MACRO x
        ldr     r0,=0x30000000
        ldr     r1,[r0]
        bic     r1,r1,#0xE0000000
        orr     r1,r1,#((0x7 & ~(\x))<<29)
        str     r1, [r0]
	.endm

// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1
        RAW_LED_MACRO 0
#ifndef CYG_HAL_STARTUP_RAM
        // Prevent all interrupts
        ldr    r0,=AAEC_INT_ENC
        mov    r1,#-1
        str    r1,[r0]

        // Disable and clear caches
        mrc  p15,0,r0,c1,c0,0
        bic  r0,r0,#0x1000              // disable ICache
        bic  r0,r0,#0x000f              // disable DCache, write buffer,
                                        // MMU and alignment faults
        mcr  p15,0,r0,c1,c0,0
        nop
        nop
        mov  r0,#0
        mcr  p15,0,r0,c7,c6,0           // clear data cache
#if 0
        mrc  p15,0,r0,c15,c1,0          // disable streaming
        orr  r0,r0,#0x80
        mcr  p15,0,r0,c15,c1,0
#endif

        // Initialize memory controllers

        // Static memory controller
        // Area0: Flash: 32bit wide
        ldr     r0,=AAEC_SMCBCR0
        ldr     r1,=(AAEC_SMCBCR_MW32 | AAEC_SMCBCR_WST(_CS0_WST) | AAEC_SMCBCR_IDCY(_CS0_IDCY))
        str     r1,[r0]
        // Area1: Ethernet: 16bit wide
        ldr     r0,=AAEC_SMCBCR1
        ldr     r1,=(AAEC_SMCBCR_MW16 | AAEC_SMCBCR_WST(_CS1_WST) | AAEC_SMCBCR_IDCY(_CS1_IDCY))
        str     r1,[r0]
        // Area3: GPIO: 32bit wide
        ldr     r0,=AAEC_SMCBCR3
        ldr     r1,=(AAEC_SMCBCR_MW32 | AAEC_SMCBCR_WST(_CS3_WST) | AAEC_SMCBCR_IDCY(_CS3_IDCY))
        str     r1,[r0]
        RAW_LED_MACRO 1

        // Set clock frequencies
        // First set CPU to synchronous mode
        mrc p15, 0, r0, c1, c0, 0
        // configure for synchronous mode: FCLK >= HCLK by integer
        // multiple
        orr r0, r0, #0x40000000
        // configure for FastBus mode - FCLK and HCLK *must* be equal
        // bic r0, r0, #0x40000000
        bic r0, r0, #0x80000000
        mcr p15, 0, r0, c1, c0, 0

        ldr    r0,=AAEC_CSC_CLKSET
        ldr    r1,=AAEC_CSC_CLKSET_INIT
        str    r1,[r0]
        // follow clock change by 5 NOPs
        nop;nop;nop;nop;nop


        // Synchronous memory controller (as per table 4-12)

        ldr     r0,=AAEC_SMC_DEV0
        ldr     r1,=AAEC_SMC_DEV_INIT
        str     r1,[r0]
        str     r1,[r0, #4]
        str     r1,[r0, #8]
        str     r1,[r0, #12]

        // step1: delay 100usecs
        ldr     r2,=AAEC_TMR_T1_BASE
        ldr     r3,=AAEC_TMR_TxCONTROL_508KHZ_uS(100)
        str     r3,[r2, #AAEC_TMR_TxLOAD_OFFSET]
        ldr     r3,=(AAEC_TMR_TxCONTROL_ENABLE|AAEC_TMR_TxCONTROL_MODE_FREE|AAEC_TMR_TxCONTROL_508KHZ)
        str     r3,[r2, #AAEC_TMR_TxCONTROL_OFFSET]
1:      ldr     r3,[r2, #AAEC_TMR_TxVALUE_OFFSET]
        cmp     r3,#0
        bne     1b
        str     r3,[r2, #AAEC_TMR_TxCONTROL_OFFSET]

        // step2: issue NOP command
        ldr     r0,=AAEC_SMC_GLOBAL
        ldr     r1,=AAEC_SMC_GLOBAL_CMD_NOP
        str     r1,[r0]

        // step3: wait 200usecs
        ldr     r3,=AAEC_TMR_TxCONTROL_508KHZ_uS(200)
        str     r3,[r2, #AAEC_TMR_TxLOAD_OFFSET]
        ldr     r3,=(AAEC_TMR_TxCONTROL_ENABLE|AAEC_TMR_TxCONTROL_MODE_FREE|AAEC_TMR_TxCONTROL_508KHZ)
        str     r3,[r2, #AAEC_TMR_TxCONTROL_OFFSET]
1:      ldr     r3,[r2, #AAEC_TMR_TxVALUE_OFFSET]
        cmp     r3,#0
        bne     1b
        str     r3,[r2, #AAEC_TMR_TxCONTROL_OFFSET]

        // step4: PreCharge All
        ldr     r1,=AAEC_SMC_GLOBAL_CMD_PREALL
        str     r1,[r0]

        // step5: set refresh time to 10
        ldr     r3,=AAEC_SMC_REFRESH_TIME
        mov     r4,#10
        str     r4,[r3]

        // step6: wait 80 clock cycles, allowing 8 refresh cycles for SDRAM
        mov     r3, #100
1:      subs    r3, r3, #1
        bne     1b

        // step7: set normal refresh count
        // We need to do a refresh every 15.6usecs. The counter runs
        // at bus clock, so the delay is (15.6usecs*bus speed) or
        // (156*(bus speed/10)/1000000).
        ldr     r3,=AAEC_SMC_REFRESH_TIME
        ldr     r4,=(156*(CYGNUM_HAL_ARM_AAED2000_BUS_CLOCK/10)/1000000)
        str     r4,[r3]

        // step8: set mode
        ldr     r1,=AAEC_SMC_GLOBAL_CMD_MODE
        str     r1,[r0]
        
        // step9: program mode
        // from page 36: SDRAM, WBL=0, TM=0, CAS=3, Sequential, BL=4
        ldr     r3,=0xf000c800
        ldr     r3,[r3]

        // step10: enable SDRAM
        // step8: set mode
        ldr     r1,=AAEC_SMC_GLOBAL_CMD_ENABLE
        str     r1,[r0]
        RAW_LED_MACRO 2
#endif
#ifdef CYG_HAL_STARTUP_ROMRAM
        // Compute [logical] base address of this image in ROM
        bl      10f
10:     mov     r9,lr
        ldr     r8,=~0xFF01FFFF         // Bits to ignore
        and     r9,r9,r8
        orr     r9,r9,#0x60000000       // Turn into ROM address
#endif        

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=AAED2000_SDRAM_PHYS_BASE
        orr     sp,r1,r2

        // Create MMU tables
	RAW_LED_MACRO 4
        bl      hal_mmu_init
	RAW_LED_MACRO 5

        // Enable MMU
        ldr     r2,=10f
#ifdef CYG_HAL_STARTUP_ROMRAM
        ldr     r1,=__exception_handlers
        sub     r1,r2,r1
        add     r2,r9,r1        // r9 has ROM offset
#endif        
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r1,MMU_Control,c0
	mov	pc,r2    /* Change address spaces */
	nop
 	nop
	nop
10:
        RAW_LED_MACRO 6
        
#ifdef CYG_HAL_STARTUP_ROMRAM
        mov     r0,r9                     // Relocate FLASH/ROM to RAM
        ldr     r1,=__exception_handlers  // ram base & length
        ldr     r2,=__rom_data_end
20:     ldr     r3,[r0],#4
        str     r3,[r1],#4
        cmp     r1,r2
        bne     20b
        ldr     r0,=30f
        mov     pc,r0
        nop
        nop
        nop
        nop
30:             
#endif
        RAW_LED_MACRO 7
        .endm
        
#else // defined(CYG_HAL_STARTUP_RAM)
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
