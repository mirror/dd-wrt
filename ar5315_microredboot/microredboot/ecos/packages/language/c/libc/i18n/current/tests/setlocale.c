//=================================================================
//
//        setlocale.c
//
//        Testcase for C library setlocale() function
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
// Date:          2000-04-18
// Description:   Contains testcode for C library setlocale() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <locale.h>                // header for functions to test
#include <cyg/infra/testcase.h>    // Testcase API


// FUNCTIONS

static int
my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()

int
main( int argc, char *argv[] )
{
    char *str;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library setlocale() function");

    // Check 1
    str = setlocale(LC_CTYPE, NULL);
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Initial locale for LC_CTYPE is \"C\"" );
    
    // Check 2
    str = setlocale(LC_ALL, NULL);
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "#C#C#C#C#C#" ),
                        "Initial locale for LC_ALL is correct" );

    // Check 3
    str = setlocale(LC_COLLATE, "C");
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Set locale for LC_COLLATE to \"C\"" );

    // Check 4
    str = setlocale(LC_MONETARY, "");
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Set default locale for LC_MONETARY" );
    str = setlocale(LC_MONETARY, NULL);
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Stored default locale for LC_MONETARY" );

    // Check 5
    str = setlocale(LC_ALL, "C");
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Set locale for LC_COLLATE to \"C\"" );

    // Check 6
    str = setlocale(LC_ALL, "");
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "C" ),
                        "Set locale for LC_COLLATE to default" );

    // Check 7
    str = setlocale(LC_ALL, NULL);
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "#C#C#C#C#C#" ),
                        "Get locale for LC_ALL" );
    str = setlocale(LC_ALL, str);
    CYG_TEST_PASS_FAIL( !my_strcmp(str, "#C#C#C#C#C#" ),
                        "Set locale for LC_ALL from retrieved locale" );
    

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library setlocale() function");
} // main()

// EOF setlocale.c
