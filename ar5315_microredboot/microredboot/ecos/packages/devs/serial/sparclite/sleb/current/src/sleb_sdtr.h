#ifndef CYGONCE_SLEB_SDTR_H
#define CYGONCE_SLEB_SDTR_H
//==========================================================================
//
//      io/serial/sparclite/sleb_sdtr.c
//
//      Serial I/O interface module for SPARClite Eval Board (SLEB)
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
// Author(s):   gthomas
// Contributors:  gthomas
// Date:        1999-02-04
// Purpose:     SLEB serial I/O module
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_io.h>             // For I/O macros

#define reg(n)                   ((n)*4)

// SDTR Registers
#define SDTR_TXDATA(base)        base+reg(0)
#define SDTR_RXDATA(base)        base+reg(0)
#define SDTR_STATUS(base)        base+reg(1)
#define SDTR_CONTROL(base)       base+reg(1)

// Mode register
#define SDTR_MODE_MODE_MASK      0x03       // Mode selection bits (mask)
#define SDTR_MODE_MODE_SYNC      0x00       // Synchronous mode
#define SDTR_MODE_MODE_ASYNC1    0x01       // Async - clock/1
#define SDTR_MODE_MODE_ASYNC16   0x02       // Async - clock/16
#define SDTR_MODE_MODE_ASYNC64   0x03       // Async - clock/64
#define SDTR_MODE_DTB_MASK       0x0C       // Number of data bits (mask)
#define SDTR_MODE_DTB_5          0x00       //   5 bits / char
#define SDTR_MODE_DTB_6          0x04       //   6 bits / char
#define SDTR_MODE_DTB_7          0x08       //   7 bits / char
#define SDTR_MODE_DTB_8          0x0C       //   8 bits / char
#define SDTR_MODE_PARITY_MASK    0x30       // Parity modes (mask)
#define SDTR_MODE_PARITY_ENABLE  0x10       // Enable parity
#define SDTR_MODE_PARITY_NONE    0x00       // No parity (parity disabled)
#define SDTR_MODE_PARITY_ODD     0x00       // Odd parity
#define SDTR_MODE_PARITY_EVEN    0x20       // Even parity
#define SDTR_MODE_STOP_BITS_MASK 0xC0       // Number of stop bits (mask)
#define SDTR_MODE_STOP_BITS_1    0x40       //   1 stop bit
#define SDTR_MODE_STOP_BITS_1_5  0x80       //   1.5 stop bits
#define SDTR_MODE_STOP_BITS_2    0xC0       //   2 stop bits

// Command register
#define SDTR_CMD_TxEN            0x01       // Enable transmitter
#define SDTR_CMD_DTR             0x02       // Assert DTR
#define SDTR_CMD_RxEN            0x04       // Enable receiver
#define SDTR_CMD_BREAK           0x08       // Send break
#define SDTR_CMD_EFR             0x10       // Error flag reset
#define SDTR_CMD_RTS             0x20       // Assert RTS
#define SDTR_CMD_RST             0x40       // Internal RESET
#define SDTR_CMD_EHM             0x80       // Enable Hunt mode

// Status register
#define SDTR_STAT_TxRDY          0x01       // Transmitter ready
#define SDTR_STAT_RxRDY          0x02       // Receiver ready
#define SDTR_STAT_TxEMP          0x04       // Transmitter empty
#define SDTR_STAT_PERR           0x08       // Parity error
#define SDTR_STAT_OERR           0x10       // Overrun error
#define SDTR_STAT_FERR           0x20       // Framing error
#define SDTR_STAT_SYBRK          0x40       // Break
#define SDTR_STAT_DSR            0x80       // State of DSR signal

// Offsets to standard SDTR elements
#define SLEB_SDTR0_BASE    (8*4)
#define SLEB_SDTR0_TX_INT  9
#define SLEB_SDTR0_RX_INT  10
#define SLEB_SDTR1_BASE    (12*4)
#define SLEB_SDTR1_TX_INT  6
#define SLEB_SDTR1_RX_INT  7
#define SLEB_TIMER3_CONTROL reg(29)
#define SLEB_TIMER3_RELOAD  reg(30)

// On-board switch, used to determine baud rate
#define SLEB_CLOCK_SWITCH   (volatile unsigned char *)0x01000003

static unsigned char select_word_length[] = {
    SDTR_MODE_DTB_5,    // 5 bits / word (char)
    SDTR_MODE_DTB_6,
    SDTR_MODE_DTB_7,
    SDTR_MODE_DTB_8
};

static unsigned char select_stop_bits[] = {
    0,
    SDTR_MODE_STOP_BITS_1,    // 1 stop bit
    SDTR_MODE_STOP_BITS_1_5,  // 1.5 stop bit
    SDTR_MODE_STOP_BITS_2     // 2 stop bits
};

static unsigned char select_parity[] = {
    SDTR_MODE_PARITY_NONE,                             // No parity
    SDTR_MODE_PARITY_ENABLE|SDTR_MODE_PARITY_EVEN,     // Even parity
    SDTR_MODE_PARITY_ENABLE|SDTR_MODE_PARITY_ODD,      // ODD parity
    0xFF,                                              // Mark parity
    0xFF,                                              // Space parity
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

#endif // CYGONCE_SLEB_SDTR_H

