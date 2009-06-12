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
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_pxa2x0.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/hal_mm.h>             // more MMU definitions
#include <cyg/hal/uE250.h>              // Platform specific hardware definitions
#include <cyg/hal/hal_spd.h>

#define MDCNFG_VALUE   0x03001BC9
// #define MDCNFG_VALUE 0x00001BC9
#define MDMRS_VALUE    0x00000000
#define MDREFR_VALUE_1 0x00494030
#define MDREFR_VALUE_2 0x00094030
#define MDREFR_VALUE_3 0x0009C030

#define GPCR0_VALUE 0xFFFFFFFF
#define GPCR1_VALUE 0xFFFFFFFF
#define GPCR2_VALUE 0xFFFFFFFF

#define GPSR0_VALUE 0x00028000
#define GPSR1_VALUE 0x00002122
#define GPSR2_VALUE 0x0001C000

#define GPDR0_VALUE 0x03E3A080
#define GPDR1_VALUE 0x00FFA963
#define GPDR2_VALUE 0x0001C000

#define GAFR0_L_VALUE 0x88000000
#define GAFR0_U_VALUE 0x001A8010
#define GAFR1_L_VALUE 0x90900008 
#define GAFR1_U_VALUE 0x0005AAAA
#define GAFR2_L_VALUE 0xA0000000
#define GAFR2_U_VALUE 0x00000002

#define PSSR_VALUE          0x20
#define MSC0_VALUE_NONBURST 0x2FD0
#define MSC0_VALUE_BURST    0x22D2

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1  _platform_setup1
// #define PLATFORM_EXTRAS  <cyg/hal/hal_platform_extras.h>
#define CYGHWR_HAL_ARM_HAS_MMU

// This macro represents the initial startup code for the platform        
  .macro _platform_setup1
  // This is where we wind up immediately after reset. At this point, we
  // are executing from the boot address (0x00000000), not the eventual
  // flash address. Do some basic setup using position independent code
  // then switch to real flash address

// FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 
// This is a quick and dirty workaround to an apparent gas/ld
// bug. The computed UNMAPPED_PTR(reset_vector) is off by 0x20.
  .rept 0x20/4
  nop
  .endr
// FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 


   // Set the direction of the LED GPIO's to 'output'
   // This will disable the LED's (which are on at boot-time, so we
   // can see we have safely landed here.
  ldr r1, =PXA2X0_GPDR0
  ldr r2, =0x00600000
  str r2, [r1]


  // Disable interrupts, by setting the Interrupt Mask Registers to all 0's
  ldr     r1,=PXA2X0_ICMR
  mov     r0,#0
  str     r0,[r1]
   
  // disable MMU
  mov     r0, #0x0
  mcr     p15, 0, r0, c1, c0, 0

  // flush TLB
  mov     r0, #0x0
  mcr     p15, 0, r0, c8, c7, 0   //  Flush TLB

  // flush I&D caches and BTB
  mov     r0, #0x0
  mcr     p15, 0, r0, c7, c7, 0   //  Flush caches

  CPWAIT r0 

  // Enables access to coprocessor 0 (The only extra coprocessor on the PXA250)
  ldr     r0, =0x00000001
  mcr     p15, 0, r0, c15, c1, 0

  // Disable the IRQ's and FIQ's in the program status register and 
  // enable supervisor mode 
  ldr     r0,=(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
  msr     cpsr, r0

  // Set TURBO mode

  ldr     r2, =0x00000321
  ldr     r1, =PXA2X0_CCCR
  str     r2,[r1]
  
  ldr     r1, =0x00000003
  mcr     p14, 0, r1, c6, c0, 0

  // Set-up memory according to NMI specs

  ldr     r1,=PXA2X0_RAM_BANK0_BASE
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]
  ldr     r2,[r1]

  // Set DRAM Configuration Value
  ldr     r1,=PXA2X0_MDCNFG
  ldr     r2,=MDCNFG_VALUE
  str     r2,[r1]

  // Set MDMRS
  ldr     r1,=PXA2X0_MDMRS
  ldr     r2,=MDMRS_VALUE 
  str     r2,[r1]

  // Set Refresh Values
  ldr     r1,=PXA2X0_MDREFR
  ldr     r2,=MDREFR_VALUE_1   
  str     r2,[r1]

  ldr     r1,=PXA2X0_MDREFR
  ldr     r2,=MDREFR_VALUE_2   
  str     r2,[r1]

  ldr     r1,=PXA2X0_MDREFR
  ldr     r2,=MDREFR_VALUE_3   
  str     r2,[r1]

  // Set Static memory registers
  ldr     r1,=PXA2X0_MSC0
  ldr     r2,=MSC0_VALUE_NONBURST
  str     r2,[r1]

  // Setup GPIO clear registers
  ldr r1, =PXA2X0_GPCR0
  ldr r2, =(GPCR0_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPCR1
  ldr r2, =(GPCR1_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPCR2
  ldr r2, =(GPCR2_VALUE)
  str r2, [r1]

  // Setup GPIO set registers
  ldr r1, =PXA2X0_GPSR0
  ldr r2, =(GPSR0_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPSR1
  ldr r2, =(GPSR1_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPSR2
  ldr r2, =(GPSR2_VALUE)
  str r2, [r1]

  // Setup GPIO direction registers
  ldr r1, =PXA2X0_GPDR0
  ldr r2, =(GPDR0_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPDR1
  ldr r2, =(GPDR1_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GPDR2
  ldr r2, =(GPDR2_VALUE)
  str r2, [r1]

  // Setup GPIO alternate function registers
  ldr r1, =PXA2X0_GAFR0_L
  ldr r2, =(GAFR0_L_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GAFR0_U
  ldr r2, =(GAFR0_U_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GAFR1_L
  ldr r2, =(GAFR1_L_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GAFR1_U
  ldr r2, =(GAFR1_U_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GAFR2_L
  ldr r2, =(GAFR2_L_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_GAFR2_U
  ldr r2, =(GAFR2_U_VALUE)
  str r2, [r1]

  ldr r1, =PXA2X0_PSSR
  ldr r2, =(PSSR_VALUE)
  str r2, [r1]

  // Enable the Icache
  mrc     p15, 0, r0, c1, c0, 0
  orr     r0, r0, #MMU_Control_I
  mcr     p15, 0, r0, c1, c0, 0
  CPWAIT  r0

  // Turn on red led
  ldr     r1,=PXA2X0_GPSR0
  ldr     r2,=0x00200000
  str     r2,[r1]

  // Set up a stack [for calling C code]
  ldr     r1,=__startup_stack
  ldr     r2,=PXA2X0_RAM_BANK0_BASE
  orr     sp,r1,r2

  // Create MMU tables
  bl      hal_mmu_init

  // Turn off red led
  ldr     r1,=PXA2X0_GPCR0
  ldr     r2,=0x00200000
  str     r2,[r1]

  // Enable MMU
  ldr     r2,=10f
  ldr     r1,=MMU_Control_Init|MMU_Control_M
  mcr     MMU_CP,0,r1,MMU_Control,c0
  mov     pc, r2
  nop
  nop
  nop

10:
  // Turn on green led
  ldr     r1,=PXA2X0_GPSR0
  ldr     r2,=0x00400000
  str     r2,[r1]

  .endm

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  hal_pcsr_cfg_retry
hal_pcsr_cfg_retry:   .long   0  // Boot-time value of PCSR Retry bit.
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
