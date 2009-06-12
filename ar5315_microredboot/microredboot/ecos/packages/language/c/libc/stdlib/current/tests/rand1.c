//=================================================================
//
//        rand1.c
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

#include <stdlib.h>
#include <cyg/infra/testcase.h>

// CONSTANTS

// This is the max allowable hits, where a hit is an element
// of a random array being the same as index+1 or index+2
#define TOLERANCE 2

// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    int ctr;
    static int array[1024];
    int hits=0;

    for (ctr=0; ctr<1024; ctr++)
        array[ctr] = rand();

    for (ctr=0; ctr< 1021; ctr++)
        if ((array[ctr]==array[ctr+1]) ||
            (array[ctr]==array[ctr+2]) ||
            (array[ctr]==array[ctr+3]))
            hits++;
    
    CYG_TEST_PASS_FAIL(hits <= TOLERANCE,
                       "Simple test for rand() repetition");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for "
                    "C library rand() function");
} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "rand() function");

    test(0);

    CYG_TEST_NA("Testing is not applicable to this configuration");
} // main()

// EOF rand1.c
