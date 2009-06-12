#ifndef CYGONCE_HAL_ARM_XSCALE_PRPMC1100_PRPMC1100_H
#define CYGONCE_HAL_ARM_XSCALE_PRPMC1100_PRPMC1100_H

/*=============================================================================
//
//      prpmc1100.h
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
// Date:         2003-03-27
// Purpose:      Intel PRPMC1100 specific support routines
// Description: 
// Usage:        #include <cyg/hal/prpmc1100.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_prpmc1100.h>
#include <cyg/hal/hal_ixp425.h>

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_PHYS_BASE                    0x00000000
#define SDRAM_BASE                         0x00000000
#define SDRAM_UNCACHED_BASE                0x10000000
#define SDRAM_SIZE                         0x02000000  // 32MB

// CS0 (flash optimum timing)
#define IXP425_EXP_CS0_INIT \
 (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
  EXP_RECOVERY_T(15) | EXP_SZ_16M | EXP_WR_EN | EXP_BYTE_RD16 | EXP_CS_EN)

#if 0
#define IXP425_EXP_CS7_INIT \
 (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
  EXP_RECOVERY_T(15) | EXP_SZ_512 | EXP_WR_EN | EXP_CS_EN)
#endif

#define IXP425_SDRAM_CONFIG_INIT  (SDRAM_CONFIG_CAS_3 | SDRAM_CONFIG_4x32Mx16)
#define IXP425_SDRAM_REFRESH_CNT  0x081
#define IXP425_SDRAM_SET_MODE_CMD SDRAM_IR_MODE_SET_CAS3

// control register
#define PRPMC_CTL_REG        REG16(0, 0x57000000)
#define PRPMC_CTL_EREADY     1
#define PRPMC_CTL_RESETOUT   2
#define PRPMC_CTL_INTN_GPIO  4

// status register
#define PRPMC_STS_REG        REG16(0, 0x57000002)
#define PRPMC_STS_RUN_LED    1
#define PRPMC_STS_FAIL_LED   2

// ------------------------------------------------------------------------
// GPIO lines

// lines used for 2-wire interface to EEPROM
#define GPIO_EEPROM_SCL 6
#define GPIO_EEPROM_SDA 7


// ------------------------------------------------------------------------
// No Hex Display
//
#ifdef __ASSEMBLER__
        // Display hex digits in 'value' not masked by 'mask'.
	.macro DISPLAY value, reg0, reg1
	.endm
#else
static inline void HEX_DISPLAY(int value)
{
}
#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

#endif // CYGONCE_HAL_ARM_XSCALE_PRPMC1100_PRPMC1100_H
// EOF prpmc1100.h
