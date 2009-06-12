//=================================================================
//
//        asctime.c
//
//        Testcase for C library asctime() function
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
// Date:          1999-03-05
// Description:   Contains testcode for C library asctime() function
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
    struct tm tm1;
    char *ret;
    
    tm1.tm_sec = 4;
    tm1.tm_min = 23;
    tm1.tm_hour = 20;
    tm1.tm_mday = 21;
    tm1.tm_mon = 1;
    tm1.tm_year = 74;
    tm1.tm_wday = 4;
    tm1.tm_yday = 51;
    tm1.tm_isdst = 0;

    ret = asctime(&tm1);

    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Thu Feb 21 20:23:04 1974\n"),
                       "asctime test #1");
    tm1.tm_sec = 3;
    tm1.tm_min = 51;
    tm1.tm_hour = 5;
    tm1.tm_mday = 2;
    tm1.tm_mon = 11;
    tm1.tm_year = 68;
    tm1.tm_wday = 1;
    tm1.tm_yday = 336;
    tm1.tm_isdst = 0;

    ret = asctime(&tm1);

    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Mon Dec 02 05:51:03 1968\n"),
                       "asctime test #2");

    // make this predictable - independent of the user option
    cyg_libc_time_setzoneoffsets(0, 3600);

    tm1.tm_sec = 3;
    tm1.tm_min = 51;
    tm1.tm_hour = 5;
    tm1.tm_mday = 2;
    tm1.tm_mon = 6;
    tm1.tm_year = 68;
    tm1.tm_wday = 2;
    tm1.tm_yday = 183;
    tm1.tm_isdst = 1;

    ret = asctime(&tm1);

    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Tue Jul 02 05:51:03 1968\n"),
                       "asctime test #3");

    tm1.tm_sec = 0;
    tm1.tm_min = 0;
    tm1.tm_hour = 0;
    tm1.tm_mday = 1;
    tm1.tm_mon = 0;
    tm1.tm_year = 0;
    tm1.tm_wday = 1;
    tm1.tm_yday = 0;
    tm1.tm_isdst = 0;

    ret = asctime(&tm1);

    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Mon Jan 01 00:00:00 1900\n"),
                       "asctime test #4");

    tm1.tm_sec = 0;
    tm1.tm_min = 0;
    tm1.tm_hour = 0;
    tm1.tm_mday = 1;
    tm1.tm_mon = 0;
    tm1.tm_year = 100;
    tm1.tm_wday = 6;
    tm1.tm_yday = 0;
    tm1.tm_isdst = 0;

    ret = asctime(&tm1);

    CYG_TEST_PASS_FAIL(!my_strcmp(ret, "Sat Jan 01 00:00:00 2000\n"),
                       "asctime Y2K test #5");

#ifdef CYGFUN_LIBC_TIME_POSIX
    {
        char ret2[100];
        
        tm1.tm_sec = 3;
        tm1.tm_min = 51;
        tm1.tm_hour = 5;
        tm1.tm_mday = 2;
        tm1.tm_mon = 11;
        tm1.tm_year = 68;
        tm1.tm_wday = 1;
        tm1.tm_yday = 336;
        tm1.tm_isdst = 0;
        
        ret = asctime_r(&tm1, ret2);
        
        CYG_TEST_PASS_FAIL((ret==ret2) &&
                           !my_strcmp(ret, "Mon Dec 02 05:51:03 1968\n"),
                           "asctime_r test #1");

    }
#endif


    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "asctime() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "asctime() function");

    START_TEST( test );

    CYG_TEST_NA("Testing is not applicable to this configuration");

} // main()

// EOF asctime.c
