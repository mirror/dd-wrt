#ifndef CYGONCE_IO_WALLCLOCK_INL
#define CYGONCE_IO_WALLCLOCK_INL

//==========================================================================
//
//      wallclock.inl
//
//      Wallclock internal helper functions
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
// Date:          2000-05-26
// Purpose:       Wall Clock internal helper functions
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>          // assertions

// -------------------------------------------------------------------------
// Some helper functions

#define is_leap(_y_) (((0==(_y_)%4 && 0!=(_y_)%100) || 0==(_y_)%400) ? 1 : 0)
#define year_days(_y_) (is_leap(_y_) ? 366 : 365)

static cyg_int32 days_per_month[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

#ifndef time_t
#define time_t cyg_uint32
#endif

static time_t
_simple_mktime(cyg_uint32 year, cyg_uint32 mon,
               cyg_uint32 day, cyg_uint32 hour,
               cyg_uint32 min, cyg_uint32 sec)
{
    time_t secs;
    cyg_uint32 y, m, days;

    CYG_ASSERT(year <= 3124, "Year is unreasonably large");
    CYG_ASSERT(mon <= 12, "Month is invalid");
    CYG_ASSERT(day <= 31, "Day is invalid");
    CYG_ASSERT(hour <= 23, "Hour is invalid");
    CYG_ASSERT(min <= 59, "Minutes is invalid");
    CYG_ASSERT(sec <= 61, "Seconds is invalid");

    // Number of days due to years
    days = 0;
    for (y = 1970; y < year; y++)
        days += year_days(y);

    // Due to months
    for (m = 0; m < mon-1; m++)
        days += days_per_month[is_leap(year)][m];
    // Add days
    days += day - 1;

    // Add hours, minutes, and seconds
    secs = ((days * 24 + hour) * 60 + min) * 60 + sec;

    return secs;
}

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE


static void
_simple_mkdate(time_t time,
               cyg_uint32* year, cyg_uint32* mon,
               cyg_uint32* day, cyg_uint32* hour,
               cyg_uint32* min, cyg_uint32* sec)
{
    cyg_int32 days, hms, y, m, *dpm;

    days = (cyg_int32) (time / (24*60*60));
    hms  = (cyg_int32) (time % (24*60*60));

    // Nothing fancy about the time - no leap year magic involved
    *sec = hms % 60;
    *min = (hms % (60*60)) / 60;
    *hour = hms / (60*60);

    // Find year
    for (y = 1970; days >= year_days(y); y++)
        days -= year_days(y);
    *year = y;
    dpm = &days_per_month[is_leap(y)][0];

    // Find month
    for (m = 0; days >= dpm[m]; m++)
        days -= dpm[m];
    m++;
    *mon = m;

    *day = days+1;
}

#endif

//-----------------------------------------------------------------------------
// BCD helper macros
#define TO_BCD(x) ((((x)/10)<<4) | ((x)%10))
#define TO_DEC(x) ((((x)>>4)*10) + ((x)&0xf))

#endif // ifndef CYGONCE_DEVS_WALLCLOCK_INL
// EOF wallclock.inl
