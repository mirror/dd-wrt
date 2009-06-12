#ifndef CYGONCE_ARM_SA11X0_SERIAL_H
#define CYGONCE_ARM_SA11X0_SERIAL_H

// ====================================================================
//
//      sa11x0_serial.h
//
//      Device I/O - Description of StrongARM SA11x0 serial hardware
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-05-08
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on StrongARM SA11x0

#include <cyg/hal/hal_sa11x0.h>  // Register definitions

struct serial_port {
    unsigned long ctl0;   // 0x00
    unsigned long ctl1;   // 0x04
    unsigned long ctl2;   // 0x08
    unsigned long ctl3;   // 0x0C
    unsigned long _unused0;
    unsigned long data;   // 0x14
    unsigned long _unused1;
    unsigned long stat0;  // 0x1C
    unsigned long stat1;  // 0x20
};

#define SA11X0_UART_RX_INTS (SA11X0_UART_RX_SERVICE_REQUEST|SA11X0_UART_RX_IDLE)

static unsigned char select_word_length[] = {
    0xFF,                         // 5 bits / word (char)
    0xFF,
    SA11X0_UART_DATA_BITS_7,
    SA11X0_UART_DATA_BITS_8
};

static unsigned char select_stop_bits[] = {
    0,
    SA11X0_UART_STOP_BITS_1,      // 1 stop bit
    0xFF,                         // 1.5 stop bit
    SA11X0_UART_STOP_BITS_2       // 2 stop bits
};

static unsigned char select_parity[] = {
    SA11X0_UART_PARITY_DISABLED,  // No parity
    SA11X0_UART_PARITY_ENABLED|   // Even parity
    SA11X0_UART_PARITY_EVEN,
    SA11X0_UART_PARITY_ENABLED|   // Odd parity
    SA11X0_UART_PARITY_ODD,
    0xFF,                         // Mark parity
    0xFF,                         // Space parity
};

static cyg_int32 select_baud[] = {
    0,      // Unused
    50,     // 50
    75,     // 75
    110,    // 110
    0,      // 134.5
    150,    // 150
    200,    // 200
    300,    // 300
    600,    // 600
    1200,   // 1200
    1800,   // 1800
    2400,   // 2400
    3600,   // 3600
    4800,   // 4800
    7200,   // 7200
    9600,   // 9600
    14400,  // 14400
    19200,  // 19200
    38400,  // 38400
    57600,  // 57600
    115200, // 115200
    230400, // 230400
};

#endif // CYGONCE_ARM_SA11X0_SERIAL_H
