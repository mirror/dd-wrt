#ifndef CYGONCE_HAL_ARM_XSCALE_IQ80321_IQ80321_H
#define CYGONCE_HAL_ARM_XSCALE_IQ80321_IQ80321_H

/*=============================================================================
//
//      iq80321.h
//
//      Platform specific support (register layout, etc)
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
// Date:         2001-12-03
// Purpose:      Intel IQ80321 platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/iq80321.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_iq80321.h>
#include <cyg/hal/hal_verde.h>  // IO Processor defines

#define IQ80321_FLASH_ADDR          0xf0000000   // Verde PBIU CS0
#define IQ80321_UART_ADDR           0xfe800000   // Verde PBIU CS1
#define IQ80321_DISPLAY_RIGHT_ADDR  0xfe850000   // Verde PBIU CS2
#define IQ80321_DISPLAY_LEFT_ADDR   0xfe840000   // Verde PBIU CS3
#define IQ80321_ROTARY_SWITCH_ADDR  0xfe8d0000   // Verde PBIU CS4
#define IQ80321_BATTERY_STATUS_ADDR 0xfe8f0000   // Verde PBIU CS5

#define SDRAM_PHYS_BASE     0xa0000000
#ifdef CYG_HAL_MEMORY_MAP_NORMAL
#define SDRAM_BASE          0x00000000
#else
#define SDRAM_BASE          0xa0000000
#endif
#define SDRAM_SIZE          0x08000000  // 128MB
#define SDRAM_MAX           0x20000000  // 512MB

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_UNCACHED_BASE 0xc0000000
#define DCACHE_FLUSH_AREA   0xe0000000

// Pin used to enable Gigabit Ethernet NIC
// The GPIO pin only has effect when switch S7E1-5 is closed (ON).
#define IQ80321_GBE_GPIO_PIN 4

// ------------------------------------------------------------------------
// SDRAM configuration

// I2C slave address to which the unit responds when in slave-receive mode
#define I2C_DEVID   0x02
#define SDRAM_DEVID 0xAE

// Timeout limit for SDRAM EEPROM to respond
#define I2C_TIMOUT	0x1000000

// ------------------------------------------------------------------------
// Battery Status
//
#define IQ80321_BATTERY_NOT_PRESENT 0x01
#define IQ80321_BATTERY_CHARGE      0x02
#define IQ80321_BATTERY_ENABLE      0x04
#define IQ80321_BATTERY_DISCHARGE   0x08

#define IQ80321_BATTERY_STATUS ((volatile unsigned short *)IQ80321_BATTERY_STATUS_ADDR)

// Address used for battery backup test
#define SDRAM_BATTERY_TEST_ADDR  (SDRAM_UNCACHED_BASE + 0x100000)


// ------------------------------------------------------------------------
// 7 Segment Display
#define DISPLAY_LEFT  IQ80321_DISPLAY_LEFT_ADDR
#define DISPLAY_RIGHT IQ80321_DISPLAY_RIGHT_ADDR

#define DISPLAY_0  0x03
#define DISPLAY_1  0x9f
#define DISPLAY_2  0x25
#define DISPLAY_3  0x0d
#define DISPLAY_4  0x99
#define DISPLAY_5  0x49
#define DISPLAY_6  0x41
#define DISPLAY_7  0x1f
#define DISPLAY_8  0x01
#define DISPLAY_9  0x19
#define DISPLAY_A  0x11
#define DISPLAY_B  0xc1
#define DISPLAY_C  0x63
#define DISPLAY_D  0x85
#define DISPLAY_E  0x61
#define DISPLAY_F  0x71
#define DISPLAY_G  0x43            /* 0100001-1 */
#define DISPLAY_H  0x91            /* 1001000-1 */
#define DISPLAY_I  0xf3            /* 1111001-1 */
#define DISPLAY_J  0x8f            /* 1000111-1 */
#define DISPLAY_K  0x90            /* 1001000-0 *//* cannot do a K, H with a decimal point */
#define DISPLAY_L  0xe3            /* 1110001-1 */
#define DISPLAY_M  0x7e            /* 0111111-0 *//* Cannot do an M, overscore with the decimal point */
#define DISPLAY_N  0x13            /* 0001001-1 */
#define DISPLAY_O  0x03            /* 0000001-1 */
#define DISPLAY_P  0x31            /* 0011000-1 */
#define DISPLAY_Q  0x02            /* 0000001-0 */
#define DISPLAY_R  0x10            /* 0001000-0 *//* same as an "A", except with the decimal point */
#define DISPLAY_S  0x48            /* 0100100-0 *//* same as a "5", except with the decimal point  */
#define DISPLAY_T  0x1e            /* 0001111-0 *//* same as a "7", except with the decimal point  */
#define DISPLAY_U  0x13            /* 1000001-1 */
#define DISPLAY_V  0x82            /* 1000001-0 *//* same as a "U", except with the decimal point */
#define DISPLAY_W  0xee            /* 1110111-0 *//* Cannot do an W, underscore with the decimal point */
#define DISPLAY_X  0xb0            /* 1011000-0 *//* cannot do an X, upside-down h, with decimal point */
#define DISPLAY_Y  0x8b            /* 1000101-1 */
#define DISPLAY_Z  0x6c            /* 0110110-0 *//* cannot do a Z, dash/dash/dash with decimal point */
                                               
