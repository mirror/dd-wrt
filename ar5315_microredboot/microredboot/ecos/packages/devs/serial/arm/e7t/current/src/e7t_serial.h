#ifndef CYGONCE_ARM_E7T_SERIAL_H
#define CYGONCE_ARM_E7T_SERIAL_H
// ====================================================================
//
//      e7t_serial.h
//
//      Device I/O - Description of ARM AEB-2 serial hardware
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
// Author(s):    Lars.Lindqvist@combitechsystems.com
// Contributors: jlarmour
// Date:         2001-10-19
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

#include <pkgconf/hal.h>        // Value CYGNUM_HAL_ARM_E7T_CLOCK_SPEED needed
#include <cyg/infra/cyg_type.h> // base types

// Description of serial ports on ARM AEB-2

struct serial_port {
    cyg_uint32 _reg[8];
};

#define REG(n) _reg[n]

// Misc values
#define U_NOT_SUPP    (0xFFFFFFFF)  // Used to indicate unsupported parameter values

// Registers
#define REG_ULCON  REG(0)    // Line control registers
#define REG_UCON   REG(1)    // Control registers
#define REG_USTAT  REG(2)    // Status registers
#define REG_UTXBUF REG(3)    // Transmit buffer registers
#define REG_URXBUF REG(4)    // Receive buffer registers
#define REG_UBRDIV REG(5)    // Baud rate divisor registers

// Line Control Register Values
#define ULCON_WL5     (0x00000000 << 0)  // Word length 5
#define ULCON_WL6     (0x00000001 << 0)  // Word length 6
#define ULCON_WL7     (0x00000002 << 0)  // Word length 7
#define ULCON_WL8     (0x00000003 << 0)  // Word length 8
#define ULCON_STB1    (0x00000000 << 2)  // One stop bit
#define ULCON_STB2    (0x00000001 << 2)  // Two stop bits
#define ULCON_PMDOFF  (0x00000000 << 3)  // No parity
#define ULCON_PMDODD  (0x00000004 << 3)  // Odd parity
#define ULCON_PMDEVEN (0x00000005 << 3)  // Even parity
#define ULCON_PMDFC1  (0x00000006 << 3)  // Parity forced/checked as 1
#define ULCON_PMDFC0  (0x00000007 << 3)  // Parity forced/checked as 0
#define ULCON_SCI     (0x00000000 << 6)  // Internal clock
#define ULCON_SCE     (0x00000001 << 6)  // External clock
#define ULCON_IROFF   (0x00000000 << 7)  // Normal mode
#define ULCON_IRON    (0x00000001 << 7)  // IR mode

// Control Register Values
#define UCON_RXMOFF   (0x00000000 << 0)  // Disable Rx mode
#define UCON_RXMINT   (0x00000001 << 0)  // Interrupt request Rx mode
#define UCON_RXMDMA0  (0x00000002 << 0)  // GDMA channel 0 request Rx mode
#define UCON_RXMDMA1  (0x00000003 << 0)  // GDMA channel 1 request Rx mode
#define UCON_RXSIOFF  (0x00000000 << 2)  // Rx status interrupt disabled
#define UCON_RXSION   (0x00000001 << 2)  // Rx status interrupt enabled
#define UCON_TXMOFF   (0x00000000 << 3)  // Disable Tx mode
#define UCON_TXMINT   (0x00000001 << 3)  // Interrupt request Tx mode
#define UCON_TXMDMA0  (0x00000002 << 3)  // GDMA channel 0 request Tx mode
#define UCON_TXMDMA1  (0x00000003 << 3)  // GDMA channel 1 request Tx mode
#define UCON_DSROFF   (0x00000000 << 5)  // Data set ready output off
#define UCON_DSRON    (0x00000001 << 5)  // Data set ready output on
#define UCON_SBKOFF   (0x00000000 << 6)  // No break sent
#define UCON_SBKON    (0x00000001 << 6)  // Break sent
#define UCON_LPBOFF   (0x00000000 << 7)  // Loop back mode off
#define UCON_LPBON    (0x00000001 << 7)  // Loop back mode on

