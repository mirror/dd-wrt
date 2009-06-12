#ifndef CYGONCE_DEVS_SH_SE77X9_16X5X_H
#define CYGONCE_DEVS_SH_SE77X9_16X5X_H

//==========================================================================
//
//      io/serial/sh/sh_sh3_se77x9_16x5x.inl
//
//      Serial I/O specification for Hitachi SE77X9 platform.
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
// Author(s):   jskov
// Contributors:jskov
// Date:        2001-06-18
// Purpose:     Specifies serial resources for the platform.
// Description: This file can be include from the 16x5x driver sources.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/io_serial_sh_se77x9.h>

//-----------------------------------------------------------------------------
// Baud rate specification, based on raw 1.8462MHz clock (/16)

static unsigned short select_baud[] = {
           0,  // Unused
        2307,  // 50
        1538,  // 75
        1048,  // 110
         857,  // 134.5
         769,  // 150
         576,  // 200
         384,  // 300
         192,  // 600
          96,  // 1200
          64,  // 1800
          48,  // 2400
          32,  // 3600
          24,  // 4800
          16,  // 7200
          12,  // 9600
           8,  // 14400
           6,  // 19200
           3,  // 38400
           2,  // 57600
           1,  // 115200
           0,  // 230400
};

#ifdef CYGPKG_IO_SERIAL_SH_SE77X9_COM1
static pc_serial_info se77x9_serial_info1 = {0xb04007f0, CYGNUM_HAL_INTERRUPT_COM1};
#if CYGNUM_IO_SERIAL_SH_SE77X9_COM1_BUFSIZE > 0
static unsigned char se77x9_serial_out_buf1[CYGNUM_IO_SERIAL_SH_SE77X9_COM1_BUFSIZE];
static unsigned char se77x9_serial_in_buf1[CYGNUM_IO_SERIAL_SH_SE77X9_COM1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(se77x9_serial_channel1,
                                       pc_serial_funs, 
                                       se77x9_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_SE77X9_COM1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &se77x9_serial_out_buf1[0], sizeof(se77x9_serial_out_buf1),
                                       &se77x9_serial_in_buf1[0], sizeof(se77x9_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(se77x9_serial_channel1,
                      pc_serial_funs, 
                      se77x9_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_SE77X9_COM1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(se77x9_serial_io1, 
             CYGDAT_IO_SERIAL_SH_SE77X9_COM1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &se77x9_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_SH_SE77X9_COM1

#endif // CYGONCE_DEVS_SH_SE77X9_SCIF_H
