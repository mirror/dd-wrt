//=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   proven
// Contributors:proven
// Date:        1998-10-05
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_diag.h>

#include <cyg/hal/plf_misc.h>

//-----------------------------------------------------------------------------
// New Hal_Diag init to comply with the eCos/ROM Calling Interface.

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions

#include <cyg/hal/pcmb_serial.h>

//=============================================================================

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG) \
    || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)

channel_data_t pc_ser_channels[CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS];

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;
    int num_serial;

    if (initialized)
        return;

    initialized = 1;

    num_serial = CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS;
#ifdef CYGSEM_HAL_I386_PC_DIAG_SCREEN
    --num_serial;
#endif
    if (num_serial > 0) {
	// COM1
	pc_ser_channels[0].base = 0x3F8;
	pc_ser_channels[0].msec_timeout = 1000;
	pc_ser_channels[0].isr_vector = 36;
    }
    if (num_serial > 1) {
	// COM2
	pc_ser_channels[1].base = 0x2F8;
	pc_ser_channels[1].msec_timeout = 1000;
	pc_ser_channels[1].isr_vector = 35;
    }

    cyg_hal_plf_serial_init();

#ifdef CYGSEM_HAL_I386_PC_DIAG_SCREEN
    
    pc_ser_channels[num_serial].base = 0x060;
    pc_ser_channels[num_serial].msec_timeout = 1000;
    pc_ser_channels[num_serial].isr_vector = 33;

    cyg_hal_plf_screen_init();

#endif    
}

//=============================================================================

#endif  //defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
	//  || defined(CYGPRI_HAL_IMPLEMENTS_IF_SERVICES)

#endif

//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

// TODO: add stand-alone code

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

//-----------------------------------------------------------------------------
// End of hal_diag.c

