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
// Author(s):    David A Rusling
// Contributors: Philippe Robin
// Date:         November 7, 2000
// Purpose:     ARM INTEGRATOR platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/hal_integrator.h>

#ifdef CYG_HAL_STARTUP_ROMRAM
#define CYGSEM_HAL_ROM_RESET_USES_JUMP
#endif

// Define macro used to diddle the LEDs during early initialization.
// Can use r0+r1. Argument in \x.
// Control the LEDs PP0-PP3. This requires the jumpers on pins 9-16 to
// be set on LK11 in order to be visible. Otherwise the parallel port
// data pins are diddled instead.


#ifdef CYGHWR_HAL_ARM_INTEGRATOR_DIAG_LEDS
#define CYGHWR_LED_MACRO				\
        mov     r1, #(15 & (\x))			;\
        ldr     r0,=INTEGRATOR_DBG_BASE			;\
        strb    r1, [r0, #INTEGRATOR_DBG_LEDS_OFFSET]                    ;
#endif

#define PLATFORM_SETUP1 platform_setup1

        .macro  platform_setup1
#ifdef CYG_HAL_STARTUP_ROMRAM
	// This warps execution away from location 0+x to 0x24000000+x
	// so that we can turn the FLASH remapping off. 
	orr	pc,pc,#0x24000000
	nop
	nop
	nop
	nop
#endif
        ldr     r0,=INTEGRATOR_DBG_BASE                                  
        ldr     r1,=0                                                    
        strb    r1, [r0, #INTEGRATOR_DBG_LEDS_OFFSET]                    
	ldr	r0, =INTEGRATOR_HDR_BASE                                 
	ldr	r1, [r0, #INTEGRATOR_HDR_CTRL_OFFSET]                    
	orr	r1, r1, #INTEGRATOR_HDR_CTRL_REMAP	                 
        str     r1, [r0, #INTEGRATOR_HDR_CTRL_OFFSET]                    
	ldr	r1, =INTEGRATOR_IRQCONT_BASE                             
        ldr	r0, =0xFFFFFFFF                                          
	str	r0, [r1, #INTEGRATOR_IRQENABLECLEAR]             	 
	str	r0, [r1, #INTEGRATOR_FIQENABLECLEAR]             	 
        ldr     r0,=INTEGRATOR_DBG_BASE                                  
        ldr     r1,=0xF                                                  
        strb    r1, [r0, #INTEGRATOR_DBG_LEDS_OFFSET]

	LED     7
#ifdef CYG_HAL_STARTUP_ROMRAM
#if 0
        // Compute [logical] base address of this image in ROM
        bl      10f
10:     mov     r9,lr				// R9 = ROM address of 10:
        ldr     r8,=10b				// R8 = RAM address of 10:
	sub     r9,r9,r8			// R9 = ROM-RAM
	ldr     r0,=__exception_handlers	// R0 = RAM address of __exception_handlers
	add	r0,r0,r9			// R0 = ROM address of __exception_handlers
#endif
	    
	ldr	r0,=0x24000000
        ldr     r1,=__exception_handlers  // ram base & length
        ldr     r2,=__rom_data_end
20:     ldr     r3,[r0],#4
        str     r3,[r1],#4
        cmp     r1,r2
        bne     20b
        ldr     r0,=30f
        mov     pc,r0
        nop
        nop
        nop
        nop
30:             
#endif
	LED     6

	.endm

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
