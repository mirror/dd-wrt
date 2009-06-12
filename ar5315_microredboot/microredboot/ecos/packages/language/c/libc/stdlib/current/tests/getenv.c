//=================================================================
//
//        getenv.c
//
//        Testcase for C library getenv()
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
// Date:          2000-04-30
// Description:   Contains testcode for C library getenv() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>                 // Main header for stdlib functions
#include <cyg/infra/testcase.h>     // Testcase API

// GLOBALS

extern char **environ;              // Standard environment definition

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

    char *env1[] = { NULL };
    char *env2[] = { "WIBBLE=fred", NULL };
    char *env3[] = { "PATH=/usr/local/bin:/usr/bin",
                     "HOME=/home/fred",
                     "TEST=1234=5678",
                     "home=hatstand",
                     NULL };

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library getenv() function");


    // Check 1
    str = getenv("ThisIsAVeryUnlikelyName");
    CYG_TEST_PASS_FAIL( str==NULL, "Simple getenv() default environ" );

    // Check 2
    environ = (char **)&env1;
    str = getenv("wibble");
    CYG_TEST_PASS_FAIL( str==NULL, "Simple getenv() with empty environ" );
    
    // Check 3
    environ = (char **)&env2;
    str = getenv("WIBBLE");
    CYG_TEST_PASS_FAIL( (str != NULL) && !my_strcmp(str, "fred"),
                        "Simple getenv()" );

    // Check 4
    str = getenv("wibble");
    CYG_TEST_PASS_FAIL( str==NULL,
                        "Simple getenv() for something not in the "
                        "environment" );

    // Check 5
    environ = (char **)&env3;
    str = getenv("PATH");
    CYG_TEST_PASS_FAIL( (str!= NULL) &&
                        !my_strcmp(str,"/usr/local/bin:/usr/bin"),
                        "Multiple string environment" );

    // Check 6
    str = getenv("PATh");
    CYG_TEST_PASS_FAIL( str==NULL, "getenv() for something not in the "
                        "environment for multiple string environment" );

    // Check 7
    str = getenv("home");
    CYG_TEST_PASS_FAIL( (str != NULL) && !my_strcmp(str, "hatstand"),
                        "Case-sensitive environment names" );

    // Check 8
    str = getenv("TEST");
    CYG_TEST_PASS_FAIL( (str != NULL) && !my_strcmp(str, "1234=5678"),
                        "environment value containing '='" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library getenv() function");

} // main()

// EOF getenv.c
