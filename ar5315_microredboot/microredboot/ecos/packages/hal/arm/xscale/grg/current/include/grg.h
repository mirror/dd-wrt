#ifndef CYGONCE_HAL_ARM_XSCALE_GRG_GRG_H
#define CYGONCE_HAL_ARM_XSCALE_GRG_GRG_H

/*=============================================================================
//
//      grg.h
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
// Date:         2003-02-05
// Purpose:      Intel GRG specific support routines
// Description: 
// Usage:        #include <cyg/hal/grg.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <pkgconf/hal_arm_xscale_grg.h>
#include <cyg/hal/hal_ixp425.h>

// These must match setup in the page table in hal_platform_extras.h
#define SDRAM_PHYS_BASE                    0x00000000
#define SDRAM_BASE                         0x00000000
#define SDRAM_ALIAS_BASE                   0x10000000
#define SDRAM_UNCACHED_BASE                0x20000000
#define SDRAM_DC_BASE                      0x30000000
#define SDRAM_SIZE                         0x02000000  // 32MB

#define IXDP_FLASH_BASE                    0x50000000
#define IXDP_FLASH_SIZE                    0x01000000
#define IXDP_FLASH_DC_BASE                 0xA0000000

// CS0 (flash optimum timing)
#define IXP425_EXP_CS0_INIT \
 (EXP_ADDR_T(3) | EXP_SETUP_T(3) | EXP_STROBE_T(15) | EXP_HOLD_T(3) | \
  EXP_RECOVERY_T(15) | EXP_SZ_16M | EXP_WR_EN | EXP_BYTE_RD16 | EXP_CS_EN)

#define IXP425_SDRAM_CONFIG_INIT  (SDRAM_CONFIG_CAS_3 | SDRAM_CONFIG_2x8Mx16)
#define IXP425_SDRAM_REFRESH_CNT  0x81A
#define IXP425_SDRAM_SET_MODE_CMD SDRAM_IR_MODE_SET_CAS3


// ------------------------------------------------------------------------
// GPIO lines
//
#define GPIO_PWR_FAIL_IRQ_N 0
#define GPIO_DSL_IRQ_N      1
#define GPIO_SLIC_A_IRQ_N   2
#define GPIO_SLIC_B_IRQ_N   3
#define GPIO_DSP_IRQ_N      4
#define GPIO_IDE_IRQ_N      5

// GPIO lines used for SPI bus
#define GPIO_SPI_CS1_N      7
#define GPIO_SPI_CS0_N      8
#define GPIO_SPI_SCK        9
#define GPIO_SPI_SDI       10
#define GPIO_SPI_SDO       13

#define GPIO_IO_RESET_N    12

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

#endif // CYGONCE_HAL_ARM_XSCALE_GRG_GRG_H
// EOF grg.h
