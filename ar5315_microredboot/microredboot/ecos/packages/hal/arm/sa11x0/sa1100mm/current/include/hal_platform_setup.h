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
// Contributors: gthomas, dmoseley
//               Travis C. Furrer <furrer@mit.edu>
// Date:         2000-05-08
// Purpose:      Intel SA1100 Multimedia platform specific support routines
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

#define CYGHWR_LED_MACRO _set_LEDS \x
        
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

#define DISCRETE_LED_REG_BASE     0x18800000
#define KEYPAD_IO_O               0x4
#define DISCRETE_LED_O            0x6
#define HEX_LED_REG_BASE          0x18c00000
#define HEX_LED_O                 0x0
#define HEX_DATA_MASK             0x0F
#define HEX_LED_0_STROBE          0x10
#define HEX_LED_1_STROBE          0x20
#define HEX_LED_BOTH_STROBES      (HEX_LED_0_STROBE | HEX_LED_1_STROBE)

// Display value on hex LED display
        .macro  _set_LEDS,val
        ldr     r1, =HEX_LED_REG_BASE

        ldr     r2, =\val
        and     r2, r2, #0xf
        orr	r0, r2, #HEX_LED_0_STROBE
        str     r0, [r1, #HEX_LED_O]

        ldr     r2, =\val
        mov     r2, r2, LSR #4
        orr	r0, r2, #HEX_LED_1_STROBE
        str     r0, [r1, #HEX_LED_O]
        
        nop
        .endm

// Initialize HEX display.
        .macro  _init_HEX_DISPLAY
        ldr     r1, =DISCRETE_LED_REG_BASE
        ldr     r2, =~0
        str     r2, [r1, #KEYPAD_IO_O]
        str     r2, [r1, #DISCRETE_LED_O]
        _set_LEDS 0x23
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
// SA1100 Multimedia memory is as follows:
//   ROM     128K (assuming 32 bit accesses)
//   Flash   4M (assuming 32 bit accesses)
//   RAM     8M
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
#define DRAM_CONFIG_VALUE      (SA11X0_DRAM_REFRESH_INTERVAL(312)              | \
                                SA11X0_DATA_INPUT_LATCH_CAS_PLUS_THREE         | \
                                SA11X0_DRAM_CAS_BEFORE_RAS(5)                  | \
                                SA11X0_DRAM_RAS_PRECHARGE(4)                   | \
                                SA11X0_DRAM_CLOCK_CPU_CLOCK                    | \
                                SA11X0_DRAM_ROW_ADDRESS_BITS_11                | \
                                SA11X0_DRAM_BANK_0_ENABLED                     | \
                                SA11X0_DRAM_BANK_1_DISABLED                    | \
                                SA11X0_DRAM_BANK_2_DISABLED                    | \
                                SA11X0_DRAM_BANK_3_DISABLED)

#define DRAM_CAS0_WAVEFORM     0xF0F0F00F
#define DRAM_CAS1_WAVEFORM     0XF0F0F0F0
#define DRAM_CAS2_WAVEFORM     0xFFFFFFF0

#define BANK_0_CONTROL_VALUE   SA11X0_STATIC_ROM_TYPE_FLASH                    | \
                               SA11X0_STATIC_ROM_BUS_WIDTH_16_BITS             | \
                               SA11X0_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA11X0_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA11X0_STATIC_ROM_RECOVERY(0x7)
#define BANK_1_CONTROL_VALUE   SA11X0_STATIC_ROM_TYPE_FLASH                    | \
                               SA11X0_STATIC_ROM_BUS_WIDTH_32_BITS             | \
                               SA11X0_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA11X0_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA11X0_STATIC_ROM_RECOVERY(0x7)
#define STATIC_CONTROL_0_VALUE (SA11X0_STATIC_ROM_BANK_0(BANK_0_CONTROL_VALUE) | \
                                SA11X0_STATIC_ROM_BANK_1(BANK_1_CONTROL_VALUE))

#define BANK_2_CONTROL_VALUE   SA11X0_STATIC_ROM_TYPE_FLASH                    | \
                               SA11X0_STATIC_ROM_BUS_WIDTH_16_BITS             | \
                               SA11X0_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA11X0_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA11X0_STATIC_ROM_RECOVERY(0x7)
#define BANK_3_CONTROL_VALUE   SA11X0_STATIC_ROM_TYPE_FLASH                    | \
                               SA11X0_STATIC_ROM_BUS_WIDTH_32_BITS             | \
                               SA11X0_STATIC_ROM_DELAY_FIRST_ACCESS(0x1F)      | \
                               SA11X0_STATIC_ROM_DELAY_NEXT_ACCESS(0x1F)       | \
                               SA11X0_STATIC_ROM_RECOVERY(0x7)
#define STATIC_CONTROL_1_VALUE (SA11X0_STATIC_ROM_BANK_2(BANK_2_CONTROL_VALUE) | \
                                SA11X0_STATIC_ROM_BANK_3(BANK_3_CONTROL_VALUE))

        .macro  _init_MEM_INTERFACES
        /*
         * Initialize the DRAM Controller
         */
        ldr     r0, =SA11X0_DRAM_CONFIGURATION
        ldr     r1, =DRAM_CONFIG_VALUE
        str     r1, [r0]
                
        ldr     r0, =SA11X0_DRAM0_CAS_0
        ldr     r1, =DRAM_CAS0_WAVEFORM
        str     r1, [r0]
                
        ldr     r0, =SA11X0_DRAM0_CAS_1
        ldr     r1, =DRAM_CAS1_WAVEFORM
        str     r1, [r0]
                
        ldr     r0, =SA11X0_DRAM0_CAS_2
        ldr     r1, =DRAM_CAS2_WAVEFORM
        str     r1, [r0]
                
        ldr     r0, =SA11X0_STATIC_CONTROL_0

        /*
         * Get the reset ROM setup
         */
        ldr     r1, [r0]

        /*
         * Get the 16/32 bit setting to merge into the appropriate
         * register values later on.
         */
        and     r1, r1, #SA11X0_STATIC_ROM_BUS_WIDTH_MASK

        /*
         * MSC0 - bank 0 ROM, bank 1 FLASH
         */
        ldr     r2, =STATIC_CONTROL_0_VALUE
        orr     r1, r1, r2

        str     r1, [r0]

        /*
         * MSC1 - bank 2 SRAM, bank 3 REG
         */
        ldr     r0, =SA11X0_STATIC_CONTROL_1
        ldr     r1, =STATIC_CONTROL_1_VALUE
        str     r1, [r0]

        /*
         * Delay to let the DRAM warm up
         */
        ldr     r0, =0x200
0:      subs    r0, r0, #1
        bne     0b

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

        _set_LEDS 0x15

        _init_PERIPHERAL_PINS                                               
        _set_LEDS 0x13
                                                                           
        _set_CLOCK_FREQUENCY                                                
        _enable_CLOCK_SWITCHING                                             
        _set_LEDS 0x12
                                                                           
        _init_MEM_INTERFACES                                                
        _set_LEDS 0x11
                                                                           
        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init

        _set_LEDS 0x09
        // Enable MMU
        ldr     r2,=10f
        ldr     r1,=MMU_Control_Init|MMU_Control_M
        mcr     MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2    /* Change address spaces */

        nop
        nop
        nop
10:
        _set_LEDS 0x08

        .endm
                        
#else // STARTUP_ROM        
#define PLATFORM_SETUP1
#endif

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
