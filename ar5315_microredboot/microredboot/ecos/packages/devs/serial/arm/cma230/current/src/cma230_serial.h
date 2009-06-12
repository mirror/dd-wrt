#ifndef CYGONCE_ARM_CMA230_SERIAL_H
#define CYGONCE_ARM_CMA230_SERIAL_H

// ====================================================================
//
//      cma230_serial.h
//
//      Device I/O - Description of Cogent CMA230 serial hardware
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
// Author(s):           gthomas
// Contributors:        gthomas
// Date:        1999-05-20
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports on Cogent CMA230

struct serial_port {
    unsigned char _byte[32];
};

#define reg(n) _byte[n*8]

// Receive control registers
#define rhr reg(0)    // Receive holding register
#define isr reg(2)    // Interrupt status register
#define lsr reg(5)    // Line status register
#define msr reg(6)    // Modem status register
#define scr reg(7)    // Scratch register

// Transmit control registers
#define thr reg(0)    // Transmit holding register
#define ier reg(1)    // Interrupt enable register
#define fcr reg(2)    // FIFO control register
#define lcr reg(3)    // Line control register
#define mcr reg(4)    // Modem control register
#define ldl reg(0)    // LSB of baud rate
#define mdl reg(1)    // MSB of baud rate

// Interrupt Enable Register
#define IER_RCV 0x01
#define IER_XMT 0x02
#define IER_LS  0x04
#define IER_MS  0x08

// Line Control Register
#define LCR_WL5 0x00    // Word length
#define LCR_WL6 0x01
#define LCR_WL7 0x02
#define LCR_WL8 0x03
#define LCR_SB1 0x00    // Number of stop bits
#define LCR_SB1_5 0x04  // 1.5 -> only valid with 5 bit words
#define LCR_SB2 0x04
#define LCR_PN  0x00    // Parity mode - none
#define LCR_PE  0x0C    // Parity mode - even
#define LCR_PO  0x08    // Parity mode - odd
#define LCR_PM  0x28    // Forced "mark" parity
#define LCR_PS  0x38    // Forced "space" parity
#define LCR_DL  0x80    // Enable baud rate latch

// Line Status Register
#define LSR_RSR 0x01
#define LSR_THE 0x20

// Modem Control Register
#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_INT 0x08   // Enable interrupts

// Interrupt status register
#define ISR_Tx  0x02
#define ISR_Rx  0x04

static unsigned char select_word_length[] = {
    LCR_WL5,    // 5 bits / word (char)
    LCR_WL6,
    LCR_WL7,
    LCR_WL8
};

static unsigned char select_stop_bits[] = {
    0,
    LCR_SB1,    // 1 stop bit
    LCR_SB1_5,  // 1.5 stop bit
    LCR_SB2     // 2 stop bits
};

static unsigned char select_parity[] = {
    LCR_PN,     // No parity
    LCR_PE,     // Even parity
    LCR_PO,     // Odd parity
    LCR_PM,     // Mark parity
    LCR_PS,     // Space parity
};

// The Cogent board has a 3.6864 MHz crystal
static unsigned short select_baud[] = {
    0,    // Unused
    4608, // 50
    0,    // 75
    2094, // 110
    0,    // 134.5
    1536, // 150
    0,    // 200
    768,  // 300
    384,  // 600
    182,  // 1200
    0,    // 1800
    96,   // 2400
    0,    // 3600
    48,   // 4800
    32,   // 7200
    24,   // 9600
    16,   // 14400
    12,   // 19200
    6,    // 38400
    4,    // 57600
    2,    // 115200
    0,    // 230400
};

#endif // CYGONCE_ARM_CMA230_SERIAL_H
