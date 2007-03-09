//=================================================================
//
//        strcspn.c
//
//        Testcase for C library strcspn()
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
// Description:   Contains testcode for C library strcspn() function
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
    int ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strcspn() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "ah blow my nose at you, so-called Arthur king!");
    my_strcpy(y, "A");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == 34), "Simple strcspn() for one char" );


    // Check 2
    my_strcpy(x, "Not bad for a little fur ball. You! Stay here.");
    my_strcpy(y, "fd");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == 6), "Simple strcspn() for two chars" );

    // Check 3 (boundary condition)
    my_strcpy(x, "");
    my_strcpy(y, "zx");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == 0), "Empty string to search");

    
    // Check 4 (boundary condition)
    my_strcpy(x, "So, if she weighs the same as a duck, she's made of wood!");
    my_strcpy(y, "zx");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == my_strlen(x)), "Characters not in string" );

    // Check 5 (boundary condition)
    my_strcpy(x, "So, if she weighs the same as a duck, she's made of wood!");
    my_strcpy(y, "S");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == 0), "First character of string" );

    // Check 6 (boundary condition)
    my_strcpy(x, "So, if she weighs the same as a duck, she's made of wood!");
    my_strcpy(y, "!");
    ret = strcspn(x, y);
    CYG_TEST_PASS_FAIL( (ret == (my_strlen(x) - 1)),
                        "Penultimate character of string" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strcspn() function");
} // main()

// EOF strcspn.c
