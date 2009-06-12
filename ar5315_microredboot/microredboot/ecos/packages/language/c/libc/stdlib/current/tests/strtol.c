//=================================================================
//
//        strtol.c
//
//        Testcase for C library strtol()
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
// Author(s):     ctarpy, jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library strtol() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

static char *
my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()


static void
test( CYG_ADDRWORD data )
{
    char x[30];
    long z;
    char *endptr;

    my_strcpy(x, "20");
    CYG_TEST_PASS_FAIL( strtol(x, (char**)NULL, 10) == 20,
                        "Simple strtol(20 ,...)" );

    my_strcpy(x, "1972100");
    CYG_TEST_PASS_FAIL( strtol(x, (char**)NULL, 10) == 1972100,
                        "Simple strtol(1972100, ..." );

    my_strcpy(x, "0xFFEE");
    z = strtol(x, (char**)NULL, 16);
    CYG_TEST_PASS_FAIL(z == 65518, "Hex base strtol()");

    my_strcpy(x, "100111011");
    z = strtol(x, (char**)NULL, 2);
    CYG_TEST_PASS_FAIL(z == 315, "Binary base strtol()");

    my_strcpy(x, "10372");
    z = strtol(x, (char**)NULL, 8);
    CYG_TEST_PASS_FAIL(z == 4346, "Octal base strtol()" );

    my_strcpy(x, "317823");
    z = strtol(x, (char**)NULL, 8);
    CYG_TEST_PASS_FAIL(z == 207, "Partial string" );

    my_strcpy(x, " 53ab823");
    z = strtol(x, &endptr, 10);
    CYG_TEST_PASS_FAIL((z == 53) && (endptr==&x[3]), "Correct end pointer" );

    my_strcpy(x, "-479");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL(z == -479, "Negative string");

    my_strcpy(x, "+4796");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL(z == 4796, "Positive string");

    my_strcpy(x, "");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL(z == 0, "Empty string");

    my_strcpy(x, "");
    z = strtol(x, &endptr, 10);
    CYG_TEST_PASS_FAIL((z == 0) && (endptr==x),
                       "Empty string sets endptr correctly");

    my_strcpy(x, "    ");
    z = strtol(x, &endptr, 10);
    CYG_TEST_PASS_FAIL((z == 0) && (endptr==x),
                       "White space only string sets endptr correctly");

    my_strcpy(x, "0XFFEE");
    z = strtol(x, (char**)NULL, 0);
    CYG_TEST_PASS_FAIL(z == 65518, "Base 0 but hex");

    my_strcpy(x, "\t    0629");
    z = strtol(x, (char**)NULL, 0);
    CYG_TEST_PASS_FAIL(z == 50, "Base 0 but octal");

    my_strcpy(x, "42");
    z = strtol(x, (char**)NULL, 0);
    CYG_TEST_PASS_FAIL(z == 42, "Base 0 but decimal");

    my_strcpy(x, "hello");
    z = strtol(x, &endptr, 0);
    CYG_TEST_PASS_FAIL((z == 0) && (endptr==x),
                       "endptr set correctly on conversion failure");

    my_strcpy(x, "z2f");
    z = strtol(x, (char**)NULL, 36);
    CYG_TEST_PASS_FAIL(z == 45447, "Base==36");

    my_strcpy(x, "h547324");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL(z == 0, "No valid number string");

    my_strcpy(x, "545425876654547324");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL( (z == LONG_MAX) && (errno == ERANGE),
                        "Number out of range");

    my_strcpy(x, "-545425876654547324");
    z = strtol(x, (char**)NULL, 10);
    CYG_TEST_PASS_FAIL( (z == LONG_MIN) && (errno == ERANGE),
                        "Number out of range");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strtol() function");
} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strtol() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    test(0);

    CYG_TEST_NA("Testing is not applicable to this configuration");
} // main()

// EOF strtol.c