#define DISPLAY_OFF 0xff            /* 1111111-1 */
#define DISPLAY_ON  0x00            /* 0000000-0 */

#define DISPLAY_SPACE       0xff    /* 1111111-1 */
#define DISPLAY_ERROR       0x60    /* 0110000-0 */
#define DISPLAY_UNDERSCORE  0xef    /* 1110111-1 */
#define DISPLAY_DASH        0xfd    /* 1111110-1 */
#define DISPLAY_PERIOD      0xfe    /* 1111111-0 */
#define DISPLAY_EXCLAMATION 0x9e    /* 1001111-0 */

#ifdef __ASSEMBLER__
        // Display 'lvalue:rvalue' on the hex display
        // lvalue and rvalue must be of the form 'DISPLAY_x'
        // where 'x' is a hex digit from 0-F.
	.macro HEX_DISPLAY reg0, reg1, lvalue, rvalue	
	ldr	\reg0, =DISPLAY_LEFT		// display left digit
	ldr	\reg1, =\lvalue
	strb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	ldr	\reg1, =\rvalue			// display right digit
	strb	\reg1, [\reg0]
#if 0
	// delay
        ldr     \reg0, =0x7800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
#endif
	.endm

	.macro REG_DISPLAY reg0, reg1, reg2
	b	667f
   666:
	.byte	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3 
	.byte	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7
	.byte	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B
	.byte	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
   667:
	ldr	\reg0, =666b
	add	\reg0, \reg0, \reg2, lsr #4
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_LEFT
	str	\reg1, [\reg0]
	ldr	\reg0, =666b
	and     \reg2, \reg2, #0xf
	add	\reg0, \reg0, \reg2
	ldrb	\reg1, [\reg0]
	ldr	\reg0, =DISPLAY_RIGHT
	str	\reg1, [\reg0]

	// delay
        ldr     \reg0, =0x7800000
        mov     \reg1, #0
    0:
        add     \reg1, \reg1, #1
        cmp     \reg1, \reg0
        ble     0b
	.endm
#else
static inline void HEX_DISPLAY(int lval, int rval)
{
    int i;
    static unsigned char hchars[] = {
	DISPLAY_0, DISPLAY_1, DISPLAY_2, DISPLAY_3,
	DISPLAY_4, DISPLAY_5, DISPLAY_6, DISPLAY_7,
	DISPLAY_8, DISPLAY_9, DISPLAY_A, DISPLAY_B,
	DISPLAY_C, DISPLAY_D, DISPLAY_E, DISPLAY_F
    };
    volatile unsigned int *ldisp = (volatile unsigned int *)DISPLAY_LEFT;
    volatile unsigned int *rdisp = (volatile unsigned int *)DISPLAY_RIGHT;

    *ldisp = hchars[lval & 0xf];
    *rdisp = hchars[rval & 0xf];

    for (i = 0; i < 0x10000000; i++);
}
#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_IQ80321_IQ80321_H
// EOF iq80321.h
