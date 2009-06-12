//=================================================================
//
//        mktime.c
//
//        Testcase for C library mktime() function
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
// Description:   Contains testcode for C library mktime() function
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

static int
cmp_structtm(struct tm *tm1, struct tm *tm2)
{
    if ((tm1->tm_sec   != tm2->tm_sec)   ||
        (tm1->tm_min   != tm2->tm_min)   ||
        (tm1->tm_hour  != tm2->tm_hour)  ||
        (tm1->tm_mday  != tm2->tm_mday)  ||
        (tm1->tm_mon   != tm2->tm_mon)   ||
        (tm1->tm_year  != tm2->tm_year)  ||
        (tm1->tm_wday  != tm2->tm_wday)  ||
        (tm1->tm_yday  != tm2->tm_yday)  ||
        (tm1->tm_isdst != tm2->tm_isdst))
        return 1;
    else
        return 0;
}

static void
test( CYG_ADDRWORD data )
{
    struct tm tm1, tm2;
    time_t t;
    
    // make this predictable - independent of the user option
    cyg_libc_time_setzoneoffsets(0, 3600);

    tm1.tm_sec = 4;
    tm1.tm_min = 23;
    tm1.tm_hour = 20;
    tm1.tm_mday = 21;
    tm1.tm_mon = 1;
    tm1.tm_year = 74;
    tm1.tm_isdst = 0;

    tm2 = tm1;
    tm2.tm_wday = 4;
    tm2.tm_yday = 51;

    t = mktime(&tm1);
    CYG_TEST_PASS_FAIL(!cmp_structtm(&tm1, &tm2), "mktime test #1");
    CYG_TEST_PASS_FAIL(t == 130710184, "mktime return test #1");

    tm1.tm_sec = 61;
    tm1.tm_min = 20;
    tm1.tm_hour = 4;
    tm1.tm_mday = 5;
    tm1.tm_mon = 0;
    tm1.tm_year = 99;
    tm1.tm_isdst = 0;

    tm2 = tm1;
    tm2.tm_sec=1;
    tm2.tm_min = 21;
    tm2.tm_wday = 2;
    tm2.tm_yday = 4;

    t = mktime(&tm1);
    CYG_TEST_PASS_FAIL(!cmp_structtm(&tm1, &tm2), "mktime test #2");
    CYG_TEST_PASS_FAIL(t == 915510061, "mktime return test #2");

    tm1.tm_sec = 54;
    tm1.tm_min = 24;
    tm1.tm_hour = 1;
    tm1.tm_mday = 1;
    tm1.tm_mon = 0;
    tm1.tm_year = 100;
    tm1.tm_isdst = 0;

    tm2 = tm1;
    tm2.tm_wday = 6;
    tm2.tm_yday = 0;

    t = mktime(&tm1);
    CYG_TEST_PASS_FAIL(!cmp_structtm(&tm1, &tm2), "mktime Y2K test #3");
    CYG_TEST_PASS_FAIL(t == 946689894, "mktime Y2K return test #3");

    tm1.tm_sec = 54;
    tm1.tm_min = 24;
    tm1.tm_hour = 23;
    tm1.tm_mday = 31;
    tm1.tm_mon = 4;
    tm1.tm_year = 66;
    tm1.tm_isdst = 1;

    tm2 = tm1;
    tm2.tm_wday = 2;
    tm2.tm_yday = 150;

    t = mktime(&tm1);
    CYG_TEST_PASS_FAIL(!cmp_structtm(&tm1, &tm2), "mktime test #4");
    CYG_TEST_PASS_FAIL(t == -113186106, "mktime return test #4");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "mktime() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "mktime() function");

    START_TEST( test );

    CYG_TEST_NA("Testing is not applicable to this configuration");

} // main()

// EOF mktime.c
