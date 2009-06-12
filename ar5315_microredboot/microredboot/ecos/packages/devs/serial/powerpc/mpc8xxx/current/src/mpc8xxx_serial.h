#ifndef CYGONCE_POWERPC_MPC8XXX_SERIAL_H
#define CYGONCE_POWERPC_MPC8XXX_SERIAL_H

// ====================================================================
//
//      mpc8xxx_serial.h
//
//      Device I/O - Description of PowerPC MPC8XXX (QUICC-II) serial hardware
//
// ====================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:                1999-06-21
// Purpose:     Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Description of serial ports using MPC8XXX (QUICC-II)

// SMC Mode Register
#define MPC8XXX_SMCMR_CLEN(n)   ((n+1)<<11)   // Character length
#define MPC8XXX_SMCMR_SB(n)     ((n-1)<<10)   // Stop bits (1 or 2)
#define MPC8XXX_SMCMR_PE(n)     (n<<9)        // Parity enable (0=disable, 1=enable)
#define MPC8XXX_SMCMR_PM(n)     (n<<8)        // Parity mode (0=odd, 1=even)
#define MPC8XXX_SMCMR_UART      (2<<4)        // UART mode
#define MPC8XXX_SMCMR_TEN       (1<<1)        // Enable transmitter
#define MPC8XXX_SMCMR_REN       (1<<0)        // Enable receiver

static unsigned int smc_select_word_length[] = {
    MPC8XXX_SMCMR_CLEN(5),  // 5 bits / word (char)
    MPC8XXX_SMCMR_CLEN(6),
    MPC8XXX_SMCMR_CLEN(7),
    MPC8XXX_SMCMR_CLEN(8)
};

static unsigned int smc_select_stop_bits[] = {
    0, 
    MPC8XXX_SMCMR_SB(1),   // 1 stop bit
    MPC8XXX_SMCMR_SB(1),   // 1.5 stop bit
    MPC8XXX_SMCMR_SB(2)    // 2 stop bits
};

static unsigned int smc_select_parity[] = {
    MPC8XXX_SMCMR_PE(0),                     // No parity
    MPC8XXX_SMCMR_PE(1)|MPC8XXX_SMCMR_PM(1),   // Even parity
    MPC8XXX_SMCMR_PE(1)|MPC8XXX_SMCMR_PM(0),   // Odd parity
    0,                                     // Mark parity
    0,                                     // Space parity
};

// SCC PSMR masks ....
#define MPC8XXX_SCC_PSMR_ASYNC   0x8000
#define MPC8XXX_SCC_PSMR_SB(n)   ((n-1)<<14)  // Stop bits (1=1sb, 2=2sb)
#define MPC8XXX_SCC_PSMR_CLEN(n) ((n-5)<<12)  // Character Length (5-8)
#define MPC8XXX_SCC_PSMR_PE(n)   (n<<4)       // Parity enable(0=disabled, 1=enabled)
#define MPC8XXX_SCC_PSMR_RPM(n)  (n<<2)       // Rx Parity mode (0=odd,  1=low, 2=even, 3=high)
#define MPC8XXX_SCC_PSMR_TPM(n)  (n)          // Tx Parity mode (0=odd,  1=low, 2=even, 3=high)

static unsigned int scc_select_word_length[] = {
  MPC8XXX_SCC_PSMR_CLEN(5),  // 5 bits / word (char)
  MPC8XXX_SCC_PSMR_CLEN(6),
  MPC8XXX_SCC_PSMR_CLEN(7),
  MPC8XXX_SCC_PSMR_CLEN(8)
};

static unsigned int scc_select_stop_bits[] = {
  MPC8XXX_SCC_PSMR_SB(1),   // 0.5 stop bit ??
  MPC8XXX_SCC_PSMR_SB(1),   // 1   stop bit
  MPC8XXX_SCC_PSMR_SB(2),   // 1.5 stop bit
  MPC8XXX_SCC_PSMR_SB(2)    // 2   stop bits
};


static unsigned int scc_select_parity[] = {
  MPC8XXX_SCC_PSMR_PE(0),                                               // No parity
  MPC8XXX_SCC_PSMR_PE(1)|MPC8XXX_SCC_PSMR_TPM(2)|MPC8XXX_SCC_PSMR_RPM(2), // Even parity
  MPC8XXX_SCC_PSMR_PE(1)|MPC8XXX_SCC_PSMR_TPM(0)|MPC8XXX_SCC_PSMR_RPM(0), // Odd parity
  MPC8XXX_SCC_PSMR_PE(1)|MPC8XXX_SCC_PSMR_TPM(3)|MPC8XXX_SCC_PSMR_RPM(3), // High (mark) parity
  MPC8XXX_SCC_PSMR_PE(1)|MPC8XXX_SCC_PSMR_TPM(1)|MPC8XXX_SCC_PSMR_RPM(1), // Low (space) parity
};

// Baud rate values, based on board clock

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
    0,      // 230400
};

#define UART_BITRATE(n) \
    ((((int)(((CYGHWR_HAL_POWERPC_CPM_SPEED*2)*1000000)/16))/(n * 16))-1)

// Channel type select
#define _SCC_CHAN 0
#define _SMC_CHAN 1

#endif // CYGONCE_POWERPC_MPC8XXX_SERIAL_H
