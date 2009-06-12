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
// Author(s):   gthomas
// Contributors: amichelotti
// Date:        2004-06-4
// Purpose:     JTST platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/var_io.h>
#include <cyg/hal/hal_misc.h>

        .macro __pio_clkgen_init
        ldr     r0,=AT91_PIO
        ldr     r1,=BIT21|BIT22|BIT23|BIT24|BIT25|BIT26|BIT27
        str     r1,[r0,#AT91_PIO_SODR] // set this bit must be always 1, 
                                 // otherwise resets
        ldr     r1,=BIT21|BIT22|BIT23|BIT24|BIT25|BIT26|BIT27
        str     r1,[r0,#AT91_PIO_OER]  // set to output
        str     r1,[r0,#AT91_PIO_PER]  // software control
        ldr     r1,=BIT21|BIT22|BIT23|BIT24|BIT25|BIT26|BIT27
        str     r1,[r0,#AT91_PIO_CODR] // leds all on
        ldr     r1,=BIT30|BIT3
        ldr     r1,[r0,#AT91_PIO_PDR]  // (BIT30) usb interrupt line in
                                       // (BIT3) active watchdog reset line
        ldr     r0,=AT91_CLKGEN
        ldr     r1,=32
        str 	r1,[r0,#AT91_CLKGEN_CPTMAX0]
        str     r1,[r0,#AT91_CLKGEN_CPTMAX1]
	ldr     r1,=128
	str     r1,[r0,#AT91_CLKGEN_CPTMAX2] //watch dog divider1
	ldr     r1,=1024
	str     r1,[r0,#AT91_CLKGEN_CPTMAX3] //watch dog divider2
	ldr     r1,=2046
	str     r1,[r0,#AT91_CLKGEN_CPTMAX4] //watch dog divider3 (max accepted value)

        ldr     r1,=1
        str     r1,[r0,#AT91_CLKGEN_CLKENABLE]

#if defined(CYG_HAL_STARTUP_RAM)
  // set internal ram as interrupt stack and other
        ldr     r0, =0x7ffc
        ldr     r1,=__startup_stack
        ldr     r2,=__startup_stack_base
        sub     r3,r1,r2
        ldr     r1,=.__startup_stack
        str     r0,[r1]
        sub     r0,r0,r3
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
        ldr     r1,=__interrupt_stack
        ldr     r2,=__interrupt_stack_base
        sub     r3,r1,r2
        ldr     r1,=.__interrupt_stack
        str     r0,[r1]
#endif
#endif
        .endm

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

        .macro  _setup
        ldr     r10,=_InitMemory        // Initialize memory controller
        movs    r0,pc,lsr #20           // If ROM startup, PC < 0x100000
        moveq   r10,r10,lsl #12         //   mask address to low 20 bits
        moveq   r10,r10,lsr #12
        ldmia   r10!,{r0-r9,r11-r12}    // Table of initialization constants
#if defined(CYG_HAL_STARTUP_ROMRAM)
        ldr     r10,=0x0000FFFF
        and     r12,r12,r10
        ldr     r10,=0x00510000
        orr     r12,r12,r10
#endif
        stmia   r11!,{r0-r9}            // Write to controller
        mov     pc,r12                  // Change address space, break pipeline
_InitMemory:
        .long   0x00502031  // FLASH
        .long   0x00602021  // RAM
        .long   0x00702021  // unused
        .long   0x00802021  // unused
        .long   0x00902021  // unused
        .long   0x00A02021  // unused
        .long   0x00b02021  // unused
        .long   0x00c02021  // unused
        .long   0x00000001  // REMAP commande
        .long   0x00000006  // 7 memory regions, standard read
        .long   AT91_EBI    // External Bus Interface address
        .long   10f         // address where to jump
10:

#if defined(CYG_HAL_STARTUP_ROMRAM)
        ldr     r0,=0x00510000          // Relocate FLASH/ROM to on-chip RAM
        ldr     r1,=0x00600000          // RAM base & length
        ldr     r2,=0x00610000
20:     ldr     r3,[r0],#4
        str     r3,[r1],#4
        cmp     r1,r2
        bne     20b
        ldr     r0,=30f
        mov     pc,r0
30:

#endif

__pio_clkgen_init
        .endm

#define CYGSEM_HAL_ROM_RESET_USES_JUMP
#define PLATFORM_SETUP1     _setup
#else

#define PLATFORM_SETUP1 __pio_clkgen_init

#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
