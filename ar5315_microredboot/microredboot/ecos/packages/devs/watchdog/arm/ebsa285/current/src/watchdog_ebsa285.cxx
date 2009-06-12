//==========================================================================
//
//      watchdog/ebsa285.cxx
//
//      Watchdog implementation for Intel EBSA-285 StronARM board
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
// Author(s):    jskov (based on the MN10300 watchdog code by nickg)
// Contributors: jskov, nickg
// Date:         1999-08-26
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the EBSA285/21285 hardware watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package
#include <pkgconf/kernel.h>             // kernel config

#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/kernel/instrmnt.h>        // instrumentation

#include <cyg/hal/hal_io.h>             // IO register access

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// 21285 watchdog works by letting timer4 run; if it ever underflows,
// it resets the system. 
// Timer 4 is set to run at fclk_in/16 = 50MHz/16 = 3.125MHz
// The timer register is 24 bits, so it can easily hold a value that
// gives a 1s timeout.
#define WATCHDOG_TIMER_TICKS            3125000
#define WATCHDOG_RESOLUTION             (1000000000)

// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();

    // HW doesn't need init
    
    resolution          = WATCHDOG_RESOLUTION;
        
    CYG_REPORT_RETURN();
}



// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start(void)
{
    CYG_REPORT_FUNCTION();

    // Init the watchdog timer.
    HAL_WRITE_UINT32(SA110_TIMER4_LOAD, WATCHDOG_TIMER_TICKS);
    HAL_WRITE_UINT32(SA110_TIMER4_CLEAR, 0);
    HAL_WRITE_UINT32(SA110_TIMER4_CONTROL, 
                     SA110_TIMER_CONTROL_ENABLE|SA110_TIMER_CONTROL_SCALE_16);
    // Enable the watchdog.
    cyg_uint32 ctrl;
    HAL_READ_UINT32(SA110_CONTROL, ctrl);
    ctrl |= SA110_CONTROL_WATCHDOG;
    HAL_WRITE_UINT32(SA110_CONTROL, ctrl);

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    HAL_WRITE_UINT32(SA110_TIMER4_LOAD, WATCHDOG_TIMER_TICKS);
    
    CYG_REPORT_RETURN();
}

#if 0
// -------------------------------------------------------------------------
// Trigger the watchdog as if the timer had expired.

void
Cyg_Watchdog::reset_action(void)
{
    CYG_REPORT_FUNCTION();
    
    // Init the watchdog timer.
    HAL_WRITE_UINT32(SA110_TIMER4_LOAD, 1);
    HAL_WRITE_UINT32(SA110_TIMER4_CONTROL, SA110_TIMER_CONTROL_ENABLE);
    // Enable the watchdog.
    cyg_uint32 ctrl;
    HAL_READ_UINT32(SA110_CONTROL, ctrl);
    ctrl |= SA110_CONTROL_WATCHDOG;
    HAL_WRITE_UINT32(SA110_CONTROL, ctrl);

    CYG_REPORT_RETURN();
}
#endif


// -------------------------------------------------------------------------
// EOF watchdog_ebsa285.cxx
