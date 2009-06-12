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
// Author(s):    Jordi Colomer <jco@ict.es>
// Contributors: Jordi Colomer <jco@ict.es>
// Date:         2001-06-15
// Purpose:      SA1110/Flexanet platform specific support routines
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
#include <cyg/hal/flexanet.h>            // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

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
        
// DRAM settings

// Banks configured for 256-Mbit SDRAM devices (15 row x 9 col)
// Initially disabled.
// CAS latency = 3, 32-bit bus width, TWR = 1, TRP = 2
#define DRAM_CONFIG_VALUE	0x72647264

// SMROM settings (disabled in all banks)
#define SMROM_CONFIG_VALUE	0xAFCCAFCC	

// CAS waveforms
//#define CAS0_WAVEFORM_VALUE	0xAAAAAA7F
#define CAS0_WAVEFORM_VALUE	0xAAAAAAA7
#define CAS1_WAVEFORM_VALUE	0xAAAAAAAA
#define CAS2_WAVEFORM_VALUE	0xAAAAAAAA

// Expansion memory (PCMCIA) settings (MECR)
#define EXPMEM_CONFIG_VALUE	0x994A994A

// DRAM refresh configuration

// CAS before RAS = 7
// f(SDCLK) = f(mem) = 1/2 f(cpu) = 103 MHz, ok for PC-133 SDRAMs
// If PC-100 are installed, may be better to run the CPU at 192 MHz
// and still clock the SDRAM at 1/2 f(cpu) = 96 MHz.
// No auto-power-down,
// Refresh period for 8192 rows = 64 ms (25 memory cycles)
// DRI = 64 ms * 103 MHz / (8192 * 32) = 25
// (longest burst access time not considered by now)
#define RFSH_CONFIG_VALUE	0x00340197

// Static memory configuration

// CS0/1 : RDF=14, RDN=4 , RRR=2, 32 bits Flash
// Suitable for 128-Mbit StrataFlash (Tcyc = 150 ns)
#define STATIC_0_CONFIG_VALUE	0x44704470

// CS2   : RDF=0, RDN=0 , RRR=0, 32 bits SRAM
// CS3   : RDF=5, RDN=12, RRR=1, 32 bits SRAM
#define STATIC_1_CONFIG_VALUE	0x2C290001


// Macros that handle the red debug LED wired to GPIO-1

        .macro    _red_led_on
        
        // Turn on the red LED on GPIO-1
        ldr     r3,=SA11X0_GPIO_PIN_OUTPUT_SET
        ldr     r2,=0x02
        str     r2,[r3]
        .endm
        
        .macro    _red_led_off
        
        // Turn off the red LED on GPIO-1
        ldr     r3,=SA11X0_GPIO_PIN_OUTPUT_CLEAR
        ldr     r2,=0x02
        str     r2,[r3]        
        .endm


// This macro represents the initial startup code for the platform,
// when the startup is ROM.

// Red LED is turned on during redboot execution and turned off
// right before entering the operating system.

// Green LED is turned off during the redboot execution and
// on right before entering the operating system on a reset
// (not on a wake-up).

        .macro  _platform_setup1
        
        // Disable all interrupts (ICMR not specified on power-up)
        ldr     r1,=SA11X0_ICMR
        mov     r0,#0
        str     r0,[r1]
        
        // Disable IRQs and FIQs
        mov     r0, #(CPSR_IRQ_DISABLE | \
                      CPSR_FIQ_DISABLE | \
                      CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0
        
        // Reset the BCR (green LED off)
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        str     r2,[r1]

        // Set up GPIOs (red LED off)
        ldr     r1,=SA11X0_GPIO_PIN_DIRECTION
        ldr     r2,=SA1110_GPIO_DIR
        str     r2,[r1]
        
        ldr     r1,=SA11X0_GPIO_ALTERNATE_FUNCTION
        ldr     r2,=SA1110_GPIO_ALT
        str     r2,[r1]
        
        ldr     r1,=SA11X0_GPIO_PIN_OUTPUT_CLEAR
        ldr     r2,=SA1110_GPIO_CLR
        str     r2,[r1]
        
        ldr     r1,=SA11X0_GPIO_PIN_OUTPUT_SET
        ldr     r2,=SA1110_GPIO_SET
        str     r2,[r1]

        // Turn on the red LED
        _red_led_on

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

        // Let the PLL settle down	
        ldr     r1,=20000
