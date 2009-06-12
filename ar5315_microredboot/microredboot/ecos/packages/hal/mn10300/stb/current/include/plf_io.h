#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
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
// Author(s):    nickg
// Contributors: dhowells
// Date:         2001-08-02
// Purpose:      STB platform IO support
// Description:
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef __ASSEMBLER__
#define HAL_REG_8(x)              x
#define HAL_REG_16(x)             x
#define HAL_REG_32(x)             x
#else
#define HAL_REG_8(x)              (volatile cyg_uint8*)(x)
#define HAL_REG_16(x)             (volatile cyg_uint16*)(x)
#define HAL_REG_32(x)             (volatile cyg_uint32*)(x)
#endif

//-----------------------------------------------------------------------------

/* STB GPIO Registers */
#define HAL_GPIO_BASE                           0xDB000000

#define HAL_GPIO_0_MODE_OFFSET                  0x0000
#define HAL_GPIO_0_IN_OFFSET                    0x0004
#define HAL_GPIO_0_OUT_OFFSET                   0x0008
#define HAL_GPIO_1_MODE_OFFSET                  0x0100
#define HAL_GPIO_1_IN_OFFSET                    0x0104
#define HAL_GPIO_1_OUT_OFFSET                   0x0108
#define HAL_GPIO_2_MODE_OFFSET                  0x0200
#define HAL_GPIO_2_IN_OFFSET                    0x0204
#define HAL_GPIO_2_OUT_OFFSET                   0x0208
#define HAL_GPIO_3_MODE_OFFSET                  0x0300
#define HAL_GPIO_3_IN_OFFSET                    0x0304
#define HAL_GPIO_3_OUT_OFFSET                   0x0308
#define HAL_GPIO_4_MODE_OFFSET                  0x0400
#define HAL_GPIO_4_IN_OFFSET                    0x0404
#define HAL_GPIO_4_OUT_OFFSET                   0x0408
#define HAL_GPIO_5_MODE_OFFSET                  0x0500
#define HAL_GPIO_5_IN_OFFSET                    0x0504
#define HAL_GPIO_5_OUT_OFFSET                   0x0508

#define HAL_GPIO_0_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_0_MODE_OFFSET)
#define HAL_GPIO_0_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_0_IN_OFFSET)
#define HAL_GPIO_0_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_0_OUT_OFFSET)
#define HAL_GPIO_1_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_1_MODE_OFFSET)
#define HAL_GPIO_1_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_1_IN_OFFSET)
#define HAL_GPIO_1_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_1_OUT_OFFSET)
#define HAL_GPIO_2_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_2_MODE_OFFSET)
#define HAL_GPIO_2_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_2_IN_OFFSET)
#define HAL_GPIO_2_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_2_OUT_OFFSET)
#define HAL_GPIO_3_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_3_MODE_OFFSET)
#define HAL_GPIO_3_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_3_IN_OFFSET)
#define HAL_GPIO_3_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_3_OUT_OFFSET)
#define HAL_GPIO_4_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_4_MODE_OFFSET)
#define HAL_GPIO_4_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_4_IN_OFFSET)
#define HAL_GPIO_4_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_4_OUT_OFFSET)
#define HAL_GPIO_5_MODE                         HAL_REG_16 (HAL_GPIO_BASE + HAL_GPIO_5_MODE_OFFSET)
#define HAL_GPIO_5_IN                           HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_5_IN_OFFSET)
#define HAL_GPIO_5_OUT                          HAL_REG_8  (HAL_GPIO_BASE + HAL_GPIO_5_OUT_OFFSET)

//-----------------------------------------------------------------------------
#define HAL_LED_ADDRESS                         0x83f90000
#define HAL_GPIO_MODE_ALL_OUTPUT                0x5555


#ifdef __ASSEMBLER__

#  include <cyg/hal/platform.inc>
#  define DEBUG_DISPLAY(hexdig)   hal_diag_led hexdig
#  define DEBUG_DELAY()                                        \
     mov    0x20000, d0;                                       \
0:   sub    1, d0;                                             \
     bne    0b;                                                \
     nop

#else

extern cyg_uint8 cyg_hal_plf_led_val(CYG_WORD hexdig);
#  define DEBUG_DISPLAY(hexdig) HAL_WRITE_UINT8(HAL_LED_ADDRESS, cyg_hal_plf_led_val(hexdig))
#  define DEBUG_DELAY()                                        \
   {                                                           \
     volatile int i = 0x80000;                                 \
     while (--i) ;                                             \
   }

#endif

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
