//=================================================================
//
//        atexit.c
//
//        Testcase for C library atexit()
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
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library atexit() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_startup.h>   // Configuration header

// INCLUDES

#include <stdlib.h>                 // Main header for stdlib functions
#include <cyg/infra/testcase.h>     // Testcase API


// STATICS

static int stage;
static int failed;

// FUNCTIONS

#if defined(CYGFUN_LIBC_ATEXIT)

static void
myfun1( void )
{
    if (!failed)
    {
        CYG_TEST_PASS_FAIL( stage==0, "first function called correctly" );
        stage=1;
    } // if
} // myfun1()

static void
myfun2( void )
{
    if (!failed)
    {
        CYG_TEST_PASS_FAIL( stage==1, "Second function called correctly" );
        stage=2;
    } // if
} // myfun2()

static void
myfun3( void )
{
    if (!failed)
    {
        CYG_TEST_PASS_FAIL( stage==2, "Second function called correctly" );
        stage=3;
    } // if

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library atexit() function");

} // myfun3()

#endif // if defined(CYGFUN_LIBC_ATEXIT)

int
main( int argc, char *argv[] )
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library atexit() function");

#if defined(CYGFUN_LIBC_ATEXIT)

    // we only have one test in us! We can only exit once :-)

    CYG_TEST_PASS_FAIL( atexit(&myfun3)==0, 
                        "Simple registration of first atexit() function" );

    CYG_TEST_PASS_FAIL( atexit(&myfun2)==0, 
                       "Simple registration of second atexit() function" );

    CYG_TEST_PASS_FAIL( atexit(&myfun1)==0, 
                        "Simple registration of third atexit() function" );

    return 0;
#else
    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library atexit() function");

#endif // if defined(CYGFUN_LIBC_ATEXIT)

} // main()

// EOF atexit.c
