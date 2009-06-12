//==========================================================================
//
//      eb40_misc.c
//
//      HAL misc board support code for Atmel EB40 eval board
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
// Contributors: gthomas, jskov, tkoeller, nickg
// Date:         2002-05-30
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/hal/var_io.h>             // platform registers

// 
// Diagnostic LEDs - there are three colored LEDs which can be used
// to send a simple diagnostic value (8 bits)
//

void 
hal_at91_led(int val)
{
    int i, to;

    HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_CODR, 0x06);  // DATA+CLOCK LEDs off
    for (to = 0;  to < 0x200000; to++) ;
    for (i = 0;  i < 8;  i++) {        
        HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_SODR, ((val>>(7-i)) & 0x01)<<2);  // DATA LED
        HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_SODR, 0x02);  // CLOCK LED on
        for (to = 0;  to < 0x80000; to++) ;
        HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_CODR, 0x02);  // CLOCK LED off
        for (to = 0;  to < 0x40000; to++) ;
        HAL_WRITE_UINT32(AT91_PIO+AT91_PIO_CODR, 0x04);  // DATA LED off
    }
}

void
hal_at91_set_leds(int val)
{
    hal_at91_led(val);
}



//--------------------------------------------------------------------------
// EOF eb40_misc.c
