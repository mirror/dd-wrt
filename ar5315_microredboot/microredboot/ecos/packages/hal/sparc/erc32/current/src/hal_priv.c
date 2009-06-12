//===========================================================================
//
//      hal_priv.c
//
//      SPARC ERC32 Architecture sim-specific private variables
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         1999-02-20
// Purpose:      private vars for SPARC ERC32.
//              
//####DESCRIPTIONEND####
//
//===========================================================================


#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arch.h>

// ------------------------------------------------------------------------
// Clock static to keep period recorded.
cyg_int32 cyg_hal_sparc_clock_period = 0;

// ------------------------------------------------------------------------
// Board specific startups.

extern void hal_board_prestart( void );
extern void hal_board_poststart( void );

void hal_board_prestart( void )
{
}

void hal_board_poststart( void )
{
    HAL_ENABLE_INTERRUPTS();
    // OK to do this post constructors, and good for testing.
}

cyg_uint32
hal_lsbit_index(cyg_uint32 mask)
{
    int i;
    for (i = 0;  i < 32;  i++) {
	if (mask & (1<<i)) return ((cyg_uint32)i);
    }
    return ((cyg_uint32)-1);
}

cyg_uint32
hal_msbit_index(cyg_uint32 mask)
{
    int i;
    for (i = 31;  i >= 0;  i--) {
        if (mask & (1<<i)) return ((cyg_uint32)i);
    }
    return ((cyg_uint32)-1);
}

void
hal_idle_thread_action(cyg_uint32 loop_count)
{
    *((volatile cyg_uint32 *) 0x01f80008) = 0;
}


// EOF hal_priv.c
