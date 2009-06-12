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
// Author(s):    hmt
// Contributors: hmt
// Date:         1999-04-21
// Purpose:      Intel EBSA285 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include CYGHWR_MEMORY_LAYOUT_H         // Location of the ROM
#include <cyg/hal/hal_ebsa285.h>        // Platform specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions

// Note that we do NOT define CYGHWR_HAL_ARM_HAS_MMU so that at reset we
// jump straight into the ROM; this makes it unnecessary to take any
// special steps to switch from executing in the ROM alias at low
// addresses.  Make no difference for RAM start. For ROMRAM startup the
// application is linked with RAM addresses, but we have to jump to
// the ROM address at startup. Diddle the UNMAPPED macro to do this

#if defined (CYG_HAL_STARTUP_ROMRAM)
#define UNMAPPED(x) (x + CYGMEM_REGION_rom)
#endif

// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1.  Argument in \x.
#define CYGHWR_LED_MACRO                                                  \
        ldr     r0,=0x42000148    /* SA110_XBUS_CYCLE_ARBITER */          ;\
        ldr     r0,[r0]                                                   ;\
        tsts    r0,#0x00800000 /* SA110_XBUS_CYCLE_ARBITER_ENABLED */     ;\
        bne     667f              /* Don't touch if PCI arbiter enabled */;\
        ldr     r0,=0x40012800    /* SA110_XBUS_XCS2 */                   ;\
        mov     r1,#7&(~(\x))                                             ;\
        str     r1,[r0]                                                   ;\
667:

// The main useful output of this file is PLATFORM_SETUP1: it invokes lots
// of other stuff (may depend on RAM or ROM start).  The other stuff is
// divided into further macros to make it easier to manage what's enabled
// when.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || \
    defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS) ||      \
    !defined(CYGSEM_HAL_USE_ROM_MONITOR)

// Dependence on ROM/RAM start removed when meminit code fixed up.
// But it re-emerged when RedBoot's desire to live over RAM app
// initialization asserted itself.
//
// The correct thing is to re-initialize everything if any of:
//  o You are in ROM (duh!)
//  o You include your own stubs (ergo rule the world)
//  o You do not cooperate with a ROM Monitor
//
// [The latter two probably mean the same thing, but this way also lets us
//  support a standalone RAM startup app with no stubs in it. ]
// Hence the more complex conditional above.  See comments in the ChangeLog
// and plf_io.h wrt initializing the PCI bus world.

#define PLATFORM_SETUP1                         \
        PLATFORM_FLUSH_DISABLE_CACHES           \
        INIT_XBUS_ACCESS                        \
        ALLOW_CLOCK_SWITCHING                   \
        CALL_MEMINIT_CODE                       \
        ROMRAM_COPY                             \
        BASIC_PCI_SETUP                         
#else
#define PLATFORM_SETUP1
#endif


// Discard and disable all caches: we are about to be writing vectors...
#define PLATFORM_FLUSH_DISABLE_CACHES                                      \
        /* flush and disable the caches */                                 \
        mrc     p15,0,r1,c1,c0,0                                           ;\
        bic     r1,r1,#0x1000   /* ICache off */                           ;\
        bic     r1,r1,#0x000D   /* DCache off and MM off */                ;\
        mcr     p15,0,r1,c1,c0,0                                           ;\
        mov     r1, #0                                                     ;\
        mcr     p15,0,r1,c7,c6,0 /* DCache invalidate (discard) */         ;\
        mcr     p15,0,r1,c7,c5,0 /* ICache invalidate */                   ;\
        mcr     p15,0,r1,c8,c6,0 /* DCache TLB invalidate */               ;\
        mcr     p15,0,r1,c8,c5,0 /* ICache TLB invalidate */               ;\
        nop                                                                ;\
        nop     /* be sure invalidate "takes" before doing owt else */     ;\
        nop                                                                ;


// Allow clock switching: very early in the startup
#define ALLOW_CLOCK_SWITCHING                                              \
        mov     r0, #0                                                     ;\
        mcr     p15,0,r0,c15,c1,2                                          ;\


// Depending on jumper settings, either ignore or initialize the XBus.
#define INIT_XBUS_ACCESS                                                    \
        ldr     r1, =SA110_XBUS_CYCLE_ARBITER                              ;\
        ldr     r0, [r1]                                                   ;\
        tsts    r0, #SA110_XBUS_CYCLE_ARBITER_ENABLED                      ;\
        beq     777f                                                       ;\
        /* PCI arbiter enabled, so don't touch the XBus */                 ;\
        ldr     r0, =SA110_CONTROL                                         ;\
        ldr     r1, =0x04aa0000                                            ;\
        str     r1, [r0]                                                   ;\
        b       778f                                                       ;\
                                                                           ;\
        /* set up XBus so we can read switch and write to LEDs */          ;\
777:    ldr     r0, =SA110_CONTROL                                         ;\
        ldr     r1, =0x64aa0000                                            ;\
        str     r1, [r0]                                                   ;\
        ldr     r0, =SA110_XBUS_CYCLE_ARBITER                              ;\
        ldr     r1, =0x100016db                                            ;\
        str     r1, [r0]                                                   ;\
        ldr     r0, =SA110_XBUS_IO_STROBE_MASK                             ;\
        ldr     r1, =0xfcfcfcfc                                            ;\
        str     r1, [r0]                                                   ;\
