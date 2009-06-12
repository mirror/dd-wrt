#ifndef CYGONCE_ARM_SMDK2410_SERIAL_H
#define CYGONCE_ARM_SMDK2410_SERIAL_H

// ====================================================================
//
//      smdk2410_serial.h
//
//      Device I/O - Description of Samsung SMDK2410 serial hardware
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
// Author(s):    michael anburaj <michaelanburaj@hotmail.com>
// Contributors: michael anburaj <michaelanburaj@hotmail.com>
// Date:         2003-08-01
// Purpose:      Internal interfaces for serial I/O drivers
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// Baud rate divisor registers values
// (PCLK/16./BAUD_RATE+0.5) -1)
#define UBRDIV_50     ((PCLK/16./50)-1)
#define UBRDIV_75     ((PCLK/16./75)-1)
#define UBRDIV_110    ((PCLK/16./110)-1)
#define UBRDIV_134_5  ((PCLK/16./134.5)-1)
#define UBRDIV_150    ((PCLK/16./150)-1)
#define UBRDIV_200    ((PCLK/16./200)-1)
#define UBRDIV_300    ((PCLK/16./300)-1)
#define UBRDIV_600    ((PCLK/16./600)-1)
#define UBRDIV_1200   ((PCLK/16./1200)-1)
#define UBRDIV_1800   ((PCLK/16./1800)-1)
#define UBRDIV_2400   ((PCLK/16./2400)-1)
#define UBRDIV_3600   ((PCLK/16./3600)-1)
#define UBRDIV_4800   ((PCLK/16./4800)-1)
#define UBRDIV_7200   ((PCLK/16./7200)-1)
#define UBRDIV_9600   ((PCLK/16./9600)-1)
#define UBRDIV_14400  ((PCLK/16./14400)-1)
#define UBRDIV_19200  ((PCLK/16./19200)-1)
#define UBRDIV_38400  ((PCLK/16./38400)-1)
#define UBRDIV_57600  ((PCLK/16./57600)-1)
#define UBRDIV_115200 ((PCLK/16./115200)-1)
#define UBRDIV_230400 ((PCLK/16./230400)-1)

static unsigned char select_word_length[] = {
    VAL_ULCON_WL_5,
    VAL_ULCON_WL_6,
    VAL_ULCON_WL_7,
    VAL_ULCON_WL_8
};

static unsigned char select_stop_bits[] = {
    0,                 // Unused
    VAL_ULCON_SB_1,    // 1 stop bit
    0,                 // 1.5 stop bit (not supported)
    VAL_ULCON_SB_2     // 2 stop bits
};

static unsigned char select_parity[] = {
    VAL_ULCON_PM_N,     // No parity
    VAL_ULCON_PM_E,     // Even parity
    VAL_ULCON_PM_O,     // Odd parity
    VAL_ULCON_PM_FC1,   // Mark parity
    VAL_ULCON_PM_FC0,   // Space parity
};

// Baud rate values, based on PCLK
static unsigned short select_baud[] = {
            0,    // Unused
    UBRDIV_50,    // 50
    UBRDIV_75,    // 75
    UBRDIV_110,   // 110
    UBRDIV_134_5, // 134.5
    UBRDIV_150,   // 150
    UBRDIV_200,   // 200
    UBRDIV_300,   // 300
    UBRDIV_600,   // 600
    UBRDIV_1200,  // 1200
    UBRDIV_1800,  // 1800
    UBRDIV_2400,  // 2400
    UBRDIV_3600,  // 3600
    UBRDIV_4800,  // 4800
    UBRDIV_7200,  // 7200
    UBRDIV_9600,  // 9600
    UBRDIV_14400, // 14400
    UBRDIV_19200, // 19200
    UBRDIV_38400, // 38400
    UBRDIV_57600, // 57600
    UBRDIV_115200,// 115200
    UBRDIV_230400,// 230400
};

#endif // CYGONCE_ARM_SMDK2410_SERIAL_H
