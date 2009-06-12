//==========================================================================
//
//      io/watchdog/watchdog.cxx
//
//      Watchdog common code
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
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package

#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/drv_api.h>            // for locking

#include <cyg/io/watchdog.hxx>          // watchdog API
#include <cyg/io/watchdog.h>            // watchdog c-api

// -------------------------------------------------------------------------
// Statics

// A static pointer to the single system defined watchdog device.
Cyg_Watchdog Cyg_Watchdog::watchdog;

// -------------------------------------------------------------------------
// Constructor


Cyg_Watchdog::Cyg_Watchdog()
{
    CYG_REPORT_FUNCTION();

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT    
    action_list         = 0;
#endif

    // HW driver initialization. This must set the watchdog resolution.
    init_hw();
        
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Return reset resolution

cyg_uint64
Cyg_Watchdog::get_resolution()
{
    return resolution;
}

#ifndef CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT
// -------------------------------------------------------------------------
// Trigger the watchdog as if the timer had expired. This should be called
// from the driver's ISR.

void
Cyg_Watchdog::trigger()
{
    CYG_REPORT_FUNCTION();
    
    cyg_drv_dsr_lock();
    
    Cyg_Watchdog_Action *act = action_list;

    while( 0 != act )
    {
        act->action( act->data );

        act = act->next;
    }

    cyg_drv_dsr_unlock();

    CYG_REPORT_RETURN();
}
    
// -------------------------------------------------------------------------
// Register an action routine that will be called when the timer
// triggers.

void
Cyg_Watchdog::install_action( Cyg_Watchdog_Action *action )
{
    CYG_REPORT_FUNCTION();
    
    cyg_drv_dsr_lock();
    
    action->next = action_list;
    action_list = action;

    cyg_drv_dsr_unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Deregister a previously registered action routine.

void
Cyg_Watchdog::uninstall_action( Cyg_Watchdog_Action *action )
{
    CYG_REPORT_FUNCTION();
    
    cyg_drv_dsr_lock();

    Cyg_Watchdog_Action **act_ptr = &action_list;    

    while( 0 != *act_ptr )
    {
        Cyg_Watchdog_Action *a = *act_ptr;

        if( a == action )
        {
            *act_ptr = a->next;
            break;
        }
        act_ptr = &a->next;
    }
    
    cyg_drv_dsr_unlock();

    CYG_REPORT_RETURN();
}

#endif // CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT

// -------------------------------------------------------------------------
// Implementation of the C-api

externC void
watchdog_start(void)
{
  Cyg_Watchdog::watchdog.start();
}

externC void
watchdog_reset(void)
{
  Cyg_Watchdog::watchdog.reset();
}

externC cyg_uint64
watchdog_get_resolution(void)
{
  return Cyg_Watchdog::watchdog.get_resolution();
}

// -------------------------------------------------------------------------
// EOF io/watchdog/watchdog.cxx
