//=================================================================
//
//        memchr.c
//
//        Testcase for C library memchr()
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
// Description:   Contains testcode for C library memchr() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

// Functions to avoid having to use libc strings

static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()

int main( int argc, char *argv[] )
{
    char x[] = "Your feeble skills are no match for the power of "
               "the dark side!";
    void *ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "memchr() function");

    // Check 1
    ret = memchr(x, (int)'k', sizeof(x));
    CYG_TEST_PASS_FAIL( (my_strcmp(ret, x+13) == 0), "Simple test 1" );

    // Check 2
    ret = memchr(x, (int)'p', sizeof(x));
    CYG_TEST_PASS_FAIL( (my_strcmp(ret, x+40) == 0), "Simple test 2" );

    // Check 3
    ret = memchr(x, (int)'z', sizeof(x));
    CYG_TEST_PASS_FAIL( (ret == NULL), "Simple test 3" );

    // Check 4 (boundary condition)
    ret = memchr(x, (int)'\0', sizeof(x));
    CYG_TEST_PASS_FAIL( ret == (x+sizeof(x)-1), "Boundary test 1" );

    // Check 5 (boundary condition)
    ret = memchr(x, (int)'\0', sizeof(x)-1);
    CYG_TEST_PASS_FAIL( ret == NULL, "Boundary test 2" );

    // Check 6 (boundary condition)
    ret = memchr( (void *)"", (int)'\0', 0 );
    CYG_TEST_PASS_FAIL( ret == NULL, "Boundary test 3" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "memchr() function");
} // main()

// EOF memchr.c
