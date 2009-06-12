#ifndef CYGONCE_DEVS_SH_EDK7708_SCI_H
#define CYGONCE_DEVS_SH_EDK7708_SCI_H

//==========================================================================
//
//      io/serial/sh/sh_sh3_edk7708_sci.inl
//
//      Serial I/O specification for Hitachi EDK7708 platform.
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
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-06-16
// Purpose:     Defines SCI serial resources for SH3/7708.
// Description: 
//
//####DESCRIPTIONEND####
//==========================================================================


#include <pkgconf/io_serial_sh_edk7708.h>

static sh_sci_info sh_serial_info =
{
    data       : CYGARC_REG_SCI_SCSPTR,
    er_int_num : CYGNUM_HAL_INTERRUPT_SCI_ERI,
    rx_int_num : CYGNUM_HAL_INTERRUPT_SCI_RXI,
    tx_int_num : CYGNUM_HAL_INTERRUPT_SCI_TXI,
    ctrl_base  : CYGARC_REG_SCI_SCSMR
};

#if CYGNUM_IO_SERIAL_SH_EDK7708_SERIAL1_BUFSIZE > 0
static unsigned char sh_serial_out_buf[CYGNUM_IO_SERIAL_SH_EDK7708_SERIAL1_BUFSIZE];
static unsigned char sh_serial_in_buf[CYGNUM_IO_SERIAL_SH_EDK7708_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sh_serial_channel,
                                       sh_serial_funs, 
                                       sh_serial_info,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_EDK7708_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sh_serial_out_buf[0], 
                                       sizeof(sh_serial_out_buf),
                                       &sh_serial_in_buf[0],  
                                       sizeof(sh_serial_in_buf)
    );
#else
static SERIAL_CHANNEL(sh_serial_channel,
                      sh_serial_funs, 
                      sh_serial_info,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_EDK7708_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sh_serial_io,
             CYGDAT_IO_SERIAL_SH_EDK7708_SERIAL1_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sh_serial_init, 
             sh_serial_lookup,          // Serial driver may need initializing
             &sh_serial_channel
    );

#endif // CYGONCE_DEVS_SH_EDK7708_SCI_H
