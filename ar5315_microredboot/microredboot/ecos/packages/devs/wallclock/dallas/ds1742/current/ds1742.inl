//==========================================================================
//
//      devs/wallclock/ds1742.inl
//
//      Wallclock implementation for Dallas 1742
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
// Purpose:       Wallclock driver for Dallas 1742
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef DS_BASE
# error "Need to know base of DS1742 RAM"
#endif

#include <cyg/infra/cyg_type.h>         // Common type definitions and support
#include <cyg/hal/hal_io.h>             // IO macros

// Offsets from DS_BASE
#define DS_SECONDS       0x7f9          // control bit!
#define DS_SECONDS_MASK  0x7f
#define DS_MINUTES       0x7fa
#define DS_HOUR          0x7fb
#define DS_DAY           0x7fc          // control bits
#define DS_DAY_MASK      0x07
#define DS_DATE          0x7fd
#define DS_MONTH         0x7fe
#define DS_YEAR          0x7ff
#define DS_CENTURY       0x7f8          // control bits!
#define DS_CENTURY_MASK  0x3f

// Control bits
#define DS_SECONDS_OSC   0x80           // set to stop oscillator (power save)
#define DS_CENTURY_READ  0x40           // set during read
#define DS_CENTURY_WRITE 0x80           // set during write
#define DS_DAY_BF        0x80           // battery flag
#define DS_DAY_FT        0x40           // frequency test


// Make sure test modes are disabled and that clock is running.
static void
init_ds_hwclock(void)
{
    cyg_uint8 _tmp;

    // Enable clock
    HAL_READ_UINT8(DS_BASE+DS_SECONDS, _tmp);
    _tmp &= ~DS_SECONDS_OSC;
    HAL_WRITE_UINT8(DS_BASE+DS_SECONDS, _tmp);

    // clear frequency test
    HAL_READ_UINT8(DS_BASE+DS_DAY, _tmp);
    _tmp &= ~DS_DAY_FT;
    HAL_WRITE_UINT8(DS_BASE+DS_DAY, _tmp);
}


static void
set_ds_hwclock(cyg_uint32 year, cyg_uint32 month, cyg_uint32 mday,
               cyg_uint32 hour, cyg_uint32 minute, cyg_uint32 second)
{
    cyg_uint8 _tmp;
    // Init write operation
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= ~(DS_CENTURY_WRITE|DS_CENTURY_READ);
    _tmp |= DS_CENTURY_WRITE;
    HAL_WRITE_UINT8(DS_BASE+DS_CENTURY, _tmp);

    // Entries with control bits
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= ~DS_CENTURY_MASK;
    _tmp |= TO_BCD(year/100) & DS_CENTURY_MASK;
    HAL_WRITE_UINT8(DS_BASE+DS_CENTURY, _tmp);

    HAL_READ_UINT8(DS_BASE+DS_SECONDS, _tmp);
    _tmp &= ~DS_SECONDS_MASK;
    _tmp |= TO_BCD(second) & DS_SECONDS_MASK;
    HAL_WRITE_UINT8(DS_BASE+DS_SECONDS, _tmp);

    HAL_READ_UINT8(DS_BASE+DS_DAY, _tmp);
    _tmp &= ~DS_DAY_FT;                 // clear frequency test
    HAL_WRITE_UINT8(DS_BASE+DS_DAY, _tmp);


    // Dedicated entries
    HAL_WRITE_UINT8(DS_BASE+DS_YEAR, TO_BCD(year % 100));
    HAL_WRITE_UINT8(DS_BASE+DS_MONTH, TO_BCD(month));
    HAL_WRITE_UINT8(DS_BASE+DS_DATE, TO_BCD(mday));
    HAL_WRITE_UINT8(DS_BASE+DS_HOUR, TO_BCD(hour));
    HAL_WRITE_UINT8(DS_BASE+DS_MINUTES, TO_BCD(minute));

    // Enable clock
    HAL_READ_UINT8(DS_BASE+DS_SECONDS, _tmp);
    _tmp &= ~DS_SECONDS_OSC;
    HAL_WRITE_UINT8(DS_BASE+DS_SECONDS, _tmp);

    // Finish write operation
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= ~(DS_CENTURY_WRITE|DS_CENTURY_READ);
    HAL_WRITE_UINT8(DS_BASE+DS_CENTURY, _tmp);
}

static void
get_ds_hwclock(cyg_uint32* year, cyg_uint32* month, cyg_uint32* mday,
               cyg_uint32* hour, cyg_uint32* minute, cyg_uint32* second)
{
    cyg_uint8 _tmp;

    // Init read operation
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= ~(DS_CENTURY_WRITE|DS_CENTURY_READ);
    _tmp |= DS_CENTURY_READ;
    HAL_WRITE_UINT8(DS_BASE+DS_CENTURY, _tmp);

    // Entries with control bits
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= DS_CENTURY_MASK;
    *year = 100*TO_DEC(_tmp);

    HAL_READ_UINT8(DS_BASE+DS_SECONDS, _tmp);
    _tmp &= DS_SECONDS_MASK;
    *second = TO_DEC(_tmp);

    // Dedicated entries
    HAL_READ_UINT8(DS_BASE+DS_YEAR, _tmp);
    *year += TO_DEC(_tmp);
    HAL_READ_UINT8(DS_BASE+DS_MONTH, _tmp);
    *month = TO_DEC(_tmp);
    HAL_READ_UINT8(DS_BASE+DS_DATE, _tmp);
    *mday = TO_DEC(_tmp);
    HAL_READ_UINT8(DS_BASE+DS_HOUR, _tmp);
    *hour = TO_DEC(_tmp);
    HAL_READ_UINT8(DS_BASE+DS_MINUTES, _tmp);
    *minute = TO_DEC(_tmp);

    // Finish read operation
    HAL_READ_UINT8(DS_BASE+DS_CENTURY, _tmp);
    _tmp &= ~(DS_CENTURY_WRITE|DS_CENTURY_READ);
    HAL_WRITE_UINT8(DS_BASE+DS_CENTURY, _tmp);
}

//-----------------------------------------------------------------------------
// End of devs/wallclock/ds1742.inl
