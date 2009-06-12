//==========================================================================
//
//      eb42_misc.c
//
//      HAL misc board support code for Atmel EB42 eval board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: gthomas, jskov, tkoeller, tdrury, nickg
// Date:         2002-05-30
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/hal/hal_io.h>             // low level i/o
#include <cyg/hal/var_io.h>             // common registers
#include <cyg/hal/plf_io.h>             // platform registers

void
hal_at91_set_leds(int val)
{
    HAL_WRITE_UINT32(AT91_PIOB+AT91_PIO_SODR, 0x0000FF00); // turn all LEDs off
    HAL_WRITE_UINT32(AT91_PIOB+AT91_PIO_CODR, val<<8);      // turn desired LEDs on
}

void
hal_at91_led_on(int val)
{
    HAL_WRITE_UINT32(AT91_PIOB+AT91_PIO_CODR, val<<8);
}

void
hal_at91_led_off(int val)
{
    HAL_WRITE_UINT32(AT91_PIOB+AT91_PIO_SODR, val<<8);
}

int
hal_at91_get_leds(void)
{
    int leds = 0;
    HAL_READ_UINT32(AT91_PIOB+AT91_PIO_PDSR, leds);
    return (leds>>8)&0xFF;
}




//--------------------------------------------------------------------------
// EOF eb42_misc.c
