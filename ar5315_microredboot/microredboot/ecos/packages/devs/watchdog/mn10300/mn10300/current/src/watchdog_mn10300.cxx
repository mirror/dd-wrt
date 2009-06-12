//==========================================================================
//
//      devs/watchdog/mn10300/mn10300/watchdog_mn10300.cxx
//
//      Watchdog implementation for MN10300
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
// Author(s):    nickg
// Contributors: nickg
// Date:         1999-02-18
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the MN10300 hardware watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package
#include <pkgconf/kernel.h>             // Kernel config

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/kernel/instrmnt.h>        // instrumentation

#include <cyg/hal/hal_io.h>             // IO register access

#include <cyg/kernel/intr.hxx>          // interrupts

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// MN10300 watchdog timer registers

#if defined(CYGPKG_HAL_MN10300_AM31)

#define WATCHDOG_BASE           0x34004000
#define WATCHDOG_COUNTER        (WATCHDOG_BASE)
#define WATCHDOG_CONTROL        (WATCHDOG_BASE+2)
#define WATCHDOG_RESET          (WATCHDOG_BASE+4)

#define WATCHDOG_WDCK0          0x07
#define WATCHDOG_WDCK0_DEFAULT  0x04    // 1016.801ms cycle
#define WATCHDOG_WDRST          0x40
#define WATCHDOG_WDCNE          0x80

#define WATCHDOG_RESOLUTION     1016801000      // cycle time in nanoseconds

#elif defined(CYGPKG_HAL_MN10300_AM33)

#define WATCHDOG_BASE           0xC0001000
#define WATCHDOG_COUNTER        (WATCHDOG_BASE)
#define WATCHDOG_CONTROL        (WATCHDOG_BASE+2)
#define WATCHDOG_RESET          (WATCHDOG_BASE+4)

#define WATCHDOG_WDCK0          0x07
#define WATCHDOG_WDCK0_DEFAULT  0x04    // 621.387ms cycle
#define WATCHDOG_WDRST          0x40
#define WATCHDOG_WDCNE          0x80

#define WATCHDOG_RESOLUTION     621387000       // cycle time in nanoseconds

#else

#error Unsupported MN10300 variant

#endif

// -------------------------------------------------------------------------
// Forward definitions

static cyg_ISR    watchdog_isr;

// -------------------------------------------------------------------------
// Statics

// Interrupt object
static Cyg_Interrupt interrupt(
    CYGNUM_HAL_INTERRUPT_WATCHDOG,
    0,
    0,
    watchdog_isr,
    NULL                                // no DSR
    );

// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();

    // HW doesn't need init.
    
    resolution          = WATCHDOG_RESOLUTION;
        
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start(void)
{
    CYG_REPORT_FUNCTION();

    interrupt.attach();

    HAL_WRITE_UINT8( WATCHDOG_COUNTER, 0 );

    // Set overflow cycle 
    // Enable and reset counter
    HAL_WRITE_UINT8( WATCHDOG_CONTROL,
                     WATCHDOG_WDCK0_DEFAULT|WATCHDOG_WDCNE|WATCHDOG_WDRST);

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    cyg_uint8 ctrl;
    
    HAL_READ_UINT8( WATCHDOG_CONTROL, ctrl );

    ctrl |= WATCHDOG_WDRST;

    HAL_WRITE_UINT8( WATCHDOG_CONTROL, ctrl );
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// ISR

cyg_uint32
watchdog_isr( cyg_vector vector, CYG_ADDRWORD data)
{
    CYG_REPORT_FUNCTION();
    
    // Disable interrupt just in case
    interrupt.detach();

    // Turn watchdog off to prevent it re-triggering.
    HAL_WRITE_UINT8( WATCHDOG_CONTROL, 0 );

    Cyg_Watchdog::watchdog.trigger();

    CYG_REPORT_RETURN();

    return Cyg_Interrupt::HANDLED;
}

// -------------------------------------------------------------------------
// EOF watchdog_mn10300.cxx
