//=================================================================
//
//        strtok.c
//
//        Testcase for C library strtok()
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
// Date:          1999-01-20
// Description:   Contains testcode for C library strtok() function
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/system.h>
#include <pkgconf/libc_string.h>   // Configuration header

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

// Functions to avoid having to use libc strings


static int
my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ ) {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


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
    char x[300];

    // Check 1
    my_strcpy(x, "tok1 tok2 3kot");
    if (( my_strcmp("tok1", strtok(x, " ")) == 0 ) &&
        ( my_strcmp("tok2", strtok(NULL, " ")) == 0 ) &&
        ( my_strcmp("3kot", strtok(NULL, " ")) == 0 ))

        CYG_TEST_PASS("Simple strtok() #1");
    else
        CYG_TEST_FAIL("Simple strtok() #1");

    // Check 2
    my_strcpy(x, "Hitteth@ye *not, the vicar");
    if (( my_strcmp("Hitteth", strtok(x, "@,*")) == 0 ) &&
        ( my_strcmp("ye ", strtok(NULL, ",@*")) == 0 ) &&
        ( my_strcmp("not", strtok(NULL, "*@,")) == 0 ) &&
        ( my_strcmp(" the vicar", strtok(NULL, "*@,")) == 0 ) )

        CYG_TEST_PASS("Simple strtok() #2");
    else
        CYG_TEST_FAIL("Simple strtok() #2");

    // Check 3
    my_strcpy(x, "on his bonce, with thine football");
    if ( my_strcmp(strtok(x, "@*"), x) == 0 )
        CYG_TEST_PASS("strtok() with token delimiters not found");
    else
        CYG_TEST_FAIL("strtok() with token delimiters not found");

    // Check 4
    my_strcpy(x, "@@@,,,...@,.,.@");
    if (strtok(x, "@,.") == NULL)
        CYG_TEST_PASS("All characters of string are token delimiters");
    else
        CYG_TEST_FAIL("All characters of string are token delimiters");

    // Check 5
    my_strcpy(x, "");
    if (strtok(x, "@,.") == NULL)
        CYG_TEST_PASS("String to tokenize empty");
    else
        CYG_TEST_FAIL("String to tokenize empty");

    // Check 6
    my_strcpy(x, "fdafda");
    if (strtok(x, "") == x)
        CYG_TEST_PASS("String to tokenize empty");
    else
        CYG_TEST_FAIL("String to tokenize empty");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strtok() function");

} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strtok() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    test(0);

    CYG_TEST_NA("Testing is not applicable to this configuration");
} // main()


// EOF strtok.c
