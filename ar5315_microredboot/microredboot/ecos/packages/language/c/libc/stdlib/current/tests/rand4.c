//=================================================================
//
//        rand4.c
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
// Author(s):     rosalia
// Contributors:  jlarmour
// Date:          2000-04-30
// Description:   Contains testcode for C library rand() function. This tests
//                that random numbers do not have periodicity in the lower
//                bit
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_stdlib.h>   // Configuration header

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>

// CONSTANTS

#define TEST_LENGTH 100000        // how many samples to take

// DEFINES

#define TEST_VALID (!defined(CYGIMP_LIBC_RAND_SIMPLEST))

// FUNCTIONS

int
main(int argc, char *argv[])
{
#if TEST_VALID
    int i;
    int r, prev, prevprev;
    int how_many_periodics = 0;
#endif

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "rand() function");

    CYG_TEST_INFO("This test tests the distribution of random numbers and");
    CYG_TEST_INFO("may take some time");

#if TEST_VALID
    r = rand() % 2;
    prev = r;
    r = rand() % 2;
    for (i = 0; i < TEST_LENGTH; ++i) {
      prevprev = prev;
      prev = r;
      r = rand() % 2;
      if (r == prevprev) {
        ++how_many_periodics;
      }
      if (how_many_periodics > (2*TEST_LENGTH)/3) {
        break;
      }
    }

    CYG_TEST_PASS_FAIL( (how_many_periodics <= (2*TEST_LENGTH)/3),
                        "periodicity of rand() in lowest bit");

#else
    
    // TODO: should be an _expected_ fail i.e. XFAIL
    CYG_TEST_NA("Chosen rand algorithm is known to fail this test");
    
#endif

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for "
                    "C library rand() function");
} // main()


// EOF rand4.c
