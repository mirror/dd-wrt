//==========================================================================
//
//      io/serial/powerpc/powerpc_ppc405_ser.inl
//
//      PPC405GP Serial I/O definitions
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: 
// Date:         2003-09-16
// Purpose:      PowerPC PPC405GP serial drivers
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>

//-----------------------------------------------------------------------------
// Baud rate specification

static unsigned int select_baud[] = {
    9999, // Unused -- marker
    50,
    75,
    110,
    134,
    150,
    200,
    300,
    600,
    1200,
    1800,
    2400,
    3600,
    4800,
    7200,
    9600,
    14400,
    19200,
    38400,
    57600,
    115200,
    230400
};

externC int cyg_var_baud_generator(int baud);
#define CYG_IO_SERIAL_GENERIC_16X5X_BAUD_GENERATOR cyg_var_baud_generator

#ifdef CYGPKG_IO_SERIAL_POWERPC_PPC405_SERIAL0
static pc_serial_info ppc405_serial_info0 = {_PPC405GP_UART0, CYGNUM_HAL_INTERRUPT_UART0};
#if CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL0_BUFSIZE > 0
static unsigned char ppc405_serial_out_buf0[CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL0_BUFSIZE];
static unsigned char ppc405_serial_in_buf0[CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL0_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(ppc405_serial_channel0,
                                       pc_serial_funs, 
                                       ppc405_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &ppc405_serial_out_buf0[0], sizeof(ppc405_serial_out_buf0),
                                       &ppc405_serial_in_buf0[0], sizeof(ppc405_serial_in_buf0)
    );
#else
static SERIAL_CHANNEL(ppc405_serial_channel0,
                      pc_serial_funs, 
                      ppc405_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(ppc405_serial_io0, 
             CYGDAT_IO_SERIAL_POWERPC_PPC405_SERIAL0_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &ppc405_serial_channel0
    );
#endif //  CYGPKG_IO_SERIAL_POWERPC_PPC405_SERIAL0

#ifdef CYGPKG_IO_SERIAL_POWERPC_PPC405_SERIAL1
static pc_serial_info ppc405_serial_info1 = {_PPC405GP_UART1, CYGNUM_HAL_INTERRUPT_UART1};
#if CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL1_BUFSIZE > 0
static unsigned char ppc405_serial_out_buf1[CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL1_BUFSIZE];
static unsigned char ppc405_serial_in_buf1[CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(ppc405_serial_channel1,
                                       pc_serial_funs, 
                                       ppc405_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &ppc405_serial_out_buf1[0], sizeof(ppc405_serial_out_buf1),
                                       &ppc405_serial_in_buf1[0], sizeof(ppc405_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(ppc405_serial_channel1,
                      pc_serial_funs, 
                      ppc405_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_POWERPC_PPC405_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(ppc405_serial_io1, 
             CYGDAT_IO_SERIAL_POWERPC_PPC405_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &ppc405_serial_channel1
    );
#endif //  CYGPKG_IO_SERIAL_POWERPC_PPC405_SERIAL1

// EOF powerpc_ppc405_ser.inl
