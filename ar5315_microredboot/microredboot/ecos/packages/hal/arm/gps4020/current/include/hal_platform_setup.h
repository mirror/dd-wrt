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
// Copyright (C) 2003 Gary Thomas
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
// Date:        1999-04-13
// Purpose:     GPS4020 platform specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#define CYGSEM_HAL_ROM_RESET_USES_JUMP
	    
	.equ    MPC_BASE_ADDRESS,         0xE0008000
	.equ    MPC_AREA1_CONFIG,         0x00000000
	.equ    MPC_AREA2_CONFIG,         0x00000004
	.equ    MPC_AREA3_CONFIG,         0x00000008
	.equ    MPC_AREA4_CONFIG,         0x0000000C

	.equ    GPIO_BASE_ADDRESS,        0xE0005000
	.equ    GPIO_DIRECTION_REG_OFFSET,0x00000000
	.equ    GPIO_READ_REG_OFFSET,     0x00000004
	.equ    GPIO_WRITE_REG_OFFSET,    0x00000008

        .equ    SYSTEM_CONFIG,            0xE0002004

        .macro  gps4020_setup

// make GPIO[0..7] outputs - note: GPIO4 is used for Rx on UART1!	    
	ldr     r3,=GPIO_BASE_ADDRESS
	ldr     r0, [r3,#GPIO_DIRECTION_REG_OFFSET]
	and     r0, r0, #0b00010000    @ 0 = output, 1 = input
	str     r0, [r3,#GPIO_DIRECTION_REG_OFFSET]
        mov     r0,#0
        str     r0,[r3,#GPIO_WRITE_REG_OFFSET]

        ldr     r1,=MPC_BASE_ADDRESS
        ldr     r2,=0x4400002D                  // 0x6xxxxxxx, 16bit R/W RAM
        str     r2,[r1,#MPC_AREA1_CONFIG]

        ldr     r1,=MPC_BASE_ADDRESS
        ldr     r2,=0x00000069                  // 0x2xxxxxxx, 16bit R/W RAM
        str     r2,[r1,#MPC_AREA2_CONFIG]

        ldr     r2,[r1,#MPC_AREA3_CONFIG]
        ldr     r2,=0x00000021                  // 0x4xxxxxxx, 16bit peripheral
        str     r2,[r1,#MPC_AREA3_CONFIG]

        ldr     r2,[r1,#MPC_AREA4_CONFIG]
        ldr     r2,=0x0000006E                  // 0x0xxxxxxx, 32bit memory
        str     r2,[r1,#MPC_AREA4_CONFIG]

        ldr     r1,=SYSTEM_CONFIG               // Swap memory regions 0x0XXXXXXX, 0x6XXXXXXX
        ldr     r2,[r1]
        orr     r2,r2,#1
        mov     r0,#1
        str     r0,[r3,#GPIO_WRITE_REG_OFFSET]
        ldr     r4,=10f                         // Change address space
        str     r2,[r1]

        mov     pc,r4
        mov     pc,r4
        mov     pc,r4
10:
        mov     r0,#2
        str     r0,[r3,#GPIO_WRITE_REG_OFFSET]
        .endm

#ifdef CYG_HAL_STARTUP_ROM
#define PLATFORM_SETUP1 gps4020_setup
#else
#define PLATFORM_SETUP1
#endif

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
