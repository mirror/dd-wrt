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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2003-03-27
// Purpose:      Motorola PRPMC1100 specific support routines
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
#include <cyg/hal/hal_ixp425.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/prpmc1100.h>          // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1  _platform_setup1
#define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

// ------------------------------------------------------------------------
// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO

// Delay a bit
.macro DELAY cycles, reg0
#if 0
    ldr     \reg0, =\cycles
    subs    \reg0, \reg0, #1
    subne   pc,  pc, #0xc
#endif
.endm

// ------------------------------------------------------------------------
// This macro represents the initial startup code for the platform        
	.macro _platform_setup1

#if CYGINT_HAL_ARM_BIGENDIAN
        // set big-endian
	mrc	p15, 0, r0, c1, c0, 0
        orr	r0, r0, #0x80
	mcr	p15, 0, r0, c1, c0, 0
        CPWAIT  r0
#endif

        ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

	// invalidate I & D caches & BTB
	mcr	p15, 0, r0, c7, c7, 0
	CPWAIT	r0

	// invalidate I & Data TLB
        mcr 	p15, 0, r0, c8, c7, 0
        CPWAIT r0

	// drain write and fill buffers
	mcr	p15, 0, r0, c7, c10, 4
	CPWAIT	r0

	// disable write buffer coalescing
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #1
	mcr	p15, 0, r0, c1, c0, 1
	CPWAIT	r0
		
        // Setup chip selects
        ldr     r1, =IXP425_EXP_CFG_BASE
#ifdef IXP425_EXP_CS0_INIT
	ldr	r0, =IXP425_EXP_CS0_INIT
	str     r0, [r1, #IXP425_EXP_CS0]
#endif
#ifdef IXP425_EXP_CS1_INIT
	ldr	r0, =IXP425_EXP_CS1_INIT
	str     r0, [r1, #IXP425_EXP_CS1]
#endif
#ifdef IXP425_EXP_CS2_INIT
	ldr	r0, =IXP425_EXP_CS2_INIT
	str     r0, [r1, #IXP425_EXP_CS2]
#endif
#ifdef IXP425_EXP_CS3_INIT
	ldr	r0, =IXP425_EXP_CS3_INIT
	str     r0, [r1, #IXP425_EXP_CS3]
#endif
#ifdef IXP425_EXP_CS4_INIT
	ldr	r0, =IXP425_EXP_CS4_INIT
	str     r0, [r1, #IXP425_EXP_CS4]
#endif
#ifdef IXP425_EXP_CS5_INIT
	ldr	r0, =IXP425_EXP_CS5_INIT
	str     r0, [r1, #IXP425_EXP_CS5]
#endif
#ifdef IXP425_EXP_CS6_INIT
	ldr	r0, =IXP425_EXP_CS6_INIT
	str     r0, [r1, #IXP425_EXP_CS6]
#endif
#ifdef IXP425_EXP_CS7_INIT
	ldr	r0, =IXP425_EXP_CS7_INIT
	str     r0, [r1, #IXP425_EXP_CS7]
#endif

	// Enable the Icache
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_I
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        // Setup SDRAM controller

        ldr     r0, =IXP425_SDRAM_CFG_BASE

	ldr 	r1, =IXP425_SDRAM_CONFIG_INIT
	str     r1, [r0, #IXP425_SDRAM_CONFIG]

	// disable refresh cycles
	mov 	r1, #0
	str	r1, [r0, #IXP425_SDRAM_REFRESH]

	// send nop command
	mov 	r1, #SDRAM_IR_NOP
	str	r1, [r0, #IXP425_SDRAM_IR]
        DELAY   0x10000, r1
	
	// set SDRAM internal refresh val
	ldr	r1, =IXP425_SDRAM_REFRESH_CNT
	str	r1, [r0, #IXP425_SDRAM_REFRESH]
	DELAY   0x10000, r1

	// send precharge-all command to close all open banks
	mov     r1, #SDRAM_IR_PRECHARGE
	str	r1, [r0, #IXP425_SDRAM_IR]
	DELAY   0x10000, r1

	// provide 8 auto-refresh cycles
	mov     r1, #SDRAM_IR_AUTO_REFRESH
	mov     r2, #8
  1:
        str	r1, [r0, #IXP425_SDRAM_IR]
	DELAY   0x800, r3
	subs	r2, r2, #1
	bne	1b

	// set mode register in sdram
	mov	r1, #IXP425_SDRAM_SET_MODE_CMD
        str	r1, [r0, #IXP425_SDRAM_IR]
	DELAY   0x10000, r1

	// start normal operation
	mov	r1, #SDRAM_IR_NORMAL
        str	r1, [r0, #IXP425_SDRAM_IR]
	DELAY   0x10000, r1

	// value to load into pc to jump to real runtime address
	ldr     r0, =1f

	// Setup EXP_CNFG0 value to switch EXP bus out of low memory
	ldr 	r2, =IXP425_EXP_CFG_BASE
	ldr     r1, [r2, #IXP425_EXP_CNFG0]
	bic     r1, r1, #EXP_CNFG0_MEM_MAP

	b       icache_boundary
	.p2align 5
icache_boundary:
	// Here is where we switch from boot address (0x000000000) to the
	// actual flash runtime address. We align to cache boundary so we
        // execute from cache during the switchover. Cachelines are 8 words.
        str     r1, [r2, #IXP425_EXP_CNFG0]    // make the EXP bus switch
	nop
        nop
        nop
        nop
        mov     pc, r0
        nop
                            // display FFFF and loop forever.
    0:  b       0b
    1:

	// Move mmu tables into RAM so page table walks by the cpu
	// don't interfere with FLASH programming.
	ldr	r0, =mmu_table
	add     r2, r0, #0x4000     	// End of tables
	mov	r1, #SDRAM_PHYS_BASE
	orr	r1, r1, #0x4000		// RAM tables

	// everything can go as-is
    1:
	ldr	r3, [r0], #4
	str	r3, [r1], #4
	cmp	r0, r2
	bne	1b

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

	// Set the TTB register to DRAM mmu_table
	ldr	r0, =(SDRAM_PHYS_BASE | 0x4000) // RAM tables
	mcr	p15, 0, r0, c2, c0, 0		// load page table pointer
	CPWAIT  r0

        // enable permission checks in all domains
        ldr     r0, =0x55555555
        mcr     p15, 0, r0, c3, c0, 0
        CPWAIT  r0

        // enable mmu
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_M
	orr	r0, r0, #MMU_Control_R
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT	r0

        // enable D cache
        mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #MMU_Control_C
        mcr     p15, 0, r0, c1, c0, 0
        CPWAIT  r0

        // Enable branch target buffer
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #MMU_Control_BTB
	mcr	p15, 0, r0, c1, c0, 0
	CPWAIT  r0

        mcr     p15, 0, r0, c7, c10, 4  // drain the write & fill buffers
        CPWAIT  r0

        mcr     p15, 0, r0, c7, c7, 0   // flush Icache, Dcache and BTB
        CPWAIT  r0

        mcr     p15, 0, r0, c8, c7, 0   // flush instuction and data TLBs
        CPWAIT  r0

	mcr	p15, 0, r0, c7, c10, 4	// drain the write & fill buffers
	CPWAIT r0	
	
        // save SDRAM size
        ldr     r1, =hal_dram_size  /* [see hal_intr.h] */
        mov     r8, #SDRAM_SIZE
        str     r8, [r1]

	.endm    // _platform_setup1

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
