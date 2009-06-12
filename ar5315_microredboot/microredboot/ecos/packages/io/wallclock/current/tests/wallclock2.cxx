//==========================================================================
//
//        wallclock2.cxx
//
//        WallClock battery backed up test
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
// Date:          2000-03-29
// Description:   Tests the WallClock device.
//                Prints wallclock setting (for human verification that time
//                matches the expected value). Then tries to set/restore
//                clock value, and verifies that the WallClock epoch matches
//                that of the libc.
//####DESCRIPTIONEND####
// -------------------------------------------------------------------------

#include <pkgconf/system.h>
#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>
#ifdef CYGPKG_LIBC_TIME
#include <pkgconf/libc_time.h>
#endif
#ifdef CYGPKG_LIBC_STARTUP
#include <pkgconf/libc_startup.h>
#endif

#if !defined(CYGSEM_LIBC_TIME_CLOCK_WORKING)
# define NA_MSG "Requires libc time functions"
#elif !defined(CYGPKG_LIBC_STARTUP)
# define NA_MSG "Requires libc startup package"
#endif

#ifndef NA_MSG
#include <time.h>
#include <string.h>                     // strcmp
#include <cyg/io/wallclock.hxx>         // The WallClock API

// -------------------------------------------------------------------------


externC int
main (void )
{
    time_t now, test, test2;

    CYG_TEST_INIT();

    // make this predictable - independent of the user option
    cyg_libc_time_setzoneoffsets(0, 3600);
    cyg_libc_time_setdst( CYG_LIBC_TIME_DSTOFF );

    // Print out current setting so a human can verify that clock state
    // was valid.
    now = (time_t) Cyg_WallClock::wallclock->get_current_time();
    diag_printf("WallClock is set to: %s", ctime(&now));

    // Check that set/get works
    Cyg_WallClock::wallclock->set_current_time( 0 );
    test = (time_t) Cyg_WallClock::wallclock->get_current_time();
    // 2000.03.01 00:00:00 UTC
    Cyg_WallClock::wallclock->set_current_time( 951868800 );
    test2 = (time_t) Cyg_WallClock::wallclock->get_current_time();
    // Should test 2100 and 2400 leap year calculations as well, but 
    // these would overflow with today's time_t.
    Cyg_WallClock::wallclock->set_current_time( now );
    CYG_TEST_PASS_FAIL(2 >= test,  // each operation can take one second
                       "Can set WallClock to epoch");
    CYG_TEST_PASS_FAIL(951868800+2 > test2 &&
                       951868800 <= test2,
                       "WallClock date conversion Y2K safe");

    // Test that the wallclock and libc use same epoch.
    test = (time_t) 0;
    CYG_TEST_PASS_FAIL(!strcmp(ctime(&test), "Thu Jan 01 00:00:00 1970\n"),
                       "WallClock epoch matches libc epoch");

    CYG_TEST_FINISH("Finished wallclock battery test");
}

#else 

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_PASS_FINISH(NA_MSG);
}

#endif // NA_MSG

// -------------------------------------------------------------------------
// EOF wallclock2.cxx