10:     sub     r1,r1,#1
        cmp     r1,#0
        bne     10b        

        // Initialize DRAM controller. See table below.
        // The DRAM banks are set to disabled.
	
        ldr     r1,=dram_table
        ldr     r2,=__exception_handlers
        sub     r1,r1,r2
        ldr     r2,[r1],#4                      // First control register
11:     ldr     r3,[r1],#4
        str     r3,[r2]
        ldr     r2,[r1],#4                      // Next control register
        cmp     r2,#0
        bne     11b

        // Enable UART
        ldr     r1,=SA1110_GPCLK_CONTROL_0
        ldr     r2,=SA1110_GPCLK_SUS_UART
        str     r2,[r1]

        // Release DRAM hold (PSSR register, bit DH)
	
        // This bit is set upon exit from sleep mode and indicates that the 
        // nRAS/nSDCS 3:0 and nCAS/DQM 3:0 continue to be held low and that 
        // the DRAMs are still in self-refresh mode. This bit should be cleared 
        // by the processor (by writing a one to it) after the DRAM interface 
        // has been configured but before any DRAM access is attempted. 
        // The nRAS/nSDCS and nCAS/DQM lines are released when this bit is
        // cleared. This bit is cleared on hardware reset.

        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_DRAM_CONTROL_HOLD
        str     r2,[r1]

        // On hardware reset in systems containing DRAM or SDRAM, 
        // trigger a number (typically eight) of refresh cycles by attempting 
        // nonburst read or write accesses to any disabled DRAM bank. 
        // Each such access causes a simultaneous CBR for all four banks.
	
        ldr     r1,=SA11X0_RAM_BANK0_BASE
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
	
        // Enable DRAM bank 0
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r2,=DRAM_CONFIG_VALUE
        orr     r2, r2, #0x01
        str     r2,[r1]

        b       19f        

        // Memory controller settings (register, value)        

dram_table:

        .word   SA11X0_DRAM0_CAS_0,           CAS0_WAVEFORM_VALUE
        .word   SA11X0_DRAM0_CAS_1,           CAS1_WAVEFORM_VALUE
        .word   SA11X0_DRAM0_CAS_2,           CAS2_WAVEFORM_VALUE
        .word   SA11X0_EXP_BUS_CONFIGURATION, EXPMEM_CONFIG_VALUE
        .word   SA11X0_REFRESH_CONFIGURATION, RFSH_CONFIG_VALUE
        .word   SA11X0_DRAM2_CAS_0,           CAS0_WAVEFORM_VALUE
        .word   SA11X0_DRAM2_CAS_1,           CAS1_WAVEFORM_VALUE
        .word   SA11X0_DRAM2_CAS_2,           CAS2_WAVEFORM_VALUE
        .word   SA11X0_SMROM_CONFIGURATION,   SMROM_CONFIG_VALUE
        .word   SA11X0_DRAM_CONFIGURATION,    DRAM_CONFIG_VALUE
        .word   SA11X0_STATIC_CONTROL_0,      STATIC_0_CONFIG_VALUE
        .word   SA11X0_STATIC_CONTROL_1,      STATIC_1_CONFIG_VALUE
        .word   0, 0
19:

        // If waking up from sleep, jump to the resume function
        // pointed by the scratchpad register.
        ldr     r1,=SA11X0_RESET_STATUS
        ldr     r2,[r1]
        cmp     r2,#SA11X0_SLEEP_MODE_RESET
        bne     20f
        ldr     r1,=SA11X0_PWR_MGR_SCRATCHPAD
        ldr     r1,[r1]

        _red_led_off
        
        mov     pc,r1
        nop
20:     nop        

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init
       
        // Enable MMU
        ldr     r2,=10f
       	ldr     r1,=MMU_Control_Init|MMU_Control_M
        mcr     MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2    /* Change address spaces */
        nop
        nop
        nop
10:
       
        // Save shadow copy of BCR
        ldr     r1,=_flexanet_BCR
        str     r2,[r1]
        
        // Turn on green LED
        ldr     r1,=SA1110_BOARD_CONTROL
        ldr     r2,=SA1110_BCR_MIN
        orr     r2,r2,#SA1110_BCR_LED_GREEN
        str     r2,[r1]
        
        // Turn off red LED
        _red_led_off
        
        .endm

                
#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  _flexanet_BCR
_flexanet_BCR:  .long   0       // Board Control register shadow
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
