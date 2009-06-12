//=================================================================
//
//        strcoll1.c
//
//        Testcase for C library strcoll()
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
// Description:   Contains testcode for C library strcoll() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <string.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

// Functions to avoid having to use libc strings


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

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "strcoll() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "I have become, comfortably numb");
    my_strcpy(y, "I have become, comfortably numb");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) == 0), "Simple compare");


    // Check 2
    my_strcpy(x, "");
    my_strcpy(y, "");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) == 0), "Simple empty string compare");


    // Check 3
    my_strcpy(x, "..shall snuff it. And the Lord did grin");
    my_strcpy(y, "..shall snuff it. And the Lord did grio");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) < 0),
                        "Memory less than #1" );


    // Check 4
    my_strcpy(x, "A reading from the Book of Armaments, Chapter 4, "
                  "Verses 16 to 20:");
    my_strcpy(y, "Bless this, O Lord, that with it thou mayst blow thine "
                 "enemies to tiny bits, in thy mercy.");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) < 0),
                        "Memory less than #2");

    // Check 5
    my_strcpy(x, "Lobeth this thy holy hand grenade at thy foe");
    my_strcpy(y, "Lobeth this thy holy hand grenade at thy fod");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) > 0),
                        "Memory greater than #1" );


    // Check 6
    my_strcpy(y, "Three shall be the number of the counting and the");
    my_strcpy(x, "number of the counting shall be three");
    CYG_TEST_PASS_FAIL( (strcoll(x, y) > 0),
                        "Memory greater than #2" );

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "strcoll() function");
} // main()



// EOF strcoll1.c
