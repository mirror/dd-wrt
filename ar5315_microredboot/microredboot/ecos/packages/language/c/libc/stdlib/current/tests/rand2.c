//=================================================================
//
//        rand2.c
//
//        Testcase for C library rand()
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
// Description:   Contains testcode for C library rand() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <pkgconf/libc_stdlib.h>   // CYGNUM_LIBC_RAND_SEED
#include <stdlib.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    static int rand_data1[100];
    static int rand_data2[100];
    int count;
    int fail=0;

    // Test that rand starts with the seed as CYGNUM_LIBC_RAND_SEED
    
    for ( count=0; count < 100; ++count ) {
        rand_data1[count] = rand();
    } // for

    // set seed to CYGNUM_LIBC_RAND_SEED
    srand(CYGNUM_LIBC_RAND_SEED);

    for ( count=0; count < 100; ++count ) {
        rand_data2[count] = rand();
    } // for

    for ( count=0; count < 100; ++count ) {
        if (rand_data1[count] != rand_data2[count])
            ++fail;
    } // for

    CYG_TEST_PASS_FAIL(fail==0, "rand() has seed initialised to "
                       "CYGNUM_LIBC_RAND_SEED");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C "
                    "library rand() function");
} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library rand() function");

    test(0);

    CYG_TEST_NA("Testing is not applicable to this configuration");
} // main()


// EOF rand2.c
