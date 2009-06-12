//=================================================================
//
//        srand.c
//
//        Testcase for C library srand()
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
// Description:   Contains testcode for C library srand() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>


// CONSTANTS

// Max cross correlation value of two sequences of
// different seed
#define X_CORREL 10

// size of arrays to use for testing
#define TEST_SIZE 1024


// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    int ctr;
    static int array_1[TEST_SIZE];
    static int array_2[TEST_SIZE];
    static int array_3[TEST_SIZE];
    int hits;
    int fail;


    srand(3);
    for (ctr=0; ctr<TEST_SIZE; ++ctr)
        array_1[ctr] = rand();

    srand(9);
    for (ctr=0; ctr<TEST_SIZE; ++ctr)
        array_2[ctr] = rand();

    srand(3);
    for (ctr=0; ctr<TEST_SIZE; ++ctr)
        array_3[ctr] = rand();

    // Make sure arrays 1 and 3 are the same
    fail = 0;
    for (ctr=0; ctr<TEST_SIZE; ++ctr) {
        if (array_1[ctr] != array_3[ctr])
            ++fail;
    } // for

    CYG_TEST_PASS_FAIL( fail == 0, "resetting the seed to the same value");

    // Check sequences of different seeds are different
    hits = 0;
    for (ctr=0; ctr<TEST_SIZE; ++ctr) {
        if (array_1[ctr] == array_2[ctr])
            ++hits;
    } // for

    CYG_TEST_PASS_FAIL(hits < X_CORREL,
                       "random sequence for different seeds is different");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for "
                    "C library srand() function");
} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "srand() function");

    test(0);

    CYG_TEST_NA("Testing is not applicable to this configuration");
} // main()



// EOF srand.c
