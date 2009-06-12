//==========================================================================
//
//      devs/wallclock/ds12887.inl
//
//      Wallclock implementation for Dallas 12887
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
// Date:          2001-07-06
// Purpose:       Wallclock driver for Dallas 12887
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>          // Wallclock device config

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // interrupt enable/disable
#include <cyg/infra/cyg_type.h>         // Common type definitions and support

#include <cyg/io/wallclock.hxx>         // The WallClock API
#include <cyg/io/wallclock/wallclock.inl> // Helpers

#include <cyg/infra/diag.h>

#define nDEBUG

// Platform details
#include CYGDAT_DEVS_WALLCLOCK_DALLAS_12887_INL

#ifndef DS_READ_UINT8
# define DS_READ_UINT8(x,y) HAL_READ_UINT8(x,y)
# define DS_WRITE_UINT8(x,y) HAL_WRITE_UINT8(x,y)
#endif

#ifdef DS_LINEAR
# ifndef DS_STEP
#  define DS_STEP 0
# endif
# ifndef DS_BASE
#  error "Need to know base of DS12887 part"
# endif
# define DS_READ(offset, data) DS_READ_UINT8(DS_BASE + ((offset) << DS_STEP), (data))
# define DS_WRITE(offset, data) DS_WRITE_UINT8(DS_BASE + ((offset) << DS_STEP), (data))
#else
# if !defined(DS_ADDR) || !defined(DS_DATA)
#  error "Need to know addr/data locations of DS12887 part"
# endif
# define DS_READ(offset, data)                 \
  CYG_MACRO_START                              \
  DS_WRITE_UINT8(DS_ADDR, (offset));           \
  DS_READ_UINT8(DS_DATA, (data));              \
  CYG_MACRO_END
# define DS_WRITE(offset, data)                \
  CYG_MACRO_START                              \
  DS_WRITE_UINT8(DS_ADDR, (offset));           \
  DS_WRITE_UINT8(DS_DATA, (data));             \
  CYG_MACRO_END
#endif

// Registers
#define DS_SECONDS         0x00
#define DS_SECONDS_ALARM   0x01
#define DS_MINUTES         0x02
#define DS_MINUTES_ALARM   0x03
#define DS_HOURS           0x04
#define DS_HOURS_ALARM     0x05
#define DS_DOW             0x06
#define DS_DOM             0x07
#define DS_MONTH           0x08
#define DS_YEAR            0x09
#define DS_CENTURY         0x32

#define DS_REG_A           0x0a
#define DS_REG_B           0x0b
#define DS_REG_C           0x0c
#define DS_REG_D           0x0d

// Control bits
#define DS_REG_A_UIP       0x80
#define DS_REG_A_ENABLE    0x20

#define DS_REG_B_SET       0x80
#define DS_REG_B_DM        0x04
#define DS_REG_B_24H       0x02


//----------------------------------------------------------------------------
// Accessor functions
static inline void
init_ds_hwclock(void)
{
    cyg_uint8 _regb, _tmp;

    // Set 24H mode
    DS_WRITE(DS_REG_B, DS_REG_B_24H);
    // Enable clock
    DS_WRITE(DS_REG_A, DS_REG_A_ENABLE);
    
    // Verify that there are reasonable default settings - otherwise
    // set them.

    // Stop counting
    DS_READ(DS_REG_B, _regb);
    _regb |= DS_REG_B_SET;
    DS_WRITE(DS_REG_B, _regb);

    DS_READ(DS_CENTURY, _tmp);
    if (0xff == _tmp)
	DS_WRITE(DS_CENTURY, TO_BCD(20));

    DS_READ(DS_MONTH, _tmp);
    if (0x00 == _tmp)
	DS_WRITE(DS_MONTH, TO_BCD(1));

    DS_READ(DS_DOM, _tmp);
    if (0x00 == _tmp)
	DS_WRITE(DS_DOM, TO_BCD(1));

    DS_READ(DS_DOM, _tmp);
    if (0x00 == _tmp)
	DS_WRITE(DS_DOM, TO_BCD(1));

    // Restart counting
    _regb &= ~DS_REG_B_SET;
    DS_WRITE(DS_REG_B, _regb);
}


static inline void
set_ds_hwclock(cyg_uint32 year, cyg_uint32 month, cyg_uint32 mday,
               cyg_uint32 hour, cyg_uint32 minute, cyg_uint32 second)
{
    cyg_uint8 _regb;
    // Stop counting
    DS_READ(DS_REG_B, _regb);
    _regb |= DS_REG_B_SET;
    DS_WRITE(DS_REG_B, _regb);

    DS_WRITE(DS_CENTURY, TO_BCD((cyg_uint8)(year / 100)));
    DS_WRITE(DS_YEAR, TO_BCD((cyg_uint8)(year % 100)));
    DS_WRITE(DS_MONTH, TO_BCD((cyg_uint8)month));
    DS_WRITE(DS_DOM, TO_BCD((cyg_uint8)mday));
    DS_WRITE(DS_HOURS, TO_BCD((cyg_uint8)hour));
    DS_WRITE(DS_MINUTES, TO_BCD((cyg_uint8)minute));
    DS_WRITE(DS_SECONDS, TO_BCD((cyg_uint8)second));

    // Restart counting
    _regb &= ~DS_REG_B_SET;
    DS_WRITE(DS_REG_B, _regb);

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
    cyg_uint8 _reg, _t1, _t2;
    cyg_uint32 _old;

    // Wait for update flag clears
    do {
        DS_READ(DS_REG_A, _reg);
    } while (_reg & DS_REG_A_UIP);

    // Disable interrupts while reading to ensure it doesn't take more
    // than 244us.
    HAL_DISABLE_INTERRUPTS(_old);

    DS_READ(DS_CENTURY, _t1);
    DS_READ(DS_YEAR, _t2);
    *year = (cyg_uint32)TO_DEC(_t1)*100 + (cyg_uint32)TO_DEC(_t2);

    DS_READ(DS_MONTH, _t1);
    *month = (cyg_uint32)TO_DEC(_t1);

    DS_READ(DS_DOM, _t1);
    *mday = (cyg_uint32)TO_DEC(_t1);

    DS_READ(DS_HOURS, _t1);
    *hour = (cyg_uint32)TO_DEC(_t1);

    DS_READ(DS_MINUTES, _t1);
    *minute = (cyg_uint32)TO_DEC(_t1);

    DS_READ(DS_SECONDS, _t1);
    *second = (cyg_uint32)TO_DEC(_t1);

    // Reenable interrupts
    HAL_RESTORE_INTERRUPTS(_old);

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
// End of devs/wallclock/ds12887.inl