// Status Register Values
#define USTAT_OV      (0x00000001 << 0)  // Overrun error
#define USTAT_PE      (0x00000001 << 1)  // Parity error
#define USTAT_FE      (0x00000001 << 2)  // Frame error

#define USTAT_BKD     (0x00000001 << 3)  // Break detect
#define USTAT_DTR     (0x00000001 << 4)  // Data terminal ready
#define USTAT_RDR     (0x00000001 << 5)  // Receive data ready
#define USTAT_TBE     (0x00000001 << 6)  // Transmit buffer register empty
#define USTAT_TC      (0x00000001 << 7)  // Transmit complete

// Baud rate divisor registers
#define UBRDIV_50     ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/50)-1)<<4)|1)
#define UBRDIV_75     ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/75)-1)<<4)|1)
#define UBRDIV_110    ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/110)-1)<<4)|1)
#define UBRDIV_134_5  ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/8/269)-1)<<4)|1)
#define UBRDIV_150    ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/150)-1)<<4)|1)
#define UBRDIV_200    ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/200)-1)<<4)|1)
#define UBRDIV_300    ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/16/16/300)-1)<<4)|1)
#define UBRDIV_600    ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/600)-1)<<4)|0)
#define UBRDIV_1200   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/1200)-1)<<4)|0)
#define UBRDIV_1800   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/1800)-1)<<4)|0)
#define UBRDIV_2400   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/2400)-1)<<4)|0)
#define UBRDIV_3600   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/3600)-1)<<4)|0)
#define UBRDIV_4800   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/4800)-1)<<4)|0)
#define UBRDIV_7200   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/7200)-1)<<4)|0)
#define UBRDIV_9600   ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/9600)-1)<<4)|0)
#define UBRDIV_14400  ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/14400)-1)<<4)|0)
#define UBRDIV_19200  ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/19200)-1)<<4)|0)
#define UBRDIV_38400  ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/38400)-1)<<4)|0)
#define UBRDIV_57600  ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/57600)-1)<<4)|0)
#define UBRDIV_115200 ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/115200)-1)<<4)|0)
#define UBRDIV_230400 ((((CYGNUM_HAL_ARM_E7T_CLOCK_SPEED/2/1/16/230400)-1)<<4)|0)


// Arrays used for conversion of eCos serial driver
// configuration parameters to parameters for E7T

static cyg_uint32 select_word_length[] = {
    ULCON_WL5,    // 5 bits / word (char)
    ULCON_WL6,
    ULCON_WL7,
    ULCON_WL8
};

static cyg_uint32 select_stop_bits[] = {
    ULCON_STB1,    // 1 stop bit
    U_NOT_SUPP,    // 1.5 stop bit not supported
    ULCON_STB2     // 2 stop bits
};

static cyg_uint32 select_parity[] = {
    ULCON_PMDOFF,     // No parity
    ULCON_PMDEVEN,    // Even parity
    ULCON_PMDODD,     // Odd parity
    ULCON_PMDFC1,     // Mark parity
    ULCON_PMDFC0,     // Space parity
};

static cyg_uint32 select_baud[] = {
     UBRDIV_50,     // 50
     UBRDIV_75,     // 75
     UBRDIV_110,    // 110
     UBRDIV_134_5,  // 134.5
     UBRDIV_150,    // 150
     UBRDIV_200,    // 200
     UBRDIV_300,    // 300
     UBRDIV_600,    // 600
     UBRDIV_1200,   // 1200
     UBRDIV_1800,   // 1800
     UBRDIV_2400,   // 2400
     UBRDIV_3600,   // 3600
     UBRDIV_4800,   // 4800
     UBRDIV_7200,   // 7200
     UBRDIV_9600,   // 9600
     UBRDIV_14400,  // 14400
     UBRDIV_19200,  // 19200
     UBRDIV_38400,  // 38400
     UBRDIV_57600,  // 57600
     UBRDIV_115200, // 115200
     UBRDIV_230400, // 230400
};

#endif // CYGONCE_ARM_E7T_SERIAL_H

// EOF e7t_serial.h
