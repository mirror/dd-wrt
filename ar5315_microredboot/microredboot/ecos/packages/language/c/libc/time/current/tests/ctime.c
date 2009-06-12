//=================================================================
//
//        ctime.c
//
//        Testcase for C library ctime() function
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
// Description:   Contains testcode for C library ctime() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

#include <time.h>
#include <cyg/infra/testcase.h>

// HOW TO START TESTS

# define START_TEST( test ) test(0)

// FUNCTIONS

static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


static void
test( CYG_ADDRWORD data )
{
    time_t t;
    char *ret;
    
    // make this predictable - independent of the user option
    cyg_libc_time_setzoneoffsets(0, 3600);
    cyg_libc_time_setdst( CYG_LIBC_TIME_DSTOFF );

    t = (time_t)130710184;

    ret = ctime(&t);
    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Thu Feb 21 20:23:04 1974\n"),
                       "ctime test #1");

    t = (time_t)946689894;

    ret = ctime(&t);
    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Sat Jan 01 01:24:54 2000\n"),
                       "ctime Y2K test #2");

    cyg_libc_time_setdst( CYG_LIBC_TIME_DSTON );

    t = (time_t)-113186106;

    ret = ctime(&t);
    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Tue Jun 01 00:24:54 1966\n"),
                       "ctime test #3");

#ifdef CYGFUN_LIBC_TIME_POSIX
    cyg_libc_time_setdst( CYG_LIBC_TIME_DSTOFF );

    t = (time_t)915510061;
    
    {
        char ret2[26];

        ret = ctime_r(&t, ret2);
        CYG_TEST_PASS_FAIL(!my_strcmp(ret2, "Tue Jan 05 04:21:01 1999\n"),
                           "ctime_r test #1");
    }
#endif

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "ctime() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "ctime() function");

    START_TEST( test );

    CYG_TEST_NA("Testing is not applicable to this configuration");

} // main()

// EOF ctime.c
