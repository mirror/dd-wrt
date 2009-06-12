//==========================================================================
//
//      devs/wallclock/wallclock.cxx
//
//      Wallclock base
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
// Date:          2000-03-28
// Purpose:       Wallclock base driver
// Description:   This provides a generic wallclock base driver which
//                relies on sub-drivers to provide the interface functions
//                to the hardware device (see wallclock.hxx for details).
//
//                The driver can be configured to run in one of two modes:
//                  init-get mode:
//                    In this mode, the hardware driver only needs to
//                    implement a function to read the hardware clock
//                    and a (possibly empty) init function which makes
//                    sure the hardware clock is properly initialized.
//                    While the driver in this mode has a smaller memory
//                    foot-print, it does not support battery-backed up
//                    clocks.
//
//                  set-get mode:
//                    Requires both set and get functions for the hardware
//                    device. While bigger, it allows support of 
//                    battery-backed up clocks.
//
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>          // Wallclock device config

#include <cyg/infra/cyg_type.h>         // Common type definitions and support

#include <cyg/hal/drv_api.h>	        // Driver API

#include <cyg/io/wallclock.hxx>         // The WallClock API


//-----------------------------------------------------------------------------
// Local static variables

static Cyg_WallClock wallclock_instance CYGBLD_ATTRIB_INIT_AFTER( CYG_INIT_CLOCK );

#ifndef CYGSEM_WALLCLOCK_SET_GET_MODE
static cyg_uint32 epoch_ticks;
static cyg_uint32 epoch_time_stamp;
#endif

Cyg_WallClock *Cyg_WallClock::wallclock;

//-----------------------------------------------------------------------------
// Constructor

Cyg_WallClock::Cyg_WallClock()
{
    // install instance pointer
    wallclock = &wallclock_instance;

    // Always allow low-level driver to initialize clock, even though it
    // may not be necessary for set-get mode.
    init_hw_seconds();
}

//-----------------------------------------------------------------------------
// Returns the number of seconds elapsed since 1970-01-01 00:00:00.
// This may involve reading the hardware, so it may take anything up to
// a second to complete.

cyg_uint32 Cyg_WallClock::get_current_time()
{
    cyg_uint32 res;

    cyg_drv_dsr_lock();

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    res = get_hw_seconds();
#else
    res = epoch_time_stamp + get_hw_seconds() - epoch_ticks;
#endif

    cyg_drv_dsr_unlock();

    return res;
}

//-----------------------------------------------------------------------------
// Sets the clock. Argument is seconds elapsed since 1970-01-01 00:00:00.
// This may involve reading or writing to the hardware, so it may take
// anything up to a second to complete.
void Cyg_WallClock::set_current_time( cyg_uint32 time_stamp )
{
    cyg_drv_dsr_lock();

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    set_hw_seconds(time_stamp);
#else
    epoch_time_stamp    = time_stamp;
    epoch_ticks         = get_hw_seconds();
#endif

    cyg_drv_dsr_unlock();
}

//-----------------------------------------------------------------------------
// End of devs/wallclock/wallclock.cxx
