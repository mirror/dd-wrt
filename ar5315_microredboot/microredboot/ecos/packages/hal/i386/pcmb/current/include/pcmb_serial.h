#ifndef CYGONCE_HAL_PCMB_SERIAL_H
#define CYGONCE_HAL_PCMB_SERIAL_H

//==========================================================================
//
//      pcmb_serial.h
//
//      i386/pc Motherboard serial device support
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
// Author(s):    nickg
// Contributors: 
// Date:         2001-03-07
// Purpose:      PC serial support
// Description:  
//               
//               
//               
//               
//               
//              
// Usage:
//               #include <cyg/hal/pcmb_serial.h>
//               ...
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_i386.h>
#include <pkgconf/hal_i386_pcmb.h>

#include <cyg/infra/cyg_type.h>

//---------------------------------------------------------------------------

//=============================================================================

typedef struct {
    cyg_uint16  base;
    cyg_uint16  valid;
    cyg_int32   msec_timeout;
    cyg_int32   isr_vector;
} channel_data_t;

__externC channel_data_t pc_ser_channels[];

//=============================================================================

__externC void cyg_hal_plf_serial_init(void);

#if CYGINT_HAL_I386_PCMB_SCREEN_SUPPORT > 0

__externC void cyg_hal_plf_screen_init(void);

#endif

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PCMB_SERIAL_H
// End of pcmb_serial.h
