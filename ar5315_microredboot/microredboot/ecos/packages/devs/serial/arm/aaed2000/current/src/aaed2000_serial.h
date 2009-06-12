#ifndef CYGONCE_ARM_AAED2000_SERIAL_H
#define CYGONCE_ARM_AAED2000_SERIAL_H

// ====================================================================
//
//      aaed2000_serial.h
//
//      Device I/O - Description of Agilent AAED2000 serial hardware
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
// Author(s):   gthomas, jskov
// Contributors:gthomas, jskov
// Date:        2001-11-12
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

static unsigned char select_word_length[] = {
    AAEC_UART_LCR_WL5,    // 5 bits / word (char)
    AAEC_UART_LCR_WL6,
    AAEC_UART_LCR_WL7,
    AAEC_UART_LCR_WL8
};

static unsigned char select_stop_bits[] = {
    0,
    0,    // 1 stop bit
    AAEC_UART_LCR_S2,  // 1.5 stop bit (I think)
    AAEC_UART_LCR_S2     // 2 stop bits
};

static unsigned char select_parity[] = {
    0,     // No parity
    AAEC_UART_LCR_PEN|AAEC_UART_LCR_EP,     // Even parity
    AAEC_UART_LCR_PEN,     // Odd parity
    0,     // Mark parity
    0,     // Space parity
};

// Baud rate values, based on 7.3728MHz clock
// #define BAUD_RATE(_n_) ((7372800/((_n_)*16))-1)

static unsigned short select_baud[] = {
           0,  // Unused
      0x23ff,  // 50
      0x17ff,  // 75
      0x105c,  // 110
      0x0d60,  // 134.5
      0x0bff,  // 150
      0x08ff,  // 200
      0x05ff,  // 300
      0x02ff,  // 600
      0x017f,  // 1200
      0x00ff,  // 1800
      0x00bf,  // 2400
      0x007f,  // 3600
      0x005f,  // 4800
      0x003f,  // 7200
      0x002f,  // 9600
      0x001f,  // 14400
      0x0017,  // 19200
      0x000b,  // 38400
      0x0007,  // 57600
      0x0003,  // 115200
      0x0001,  // 230400
};

#endif // CYGONCE_ARM_AAED2000_SERIAL_H
