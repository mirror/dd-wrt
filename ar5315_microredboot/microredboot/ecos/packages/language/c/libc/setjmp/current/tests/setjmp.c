//===========================================================================
//
//      setjmp.c
//
//      Tests for ANSI standard setjmp() and longjmp() functions
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-30
// Purpose:     
// Description: Contains test code for C library setjmp() and longjmp()
//              functions
//              The ANSI standard allows setjmp calls to be used ONLY in the
//              forms:
//                while (setjmp (jumpbuf))
//                while (setjmp (jumpbuf) < 42)
//                while (!setjmp (jumpbuf))
//                setjmp (jumpbuf);
//                result = setjmp(jumpbuf);
//
//              Or "while" can also be "if" or "switch" or the implicit
//              while of "for". 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// INCLUDES

#include <cyg/infra/testcase.h>    // Test infrastructure header
#include <setjmp.h>                // Header for what we're testing

// GLOBALS

static jmp_buf jbuf, jbuf2, jbuf3;


// FUNCTIONS

static int
test_fun( void )
{
    volatile int i=0; // add something to the stack to confuse things

    longjmp( jbuf, 5 );
    i+=5;

    return i;
} // test_fun();


int
main( int argc, char *argv[] )
{
    static int static_i=0;         // temporary variable
    int j=0;                       // ditto
    volatile int vol_k=0, vol_l=0; // ditto

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "setjmp()/longjmp() functions");

    // Test 1
    if ( setjmp(jbuf) == 0 )
    {
        static_i=1;
        longjmp(jbuf, 1);
        static_i=2;
    } // if

    CYG_TEST_PASS_FAIL( static_i==1, "Simple test 1" );

    // Test 2
    j=2;

    if ( !setjmp(jbuf) )
    {
        static_i=3;
        longjmp(jbuf, 1);
        static_i=4;
        j=3;
    } // if

    CYG_TEST_PASS_FAIL( (static_i==3) && (j==2), "Simple test 2" );

    // Test 3

    static_i=0;
    switch (setjmp(jbuf))
    {
    case 0:
        static_i++;
        if (static_i!=1)
            CYG_TEST_FAIL("Test of value passed to longjmp()");
        else
            longjmp(jbuf, 5);
        break;
    case 5:
        static_i++;
        CYG_TEST_PASS_FAIL( (static_i==2),"Test of value passed to longjmp()");
        break;
    default:
        CYG_TEST_FAIL( "Test of value passed to longjmp()");
        break;
    } // switch

    // Test 3

    static_i=0;
    switch (setjmp(jbuf))
    {
    case 0:
        static_i++;
        if (static_i!=1)
            CYG_TEST_FAIL("Test of value passed to longjmp() being 0");
        else
            longjmp(jbuf, 0);
        break;
    case 1:
        static_i++;
        CYG_TEST_PASS_FAIL( (static_i==2),
                            "Test of value passed to longjmp() being 0");
        break;
    default:
        CYG_TEST_FAIL( "Test of value passed to longjmp() being 0");
        break;
    } // switch


    // Test 4
    
    vol_k=0;
    static_i=0;
    setjmp( jbuf );

    static_i++;
    setjmp( jbuf2 );
    
    static_i++;
    setjmp( jbuf3 );

    if (vol_k==0)
    {
        vol_k++;
        longjmp( jbuf2, 1 );
    }
    else
    {
        CYG_TEST_PASS_FAIL( (vol_k==1) && (static_i==3), "Multiple setjmp's" );
    }

    // Test 5
    
    vol_k=3;
    if ( (j=setjmp(jbuf)) == 0 )
    {
        volatile int l; // put something extra on the stack to confuse things

        vol_k++;
        l=test_fun();
        l++;
        vol_k=l;
    } // if

    CYG_TEST_PASS_FAIL( (j==5) && (vol_k==4), "longjmp from a function" );

    // Test 6

    vol_k=0;

    vol_l=setjmp(jbuf);

    vol_k += 3;

    if (!vol_l)
        test_fun();

    vol_l=setjmp(jbuf);

    vol_k += 7;

    if (!vol_l)
        test_fun();

    CYG_TEST_PASS_FAIL ( (vol_k == 20), "Repeated longjmps from a function" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "setjmp()/longjmp() functions");

} // main()

// EOF setjmp.c
