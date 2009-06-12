//========================================================================
//
//      signal1.c
//
//      ISO C signal handling test
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
// Contributors:  
// Date:          2000-04-18
// Purpose:       Test signal functionality
// Description:   This file contains a number of tests for ISO C signal
//                handling
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_signals.h>  // libc signals configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/testcase.h>    // Test infrastructure
#include <signal.h>                // Signal functions
#include <setjmp.h>                // setjmp(), longjmp()
#include <stdlib.h>                // abort()

// STATICS

static int state;
static jmp_buf jbuf;

// FUNCTIONS

static void
myhandler1(int signal)
{
    CYG_TEST_INFO("myhandler1 called");
    ++state;
} // myhandler1()

static void
myhandler2(int signal)
{
    CYG_TEST_INFO("myhandler2 called");
    ++state;
    longjmp(jbuf, 1);
} // myhandler2()

int
main( int argc, char *argv[] )
{
    __sighandler_t handler1;
    int rc;

    // special callout to request GDB to alter its handling of signals
    CYG_TEST_GDBCMD("handle SIGTERM nostop");
    CYG_TEST_GDBCMD("handle SIGABRT nostop");

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library signal functions");

    // Test 1

    CYG_TEST_INFO("Test 1");
    state = 1;
    handler1 = signal(SIGTERM, &myhandler1);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGTERM handler initialized to default");

    rc = raise(SIGTERM);

    CYG_TEST_PASS_FAIL(0==rc, "raise(SIGTERM) did not return error");

    CYG_TEST_PASS_FAIL(2==state, "SIGTERM handler returned correctly");

    // Test 2

    CYG_TEST_INFO("Test 2");

    state = 2;
    handler1 = signal(SIGTERM, &myhandler2);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGTERM handler reset to default after test 1");

    handler1 = signal(SIGTERM, &myhandler1);

    CYG_TEST_PASS_FAIL(handler1 == &myhandler2,
                       "SIGTERM handler was set correctly");

    rc = raise(SIGTERM);

    CYG_TEST_PASS_FAIL(0==rc, "raise(SIGTERM) did not return error");

    CYG_TEST_PASS_FAIL(3==state, "SIGTERM handler returned correctly");

    // Test 3

    CYG_TEST_INFO("Test 3");

    handler1 = signal(SIGTERM, &myhandler2);
    
    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGTERM handler reset to default after test 2");

    handler1 = signal(SIGTERM, SIG_DFL);

    CYG_TEST_PASS_FAIL(handler1 == &myhandler2,
                       "SIGTERM handler was set correctly");

    // Test 4

    CYG_TEST_INFO("Test 4");

    state = 4;
    handler1 = signal(SIGTERM, SIG_IGN);

    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGTERM handler was set correctly after test 3");
    rc = raise(SIGTERM);

    CYG_TEST_PASS_FAIL(0==rc, "raise(SIGTERM) did not return error");

    CYG_TEST_PASS_FAIL(4==state, "SIGTERM ignored");
    
    // Test 5

    CYG_TEST_INFO("Test 5");

    state = 5;
    handler1 = signal(SIGTERM, &myhandler2);

    // SIG_IGN doesn't reset back to SIG_DFL after a raise()
    CYG_TEST_PASS_FAIL(handler1 == SIG_IGN,
                       "SIGTERM handler was set correctly after test 4");
    
    if (0==setjmp(jbuf)) {
        raise(SIGTERM);
        CYG_TEST_FAIL("raise returned");
    }
    
    CYG_TEST_PASS_FAIL(6==state, "SIGTERM handler returned correctly");

    // Test 6

    CYG_TEST_INFO("Test 6");

    state = 6;
    handler1 = signal(SIGABRT, &myhandler2);
    
    CYG_TEST_PASS_FAIL(handler1 == SIG_DFL,
                       "SIGABRT handler initialized to default");
    
    if (0==setjmp(jbuf)) {
        abort();
        CYG_TEST_FAIL("abort returned");
    }

    CYG_TEST_PASS_FAIL(7==state, "SIGABRT handler returned correctly");
    
    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library signal functions");

    return 0;
} // main()

// EOF signal1.c
