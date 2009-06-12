//==========================================================================
//
//      devs/wallclock/sh3.cxx
//
//      SH3 RTC module driver.
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
// Date:          2000-03-17
// Purpose:       Wallclock driver for SH3 CPU RTC module
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>          // Wallclock device config

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/infra/cyg_type.h>         // Common type definitions and support

#include <cyg/io/wallclock.hxx>         // The WallClock API
#include <cyg/io/wallclock/wallclock.inl> // Helpers

#include <cyg/hal/sh_regs.h>            // RTC register definitions

#include <cyg/infra/diag.h>             // For debugging



//-----------------------------------------------------------------------------
// Functions for setting and getting the hardware clock counters

// Year must be last two digits of "western calendar year". Leap year when
// divisible by four.
static void
set_sh3_hwclock(cyg_uint32 year, cyg_uint32 month, cyg_uint32 mday,
                cyg_uint32 hour, cyg_uint32 minute, cyg_uint32 second)
{
    // Stop RTC
    HAL_WRITE_UINT8(CYGARC_REG_RCR2, CYGARC_REG_RCR2_RESET);

    // Program it
    HAL_WRITE_UINT8(CYGARC_REG_RYRCNT,  TO_BCD(year));
    HAL_WRITE_UINT8(CYGARC_REG_RMONCNT, TO_BCD(month));
    HAL_WRITE_UINT8(CYGARC_REG_RDAYCNT, TO_BCD(mday));
    HAL_WRITE_UINT8(CYGARC_REG_RHRCNT,  TO_BCD(hour));
    HAL_WRITE_UINT8(CYGARC_REG_RMINCNT, TO_BCD(minute));
    HAL_WRITE_UINT8(CYGARC_REG_RSECCNT, TO_BCD(second));

    // Start RTC
    HAL_WRITE_UINT8(CYGARC_REG_RCR1, CYGARC_REG_RCR1_CIE);
    HAL_WRITE_UINT8(CYGARC_REG_RCR2, 
                    CYGARC_REG_RCR2_RTCEN | CYGARC_REG_RCR2_START);

}

static void
get_sh3_hwclock(cyg_uint32* year, cyg_uint32* month, cyg_uint32* mday,
                cyg_uint32* hour, cyg_uint32* minute, cyg_uint32* second)
{
    cyg_uint8 tmp;

    do {
        // Clear carry flag
        HAL_WRITE_UINT8(CYGARC_REG_RCR1, 0);
        
        // Read time
        HAL_READ_UINT8(CYGARC_REG_RYRCNT, tmp);
        *year = TO_DEC(tmp);
        HAL_READ_UINT8(CYGARC_REG_RMONCNT, tmp);
        *month = TO_DEC(tmp);
        HAL_READ_UINT8(CYGARC_REG_RDAYCNT, tmp);
        *mday = TO_DEC(tmp);
        HAL_READ_UINT8(CYGARC_REG_RHRCNT, tmp);
        *hour = TO_DEC(tmp);
        HAL_READ_UINT8(CYGARC_REG_RMINCNT, tmp);
        *minute = TO_DEC(tmp);
        HAL_READ_UINT8(CYGARC_REG_RSECCNT, tmp);
        *second = TO_DEC(tmp);

        // Read carry flag
        HAL_READ_UINT8(CYGARC_REG_RCR1, tmp);
    } while (CYGARC_REG_RCR1_CF & tmp); // loop if carry set
}

//-----------------------------------------------------------------------------
// Functions required for the hardware-driver API.

// Returns the number of seconds elapsed since 1970-01-01 00:00:00.
cyg_uint32 
Cyg_WallClock::get_hw_seconds(void)
{
    cyg_uint32 year, month, mday, hour, minute, second;

    get_sh3_hwclock(&year, &month, &mday, &hour, &minute, &second);

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

#ifndef CYGSEM_WALLCLOCK_SET_GET_MODE
    // We know what we initialized the hardware for : 1970, so by doing this
    // the returned time should be OK for 30 years uptime.
    year += 1900;
#else
    // Need to use sliding window or similar to figure out what the
    // century should be... Patent issue is unclear, and since there's
    // no battery backup of the clock, there's little point in
    // investigating.
# error "Need some magic here to figure out century counter"
#endif

    cyg_uint32 now = _simple_mktime(year, month, mday, hour, minute, second);
    return now;
}

#ifndef CYGSEM_WALLCLOCK_SET_GET_MODE

void
Cyg_WallClock::init_hw_seconds(void)
{
    // This is our base: 1970-01-01 00:00:00
    // Set the HW clock - if for nothing else, just to be sure it's in a
    // legal range. Any arbitrary base could be used.
    // After this the hardware clock is only read.
    set_sh3_hwclock(70,1,1,0,0,0);
}

#endif // CYGSEM_WALLCLOCK_SET_GET_MODE

//-----------------------------------------------------------------------------
// End of devs/wallclock/sh3.cxx
