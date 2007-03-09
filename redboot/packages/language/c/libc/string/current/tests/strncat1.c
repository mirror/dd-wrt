//=================================================================
//
//        strncat1.c
//
//        Testcase for C library strncat()
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
// Description:   Contains testcode for C library strncat() function
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
    char *ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "memmove() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "One ring to rule them all.");
    my_strcpy(y, "One ring to find them.");
    ret = strncat(x, y, my_strlen(y)+1);
    if ( my_strcmp(x, "One ring to rule them all.One ring to find them.")==0 ) 
        CYG_TEST_PASS("Simple concatenation");
    else 
        CYG_TEST_FAIL("Simple concatenation");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == x ), "Simple concatenation return value" );


    // Check 2
    my_strcpy(x, "One ring to bring them all,");
    my_strcpy(y, "");
    ret = strncat(x, y, my_strlen(y)+1);
    if ( my_strcmp(x, "One ring to bring them all,")==0 ) 
        CYG_TEST_PASS("Concatenation of empty string");
    else 
        CYG_TEST_FAIL("Concatenation of empty string");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == x ),
                        "Concatenation of empty string return value" );


    // Check 3
    my_strcpy(x, "and in the darkness bind them");
    my_strcpy(y, "");
    ret = strncat(y, x, my_strlen(x)+1 );
    if ( my_strcmp(x, "and in the darkness bind them")==0 ) 
        CYG_TEST_PASS("Concatenation to empty string");
    else 
        CYG_TEST_FAIL("Concatenation to empty string");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == y ),
                        "Concatenation to empty string return value" );

    // Check 4
    my_strcpy(x, "in the land of ");
    my_strcpy(y, "Mordor, where the shadows lie");
    ret = strncat(x, y, 6 );
    if ( my_strcmp(x, "in the land of Mordor")==0 ) 
        CYG_TEST_PASS("Concatenation of partial string string");
    else 
        CYG_TEST_FAIL("Concatenation of partial string");
    // Check return val
    CYG_TEST_PASS_FAIL( ( ret == x ),
                        "Concatenation to empty string return value" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strncat() function");
} // main()


// EOF strncat1.c
