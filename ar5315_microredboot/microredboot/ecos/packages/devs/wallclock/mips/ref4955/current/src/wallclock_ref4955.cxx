//==========================================================================
//
//      devs/wallclock/mips/ref4955/ref4955.cxx
//
//      Wallclock implementation
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          2000-05-25
// Purpose:       Wallclock driver for REF4955
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>          // Wallclock device config

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/infra/cyg_type.h>         // Common type definitions and support

#include <cyg/io/wallclock.hxx>         // The WallClock API
#include <cyg/io/wallclock/wallclock.inl> // Helpers

#include <cyg/infra/diag.h>             // For debugging

// REF4955 has a Dallas 1742W part.
#define DS_BASE 0xb5000000
#include <cyg/io/wallclock/ds1742.inl>

//-----------------------------------------------------------------------------
// Functions required for the hardware-driver API.

// Returns the number of seconds elapsed since 1970-01-01 00:00:00.
cyg_uint32 
Cyg_WallClock::get_hw_seconds(void)
{
    cyg_uint32 year, month, mday, hour, minute, second;

    get_ds_hwclock(&year, &month, &mday, &hour, &minute, &second);

#if 0
    // This will cause the test to eventually fail due to these printouts
    // causing timer interrupts to be lost...
    diag_printf("year %02d\n", year);
    diag_printf("month %02d\n", month);
    diag_printf("mday %02d\n", mday);
    diag_printf("hour %02d\n", hour);
    diag_printf("minute %02d\n", minute);
    diag_printf("second %02d\n", second);
#endif

    cyg_uint32 now = _simple_mktime(year, month, mday, hour, minute, second);
    return now;
}

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE

// Sets the clock. Argument is seconds elapsed since 1970-01-01 00:00:00.
void
Cyg_WallClock::set_hw_seconds( cyg_uint32 secs )
{
    cyg_uint32 year, month, mday, hour, minute, second;

    _simple_mkdate(secs, &year, &month, &mday, &hour, &minute, &second);

    set_ds_hwclock(year, month, mday, hour, minute, second);
}

#endif

void
Cyg_WallClock::init_hw_seconds(void)
{
#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    init_ds_hwclock();
#else
    // This is our base: 1970-01-01 00:00:00
    // Set the HW clock - if for nothing else, just to be sure it's in a
    // legal range. Any arbitrary base could be used.
    // After this the hardware clock is only read.
    set_ds_hwclock(1970,1,1,0,0,0);
#endif
}

//-----------------------------------------------------------------------------
// End of devs/wallclock/mips/ref4955/wallclock_ref4955.cxx
