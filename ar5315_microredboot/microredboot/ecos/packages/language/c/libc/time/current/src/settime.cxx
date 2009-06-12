//========================================================================
//
//      settime.cxx
//
//      Date and time support function to set time
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-02-26
// Purpose:       Provide support function to set the time
// Description:   The wallclock device is used to store the time in UTC
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <time.h>                  // Main date and time definitions
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure
#ifdef CYGSEM_LIBC_TIME_SETTIME_WORKING
# include <cyg/io/wallclock.hxx> // Wallclock class definitions
#endif

// FUNCTIONS

/////////////////////////////
// cyg_libc_time_settime() //
/////////////////////////////
//
// This function sets the current time for the system. The time is
// specified as a time_t in UTC. It returns non-zero on error.
//

externC cyg_bool
cyg_libc_time_settime( time_t utctime )
{
    CYG_REPORT_FUNCNAMETYPE("cyg_libc_time_settime", "returning status %d");
    CYG_REPORT_FUNCARG1DV(utctime);

#ifdef CYGSEM_LIBC_TIME_SETTIME_WORKING

    Cyg_WallClock::wallclock->set_current_time((cyg_uint32) utctime);
    
    CYG_REPORT_RETVAL(false);
    return false;
#else
    CYG_REPORT_RETVAL(true);
    return true;
#endif

} // cyg_libc_time_settime()

// EOF settime.cxx
