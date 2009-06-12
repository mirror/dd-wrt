#ifndef CYGONCE_V85X_V850_SERIAL_H
#define CYGONCE_V85X_V850_SERIAL_H

// ====================================================================
//
//      v850_ceb_serial.h
//
//      Device I/O - Description of NEC V850 serial hardware
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
// Contributors: gthomas,jlarmour
// Date:         2001-03-21
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on NEC V850/SA1 & SB1

#include <pkgconf/system.h>
#include CYGBLD_HAL_TARGET_H

#include <cyg/hal/v850_common.h>

struct serial_port {
    unsigned char asim;        // Serial interface mode
    unsigned char _filler0;
    unsigned char asis;        // Serial interface status
    unsigned char _filler1;
    unsigned char brgc;        // Baud rate control
    unsigned char _filler2;
    unsigned char txs;         // Transmit shift register
    unsigned char _filler3;
    unsigned char rxs;         // Receive shift register
    unsigned char _filler4[5];
    unsigned char brgm;        // Baud rate mode
    unsigned char _filler5;
#if CYGINT_HAL_V850_VARIANT_SB1
    unsigned char _filler6[0x10];
    unsigned char brgm1;       // Baud rate overflow
#endif
};

// Relative interrupt numbers
#define INT_ERR 0              // Receive error condition
#define INT_Rx  1              // Receive data
#define INT_Tx  2              // Transmit data

// Serial interface mode
#define ASIM_TxRx_MASK     (3<<6)  // Receive & Transmit enables
#define ASIM_TxRx_Rx       (1<<6)  // Receive enable
#define ASIM_TxRx_Tx       (2<<6)  // Transmit enable
#define ASIM_Parity_MASK   (3<<4)  // Parity mode bits
#define ASIM_Parity_none   (0<<4)  // No parity
#define ASIM_Parity_space  (1<<4)  // Send zero bit, ignore errors
#define ASIM_Parity_odd    (2<<4)  // Odd parity
#define ASIM_Parity_even   (3<<4)  // Even parity
#define ASIM_Length_MASK   (1<<3)  // Character length select
#define ASIM_Length_7      (0<<3)  // 7 bit chars
#define ASIM_Length_8      (1<<3)  // 8 bit chars
#define ASIM_Stop_MASK     (1<<2)  // Stop bit select
#define ASIM_Stop_1        (0<<2)  // 1 stop bit
#define ASIM_Stop_2        (1<<2)  // 2 stop bits
#define ASIM_Error_MASK    (1<<1)  // Receive error select
#define ASIM_Error_enable  (0<<1)  // Issue interrupt on receive error
#define ASIM_Error_disable (1<<1)  // No interrupts on receive error

// Serial interface status (errors only)
#define ASIS_OVE           (1<<0)  // Overrun error
#define ASIS_FE            (1<<1)  // Framing error
#define ASIS_PE            (1<<2)  // Parity error

static unsigned char select_word_length[] = {
    0xFF,                         // 5 bits / word (char)
    0xFF,
    ASIM_Length_7,
    ASIM_Length_8
};

static unsigned char select_stop_bits[] = {
    0,
    ASIM_Stop_1,                  // 1 stop bit
    0xFF,                         // 1.5 stop bit
    ASIM_Stop_2                   // 2 stop bits
};

static unsigned char select_parity[] = {
    ASIM_Parity_none,             // No parity
    ASIM_Parity_even,             // Even parity
    ASIM_Parity_odd,              // Odd parity
    0xFF,                         // Mark parity
    ASIM_Parity_space,            // Space parity
};

static struct v850_baud {
    unsigned int count;
    unsigned int divisor;
} select_baud[] = {
// Baud rate values, using defined system clock
#define BAUDCOUNT(X) ((CYGHWR_HAL_V85X_CPU_FREQ/2)/(X))
      {0, 0},                  // Unused
      {0, 0},                  // 50
      {0, 0},                  // 75
      {0, 0},                  // 110
      {0, 0},                  // 134.5
      {0, 0},                  // 150
      {0, 0},                  // 200
      {0, 0},                  // 300
      {0, 0},                  // 600
      {BAUDCOUNT(1200), 1},    // 1200
      {0, 0},                  // 1800
      {BAUDCOUNT(2400), 1},    // 2400
      {0, 0},                  // 3600
      {BAUDCOUNT(4800), 1},    // 4800
      {0, 0},                  // 7200
      {BAUDCOUNT(9600), 1},    // 9600
      {0, 0},                  // 14400
      {BAUDCOUNT(19200), 1},   // 19200
      {BAUDCOUNT(38400), 1},   // 38400
      {0, 0},                  // 57600
      {0, 0},                  // 115200
      {0, 0},                  // 230400
};

#endif // CYGONCE_V85X_V850_SERIAL_H

// EOF v85x_v850_serial.h
