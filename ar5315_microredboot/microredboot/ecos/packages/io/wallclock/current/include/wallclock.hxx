#ifndef CYGONCE_IO_WALLCLOCK_HXX
#define CYGONCE_IO_WALLCLOCK_HXX

//==========================================================================
//
//      wallclock.hxx
//
//      Wallclock interface declaration
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          1999-03-05
// Purpose:       Wall Clock declarations
// Description:   This file defines the interface to the wall clock device
//                that provides real time stamps. Actual implementations of
//                this driver can be found in devs/wallclock.
// Usage:         #include <cyg/io/wallclock.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>

// -------------------------------------------------------------------------
// Wallclock class

class Cyg_WallClock
{

public:

    Cyg_WallClock();

    // Returns the number of seconds elapsed since 1970-01-01 00:00:00.
    // This may involve reading the hardware, so it may take anything
    // up to a second to complete.
    cyg_uint32 get_current_time();

    // Sets the clock. Argument is seconds elapsed since 1970-01-01 00:00:00.
    // This may involve writing to the hardware, so it may take
    // anything up to a second to complete.
    void set_current_time( cyg_uint32 time_stamp );

    // A static pointer to the single system defined wall clock device.
    static Cyg_WallClock *wallclock;

private:

    // Functions implemented by the hardware-specific part of the
    // driver.

    // Note: While the API specifies elapsed time since 1970-01-01
    // 00:00:00 this is only an actual requirement if the driver is
    // used in set-get mode - in init-get mode, the upper layer of the
    // driver only needs to know the number of elapsed seconds between
    // the init_hw_seconds and get_hw_seconds calls; the hardware
    // epoch base is factored out, so the driver can use any base
    // desired (see the emulated device).

    // Called to initialize the hardware clock to a known sane state, or
    // simply to make sure it can be accessed in set-get mode.
    void init_hw_seconds( void );

    // Returns the number of seconds elapsed since 1970-01-01 00:00:00.
    // (or any desired epoch when in init-get mode)
    cyg_uint32 get_hw_seconds( void );

#ifdef CYGSEM_WALLCLOCK_SET_GET_MODE
    // Sets the clock. Argument is seconds elapsed since 1970-01-01 00:00:00.
    void set_hw_seconds( cyg_uint32 secs );
#endif
};

#endif // ifndef CYGONCE_DEVS_WALLCLOCK_HXX
// EOF wallclock.hxx