778:                                                                       ;\
        
// Save lr and call mem init code
#define CALL_MEMINIT_CODE                                                 \
        mov     r10, lr       /* preserve lr */                           ;\
        bl      __mem285_init                                             ;\
        ldr     r1, =hal_dram_size  /* [see hal_intr.h] */                ;\
        str     r0, [ r1 ]    /* store the top of memory address */       ;\
        mov     lr, r10       /* in hal_dram_size for future use */       ;\
        
// If we are doing a ROMRAM startup copy all sections up to the start of 
// the data section to RAM.
#if defined(CYG_HAL_STARTUP_ROMRAM)
#define ROMRAM_COPY                                                      \
        ldr     r0,=(CYGMEM_REGION_rom)                                  ;\
        ldr     r1,=0                                                    ;\
        ldr     r2,=0x40                                                 ;\
810:    ldr     r3,[r0],#4                                               ;\
        str     r3,[r1],#4                                               ;\
        cmp     r1,r2                                                    ;\
        bne     810b                                                     ;\
        ldr     r0,=(CYGMEM_REGION_rom+reset_vector)                     ;\
        ldr     r1,=(reset_vector)                                       ;\
        ldr     r2,=(CYGMEM_REGION_rom_SIZE)                             ;\
820:    ldr     r3,[r0],#4                                               ;\
        str     r3,[r1],#4                                               ;\
        cmp     r1,r2                                                    ;\
        bne     820b                                                     ;\
        ldr     r0,=830f                                                 ;\
        mov     pc,r0                                                    ;\
830:                                                                   
#else
#define ROMRAM_COPY                                                      
#endif

#define BASIC_PCI_SETUP                                                   \
        /**************************************************************** \
         *  Basic PCI setup.                                              \
         ****************************************************************/\
        ldr     r0, =SA110_CONTROL_STATUS_BASE                            ;\
                                                                          ;\
        /* Disable PCI Outbound interrupts */                             ;\
        mov     r1, #12                                                   ;\
        str     r1, [r0, #SA110_OUT_INT_MASK_o]                           ;\
                                                                          ;\
        /* Disable Doorbells */                                           ;\
        mov     r1, #0                                                    ;\
        str     r1, [r0, #SA110_DOORBELL_PCI_MASK_o]                      ;\
        str     r1, [r0, #SA110_DOORBELL_SA_MASK_o]                       ;\
                                                                          ;\
        /* Map high PCI address bits to 0 */                              ;\
        str     r1, [r0, #SA110_PCI_ADDR_EXT_o]                           ;\
                                                                          ;\
        /* Interrupt ID to 1 */                                           ;\
        mov     r1, #0x100                                                ;\
        str     r1, [r0, #SA110_PCI_CFG_INT_LINE_o]                       ;\
                                                                          ;\
        /* Remove PCI_reset */                                            ;\
        ldr     r1, [r0, #SA110_CONTROL_o]                                ;\
        orr     r1, r1, #0x200                                            ;\
        str     r1, [r0, #SA110_CONTROL_o]                                ;\
                                                                          ;\
        /* Open a 2MB window */                                           ;\
        mov     r1, #0x1c0000                                             ;\
        str     r1,[r0, #SA110_SDRAM_BASE_ADDRESS_MASK_o]                 ;\
        mov     r1, #0xe00000                                             ;\
        str     r1,[r0, #SA110_SDRAM_BASE_ADDRESS_OFFSET_o]               ;\
                                                                          ;\
        /* Only init PCI if central function is set and */                ;\
        /* standalone bit is cleared                    */                ;\
        ldr     r1, [r0, #SA110_CONTROL_o]                                ;\
        tst     r1, #SA110_CONTROL_CFN                                    ;\
        beq     1f                                                        ;\
                                                                          ;\
        ldr     r1, =0x40012000                                           ;\
        ldr     r1, [r1]                                                  ;\
        tst     r1, #0x40                                                 ;\
        bne     1f                                                        ;\
                                                                          ;\
        /* Don't respond to any commands */                               ;\
        mov     r1, #0                                                    ;\
        str     r1, [r0, #SA110_PCI_CFG_COMMAND_o]                        ;\
                                                                          ;\
        str     r1, [r0, #SA110_PCI_CFG_SDRAM_BAR_o]                      ;\
        mov     r1, #0x40000000                                           ;\
        str     r1, [r0, #SA110_PCI_CFG_CSR_MEM_BAR_o]                    ;\
        mov     r1, #0xf000                                               ;\
        str     r1, [r0, #SA110_PCI_CFG_CSR_IO_BAR_o]                     ;\
                                                                          ;\
        /* respond to I/O space & Memory transactions. */                 ;\
        mov     r1, #0x17                                                 ;\
        str     r1, [r0, #SA110_PCI_CFG_COMMAND_o]                        ;\
    1:                                                                    ;\
        /* Signal PCI_init_complete */                                    ;\
        ldr     r1, [r0, #SA110_CONTROL_o]                                ;\
        orr     r1, r1, #SA110_CONTROL_INIT_COMPLETE                      ;\
        str     r1, [r0, #SA110_CONTROL_o]                                ;\
        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
