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
//               Travis C. Furrer <furrer@mit.edu>
// Date:         2000-05-08
// Purpose:      Intel SA1110/Assabet platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     This file should only be used by "vectors.S"        
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_sa11x0.h>         // Platform specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions

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

// This function is called very early on by the boot ROM (or by any ROM
// based startup).  It's job is to initialize the hardware to a known state
// so that eCos applications can execute properly.

// This version of the code is patterned after the contribution from
// Travis Furer (@MIT)                


// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.

#define CYGHWR_LED_MACRO _set_LEDS(\x)
        
// Initialize GPIOs
#define GPIO_GRER (SA11X0_GPIO_RISING_EDGE_DETECT-SA11X0_GPIO_PIN_LEVEL)
#define GPIO_GFER (SA11X0_GPIO_FALLING_EDGE_DETECT-SA11X0_GPIO_PIN_LEVEL)
#define GPIO_GAFR (SA11X0_GPIO_ALTERNATE_FUNCTION-SA11X0_GPIO_PIN_LEVEL)
#define GPIO_GEDR (SA11X0_GPIO_EDGE_DETECT_STATUS-SA11X0_GPIO_PIN_LEVEL)
#define GPIO_GPDR (SA11X0_GPIO_PIN_DIRECTION-SA11X0_GPIO_PIN_LEVEL)
#define GPIO_GPCR (SA11X0_GPIO_PIN_OUTPUT_CLEAR-SA11X0_GPIO_PIN_LEVEL)
        .macro  _init_GPIO
        ldr     r1,=SA11X0_GPIO_PIN_LEVEL
        mov     r0,#0           
        str     r0,[r1,#GPIO_GRER]      // Disable rising edge detects
        str     r0,[r1,#GPIO_GFER]      // Disable falling edge detects
        str     r0,[r1,#GPIO_GAFR]      // No alt. funcs. during init
        sub     r0,r0,#1        
        str     r0,[r1,#GPIO_GPCR]      // Force all outputs to low
        str     r0,[r1,#GPIO_GEDR]      // Clear edge detect status
        ldr     r0,=0x00100300  
        str     r0,[r1,#GPIO_GPDR]      // Only LEDs outputs (for now)
        .endm

// Initialize HEX display.
#define MCP_STAT  (SA11X0_MCP_STATUS-SA11X0_MCP_CONTROL_0)        
#define MCP_CTRL0 (SA11X0_MCP_CONTROL_0-SA11X0_MCP_CONTROL_0)        
#define MCP_DATA2 (SA11X0_MCP_DATA_2-SA11X0_MCP_CONTROL_0)        
        .macro  _init_HEX_DISPLAY
        ldr     r1, =SA11X0_MCP_CONTROL_0
        ldr     r2, =0xFFFFFFFF
        str     r2, [r1, #MCP_STAT]
        ldr     r2, =0x801F
        orr     r2, r2, #(3 << 16)
        str     r2, [r1, #MCP_DATA2]
        ldr     r2, =0x50000
        str     r2, [r1, #MCP_CTRL0]
        .endm

// Display value on hex LED display
        .macro  _set_LEDS,val
        mov     r0,#\val
        orr     r0,r0,#(1<<16)
        ldr     r1,=SA11X0_MCP_DATA_2
        str     r0,[r1]
        .endm

// Setup pin directions:
//  inputs: all serial receive pins
// outputs: all LCD pins, all serial transmit pins
        .macro  _init_PERIPHERAL_PINS
        ldr     r0,=0x00355FFF
        ldr     r1,=SA11X0_PPC_PIN_DIRECTION
        str     r0,[r1]
        mov     r0,#0                           // Force initial state
        ldr     r1,=SA11X0_PPC_PIN_STATE
        str     r0,[r1]
        ldr     r1,=SA11X0_PPC_PIN_ASSIGNMENT   // Disable any reassignments
        str     r0,[r1]
        .endm
        
// Set core frequency (this can take up to 150us)        
        .macro  _set_CLOCK_FREQUENCY
        mov     r0,#SA11X0_PLL_CLOCK
        ldr     r1,=SA11X0_PWR_MGR_PLL_CONFIG
        str     r0,[r1]
        .endm

// Enable clock switching (must be done after setting core frequency)
        .macro  _enable_CLOCK_SWITCHING
        mcr     p15,0,r1,c15,c1,2
        .endm

// Initialize memory interfaces. (ROM, SRAM, Flash, DRAM, etc)
//
// Note: This code has hardcoded values (taken from the uHAL
//       code for brutus provided at the Intel StrongARM
//       website) that are specific to the memory devices
//       used on the Brutus board.
//
// Brutus memory is as follows:
//   ROM     256K (assuming 32 bit accesses)
//   Flash   256K (assuming 32 bit accesses)
//   SRAM    512K
//   DRAM     16M (4M per bank)
//
// From SA11X0 Manual, Section 10.7.1:
//
//  The following flow should be followed when coming out of
//  reset, whether for sleep or power-up:
//
//    - Read boot ROM and write to memory configuration
//      registers, but do not enable DRAM banks.
//
//    - If necessary, finish any DRAM power-up wait period
//      (usually about 100us).
//
//    - If coming out of sleep, see Section 9.5, Power
//      Manager on page 9-26 on how to release the nCAS and
//      nRAS pins from their self-refresh state.
//
//    - If coming out of sleep, wait the DRAM-specific
//      post-self-refresh precharge period before issuing
//      a new DRAM transaction.
//
//    - If power-on reset, perform the number of
//      initialization refreshes required by the specific
//      DRAM part by reading disabled banks. A read from
//      any disabled bank will refresh all four banks.
//
//    - Enable DRAM banks by setting MDCNFG:DE3:0.
//
//
// FIXME: Values for Brutus SRAM (NOT yet supported) are:
//  MDCAS0 = 0xCCCCCCCF
//  MDCAS1 = 0xFFFFFFFC
//  MDCAS2 = 0xFFFFFFFF
//  MDCNFG = 0 //DRAM not enabled
//
//
// Configure Bank 0 (ROM) and Bank 1 (Flash) in MSC0 as
// follows:
//
// Bank 0:
//                ROM type = Nonburst ROM or Flash EPROM
//           ROM bus width = as specified on ROM_SEL pin (s11)
//  ROM delay first access = 17 MCLK cycles
//   ROM delay next access = 0 (unused)
//       ROM recovery time = 2 MCLK cycles
//
// Bank 1:
//                ROM type = Nonburst ROM or SRAM
//           ROM bus width = 32 bits
//  ROM delay first access = 17 MCLK cycles
//   ROM delay next access = 20 MCLK cycles
//       ROM recovery time = 1 MCLK cycles
//
//
// Configure Bank 2 (SRAM) and Bank 3 (External Register)
// in MSC1 as follows:
//
// Bank 2:
//                ROM type = Nonburst ROM or SRAM
//           ROM bus width = 32 bits
//  ROM delay first access = 4 MCLK cycles
//   ROM delay next access = 2 MCLK cycles
//       ROM recovery time = 2 MCLK cycles
//
// Bank 3:
//                ROM type = Nonburst ROM or SRAM
//           ROM bus width = 32 bits
//  ROM delay first access = 5 MCLK cycles
//   ROM delay next access = 3 MCLK cycles
//       ROM recovery time = 4 MCLK cycles
//
//
// Enable DRAM banks 0-3 and configure for:
//  12 row address bits (brutus has 12x8 DRAMs)
//  CAS waveform shifted on DCLK
//  RAS precharges for 4 MCLK cycles
//  RAS asserts for 6 MCLK cycles during CBR
//  Data latched 3 DCLK cycles after CAS
//  0x400 * 4 = 4096 MCLK cycles between refreshes
//
#define MDCFG  (SA11X0_DRAM_CONFIGURATION-SA11X0_DRAM_CONFIGURATION)        
#define MDCAS0 (SA11X0_DRAM0_CAS_0-SA11X0_DRAM_CONFIGURATION)        
#define MDCAS1 (SA11X0_DRAM0_CAS_1-SA11X0_DRAM_CONFIGURATION)        
#define MDCAS2 (SA11X0_DRAM0_CAS_2-SA11X0_DRAM_CONFIGURATION)        
#define MSCTL0 (SA11X0_STATIC_CONTROL_0-SA11X0_DRAM_CONFIGURATION)        
#define MSCTL1 (SA11X0_STATIC_CONTROL_1-SA11X0_DRAM_CONFIGURATION)        
        .macro  _init_MEM_INTERFACES
        /* Do nothing if DRAM already configured */
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r1,[r1]
        ands    r1,r1,#0xF
        bne     2003f
        /* Configure Memory */
        /* Configure Waveform Registers */
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r0,=0x83C1E01F
        str     r0,[r1,#MDCAS0]
        ldr     r0,=0x3C1E0F07
        str     r0,[r1,#MDCAS1]
        ldr     r0,=0xFFFFF078
        str     r0,[r1,#MDCAS2]
        /* Configure Static Memory Regs */
        /*      FIXME: I think this is right? See note on page 10-11. */
        /*             Value from uHAL was actually 13812080          */
        ldr     r0,=0x13802080
        ldr     r2,[r1,#MSCTL0]
        and     r2,r2,#4        /* Extract the ROM_SEL (s11) value */
        orr     r0,r0,r2        /* Merge in the ROM_SEL value */
        str     r0,[r1,#MSCTL0]
        /*      FIXME: I think this is right? See note on page 10-11. */
        /*             Value from uHAL was actually 42210119          */
        ldr     r0,=0x42200118
        str     r0,[r1,#MSCTL1]
        /* FIXME: According to the SA11X0 Manual, we need to force    */
        /*        a certain number of refreshes here by doing reads   */
        /*        to the disabled banks.  However, the uHAL code      */
        /*        doesn't seem to do this, so we skip it here too     */
        /*        for now.                                            */
        /* Configure DRAM Registers */
        ldr     r0,=0x0801A9BF
        str     r0,[r1,#MDCFG]
        /* Wait for DRAM to be ready for use (as in uHAL code) */
        /* FIXME: why do we need to do this? */
        mov     r0,#0x200
2002:   subs    r0,r0,#1
        bne     2002b
2003:
        .endm
        
        .macro  _platform_setup1
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
        nop                                                               
                                                                           
        _init_GPIO
        _init_HEX_DISPLAY               /* this is flaky sometimes */       
        _init_HEX_DISPLAY               /* so do it twice just in case */   
        _set_LEDS(15)
                                                                           
        _init_PERIPHERAL_PINS                                               
        _set_LEDS(13)
                                                                           
        _set_CLOCK_FREQUENCY                                                
        _enable_CLOCK_SWITCHING                                             
        _set_LEDS(12)
                                                                           
        _init_MEM_INTERFACES                                                
        _set_LEDS(11)
                                                                           
        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init

        _set_LEDS(9)
        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r1,MMU_Control,c0
	mov	pc,r2    /* Change address spaces */
	nop
 	nop
	nop
10:
        _set_LEDS(8)

        .endm
                        
#else // STARTUP_ROM        
#define PLATFORM_SETUP1
#endif

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
