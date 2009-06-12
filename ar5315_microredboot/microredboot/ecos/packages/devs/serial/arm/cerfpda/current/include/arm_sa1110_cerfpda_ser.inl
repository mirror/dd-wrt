//==========================================================================
//
//      io/serial/arm/arm_sa1110_cerpda_ser.inl
//
//      Cerfpda Serial I/O definitions
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
// Author(s):    gthomas
// Contributors: gthomas, jlarmour
// Date:         1999-02-04
// Purpose:      Cerfpda Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/cerfpda.h>

//-----------------------------------------------------------------------------
// Baud rate specification

static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    18432,// 75
    12657,// 110
    10278,// 134.5
    9216, // 150
    6912, // 200
    4608, // 300
    2304, // 600
    1152, // 1200
    768,  // 1800
    576,  // 2400
    384,  // 3600
    288,  // 4800
    192,  // 7200
    144,  // 9600
    96,   // 14400
    72,   // 19200
    36,   // 38400
    24,   // 57600
    12,   // 115200
    6,    // 230400
};

#ifdef CYGPKG_IO_SERIAL_ARM_CERFPDA_SERIAL1
static pc_serial_info cerfpda_serial_info1 = {0x18000000, SA1110_IRQ_GPIO_16X5X};
#if CYGNUM_IO_SERIAL_ARM_CERFPDA_SERIAL1_BUFSIZE > 0
static unsigned char cerfpda_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_CERFPDA_SERIAL1_BUFSIZE];
static unsigned char cerfpda_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_CERFPDA_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(cerfpda_serial_channel1,
                                       pc_serial_funs, 
                                       cerfpda_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CERFPDA_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &cerfpda_serial_out_buf1[0], sizeof(cerfpda_serial_out_buf1),
                                       &cerfpda_serial_in_buf1[0], sizeof(cerfpda_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(cerfpda_serial_channel1,
                      pc_serial_funs, 
                      cerfpda_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_CERFPDA_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(cerfpda_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_CERFPDA_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &cerfpda_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_ARM_CERFPDA_SERIAL1

// EOF arm_sa1110_cerfpda_ser.inl
