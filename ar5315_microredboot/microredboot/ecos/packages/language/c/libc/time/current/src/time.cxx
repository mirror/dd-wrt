//========================================================================
//
//      time.cxx
//
//      ISO C date and time implementation for time()
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
// Author(s):    jlarmour
// Contributors: jlarmour
// Date:         1999-03-04
// Purpose:      Provide implementation of ISO C time() as defined in
//               ISO C section 7.12.2.4
// Description:  This file provides an implementation of time() using
//               the "wallclock" device
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure
#include <time.h>                  // Main date and time definitions

#ifdef CYGSEM_LIBC_TIME_TIME_WORKING
# include <cyg/io/wallclock.hxx> // Wallclock class definitions
#endif

// FUNCTIONS

externC time_t
time( time_t *timer )
{
    CYG_REPORT_FUNCNAMETYPE("time", "returning %d");
    if (timer) // its allowed to be NULL
        CYG_CHECK_DATA_PTR( timer, "timer is not a valid pointer!");
    CYG_REPORT_FUNCARG1("timer = %08x", timer);

#ifdef CYGSEM_LIBC_TIME_TIME_WORKING
    time_t ret;

    ret = (time_t) Cyg_WallClock::wallclock->get_current_time();
    
    if (timer)
        *timer = ret;

    CYG_REPORT_RETVAL(ret);

    return ret;

#else // i.e. ifndef CYGSEM_LIBC_TIME_TIME_WORKING

    if (timer)
        *timer = (time_t)-1;
    CYG_REPORT_RETVAL(-1);
    return (time_t)-1;

#endif

} // time()


// EOF time.cxx
