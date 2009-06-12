#ifndef CYGONCE_DEVS_SERIAL_POWERPC_EC555_SERIAL_H
#define CYGONCE_DEVS_SERIAL_POWERPC_EC555_SERIAL_H
//==========================================================================
//
//      ec555_serial.h
//
//      PowerPC 5xx EC555 Serial I/O definitions.
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
// Author(s):   Bob Koninckx
// Contributors:
// Date:        2002-04-25
// Purpose:     EC555 Serial I/O definitions.
// Description: 
//
//   
//####DESCRIPTIONEND####
//==========================================================================

//----------------------------------
// Includes and forward declarations
//----------------------------------

//----------------------
// Constants definitions
//----------------------
// Base addresses for the two serial ports
#define MPC555_SERIAL_BASE_A          0x305008
#define MPC555_SERIAL_BASE_B          0x305020

// The offset from the base for all serial registers
#define MPC555_SERIAL_SCCxR0          0
#define MPC555_SERIAL_SCCxR1          2
#define MPC555_SERIAL_SCxSR           4
#define MPC555_SERIAL_SCxDR           6

// The bits in the serial registers
#define MPC555_SERIAL_SCCxR0_OTHR     0x8000
#define MPC555_SERIAL_SCCxR0_LINKBD   0x4000
#define MPC555_SERIAL_SCCxR0_SCxBR    0x1fff

#define MPC555_SERIAL_SCCxR1_LOOPS    0x4000
#define MPC555_SERIAL_SCCxR1_WOMS     0x2000
#define MPC555_SERIAL_SCCxR1_ILT      0x1000
#define MPC555_SERIAL_SCCxR1_PT       0x0800
#define MPC555_SERIAL_SCCxR1_PE       0x0400
#define MPC555_SERIAL_SCCxR1_M        0x0200
#define MPC555_SERIAL_SCCxR1_WAKE     0x0100
#define MPC555_SERIAL_SCCxR1_TIE      0x0080
#define MPC555_SERIAL_SCCxR1_TCIE     0x0040
#define MPC555_SERIAL_SCCxR1_RIE      0x0020
#define MPC555_SERIAL_SCCxR1_ILIE     0x0010
#define MPC555_SERIAL_SCCxR1_TE       0x0008
#define MPC555_SERIAL_SCCxR1_RE       0x0004
#define MPC555_SERIAL_SCCxR1_RWU      0x0002
#define MPC555_SERIAL_SCCxR1_SBK      0x0001

#define MPC555_SERIAL_SCxSR_TDRE      0x0100
#define MPC555_SERIAL_SCxSR_TC        0x0080
#define MPC555_SERIAL_SCxSR_RDRF      0x0040
#define MPC555_SERIAL_SCxSR_RAF       0x0020
#define MPC555_SERIAL_SCxSR_IDLE      0x0010
#define MPC555_SERIAL_SCxSR_OR        0x0008
#define MPC555_SERIAL_SCxSR_NF        0x0004
#define MPC555_SERIAL_SCxSR_FE        0x0002
#define MPC555_SERIAL_SCxSR_PF        0x0001

// The available baud rates
// These are calculated for a busclock of 40 MHz
// It is not necessary to let the compiler calculate these
// values, we did not provide clockfrequency as a configuarion
// option anyway.
static unsigned short select_baud[] = {
    0,        // Unused
    0,        // 50     bps unsupported
    0,        // 75     bps unsupported
    0,        // 110    bps unsupported
    0,        // 134_5  bps unsupported
    0,        // 150    bps unsupported
    0,        // 200    bps unsupported
    4167,     // 300    bps
    2083,     // 600    bps
    1042,     // 1200   bps
    0,        // 1800   bps unsupported
    521,      // 2400   bps
    0,        // 3600   bps unsupported
    260,      // 4800   bps
    0,        // 7200   bps unsupported
    130,      // 9600   bps
    87,       // 14400  bps
    65,       // 19200  bps
    33,       // 38400  bps
    22,       // 57600  bps
    11,       // 115200 bps
    0         // 230400 bps unsupported
};

static unsigned char select_word_length[] = {
    0,        // 5 bits / word (char) not supported
    0,        // 6 bits / word (char) not supported
    7,        // 7 bits / word (char) ->> 7 bits per frame
    8         // 8 bits / word (char) ->> 8 bits per frame
};

static unsigned char select_stop_bits[] = {
    0,
    1,        // 1 stop bit ->> 1 bit per frame
    0,        // 1.5 stop bit not supported
    2         // 2 stop bits ->> 2 bits per frame 
};

static unsigned char select_parity[] = {
    0,        // No parity ->> 0 bits per frame
    1,        // Even parity ->> 1 bit per frame
    1,        // Odd parityv ->> 1 bit per frame
    0,        // Mark parity not supported
    0,        // Space parity not supported
};

#endif // CYGONCE_DEVS_SERIAL_POWERPC_EC555_SERIAL_H

// EOF ec555_serial.h
