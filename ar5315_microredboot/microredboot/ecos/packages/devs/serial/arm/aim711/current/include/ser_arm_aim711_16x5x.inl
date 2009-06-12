//==========================================================================
//
//      ser_arm_aim711_16x5x.inl
//
//      ARM Industrial Module AIM 711 Serial I/O Interface Module
//      (interrupt driven) for use with 16x5x driver.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2001-06-08
// Purpose:      Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>

//-----------------------------------------------------------------------------
// Baud rate specification, based on raw 7.3728 MHz clock

static unsigned short select_baud[] = {
    0,    // Unused
    9216, // 50
    6144, // 75
    4189, // 110
    3426, // 134.5
    3072, // 150
    2304, // 200
    1537, // 300
    768,  // 600
    384,  // 1200
    256,  // 1800
    192,  // 2400
    128,  // 3600
    96,   // 4800
    64,   // 7200
    48,   // 9600
    32,   // 14400
    24,   // 19200
    12,   // 38400
    8,    // 57600
    4,    // 115200
    2,    // 230400
    1,    // 460800
};

#ifdef CYGPKG_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0
static pc_serial_info aim711_16x5x_serial_info0 = {0x7fd0008,
                                         CYGNUM_HAL_INTERRUPT_EXT0};
#if CYGNUM_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_BUFSIZE > 0
static unsigned char aim711_16x5x_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_BUFSIZE];
static unsigned char aim711_16x5x_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(aim711_16x5x_serial_channel0,
                                       pc_serial_funs, 
                                       aim711_16x5x_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &aim711_16x5x_serial_out_buf0[0], sizeof(aim711_16x5x_serial_out_buf0),
                                       &aim711_16x5x_serial_in_buf0[0], sizeof(aim711_16x5x_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(aim711_16x5x_serial_channel0,
                      pc_serial_funs, 
                      aim711_16x5x_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(aim711_16x5x_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &aim711_16x5x_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_ARM_AIM711_16X5X_SERIAL0

// EOF ser_arm_aim711_16x5x.inl
