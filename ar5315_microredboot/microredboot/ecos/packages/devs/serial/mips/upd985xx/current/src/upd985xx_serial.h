#ifndef CYGONCE_MIPS_UPD985XX_SERIAL_H
#define CYGONCE_MIPS_UPD985XX_SERIAL_H
// ====================================================================
//
//      upd985xx_serial.h
//
//      Device I/O - Description of NEC MIPS uPD985xx serial hardware
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: gthomas
// Date:         2001-07-17
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on NEC MIPS uPD985xx

#include <cyg/hal/hal_arch.h>  // Register definitions

static unsigned char select_word_length[] = {
    0xFF,                               // 5 bits / word (char)
    0xFF,                               // 6
    UARTLCR_7,                          // 7
    UARTLCR_8                           // 8
};

static unsigned char select_stop_bits[] = {
    0xFF,                               // N/A
    UARTLCR_STB1,                       // 1 stop bit
    0xFF,                               // 1.5 stop bit
    UARTLCR_STB2                        // 2 stop bits
};

static unsigned char select_parity[] = {
    UARTLCR_NOP,                        // No parity
    UARTLCR_EP,                         // Even parity
    UARTLCR_OP,                         // Odd parity
    0xFF,                               // Mark parity
    0xFF,                               // Space parity
};

static cyg_int32 select_baud[] = {
    0,      // Unused
    UARTDLL_VAL( 50     ),              // 50
    UARTDLL_VAL( 75     ),              // 75
    UARTDLL_VAL( 110    ),              // 110
    0,                                  // 134.5
    UARTDLL_VAL( 150    ),              // 150
    UARTDLL_VAL( 200    ),              // 200
    UARTDLL_VAL( 300    ),              // 300
    UARTDLL_VAL( 600    ),              // 600
    UARTDLL_VAL( 1200   ),              // 1200
    UARTDLL_VAL( 1800   ),              // 1800
    UARTDLL_VAL( 2400   ),              // 2400
    UARTDLL_VAL( 3600   ),              // 3600
    UARTDLL_VAL( 4800   ),              // 4800
    UARTDLL_VAL( 7200   ),              // 7200
    UARTDLL_VAL( 9600   ),              // 9600
    UARTDLL_VAL( 14400  ),              // 14400
    UARTDLL_VAL( 19200  ),              // 19200
    UARTDLL_VAL( 38400  ),              // 38400
    UARTDLL_VAL( 57600  ),              // 57600
    UARTDLL_VAL( 115200 ),              // 115200
    UARTDLL_VAL( 230400 ),              // 230400
};

#endif // CYGONCE_MIPS_UPD985XX_SERIAL_H

// ------------------------------------------------------------------------
// EOF upd985xx_serial.h
