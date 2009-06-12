#ifndef CYGONCE_KERNEL_TIMER_HXX
#define CYGONCE_KERNEL_TIMER_HXX

//==========================================================================
//
//      timer.hxx
//
//      Timer handler class declaration(s)
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
// Date:        1998-06-10
// Purpose:     Define Timer class interfaces
// Description: This file defines the Timer class which is derived from
//              the Alarm class to support uITRON type functionality
// Usage:       #include <cyg/kernel/timer.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/kernel/clock.hxx>         // Cyg_Alarm

// -------------------------------------------------------------------------
// Timer handler class

class Cyg_Timer 
    : public Cyg_Alarm
{
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Timer();

    ~Cyg_Timer();

    enum {
        DISABLE = 0,
        ENABLE  = 1,
        RESET   = 2,
    };
    
    void initialize(
        Cyg_Counter    *counter,
        cyg_alarm_fn   alarm_fn,
        CYG_ADDRWORD   data,
        cyg_tick_count trigger,         // absolute time
        cyg_tick_count interval,        // 0 => one shot, else repeating
        cyg_uint32     action           // (DISABLE | ENABLE)
        );
    
    void activate(cyg_uint32 action);   // (DISABLE | ENABLE) [|RESET]

    cyg_tick_count get_trigger();
    cyg_bool       is_enabled();
    cyg_bool       is_initialized();
    CYG_ADDRWORD   get_data();
};

// -------------------------------------------------------------------------
// Timer inlines

inline cyg_tick_count
Cyg_Timer::get_trigger()
{
    return trigger;
}

inline cyg_bool
Cyg_Timer::is_initialized()
{
    return NULL != counter;
}

inline cyg_bool
Cyg_Timer::is_enabled()
{
    return enabled;
}

inline CYG_ADDRWORD
Cyg_Timer::get_data()
{
    return data;
}

#endif // ifndef CYGONCE_KERNEL_TIMER_HXX
// EOF timer.hxx
