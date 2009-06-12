//==========================================================================
//
//      io/watchdog/emulate.cxx
//
//      Watchdog implementation emulation
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-07-29
// Purpose:     Watchdog class implementation
// Description: Contains an implementation of the Watchdog class for use
//              when there is no hardware watchdog timer. Instead it is
//              emulated using an Alarm object.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/watchdog.h>           // watchdog configuration file
#include <pkgconf/kernel.h>             // Kernel config

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/kernel/instrmnt.h>        // instrumentation

#include <cyg/kernel/clock.hxx>         // clock and alarm
#include <cyg/kernel/sched.hxx>         // scheduler

#include <cyg/io/watchdog.hxx>          // watchdog API

#include <cyg/kernel/sched.inl>         // scheduler inlines

// -------------------------------------------------------------------------
// Forward definitions

static cyg_alarm_fn    watchdog_alarm;

// -------------------------------------------------------------------------
// Statics

static Cyg_Alarm alarm( Cyg_Clock::real_time_clock, watchdog_alarm, 0 );

// One second's worth of ticks.
static cyg_tick_count one_sec;

// -------------------------------------------------------------------------
// HW init

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();

    Cyg_Clock::cyg_resolution res = Cyg_Clock::real_time_clock->get_resolution();

    one_sec             = ( res.divisor * 1000000000LL ) / res.dividend ;

    resolution          = 1000000000LL;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void Cyg_Watchdog::start()
{
    CYG_REPORT_FUNCTION();
    
    Cyg_Clock::cyg_resolution res = Cyg_Clock::real_time_clock->get_resolution();

    // Set alarm to a one second single-shot trigger
    alarm.initialize( Cyg_Clock::real_time_clock->current_value() + one_sec, 0 );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();
    
    Cyg_Clock::cyg_resolution res = Cyg_Clock::real_time_clock->get_resolution();

    Cyg_Scheduler::lock();
    
    // Set alarm to a one second single-shot trigger
    alarm.initialize( Cyg_Clock::real_time_clock->current_value() + one_sec, 0 );    

    Cyg_Scheduler::unlock();    

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Alarm function

void watchdog_alarm( Cyg_Alarm *a, CYG_ADDRWORD data)
{
    CYG_REPORT_FUNCTION();
    
    // Disable alarm just in case
    alarm.disable();

    Cyg_Watchdog::watchdog.trigger();
}

// -------------------------------------------------------------------------
// EOF watchdog/emulate.cxx
