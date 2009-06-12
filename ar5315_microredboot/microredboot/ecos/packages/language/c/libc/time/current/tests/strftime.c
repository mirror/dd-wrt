//=================================================================
//
//        strftime.c
//
//        Testcase for C library strftime() function
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
// Description:   Contains testcode for C library strftime() function
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
    char s[1000];
    size_t size;
    
    tm1.tm_sec = 4;
    tm1.tm_min = 23;
    tm1.tm_hour = 20;
    tm1.tm_mday = 21;
    tm1.tm_mon = 1;
    tm1.tm_year = 74;
    tm1.tm_wday = 4;
    tm1.tm_yday = 51;
    tm1.tm_isdst = 0;
    
    size = strftime(s, 1000, "", &tm1);
    CYG_TEST_PASS_FAIL((size==0) && (*s=='\0'), "strftime test #1");

    size = strftime(s, 1000, "%a", &tm1);
    CYG_TEST_PASS_FAIL((size==3) && !my_strcmp(s, "Thu"), "strftime test #2");

    size = strftime(s, 1000, "%A", &tm1);
    CYG_TEST_PASS_FAIL((size==8) && !my_strcmp(s, "Thursday"),
                       "strftime test #3");

    size = strftime(s, 1000, "%b", &tm1);
    CYG_TEST_PASS_FAIL((size==3) && !my_strcmp(s, "Feb"),
                       "strftime test #4");

    size = strftime(s, 1000, "%B", &tm1);
    CYG_TEST_PASS_FAIL((size==8) && !my_strcmp(s, "February"),
                       "strftime test #5");

    size = strftime(s, 1000, "%d", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "21"),
                       "strftime test #6");

    size = strftime(s, 1000, "%H", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "20"),
                       "strftime test #7");

    size = strftime(s, 1000, "%I", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "09"),
                       "strftime test #8");

    size = strftime(s, 1000, "%j", &tm1);
    CYG_TEST_PASS_FAIL((size==3) && !my_strcmp(s, "051"),
                       "strftime test #9");

    size = strftime(s, 1000, "%m", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "02"),
                       "strftime test #10");

    size = strftime(s, 1000, "%M", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "23"),
                       "strftime test #11");

    size = strftime(s, 1000, "%p", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "pm"),
                       "strftime test #12");

    size = strftime(s, 1000, "%S", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "04"),
                       "strftime test #13");

    size = strftime(s, 1000, "%U", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "07"),
                       "strftime test #14");

    size = strftime(s, 1000, "%w", &tm1);
    CYG_TEST_PASS_FAIL((size==1) && !my_strcmp(s, "4"),
                       "strftime test #15");

    size = strftime(s, 1000, "%W", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "07"),
                       "strftime test #16");

    size = strftime(s, 1000, "%y", &tm1);
    CYG_TEST_PASS_FAIL((size==2) && !my_strcmp(s, "74"),
                       "strftime test #17");

    size = strftime(s, 1000, "%Y", &tm1);
    CYG_TEST_PASS_FAIL((size==4) && !my_strcmp(s, "1974"),
                       "strftime test #18");

    size = strftime(s, 1000, "%%", &tm1);
    CYG_TEST_PASS_FAIL((size==1) && !my_strcmp(s, "%"),
                       "strftime test #19");

    size = strftime(s, 5, "%Y", &tm1);
    CYG_TEST_PASS_FAIL((size==4) && !my_strcmp(s, "1974"),
                       "strftime test #20");

    size = strftime(s, 4, "%Y", &tm1);
    CYG_TEST_PASS_FAIL((size==0), "strftime test #21");


    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strftime() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strftime() function");

    START_TEST( test );

    CYG_TEST_NA("Testing is not applicable to this configuration");

} // main()

// EOF strftime.c
