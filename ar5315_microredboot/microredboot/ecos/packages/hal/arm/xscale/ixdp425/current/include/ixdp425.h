#ifndef CYGONCE_HAL_ARM_XSCALE_IXDP425_IXDP425_H
#define CYGONCE_HAL_ARM_XSCALE_IXDP425_IXDP425_H

/*=============================================================================
//
//      ixdp425.h
//
//      Platform specific support (register layout, etc)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Date:         2002-12-10
// Purpose:      Intel IXDP425 specific support routines
// Description: 
// Usage:        #include <cyg/hal/ixdp425.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_ixdp425.h>
#include <cyg/hal/hal_ixp425.h>

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_PHYS_BASE                    0x00000000
#define SDRAM_BASE                         0x00000000
#define SDRAM_ALIAS_BASE                   0x10000000
#define SDRAM_UNCACHED_BASE                0x20000000
#define SDRAM_DC_BASE                      0x30000000
#define SDRAM_SIZE                         0x10000000  // 256MB

#define IXDP425_LED_BASE                   0x52000000
#define IXDP425_LED_SIZE                   0x00100000
#define IXDP425_LED_DATA                   REG16(0, IXDP425_LED_BASE)

#define IXDP_FLASH_BASE                    0x50000000
#define IXDP_FLASH_SIZE                    0x01000000
#define IXDP_FLASH_DC_BASE                 0xA0000000

// CS0 (flash optimum timing)
#define IXP425_EXP_CS0_INIT \
 (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
  EXP_RECOVERY_T(15) | EXP_SZ_16M | EXP_WR_EN | EXP_BYTE_RD16 | EXP_CS_EN)

// CS2 (LED display)
#define IXP425_EXP_CS2_INIT  \
 (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
  EXP_RECOVERY_T(15) | EXP_SZ_512 | EXP_WR_EN | EXP_CS_EN)

#define IXP425_SDRAM_CONFIG_INIT  (SDRAM_CONFIG_CAS_3 | SDRAM_CONFIG_4x32Mx16)
#define IXP425_SDRAM_REFRESH_CNT  0x081
#define IXP425_SDRAM_SET_MODE_CMD SDRAM_IR_MODE_SET_CAS3

// ------------------------------------------------------------------------
// GPIO lines

#define GPIO_EEPROM_SDA  7
#define GPIO_EEPROM_SCL  6
#define GPIO_ENET1_INT_N 5
#define GPIO_ENET0_INT_N 4
#define GPIO_HSS0_INT_N  3
#define GPIO_HSS1_INT_N  2
#define GPIO_DSL_INT_N   1

// ------------------------------------------------------------------------
// 4 Digit Hex Display
//
// The board has two 4 hex digit display controlled by three board registers.
// Each digit has a decimal point and may be individually blanked.

#ifdef __ASSEMBLER__
        // Display hex digits in 'value' not masked by 'mask'.
	.macro DISPLAY value, reg0, reg1
	ldr    \reg0, =\value
	ldr    \reg1, =IXDP425_LED_DATA
	strh   \reg0, [\reg1]
#if 0
	// delay
        ldr     \reg0, =0x780000
    0:
        subs    \reg0, \reg0, #1
        bne     0b
#endif
	.endm
#else
static inline void HEX_DISPLAY(int value)
{
    *IXDP425_LED_DATA = value;
}
#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_IXDP425_IXDP425_H
// EOF ixdp425.h
