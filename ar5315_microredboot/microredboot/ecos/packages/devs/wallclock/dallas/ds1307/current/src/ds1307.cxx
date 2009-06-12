//==========================================================================
//
//      devs/wallclock/ds1307.inl
//
//      Wallclock implementation for Dallas 1307
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
// Author(s):     gthomas
// Contributors:  
// Date:          2003-09-19
// Purpose:       Wallclock driver for Dallas 1307
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>                // Platform specific configury
#include <pkgconf/wallclock.h>          // Wallclock device config

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // interrupt enable/disable
#include <cyg/infra/cyg_type.h>         // Common type definitions and support

#include <cyg/io/wallclock.hxx>         // The WallClock API
#include <cyg/io/wallclock/wallclock.inl> // Helpers

#include <cyg/infra/diag.h>

//#define DEBUG

// Registers
#define DS_SECONDS         0x00
#define DS_MINUTES         0x01
#define DS_HOURS           0x02
#define DS_DOW             0x03
#define DS_DOM             0x04
#define DS_MONTH           0x05
#define DS_YEAR            0x06
#define DS_CONTROL         0x07
#define DS_REGS_SIZE       0x08   // Size of register space

#define DS_SECONDS_CH      0x80   // Clock Halt
#define DS_HOURS_24        0x80   // 24 hour clock mode

//
// This particular chip is always accessed via I2C (2-wire protocol)
// The platform needs to provide simple I2C access functions
//
// void DS_GET(cyg_uint8 *regs)
//    Reads the entire set of registers (8 bytes) into *regs
// void DS_PUT(cyg_uint8 *regs)
//    Updated the entire set of registers (8 bytes) from *regs
//
// Using this method, the data in the registers is guaranteed to be
// stable (if the access function manipulates the registers in an
// single operation)
//

// Platform details
#include CYGDAT_DEVS_WALLCLOCK_DALLAS_1307_INL

//----------------------------------------------------------------------------
// Accessor functions
static inline void
init_ds_hwclock(void)
{
    cyg_uint8 regs[DS_REGS_SIZE];
    
    // Verify that there are reasonable default settings - otherwise
    // set them.

    // Fetch the current state
    DS_GET(regs);

    if (0x00 == regs[DS_MONTH])
	regs[DS_MONTH] = TO_BCD(1);

    if (0x00 == regs[DS_DOW])
	regs[DS_DOW] = TO_BCD(1);

    if (0x00 == regs[DS_DOM])
	regs[DS_DOM] = TO_BCD(1);

    // Make sure clock is running and in 24 hour mode
    regs[DS_HOURS] |= DS_HOURS_24;
    regs[DS_SECONDS] &= ~DS_SECONDS_CH;

    // Update the state
    DS_PUT(regs);
}


static inline void
set_ds_hwclock(cyg_uint32 year, cyg_uint32 month, cyg_uint32 mday,
               cyg_uint32 hour, cyg_uint32 minute, cyg_uint32 second)
{
    cyg_uint8 regs[DS_REGS_SIZE];

    // Set up the registers
    regs[DS_YEAR] = TO_BCD((cyg_uint8)(year % 100));
    regs[DS_MONTH] = TO_BCD((cyg_uint8)month);
    regs[DS_DOM] = TO_BCD((cyg_uint8)mday);
    regs[DS_HOURS] = TO_BCD((cyg_uint8)hour) | DS_HOURS_24;
    regs[DS_MINUTES] = TO_BCD((cyg_uint8)minute);
    // This also starts the clock
    regs[DS_SECONDS] = TO_BCD((cyg_uint8)second);

    // Send the register set to the hardware
    DS_PUT(regs);

#ifdef DEBUG
    // This will cause the test to eventually fail due to these printouts
    // causing timer interrupts to be lost...
    diag_printf("Set -------------\n");
    diag_printf("year %02d\n", year);
    diag_printf("month %02d\n", month);
    diag_printf("mday %02d\n", mday);
    diag_printf("hour %02d\n", hour);
    diag_printf("minute %02d\n", minute);
    diag_printf("second %02d\n", second);
#endif
}

static inline void
get_ds_hwclock(cyg_uint32* year, cyg_uint32* month, cyg_uint32* mday,
               cyg_uint32* hour, cyg_uint32* minute, cyg_uint32* second)
{
    cyg_uint8 regs[DS_REGS_SIZE];

    // Fetch the current state
    DS_GET(regs);

    *year = (cyg_uint32)TO_DEC(regs[DS_YEAR]);
    // The year field only has the 2 least significant digits :-(
    if (*year >= 34) {
        *year += 1900;
    } else {
        *year += 2000;
    }
    *month = (cyg_uint32)TO_DEC(regs[DS_MONTH]);
    *mday = (cyg_uint32)TO_DEC(regs[DS_DOM]);
    *hour = (cyg_uint32)TO_DEC(regs[DS_HOURS] & 0x7F);
    *minute = (cyg_uint32)TO_DEC(regs[DS_MINUTES]);
    *second = (cyg_uint32)TO_DEC(regs[DS_SECONDS] & 0x7F);

#ifdef DEBUG
    // This will cause the test to eventually fail due to these printouts
    // causing timer interrupts to be lost...
    diag_printf("year %02d\n", *year);
    diag_printf("month %02d\n", *month);
    diag_printf("mday %02d\n", *mday);
    diag_printf("hour %02d\n", *hour);
    diag_printf("minute %02d\n", *minute);
    diag_printf("second %02d\n", *second);
#endif
}

//-----------------------------------------------------------------------------
// Functions required for the hardware-driver API.

// Returns the number of seconds elapsed since 1970-01-01 00:00:00.
cyg_uint32 
Cyg_WallClock::get_hw_seconds(void)
{
    cyg_uint32 year, month, mday, hour, minute, second;

    get_ds_hwclock(&year, &month, &mday, &hour, &minute, &second);
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
// End of devs/wallclock/ds1307.inl
