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
// Contributors: hmt
// Date:         2001-02-12
// Purpose:      Intel SA1110/NanoEngine platform specific support routines
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
#include <cyg/hal/nano.h>               // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
// Tell the architectural HAL we might not be at the base of FLASH:
#define CYGHWR_HAL_ROM_VADDR 0x50000000

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

// ------------------------------------------------------------------------
// These macros are in lieu of any LEDs on the board; in ROM <=> POST
// startup on the nanoEngine, the serial out is initialized already, so you
// can just poke chars into it:

#if 0

#define CYGHWR_ASCII                            \
	and	r1,r1,#0xf;                     \
	cmps	r1,#10;                         \
	subge	r1,r1,#10;                      \
	addge	r1,r1,#65;                      \
	addlt	r1,r1,#48;
	
#define CYGHWR_LED_MACRO                        \
	ldr	r0,=SA11X0_UART1_DATA;          \
	mov	r1,#255&((\x));                 \
        mov     r1,r1, lsr #4;                  \
	CYGHWR_ASCII                            \
	str	r1,[r0];                        \
	mov	r1,#255&((\x));                 \
	CYGHWR_ASCII                            \
	str	r1,[r0];                        \
	mov	r1,#0x2A;                       \
	str	r1,[r0];                        \
        PAUSE

#define PAUSE                                   \
        ldr     r1,=0x8000;                     \
555:    sub     r1,r1,#1;                       \
        cmp     r1,#0;                          \
        bne     555b;        

#define OUT                                     \
	mov r2,r0;                              \
	mov r3,#8;                              \
	ldr r0,=SA11X0_UART1_DATA;              \
	mov r1,#'=';                            \
	str r1,[r0];                            \
        PAUSE                                   \
444:	mov r1,r2,lsr#28;                       \
	CYGHWR_ASCII                            \
	str r1,[r0];                            \
	mov r2,r2,asl#4;                        \
	subs r3,r3,#1;                          \
	bge 444b;                               \
        PAUSE                                   \
	mov r1,#' ';                            \
	str r1,[r0];                            \
        PAUSE                                   \

#endif // 0, to define CYGHWR_LED_MACRO et al

	
// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1

        // Turn off diagnostic LEDs
        LED 0x00

#ifdef CYGBLD_HAL_STARTUP_ROM_POST_BEFORE_ECOS
        // Then we must disable caches before starting initialization, else
        // the jump to hyperspace, um, high addresses after MM is enabled
        // will fail:
       	ldr	r1,=MMU_Control_Init
        mcr	p15,0,r1,c1,c0
        // and flush/trash all caches and their TLBs:
        mov     r0,#0
        mcr     p15,0,r0,c7,c5,0 // Icache
        mcr     p15,0,r0,c8,c5,0 // ITLB
        mcr     p15,0,r0,c7,c6,0 // Dcache
        mcr     p15,0,r0,c8,c6,0 // DTLB
        // at least a linesworth of nops.
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
#endif

        LED     0x01

        // Set up GPIO - they're all inputs initially.
	ldr	r0,=SA1110_GPIO_GRER_DEFAULT_VALUE
	ldr	r1,=SA11X0_GPIO_RISING_EDGE_DETECT
	str	r0,[r1]
	
	ldr	r0,=SA1110_GPIO_GFER_DEFAULT_VALUE
	ldr	r1,=SA11X0_GPIO_FALLING_EDGE_DETECT
	str	r0,[r1]

	ldr	r0,=SA1110_GPIO_GPOSR_DEFAULT_VALUE
	ldr	r1,=SA11X0_GPIO_PIN_OUTPUT_SET
	str	r0,[r1]
		
	ldr	r0,=SA1110_GPIO_GPOCR_DEFAULT_VALUE
	ldr	r1,=SA11X0_GPIO_PIN_OUTPUT_CLEAR
	str	r0,[r1]

	ldr	r0,=SA1110_GPIO_GAFR_DEFAULT_VALUE
	ldr	r1,=SA11X0_GPIO_ALTERNATE_FUNCTION
	str	r0,[r1]

	// The other 3 leds should extinguish at this point
        ldr     r0,=SA1110_GPIO_GPDR_DEFAULT_VALUE
        ldr     r1,=SA11X0_GPIO_PIN_DIRECTION
        str	r0,[r1]

	LED	0x10
	
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

	LED	0x40
	
#ifdef CYGBLD_HAL_STARTUP_ROM_POST_OMIT_SDRAM_INIT
	// If DRAM already enabled then skip its configuration.
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r2,[r1]
	ands	r2, r2, #0xf
        bne	20f
#endif // CYGBLD_HAL_STARTUP_ROM_POST_OMIT_SDRAM_INIT

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

	LED 0x70

        // Release DRAM hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_DRAM_CONTROL_HOLD
        str     r2,[r1]

	LED	0x80
	
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
        ldr     r2,=0x00007255 // read from nanoEngine
        str     r2,[r1]

        b       19f
        
// DRAM controller initialization        
dram_table:
        // Data extracted from the setup of the nanoEngine
        .word   SA11X0_DRAM0_CAS_0,           0xAAAAAA7F
        .word   SA11X0_DRAM0_CAS_1,           0xAAAAAAAA
        .word   SA11X0_DRAM0_CAS_2,           0xAAAAAAAA
        .word   SA11X0_STATIC_CONTROL_0,      0xfff9fffc
        .word   SA11X0_STATIC_CONTROL_1,      0xfff9fff9
        .word   SA11X0_EXP_BUS_CONFIGURATION, 0x00000000
        .word   SA11X0_REFRESH_CONFIGURATION, 0x303401f5
        .word   SA11X0_DRAM2_CAS_0,           0xAAAAAA7F // uninitialized, 
        .word   SA11X0_DRAM2_CAS_1,           0xAAAAAAAA //    apparently
        .word   SA11X0_DRAM2_CAS_2,           0xAAAAAAAA //    these 3
        .word   SA11X0_STATIC_CONTROL_2,      0xfffcfff8
        .word   SA11X0_SMROM_CONFIGURATION,   0xf070c040
        .word   SA11X0_DRAM_CONFIGURATION,    0x72547254        // Disabled
        .word   0, 0
19:

        // Release peripheral hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_PERIPHERAL_CONTROL_HOLD
        str     r2,[r1]

20:
	LED	0x90

        // Enable UART
        ldr     r1,=SA1110_GPCLK_CONTROL_0
        ldr     r2,=SA1110_GPCLK_SUS_UART
        str     r2,[r1]

        LED	0xA0

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

	LED	0xC0

        // Create MMU tables
        bl      hal_mmu_init
       
#if 0        
	LED	0xC1
        mrc     p15,0,r0,c1,c0,0
        OUT
	LED	0xC2
	mrc	p15,0,r0,c2,c0,0
        OUT
	LED	0xC3
	mrc	p15,0,r0,c3,c0,0
        OUT
        LED 	0xCC
        mov     r0,pc
        OUT
#endif

        LED     0xE0
	
        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	p15,0,r1,c1,c0
	mov	pc,r2    /* Change address spaces */
	nop
 	nop
	nop
10:
       
	LED	0xF0
	
        .endm
        
#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
