//==========================================================================
//
//      common/timer.cxx
//
//      Timer class implementations
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
// Author(s):   dsm
// Contributors:        dsm
// Date:        1998-06-11
// Purpose:     Clock class implementation
// Description: This file implements the Timer class which is derived from
//              the Alarm class to support uITRON type functionality
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/timer.hxx>

#include <cyg/kernel/clock.inl>        // Clock inlines

// -------------------------------------------------------------------------

Cyg_Timer::Cyg_Timer()
{
}

Cyg_Timer::~Cyg_Timer()
{
    CYG_REPORT_FUNCTION();

    disable();
    counter = NULL;
}

// -------------------------------------------------------------------------

void
Cyg_Timer::initialize(
        Cyg_Counter    *c,
        cyg_alarm_fn   a,
        CYG_ADDRWORD   d,
        cyg_tick_count t,               // absolute time
        cyg_tick_count i,               // 0 => one shot, else repeating
        cyg_uint32     action           // (DISABLE | ENABLE)
        )
{
    CYG_REPORT_FUNCTION();

    counter  = c;
    alarm    = a;
    data     = d;
    trigger  = t;
    interval = i;
    enabled  = false;

    CYG_ASSERT(0 == (action & ~ENABLE), "unknown action");

    if(action & ENABLE)
        enable();
}
    
void
Cyg_Timer::activate(cyg_uint32 action)  // (DISABLE | ENABLE) [|RESET]
{
    // we must also disable the alarm when resetting it so as to remove it
    // from its queue, so that the enable afterwards places it on the right
    // queue instead of assuming that, being enabled, it's already there.
    // (if not enabling, the behaviour is unchanged and correct)
    if(!(action & ENABLE) || (action & RESET) )
        disable(); // otherwise, the enable below does nothing...

    if((action & RESET))
    {
        cyg_tick_count t;
        t = counter->current_value();
        trigger = t + interval;
    }

    if((action & ENABLE)) {
        enable(); // ...when it should put the timer on a new queue.
    }
}

// -------------------------------------------------------------------------
// EOF common/timer.cxx
