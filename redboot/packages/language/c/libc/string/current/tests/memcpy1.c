//=================================================================
//
//        memcpy1.c
//
//        Testcase for C library memcpy()
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
// Description:   Contains testcode for C library memcpy() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

// Functions to avoid having to use libc strings

static int my_strlen(const char *s)
{
    const char *ptr;

    ptr = s;
    for ( ptr=s ; *ptr != '\0' ; ptr++ )
        ;

    return (int)(ptr-s);
} // my_strlen()


static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


static char *my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()


int main( int argc, char *argv[] )
{
    char x[300];
    char y[300];
    void *ret, *ptr1, *ptr2;
    char *c_ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "memcpy() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    ptr1 = x; 
    ptr2 = y;
    my_strcpy(x, "Great shot kid! That was one in a million!");
    ret = memcpy(ptr2, ptr1, my_strlen(x) + 1);
    CYG_TEST_PASS_FAIL( (my_strcmp(x, ptr2)==0), "Simple copy" );

    // Check return value
    CYG_TEST_PASS_FAIL( (my_strcmp(ret, ptr2)==0), "Simple copy return value");


    // Check 2
    ptr1 = x; 
    ptr2 = y;
    my_strcpy(x, "");
    my_strcpy(y, "xxxx"); // Bogus val to get overwritten
    ret = memcpy(ptr2, ptr1, 1);
    c_ret = ret;
    if ((*c_ret == '\0') && (y[0] == '\0') && (y[1] == 'x'))
        CYG_TEST_PASS("Simple copy with boundary check worked");
    else
        CYG_TEST_FAIL("Simple copy with boundary check failed");

    // Check 3
    ptr1 = x; 
    ptr2 = y;
    my_strcpy(x, "xxxx");
    my_strcpy(y, "yyyy");
    ret = memcpy(ptr1, ptr2, 0);
    c_ret = ret;
    if ((*c_ret =='x') && (x[0] == 'x'))
        CYG_TEST_PASS("Simple copy with size=0 worked");
    else
        CYG_TEST_FAIL("Simple copy with size=0 failed");

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "memcpy() function");
} // main()


// EOF memcpy1.c
