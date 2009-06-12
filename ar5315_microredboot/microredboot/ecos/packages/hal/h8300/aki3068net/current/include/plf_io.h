#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      platform specific IO support
//
//=============================================================================
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
// Author(s):    ysato
// Contributors: ysato
// Date:         2003-02-28
// Purpose:      aki3068net + CF IO support
// Description:  The macros defined here provide the HAL APIs for handling
//               basic IO 
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

//-----------------------------------------------------------------------------
// IDE interface macros
//
#define HAL_IDE_NUM_CONTROLLERS 1

// Initialize the IDE controller(s).
#define HAL_IDE_INIT() aki3068net_ide_setup()

#define HAL_IDE_READ_UINT8( __ctlr, __regno, __val )  \
    aki3068net_read_command(__regno, &(__val))
#define HAL_IDE_READ_UINT16( __ctlr, __regno, __val )  \
    aki3068net_read_data(__regno, &(__val))
#define HAL_IDE_READ_ALTSTATUS( __ctlr, __val )  \
    aki3068net_read_control( &(__val))

#define HAL_IDE_WRITE_UINT8( __ctlr, __regno, __val )  \
    aki3068net_write_command(__regno, __val)
#define HAL_IDE_WRITE_UINT16( __ctlr, __regno, __val )  \
    aki3068net_write_data(__regno, __val)
#define HAL_IDE_WRITE_CONTROL( __ctlr, __val )  \
    aki3068net_write_control( __val)

void aki3068net_read_command(cyg_uint16 r, cyg_uint8 *d);
void aki3068net_read_data   (cyg_uint16 r, cyg_uint16 *d);
void aki3068net_read_control(cyg_uint8 *d);

void aki3068net_write_command(cyg_uint16 r, cyg_uint8 d);
void aki3068net_write_data   (cyg_uint16 r, cyg_uint16 d);
void aki3068net_write_control(cyg_uint8 d);

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_PLF_IO_H
// end of plf_io.h
