/*=============================================================================
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
// Author(s):   ysato
// Contributors: ysato
// Date:        2002-04-05
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/hal_diag.h>
#include <cyg/hal/h8_sci.h>
#include <cyg/hal/var_intr.h>

#define SCI_BASE ((cyg_uint8*)CYGHAL_PLF_SCI_BASE)

static channel_data_t channel = { (cyg_uint8*)SCI_BASE, 0, 0};

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_sci_init(0, 0, CYGNUM_HAL_INTERRUPT_RXI0, SCI_BASE);
}

void hal_diag_init(void)
{
    cyg_hal_plf_sci_init_channel(&channel);
}

void 
hal_diag_write_char( cyg_uint8 c )
{
    cyg_hal_plf_sci_putc(&channel, c);
}

void
hal_diag_read_char(cyg_uint8 *c)
{
    *c = (char) cyg_hal_plf_sci_getc(&channel);
}

/*===========================================================================*/
/* EOF hal_diag.c                                                            */

