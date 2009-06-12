//=================================================================
//
//        bsearch.c
//
//        Testcase for C library bsearch()
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
// Description:   Contains testcode for C library bsearch() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

static int
Compar( const void *int1, const void *int2 )
{
    if ( *(int*)int1 < *(int*)int2 )
        return -1;
    else if ( *(int*)int1 == *(int*)int2 )
        return 0;
    else
        return 1;
} // Compar()

int
main( int argc, char *argv[] )
{
    int key;
    int *result;
    int i_array[] = {1, 5, 8, 35, 84, 258, 1022, 1022, 5300, 7372, 9029};

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "bsearch() function");

    // Test 1
    key = 8;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 8),
                        "bsearch() something in the middle");

    // Test 2
    key = 5300;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 5300),
                        "bsearch() something else in the middle");

    // Test 3
    key = 1;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 1),
                        "bsearch() first element");

    // Test 4
    key = 9029;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 9029),
                        "bsearch() last element");

    // Test 5
    key = 1022;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( (result!=NULL) && (*result == 1022),
                        "bsearch() duplicate element");

    // Test 6
    key = 2;
    result = bsearch(&key, i_array, sizeof(i_array)/sizeof(i_array[0]),
                     sizeof(i_array[0]), &Compar);
    CYG_TEST_PASS_FAIL( result==NULL, "bsearch() nonexistent element");

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "bsearch() function");

} // main()


// EOF bsearch.c
