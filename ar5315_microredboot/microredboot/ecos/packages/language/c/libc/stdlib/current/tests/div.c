//=================================================================
//
//        div.c
//
//        Testcase for C library div()
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
// Description:   Contains testcode for C library div() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

int
main( int argc, char *argv[] )
{
    int num, denom;
    div_t result;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "div() function");

    num = 10232;
    denom = 43;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==237) && (result.rem==41),
                        "div( 10232, 43 )");

    num = 4232;
    denom = 2000;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==2) && (result.rem==232),
                        "div( 4232, 2000 )");


    num = 20;
    denom = 20;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==0),
                        "div( 20, 20 )");

    num = -5;
    denom = 4;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==-1) && (result.rem==-1),
                        "div( -5, 4 )");

    num = 5;
    denom = -4;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==-1) && (result.rem==1),
                        "div( 5, -4 )");

    num = -5;
    denom = -3;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==-2),
                        "div( -5, -3 )");

    num = -7;
    denom = -7;
    result = div(num, denom);
    CYG_TEST_PASS_FAIL( (result.quot==1) && (result.rem==0),
                        "div( -7, -7 )");


    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "div() function");

} // main()

// EOF div.c
