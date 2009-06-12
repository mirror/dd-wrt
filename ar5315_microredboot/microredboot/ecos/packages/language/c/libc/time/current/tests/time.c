//=================================================================
//
//        time.c
//
//        Testcase for C library time()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-03-05
// Description:   Contains testcode for C library time() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_time.h>   // Configuration header

// INCLUDES

#include <time.h>
#include <cyg/infra/testcase.h>

// CONSTANTS

// This defines how many loops before we decide that
// time() doesn't work
#define MAX_TIMEOUT 50000000


// FUNCTIONS

int
main(int argc, char *argv[])
{
    time_t t1, t2;
    unsigned long ctr;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "time() function");

    t1 = time(&t2);

    CYG_TEST_PASS_FAIL(t1==t2, "time() return value == argument");
    
    if (t1 == (time_t)-1)  // unimplemented is just as valid
    {
#ifndef CYGSEM_LIBC_TIME_TIME_WORKING
        CYG_TEST_PASS_FINISH( "time() returns -1, meaning unimplemented");
#else
        CYG_TEST_FAIL("time() returned -1 unnecessarily");
#endif
    } // if

    // First wait for a clock tick 

    for (ctr = 0; ctr<MAX_TIMEOUT; ctr++) {
        if ((t2=time(NULL)) > t1)
            break; // Hit the next time pulse
    }
    CYG_TEST_PASS_FAIL( ctr< MAX_TIMEOUT, "time()'s state changes");
    
#ifdef CYGSEM_LIBC_TIME_SETTIME_WORKING
    CYG_TEST_PASS_FAIL(cyg_libc_time_settime(0)==0, "Set time to 0");
    
    t1 = time(NULL);
    
    // give it a small amount of tolerance
    CYG_TEST_PASS_FAIL(t1 < 3, "t1 remembered setting");
    
    CYG_TEST_PASS_FAIL(cyg_libc_time_settime(1000)==0, "Set time to 1000");
    
    // give it a small amount of tolerance
    CYG_TEST_PASS_FAIL(t1 < 1003, "t1 remembered setting");
    
#else // ! CYGSEM_LIBC_TIME_SETTIME_WORKING
    CYG_TEST_PASS_FAIL(cyg_libc_time_settime(0)!=0,
                       "Set time expected fail");
#endif // CYGSEM_LIBC_TIME_SETTIME_WORKING

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "time() function");
} // main()

// EOF time.c
