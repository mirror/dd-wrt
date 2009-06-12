#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H
//=============================================================================
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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2002-12-02
// Purpose:      Innovator platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/innovator.h>          // Platform specific hardware definitions

#define nDEBUG

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

// This is a trick.  If the first two words of SRAM are 0x12345678 and
// 0x87654321, then, then the reset routine in FLASH branches to the
// third location in SRAM.  This allows us to test startup code (which may
// be broken) without writing it to FLASH and rendering the board useless.
// (Well, not permanently useless.  Just useless until we track down an
// emulator and reload a working copy of RedBoot.)  The nifty thing with
// the innovator is that, if you press and hold the reset button for
// 2 seconds it triggers a power-on-reset.  The contents of the internal
// SRAM are maintained across such a reset.  Thus, we can write our
// new test version of RedBoot to SRAM (configured with CYGPRI_HAL_ROM_MLT
// set to SRAM) (more on that later), press and hold the reset button,
// and see if the new startup code works.
//
// Now for some notes about this
// 1) I am guessing about the "2 seconds" part.  If you press and hold
//    the reset button long enough, the FPGA triggers a power-on-reset.
//
// 2) In order to test the SRAM version of RedBoot, import the
//    redboot_SRAM.ecm file (instead of redboot_RAM.ecm or redboot_ROM.ecm)
//    and build RedBoot.  If you already have RedBoot in FLASH, you can
//    use that to load redboot.bin with a base address of 0x20000000.
//    You will be prompted with a "Gee, I don't think 0x20000000 is
//    a valid address in RAM, are you sure you want to do this?" message.
//    You should answer "Yes".  Here is the command I use:
//
//    RedBoot> load -v -r -b 0x20000000 redboot.bin
//
//    You can also use the "sloader" application (loaded via
//    Code Composer Studio) to to load the S-Record file for the SRAM
//    version of RedBoot.
//
// 3) I may have seen a case where the code tested fine in SRAM, but didn't
//    work when I placed it in FLASH.  But other things could have been
//    going on.

#ifdef CYGPRI_HAL_ROM_MLT_SRAM
#define PLATFORM_PREAMBLE _platform_preamble
        .macro  _platform_preamble
        .long   0x12345678
        .long   0x87654321
        .endm
#endif


#if defined(DEBUG) && !defined(CYGPRI_HAL_ROM_MLT_SRAM)
// Don't enable these macro when we are executing from SRAM because
// they overwrite SRAM.

#define FAKE_LED_MACRO_SETUP                    \
        ldr     r0,=0x20000000;                 \
        ldr     r1,[r0];                        \
	subs	r2,r1,#0x20000000;              \
	movlo	r1,#0x20000000;                 \
	ldr	r4,=0x2002fff8;                 \
	subs	r2,r1,r4;                       \
	movhi	r1,#0x20000000;                 \
	bic	r1,r1,#0x3;                     \
	add	r1,r1,#4;                       \
	str	r1, [r0];

#define FAKE_LED_MACRO(n)                       \
        ldr     r11,=0x20000000;                \
        ldr     r11,[r11];                      \
        ldr     r12,=n;                         \
        str     r12,[r11]                       
#else
#define FAKE_LED_MACRO_SETUP
#define FAKE_LED_MACRO(n)
#endif

// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1
        // See if we should branch to an application stored in
        // internal SRAM.  We do this by checking for a magic cookie
        // in the first two locations of SRAM and jumping to the
        // third location in SRAM if we find it (after zeroing those
        // two locations so we don't create an infinite reboot loop).
        ldr     r0,=0x12345678
        ldr     r1,=0x87654321
        ldr     r2,=0x20000000
        ldr     r3,[r2]
        cmp     r3,r0
        ldr     r3,[r2,#4]
        cmpeq   r3,r1
        ldr     r3,=0
        streq   r3,[r2],#4
        streq   r3,[r2],#4
        moveq   pc,r2

	FAKE_LED_MACRO_SETUP
        FAKE_LED_MACRO(1)

        //#define PLATFORM_SETUP_FROM_CCS_GEL_SCRIPT
#ifdef PLATFORM_SETUP_FROM_CCS_GEL_SCRIPT	
        // This is the version of _platform_setup adapted from the contents
        // of the GEL script shipped with Code Composer Studio

	// This is all stolen from the ipaq setup

        // Make sure MMU is OFF
	mov r0,#INTERNAL_SRAM_BASE // Force cache writeback by reloading
	add r2,r0,#0x2000	   // cache from the internal memory bank
123:    ldr r1,[r0],#16
	cmp r0, r2
	bne 123b
	mov r0,#0
	mov r1,#0x0070		// MMU Control System bit
	mcr p15,0,r0,c7,c7,0	// Flush data and instruction cache
	mcr p15,0,r0,c8,c7,0	// Flush ID TLBs
	mcr p15,0,r0,c9,c0,0	// Flush Read-Buffer
	mcr p15,0,r0,c7,c10,4	// Drain write buffer
	mcr p15,0,r0,c13,c0,0	// Disable virtual ID mapping
	mcr p15,0,r1,c1,c0,0	// Write MMU control register
	nop; nop; nop; nop

        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0

        // The rest of this is stolen from "init.c" in the sloader program.
        // FIXME -- add configury
        // Set up DPLL1:
        // (reserved)                                 00
        // IOB        = 1     Initialize on break       1
        // (reserved)                                    0
        // PLL_MULT   = 5     60 MHz clock                 0010 1
        // PLL_DIV    = 00:   CLKOUT = CLKREF                    00
        // PLL_ENABLE = 1     Enable DPLL                          1
        // BYPASS_DIV = 00:   CLKOUT = CLKREF                        00
        // (read only)                                                 00
        //
        // 0x2290:                                    0010 0010 1001 0000
        //

	ldr	r1,=DPLL1_BASE
	ldr	r2,=0x2290
	str	r2,[r1,#_DPLL_CTL_REG]
  /* Wait for lock */
1:	ldr	r2,[r1,#_DPLL_CTL_REG]
	and	r2,r2,#1
	cmp	r2,#1
	bne	1b

  /* memif_init() */
  /* Configure ARM9 Memory Interface */
  /* Set up CS0 for memory & bus size of 16 bits, asynchronous read,
   * 3 wait states, and a divide by 2 clock.
   * Set up CS1, CS2, & CS3 the same way, except with 1 wait state.
   */
/*
  TC_EMIFS_CS0_CONFIG   = 0x00003339;
  TC_EMIFS_CS1_CONFIG   = 0x00001139;
  TC_EMIFS_CS2_CONFIG   = 0x00001139;
  TC_EMIFS_CS3_CONFIG   = 0x00001139;
*/
	ldr	r1,=TC_BASE
	ldr	r2,=0x3339
	str	r2,[r1,#0x10]
	ldr	r2,=0x1149
	str	r2,[r1,#0x14]
	ldr	r2,=0x1139
	str	r2,[r1,#0x18]
	str	r2,[r1,#0x1c]
	
  /* Configure the SDRAM */
  /* EMIFF (nCS4) configuration */
  /* TC_EMIFF_SDRAM_CONFIG = 0x000100F4; */
  /* MRS (nCS4) initialization  */
  /* TC_EMIFF_MRS          = 0x00000037; */
	ldr	r2,=0x000100F4
	str	r2,[r1,#0x20]
	ldr	r2,=0x00000037
	str	r2,[r1,#0x24]

  /* Disable ARM9 Watchdog Timer by writing the special sequence to it */
/*
  WATCHDOG_TIMER_MODE = 0x00F5;
  WATCHDOG_TIMER_MODE = 0x00A0;
*/
	ldr	r1,=WATCHDOG_BASE
	ldr	r2,=0xF5
	strh	r2,[r1,#0x08]
	ldr	r2,=0xA0
	strh	r2,[r1,#0x08]
  /* Select the 12MHz oscillator for the frequency reference for the
   * internal timers.  I am doing this today (12/19/02) to simplify my
   * life -- This way, I don't care what the clock rate of the core is.
   */
        ldr     r1,=CLKM_BASE
        ldrh    r2,[r1,#0x00]   // ARM_CKCTL
        bic     r2,r2,#0x1000   // Set ARM_TIMXO = 0
        strh    r2,[r1,#0x00]

  /* Enable the MPUXOR_CK by:
   * "MPUXOR_CK ... is derived from CK_REF ... and is enabled by EN_XORPCK"
   *
   * EN_XORPCK is bit 1 of ARM_IDLECT2
   */
/*
  CLKM_ARM_IDLECT2 |= 0x0002;
*/
	ldrh	r2,[r1,#0x08]
	orr	r2,r2,#0x0082   // Bits 7 (EN_TIMCK) and 1 (EN_XORPCK)
	strh	r2,[r1,#0x08]
	
  /* Then set the PER_EN bit to 1
   * 
   * PER_EN is bit 0 of ARM_RSTCT2
   */
/*
  CLKM_ARM_RSTCT2 |= 0x0001;
*/
	ldrh	r2,[r1,#0x14]
	orr	r2,r2,#0x0001
	strh	r2,[r1,#0x14]

  /* Set the "BT_UART_GATING" bit to 1 in the FUNC_MUX_CTRL_0 register.
   * This enables the TX1 and RTS1 pins.
   */
/*
  CONFIG_FUNC_MUX_CTRL_0 |= BIT_25;
*/
	ldr	r1,=CONFIG_BASE
        ldr     r3,=0x02000000
	ldr	r2,[r1,#0x00]
	orr	r2,r2,r3
	str	r2,[r1,#0x00]

  /* Set bit 6 of the FPGA Power Control Register.  If I could find some
   * documentation on this, I could explain better why I am doing this, but
   * for now, emperical evidence suggests that this disables the "shutdown"
   * signal to the RS232 level shifter.
   */
/*
  FPGA_PWR_CTRL_REG |= BIT_06;
*/
	ldr	r1,=FPGA_BASE
	ldrb	r2,[r1,#0x05]
	orr	r2,r2,#0x20
#ifdef ADD_COMPATIBILITY_FOR_THE_EVM_SOMEDAY
        orr     r2,r2,#0x40
#endif
	strb	r2,[r1,#0x05]

        // Set up a stack [for calling C code]
#if defined(CYG_HAL_STARTUP_SLOADER) || defined(CYG_HAL_STARTUP_ROM)
        // The startup stack is in internal SRAM
        ldr     sp,=__startup_stack
        // This _MOST_DEFINATELY_ needs to be fixed
        orr     sp,sp,#0x10000000
#else
        // The startup stack is in SDRAM, at some virtual address, but
        // we have not set up the MMU yet, so we need to initialize SP
        // with the physical address of '__startup_stack'
#error "Somehow"
#endif
        bl      hal_mmu_init

        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
        mcr	MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2

        //        mcr     MMU_CP,0,r0,MMU_InvalidateCache,c7,0	// Flush data and instruction cache
        //	mcr     MMU_CP,0,r0,MMU_TLB,c7,0        	// Flush ID TLBs
10:     
        nop
        nop
        nop

#if 0
        ldr     r3,=0x20000000
        str     r1,[r3]
        mrc     MMU_CP,0,r1,MMU_Control,c0
        str     r1,[r3, #0x04]
        mrc     p15,0,r1,c15,c1,0
        str     r1,[r3, #0x08]

here:
        //        b       here
#endif
#else   // PLATFORM_SETUP_FROM_CCS_GEL_SCRIPT
	// This is all stolen from the ipaq setup

        // Make sure MMU is OFF
	mov r0,#INTERNAL_SRAM_BASE // Force cache writeback by reloading
	add r2,r0,#0x2000	   // cache from the internal memory bank
123:    ldr r1,[r0],#16
	cmp r0, r2
	bne 123b
	mov r0,#0
	mov r1,#0x0070		// MMU Control System bit
	mcr p15,0,r0,c7,c7,0	// Flush data and instruction cache
	mcr p15,0,r0,c8,c7,0	// Flush ID TLBs
	mcr p15,0,r0,c9,c0,0	// Flush Read-Buffer
	mcr p15,0,r0,c7,c10,4	// Drain write buffer
	mcr p15,0,r0,c13,c0,0	// Disable virtual ID mapping
	mcr p15,0,r1,c1,c0,0	// Write MMU control register
	nop; nop; nop; nop
        FAKE_LED_MACRO(2)

        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0
        FAKE_LED_MACRO(3)

        // This is the platform setup adapted from the rrload setup implied
        // by head_omap1510.S

// Disable the Watchdog Timer.
// ---------------------------
        mov  r1, #0xF5
        ldr  r0, REG_WDT_TIMER_MODE
        strh r1, [r0]  // Set WDTIM Mode
        mov  r1, #0xA0
        strh r1, [r0]  // Set WDTIM Mode
        FAKE_LED_MACRO(4)

// setting for DPLL1 control register.
// ----------------------------------
        ldr  r0, REG_DPLL1_CTL
        mov  r1, #0x10
        strh r1, [r0]
// Continue to loop if bit shows "not locked"
poll1:
        ldrh r1, [r0]
        ands r1, r1, #0x01
        beq  poll1
        FAKE_LED_MACRO(5)

// Init Arm9 processor.
// --------------------
        mrs r0, cpsr            // Get current mode bits.
        bic r0, r0, #0x1f        // Clear mode bits.
        orr r0, r0, #0xd3        // Disable IRQs/FIQs, supervisor mode.
        msr cpsr, r0            // Enter Supervisor mode.
        mov r1, #0x81           // Set ARM925T configuration.
        mcr p15, 0, r1, c15, c1, 0  // Write ARM925T configuration register.
        FAKE_LED_MACRO(6)

// Disable All Interrupts
// -----------
        ldr r1, V_0xffffffff
        ldr r0, REG_IHL1_MIR
        str r1, [r0]
        ldr r0, REG_IHL2_MIR
        str r1, [r0]
        FAKE_LED_MACRO(7)

// Determine if this is a 1509 or 1510, then
// set the Configuration Registers accordingly
// 1509 shows 0, 1510 shows 0x1b47002f
// -------------------
        ldr     r0, REG_IDCODE
        ldr     r1, [r0]
        cmp     r1, #0x0
        beq     omap1509
        FAKE_LED_MACRO(8)
        
// OK, so we're a 1510.  
omap1510:
/*
        Errata for ES1 says to do this:

        1)  Check for power-on or warm reset.
        2)  Configure SDRAM controller depending on reset type.
*/
        // Check for reset type
        ldr     r0, REG_ARM_SYSST
        ldrh    r1, [r0]
        mov     r2, #0x20
        tst     r2, r1
        beq     zzz_warm_reset
        FAKE_LED_MACRO(9)

POR:
        // Wait 100mS for SDRAM to stabilize before
        // configuring SDRAM controller.
        // Number guessed at.
        mov     r0, #0x2000
1:      subs    r0, r0, #0x1
        bne     1b
        FAKE_LED_MACRO(10)
        
        b       after_initial_configure_SDRAM
        
zzz_warm_reset:
        FAKE_LED_MACRO(11)
      
        // Set auto-refresh counter value to 1.
        // Program MRS to appropriate CAS latency
        // Wait for SDRAM array to be completely
        // refreshed, 1 cycle * #SDRAM rows.
        ldr     r0, REG_TC_EMIFF_SDRAM_CONFIG
        mov     r1, #0x100
        str     r1, [r0]
        ldr     r0, REG_TC_EMIFF_MRS
        ldr     r1, VAL_TC_EMIFF_MRS
        str     r1, [r0]
        mov     r2, #0x2000
1:      subs    r2, r2, #0x1
        bne     1b
        ldr     r0, REG_TC_EMIFF_SDRAM_CONFIG
        mov     r1, #0x10000
        str     r1, [r0]     
                
after_initial_configure_SDRAM:        
        FAKE_LED_MACRO(12)
                        

// Config Spec says to write values
// to each of the configuration registers,
// then take the chip out of 1509 compatibility mode.

        ldr     r0, REG_PULL_DWN_CTRL_0
        ldr     r1, VAL_PULL_DWN_CTRL_0
        str     r1, [r0]
        ldr     r0, REG_PULL_DWN_CTRL_1
        ldr     r1, VAL_PULL_DWN_CTRL_1
        str     r1, [r0]
        ldr     r0, REG_PULL_DWN_CTRL_2
        ldr     r1, VAL_PULL_DWN_CTRL_2
        str     r1, [r0]
        ldr     r0, REG_PULL_DWN_CTRL_3
        ldr     r1, VAL_PULL_DWN_CTRL_3
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_4
        ldr     r1, VAL_FUNC_MUX_CTRL_4
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_5
        ldr     r1, VAL_FUNC_MUX_CTRL_5
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_6
        ldr     r1, VAL_FUNC_MUX_CTRL_6
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_7
        ldr     r1, VAL_FUNC_MUX_CTRL_7
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_8
        ldr     r1, VAL_FUNC_MUX_CTRL_8
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_9
        ldr     r1, VAL_FUNC_MUX_CTRL_9
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_A
        ldr     r1, VAL_FUNC_MUX_CTRL_A
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_B
        ldr     r1, VAL_FUNC_MUX_CTRL_B
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_C
        ldr     r1, VAL_FUNC_MUX_CTRL_C
        str     r1, [r0]
        ldr     r0, REG_FUNC_MUX_CTRL_D
        ldr     r1, VAL_FUNC_MUX_CTRL_D
        str     r1, [r0]
        ldr     r0, REG_VOLTAGE_CTRL_0
        ldr     r1, VAL_VOLTAGE_CTRL_0
        str     r1, [r0]
        ldr     r0, REG_TEST_DBG_CTRL_0
        ldr     r1, VAL_TEST_DBG_CTRL_0
        str     r1, [r0]
        ldr     r0, REG_MOD_CONF_CTRL_0
        ldr     r1, VAL_MOD_CONF_CTRL_0
        str     r1, [r0]
        FAKE_LED_MACRO(13)

        // Take out of compatibility mode
        ldr     r0, REG_COMP_MODE_CTRL_0
        ldr     r1, VAL_COMP_MODE_CTRL_0
        str     r1, [r0]
        FAKE_LED_MACRO(14)

        b post_config_registers
        
omap1509:       
        ldr     r0, REG_FUNC_MUX_CTRL_0
        ldr     r1, [r0]
        orr     r1, r1, #0x6000000  // UART_GIGA_GATE bit as well as UART_BT_GATE bit
        str     r1, [r0]
// Errata for ES5.5 says this must be done before DSP or MPU can
// access internal RAMs.  This is benign for earlier revs.
        ldr     r0, REG_FUNC_MUX_CTRL_1
        mov     r1, #0xc
        str     r1, [r0]
        
post_config_registers:          
        FAKE_LED_MACRO(15)
        mov r0, #0x1800
again:        
        subs r0, r0, #0x1
        bne again
        FAKE_LED_MACRO(16)

// Invalidate cache
// -----------------
        mov r0,#0
        nop
        mcr p15, 0x0, r0, c7, c5, 0x0  
        nop
        nop
        nop
        nop
        FAKE_LED_MACRO(17)

//  Enable I-Cache
// -------------
        mrc p15, 0x0, r1, c1, c0, 0x0
        orr r1, r1, #0x1000
        nop
        mcr p15, 0x0, r1, c1, c0, 0x0
        nop
        nop
        nop
        nop
        FAKE_LED_MACRO(18)

// Initialize Traffic Controller (TC)
// ----------------------------------
        ldr r0, REG_TC_IMIF_PRIO
        mov r1, #0x0
        str r1, [r0]
        ldr r0, REG_TC_EMIFS_PRIO
        str r1, [r0]
        ldr r0, REG_TC_EMIFF_PRIO
        str r1, [r0]
        
        ldr r0, REG_TC_EMIFS_CONFIG
        ldr r1, [r0]
        bic r1, r1, #0x08	/* clear the global power-down enable PDE bit */
        bic r1, r1, #0x01	/* write protect flash by clearing the WP bit */
        str r1, [r0]  // EMIFS GlB Configuration. (value 0x12 most likely)

// Set TC chip select registers
// SDRAM value based on 168MHz 1510.
// ----------------------------
        ldr r0, REG_TC_EMIFS_CS1_CONFIG
        ldr r1, VAL_TC_EMIFS_CS1_CONFIG_PRELIM
        str r1, [r0]
        ldr r0, REG_TC_EMIFS_CS2_CONFIG
        ldr r1, VAL_TC_EMIFS_CS2_CONFIG_PRELIM
        str r1, [r0]
        ldr r0, REG_TC_EMIFF_SDRAM_CONFIG
        ldr r1, VAL_TC_EMIFF_SDRAM_CONFIG
        str r1, [r0]
        ldr r0, REG_TC_EMIFF_MRS
        ldr r1, VAL_TC_EMIFF_MRS
        str r1, [r0]
        
        mov r0,#0x1800
again2:        
        subs r0,r0,#0x1
        bne again2
        FAKE_LED_MACRO(19)

 // Next, Enable the RS232 Line Drivers in the FPGA.
 // Also, power on the audio CODEC's amplifier here,
 // which will make a noise on the audio output.
 // This is done here instead of in the kernel so there
 // isn't a loud popping noise at the start of each
 // song.
 // Also, disable the CODEC's clocks.
 // omap1510-HelenP1 [specific]
        ldr r0, REG_FPGA_POWER
        mov r1, #0
        ldr r2, REG_FPGA_DIP_SWITCH
        ldrb r3, [r2]
        cmp r3, #0x8
        movne r1, #0x62     // Enable the RS232 Line Drivers in the EPLD
        strb r1, [r0]
        ldr r0, REG_FPGA_AUDIO
        mov r1, #0x0     // Disable sound driver (CODEC clocks)
        strb r1, [r0]
                 
        mov r0, #0x1800
again0:        
        subs r0, r0, #0x1
        bne again0
        FAKE_LED_MACRO(20)

// Init RHEA
// ----------        
        ldr r1, V_0x0000ff22
        mov r0, #0x0
        str r1, [r0]   // yep, that's really a write to address 0x00000000.

// *revisit-skranz* is needed?        
        mov r0, #0x1800
again12:        
        subs r0, r0, #0x1
        bne again12
        FAKE_LED_MACRO(21)
        
// Misc 2
// ------        
        mov r1, #0xfb
        ldr r0, REG_LB_CLOCK_DIV
        str r1, [r0]

// *revisit-skranz* is needed?        
        mov r0, #0x1800
again4:        
        subs r0, r0, #0x1
        bne again4
        FAKE_LED_MACRO(22)

// ARM Clock Module Setup
// ----------------------        
        mov r1, #0x40
        ldr r0, REG_ARM_IDLECT2
        strh r1, [r0]  // CLKM, Clock domain control.
                
        mov r1, #0x01  // PER_EN bit
        ldr r0, REG_ARM_RSTCT2 
        strh r1, [r0]  // CLKM; Peripheral reset.
        
        // Reset CLKM
#ifdef ORIGINAL_CODE
        mov r1, #0x06  // Needed for UART[12]
#else
        mov r1, #0x86  // Needed for UART[12]
#endif
        ldr r0, REG_ARM_IDLECT2
        strh r1, [r0]  // CLKM, Clock domain control.
        
        // Set CLKM to Sync-Scalable
        mov r1, #0x1000  // Needed for UART[12]
        ldr r0, REG_ARM_SYSST
        strh r1, [r0]
        
// *revisit-skranz* is needed?        
        mov r0, #0x1800
again6:        
        subs r0, r0, #0x1
        bne again6
        FAKE_LED_MACRO(23)

        ldr r1, VAL_ARM_CKCTL
        ldr r0, REG_ARM_CKCTL
        strh r1, [r0]
        
// setup DPLL1 Control Register
// ----------------------------
        ldr r1, VAL_DPLL1_CTL
        ldr r0, REG_DPLL1_CTL
        strh r1, [r0]
        ands r1, r1, #0x10   // Check if PLL is enabled.
        beq finish2          // Do not look for lock if BYPASS selected
poll2:
        ldrh r1, [r0]
        ands r1, r1, #0x01   // Check the LOCK bit.
        beq poll2            // ...loop until bit goes hi.
finish2:         
        FAKE_LED_MACRO(24)
        
        // Setup TC EMIFS configuration.
        // CS0 value based on 168MHz
        // ---------------------------------------------------
        ldr r1, VAL_TC_EMIFS_CS0_CONFIG // increase flash speed.
        ldr r0, REG_TC_EMIFS_CS0_CONFIG
        str r1, [r0] // Chip Select 0
        ldr r1, VAL_TC_EMIFS_CS1_CONFIG
        ldr r0, REG_TC_EMIFS_CS1_CONFIG
        str r1, [r0] // Chip Select 1
        ldr r1, VAL_TC_EMIFS_CS2_CONFIG
        ldr r0, REG_TC_EMIFS_CS2_CONFIG
        str r1, [r0] // Chip Select 2
        ldr r1, VAL_TC_EMIFS_CS3_CONFIG
        ldr r0, REG_TC_EMIFS_CS3_CONFIG
        str r1, [r0] // Chip Select 3
        
// *revisit-skranz* is needed?        
        mov r0, #0x1800
again9:        
        subs r0, r0, #0x1
        bne again9
        FAKE_LED_MACRO(25)

        // The following was added by WPD
        // Set up a stack [for calling C code]
#ifdef CYG_HAL_STARTUP_ROM
        // The startup stack is in internal SRAM
        ldr     sp,=__startup_stack
        // This _MOST_DEFINATELY_ needs to be fixed
        orr     sp,sp,#0x10000000
#else
        // The startup stack is in SDRAM, at some virtual address, but
        // we have not set up the MMU yet, so we need to initialize SP
        // with the physical address of '__startup_stack'
#error "Somehow"
#endif
        bl      hal_mmu_init
        FAKE_LED_MACRO(26)

        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
        mcr	MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2

        //        mcr     MMU_CP,0,r0,MMU_InvalidateCache,c7,0	// Flush data and instruction cache
        //	mcr     MMU_CP,0,r0,MMU_TLB,c7,0        	// Flush ID TLBs
10:     
        nop
        nop
        nop
        FAKE_LED_MACRO(27)

#if 0
        ldr     r3,=0x20000000
        str     r1,[r3]
        mrc     MMU_CP,0,r1,MMU_Control,c0
        str     r1,[r3, #0x04]
        mrc     p15,0,r1,c15,c1,0
        str     r1,[r3, #0x08]

here:
        //        b       here
#endif
#endif
        .endm
        
#else // defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// end of hal_platform_setup.h

// ------------------------------------------------------        
// --------------Static Data Definitions-----------------
// ------------------------------------------------------

/* inernal OMAP registers */
	/* interrupt handler level 2 registers */
REG_IHL2_MIR:			/* 32 bits */
	.word 0xfffe0004
	/* OMAP configuration registers */
REG_FUNC_MUX_CTRL_0:		/* 32 bits */
	.word 0xfffe1000
REG_FUNC_MUX_CTRL_1:		/* 32 bits */
	.word 0xfffe1004
REG_FUNC_MUX_CTRL_2:		/* 32 bits */
	.word 0xfffe1008
REG_COMP_MODE_CTRL_0:		/* 32 bits */
	.word 0xfffe100c
REG_FUNC_MUX_CTRL_3:		/* 32 bits */
	.word 0xfffe1010
REG_FUNC_MUX_CTRL_4:		/* 32 bits */
	.word 0xfffe1014
REG_FUNC_MUX_CTRL_5:		/* 32 bits */
	.word 0xfffe1018
REG_FUNC_MUX_CTRL_6:		/* 32 bits */
	.word 0xfffe101c
REG_FUNC_MUX_CTRL_7:		/* 32 bits */
	.word 0xfffe1020
REG_FUNC_MUX_CTRL_8:		/* 32 bits */
	.word 0xfffe1024
REG_FUNC_MUX_CTRL_9:		/* 32 bits */
	.word 0xfffe1028
REG_FUNC_MUX_CTRL_A:		/* 32 bits */
	.word 0xfffe102C
REG_FUNC_MUX_CTRL_B:		/* 32 bits */
	.word 0xfffe1030
REG_FUNC_MUX_CTRL_C:		/* 32 bits */
	.word 0xfffe1034
REG_FUNC_MUX_CTRL_D:		/* 32 bits */
	.word 0xfffe1038
REG_PULL_DWN_CTRL_0:		/* 32 bits */
	.word 0xfffe1040
REG_PULL_DWN_CTRL_1:		/* 32 bits */
	.word 0xfffe1044
REG_PULL_DWN_CTRL_2:		/* 32 bits */
	.word 0xfffe1048
REG_PULL_DWN_CTRL_3:		/* 32 bits */
	.word 0xfffe104c
REG_VOLTAGE_CTRL_0:		/* 32 bits */
	.word 0xfffe1060
REG_TEST_DBG_CTRL_0:		/* 32 bits */
	.word 0xfffe1070
REG_MOD_CONF_CTRL_0:		/* 32 bits */
	.word 0xfffe1080
	/* local bus control registers */
REG_LB_CLOCK_DIV:		/* 32 bits */
        .word 0xfffec10c
	/* watchdog timer registers */
REG_WDT_TIMER_MODE:		/* 16 bits */
	.word 0xfffec808
	/* interrupt handler level 1 registers */
REG_IHL1_MIR:			/* 32 bits */
	.word 0xfffecb04
	/* traffic controller memory interface registers */
REG_TC_IMIF_PRIO:		/* 32 bits */
	.word 0xfffecc00
REG_TC_EMIFS_PRIO:		/* 32 bits */
	.word 0xfffecc04
REG_TC_EMIFF_PRIO:		/* 32 bits */
	.word 0xfffecc08
REG_TC_EMIFS_CONFIG:		/* 32 bits */
	.word 0xfffecc0c
REG_TC_EMIFS_CS0_CONFIG:	/* 32 bits */
        .word 0xfffecc10
REG_TC_EMIFS_CS1_CONFIG:	/* 32 bits */
        .word 0xfffecc14
REG_TC_EMIFS_CS2_CONFIG:	/* 32 bits */
        .word 0xfffecc18
REG_TC_EMIFS_CS3_CONFIG:	/* 32 bits */
        .word 0xfffecc1c
REG_TC_EMIFF_SDRAM_CONFIG:	/* 32 bits */
	.word 0xfffecc20
REG_TC_EMIFF_MRS:		/* 32 bits */
	.word 0xfffecc24
	/* MPU clock/reset/power mode control registers */
REG_ARM_CKCTL:			/* 16 bits */
	.word 0xfffece00
REG_ARM_IDLECT2:		/* 16 bits */
        .word 0xfffece08
REG_ARM_RSTCT2:			/* 16 bits */
        .word 0xfffece14
REG_ARM_SYSST:			/* 16 bits */
        .word 0xfffece18
	/* DPLL control registers */
REG_DPLL1_CTL:			/* 16 bits */
	.word 0xfffecf00
	/* identification code register */
REG_IDCODE:			/* 32 bits */
        .word 0xfffed404
	
/* board-specific registers */

REG_FPGA_LED_DIGIT:		/* 8 bits (not used on Innovator) */
        .word 0x08000003
REG_FPGA_POWER:			/* 8 bits */
        .word 0x08000005
REG_FPGA_AUDIO:			/* 8 bits (not used on Innovator) */
	.word 0x0800000c
REG_FPGA_DIP_SWITCH:		/* 8 bits (not used on Innovator) */
        .word 0x0800000e

/* constants */

VAL_COMP_MODE_CTRL_0:
	.word 0x0000eaef
VAL_FUNC_MUX_CTRL_4:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_5:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_6:
	.word 0x00000001
VAL_FUNC_MUX_CTRL_7:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_8:
	.word 0x10001200
VAL_FUNC_MUX_CTRL_9:
	.word 0x01201012
VAL_FUNC_MUX_CTRL_A:
	.word 0x00000248
VAL_FUNC_MUX_CTRL_B:
	.word 0x00000248
VAL_FUNC_MUX_CTRL_C:
	.word 0x09000000
VAL_FUNC_MUX_CTRL_D:
	.word 0x00000000
VAL_PULL_DWN_CTRL_0:
	.word 0x11a10000
VAL_PULL_DWN_CTRL_1:
	.word 0x2e047fff
VAL_PULL_DWN_CTRL_2:
	.word 0xffd7d3e6
VAL_PULL_DWN_CTRL_3:
	.word 0x00003f03
VAL_VOLTAGE_CTRL_0:
	.word 0x00000007
VAL_TEST_DBG_CTRL_0:
	/* The OMAP5910 TRM says this register must be 0, but HelenConfRegs 
	 * says to write a 7.  Don't know what the right thing is to do, so 
	 * I'm leaving it at 7 since that's what was already here.
	 */
	.word 0x00000007
VAL_MOD_CONF_CTRL_0:
	.word 0x0b000008
VAL_ARM_CKCTL:
#ifdef ORIGINAL_CODE
	.word 0x110f
#else
	.word 0x010f
#endif
VAL_DPLL1_CTL:
	.word 0x2710
VAL_TC_EMIFS_CS1_CONFIG_PRELIM:
	.word 0x00001149
VAL_TC_EMIFS_CS2_CONFIG_PRELIM:
	.word 0x00004158
VAL_TC_EMIFS_CS0_CONFIG:     
	.word 0x002130b0
VAL_TC_EMIFS_CS1_CONFIG:   
	.word 0x0000f559
VAL_TC_EMIFS_CS2_CONFIG:   
	.word 0x000055f0
VAL_TC_EMIFS_CS3_CONFIG:   
	.word 0x00003331
VAL_TC_EMIFF_SDRAM_CONFIG:   
	.word 0x010290fc
VAL_TC_EMIFF_MRS:   
	.word 0x00000027

V_0xffffffff:
	.word 0xffffffff
      
V_0x0000ff22:   
        .word 0x0000ff22
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
