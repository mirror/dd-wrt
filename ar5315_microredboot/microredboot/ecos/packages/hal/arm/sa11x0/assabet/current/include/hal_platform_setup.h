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
// Date:         2000-05-08
// Purpose:      Intel SA1110/Assabet platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant (SA11x0) specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_sa11x0.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/assabet.h>            // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU

#if (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 59000)
#define SA11X0_PLL_CLOCK 0x0        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 73700)
#define SA11X0_PLL_CLOCK 0x1
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 88500)
#define SA11X0_PLL_CLOCK 0x2        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 103200) 
#define SA11X0_PLL_CLOCK 0x3        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 118000)
#define SA11X0_PLL_CLOCK 0x4        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 132700)
#define SA11X0_PLL_CLOCK 0x5        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 147500)
#define SA11X0_PLL_CLOCK 0x6        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 162200)
#define SA11X0_PLL_CLOCK 0x7        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 176900)
#define SA11X0_PLL_CLOCK 0x8        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 191700)
#define SA11X0_PLL_CLOCK 0x9        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 206400)
#define SA11X0_PLL_CLOCK 0xA        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 221200)
#define SA11X0_PLL_CLOCK 0xB        
#else
#error Invalid processor clock speed
#endif                
        
        
// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1

        // Turn off LEDs
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        orr     r2,r2,#SA1110_BCR_RED_LED
        orr     r2,r2,#SA1110_BCR_GREEN_LED
        str     r2,[r1]

        // Set up GPIO to read hardware configuration
        ldr     r1,=SA11X0_GPIO_PIN_DIRECTION
        ldr     r2,=0x0807A7FC
        str     r2,[r1]
        ldr     r3,=SA11X0_GPIO_PIN_OUTPUT_SET
        ldr     r2,=0x2FC               // Bits 2..9
        str     r2,[r3]                 // Requests hardware configuration
        ldr     r2,=0x0807A400          // Reconfigure 2..9 as inputs
        str     r2,[r1]
        ldr     r3,=SA11X0_GPIO_PIN_LEVEL
        mov     r2,#100
10:     ldr     r9,[r3]                 // Read multiple times to settle
        subs    r2,r2,#1
        bne     10b        
        ldr     r2,=0x0807A7FC          // Restore proper pin configuration
        str     r2,[r1]
        ldr     r3,=SA11X0_GPIO_PIN_OUTPUT_SET
        ldr     r2,=0x10000             // Enable high power mode
        str     r2,[r3]

        // Disable clock switching
        mcr     p15,0,r0,\
                SA11X0_TEST_CLOCK_AND_IDLE_REGISTER,\
                SA11X0_DISABLE_CLOCK_SWITCHING_RM,\
                SA11X0_DISABLE_CLOCK_SWITCHING_OPCODE

        // Set up processor clock
        ldr     r1,=SA11X0_PWR_MGR_PLL_CONFIG
        ldr     r2,=SA11X0_PLL_CLOCK
        str     r2,[r1]

        // Turn clock switching back on
        mcr     p15,0,r0,\
                SA11X0_TEST_CLOCK_AND_IDLE_REGISTER,\
                SA11X0_ENABLE_CLOCK_SWITCHING_RM,\
                SA11X0_ENABLE_CLOCK_SWITCHING_OPCODE
        nop
        nop

        // Turn on Red LED
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        bic     r2,r2,#SA1110_BCR_RED_LED
        orr     r2,r2,#SA1110_BCR_GREEN_LED
        str     r2,[r1]

        // Pause
        ldr     r1,=100000
10:     sub     r1,r1,#1
        cmp     r1,#0
        bne     10b        

        // Initialize DRAM controller
        ldr     r1,=dram_table
        ldr     r2,=__exception_handlers
        sub     r1,r1,r2
        ldr     r2,[r1],#4                      // First control register
10:     ldr     r3,[r1],#4
        str     r3,[r2]
        ldr     r2,[r1],#4                      // Next control register
        cmp     r2,#0
        bne     10b

        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        orr     r2,r2,#SA1110_BCR_RED_LED
        bic     r2,r2,#SA1110_BCR_GREEN_LED
        str     r2,[r1]

        // Enable UART
        ldr     r1,=SA1110_GPCLK_CONTROL_0
        ldr     r2,=SA1110_GPCLK_SUS_UART
        str     r2,[r1]

        // Release DRAM hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_DRAM_CONTROL_HOLD
        str     r2,[r1]

        // Perform 8 reads from unmapped/unenabled DRAM
        ldr     r1,=SA11X0_RAM_BANK0_BASE
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]

        // Enable DRAM controller
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r2,=0x72547255
        str     r2,[r1]

        b       19f
        
// DRAM controller initialization        
dram_table:
        .word   SA11X0_DRAM0_CAS_0,           0xAAAAAA7F
        .word   SA11X0_DRAM0_CAS_1,           0xAAAAAAAA
        .word   SA11X0_DRAM0_CAS_2,           0xAAAAAAAA
#ifdef CYGSEM_HAL_ARM_SA11X0_ASSABET_NEPONSET_TIMINGS
        .word   SA11X0_STATIC_CONTROL_0,      0x4B384370
        .word   SA11X0_STATIC_CONTROL_1,      0x22212419
#endif
        .word   SA11X0_EXP_BUS_CONFIGURATION, 0x994A994A
        .word   SA11X0_REFRESH_CONFIGURATION, 0x49FC0327
        .word   SA11X0_DRAM2_CAS_0,           0xAAAAAA7F
        .word   SA11X0_DRAM2_CAS_1,           0xAAAAAAAA
        .word   SA11X0_DRAM2_CAS_2,           0xAAAAAAAA
#ifdef CYGSEM_HAL_ARM_SA11X0_ASSABET_NEPONSET_TIMINGS
        .word   SA11X0_STATIC_CONTROL_2,      0x42196669
#endif
        .word   SA11X0_SMROM_CONFIGURATION,   0xAFCCAFCC
        .word   SA11X0_DRAM_CONFIGURATION,    0x72547254        // Disabled
        .word   0, 0
19:

        // Wakeup from deep sleep mode
        ldr     r1,=SA11X0_RESET_STATUS
        ldr     r2,[r1]
        cmp     r2,#SA11X0_SLEEP_MODE_RESET
        bne     45f
        ldr     r1,=SA11X0_PWR_MGR_SCRATCHPAD
        ldr     r1,[r1]
        mov     pc,r1
        nop
45:     nop

        // Release peripheral hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_PERIPHERAL_CONTROL_HOLD
        str     r2,[r1]

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init
       
        // Turn on green LED
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        orr     r2,r2,#SA1110_BCR_RED_LED
        bic     r2,r2,#SA1110_BCR_GREEN_LED
        str     r2,[r1]

        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r1,MMU_Control,c0
	mov	pc,r2    /* Change address spaces */
	nop
 	nop
	nop
10:
       
        // Turn on both LEDs
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        bic     r2,r2,#SA1110_BCR_RED_LED
        bic     r2,r2,#SA1110_BCR_GREEN_LED
        str     r2,[r1]

        // Save shadow copy of BCR, also hardware configuration
        ldr     r1,=_assabet_BCR
        str     r2,[r1]
        ldr     r1,=_assabet_CFG
        str     r9,[r1]                // Saved far above...
        .endm
        
#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  _assabet_BCR, _assabet_CFG
_assabet_BCR:   .long   0       // Board Control register shadow
_assabet_CFG:   .long   0       // Board Configuration (read at RESET)
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
