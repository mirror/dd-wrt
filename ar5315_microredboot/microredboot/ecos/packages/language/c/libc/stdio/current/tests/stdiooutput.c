//=================================================================
//
//        stdiooutput.c
//
//        Testcase for miscellaneous C library output functions
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
// Author(s):       jlarmour
// Contributors:    
// Date:            2000-04-20
// Description:     Contains testcode for C library stdio output
//                  functions
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <stdio.h>                 // All the stdio functions
#include <errno.h>                 // errno
#include <cyg/infra/testcase.h>    // Testcase API

// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    int i;


    // Check 1
    CYG_TEST_INFO("Expect: Hello printf world 42!");
    printf("Hello printf %s %d!\n", "world", 42);

    // Check 2
    CYG_TEST_INFO("Expect: This is fprintf 0x07f!");
    fprintf(stdout, "This is fprintf 0x%03x!\n", 0x7f);

    // Check 3
    CYG_TEST_INFO("Expect: char tests");
    fputc( 'c', stderr );
    putc( 'h', stdout );
    putchar( 'a' );
    fputc( 'r', stdout );
    putchar( ' ' );
    fflush(stdout);
    putc( 't', stderr );
    fputc( 'e', stdout );
    putchar( 's' );
    fflush( NULL );
    putc( 't', stderr );
    fputc( 's', stderr );
    putchar( '\n' );

    // Check 4
    CYG_TEST_INFO("Expect: puts test");
    puts("puts test");

    // Check 5
    CYG_TEST_INFO("Expect: fputs test");
    fputs( "fputs test\n", stdout );

    // Check 6
    CYG_TEST_INFO("Expect: fwrite test");
    fwrite( "fwrite test\n", 6, 2, stderr );

    // Check 7
    CYG_TEST_INFO("Expect: wibble: No error");
    errno = ENOERR;
    perror( "wibble" );

    // Check 8
    CYG_TEST_INFO("Expect: Long string test!\nSitting on the side of the "
                  "highway waiting to catch speeding drivers, a State "
                  "Police Officer sees a car puttering along at 22 MPH. "
                  "He thinks to himself, \"This driver is just as "
                  "dangerous as a speeder!\" So he turns on his lights "
                  "and pulls the driver over. Approaching the car, "
                  "he notices that there are five old ladies-two in "
                  "the front seat and three in the back-wide eyed and "
                  "white as ghosts. The driver, obviously confused, says "
                  "to him, \"Officer, I don't understand, I was doing "
                  "exactly the speed limit! What seems to be the "
                  "problem?\" \"Ma'am,\" the officer replies, \"you "
                  "weren't speeding, but you should know that driving "
                  "slower than the speed limit can also be a danger to "
                  "other drivers.\" \"Slower than the speed limit? No "
                  "sir, I was doing the speed limit exactly...Twenty-two "
                  "miles an hour!\" the old woman says a bit proudly. "
                  "The State Police officer, trying to contain a "
                  "chuckle explains to her that \"22\" was the route "
                  "number, not the speed limit. A bit embarrassed, the "
                  "woman grinned and thanked the officer for pointing "
                  "out her error. \"But before I let you go, Ma'am, I "
                  "have to ask... Is everyone in this car OK? These "
                  "women seem awfully shaken and they haven't muttered a "
                  "single peep this whole time,\" the officer asks. \"Oh, "
                  "they'll be all right in a minute officer. We just got "
                  "off Route 119\".");

    printf("Long string test!\nSitting on the side of the "
           "highway waiting to catch speeding drivers, a State "
           "Police Officer sees a car puttering along at %d MPH. "
           "He thinks to himself, \"This driver is just as "
           "dangerous as a speeder!\" So he turns on his lights "
           "and pulls the driver over. Approaching the car, "
           "he notices that there are five old ladies-two in "
           "the front seat and three in the back-wide eyed and "
           "white as ghosts. The driver, obviously confused, says "
           "to him, \"Officer, I don't understand, I was doing "
           "exactly the speed limit! What seems to be the "
           "problem?\" \"Ma'am,\" the officer replies, \"you "
           "weren't speeding, but you should know that driving "
           "slower than the speed limit can also be a danger to "
           "other drivers.\" \"Slower than the speed limit? No "
           "sir, I was doing the speed limit exactly...Twenty-two "
           "miles an hour!\" the old woman says a bit proudly. "
           "The State Police officer, trying to contain a "
           "chuckle explains to her that \"%d\" was the route "
           "number, not the speed limit. A bit embarrassed, the "
           "%coman grinned and thanked the officer for pointing "
           "out her error. \"But before I let you go, Ma'am, I "
           "have to ask... Is everyone in this car OK? These "
           "women seem awfully shaken and they haven't muttered a "
           "single peep this whole time,\" the officer asks. \"Oh, "
           "they'll be all right in a minute officer. We just got "
           "off Route %d\".\n", 22, 22, 'w', 119);

    // Check 9
    CYG_TEST_INFO("Expect: Another puts test, just for fun");
    puts("Another puts test, just for fun");

    // Check 10
    CYG_TEST_INFO("Expect: more fputs testing");
    fputs( "more fputs testing\n", stderr );

    // Check 11
    CYG_TEST_INFO("Expect all the numbers from 1 to 100 in 5 columns");
    for (i=1; i<101; ++i) {

        printf("%5d ", i);
        if (i%5 == 0)
            fputc( '\n', stdout);
        
    } // for

    CYG_TEST_PASS("Stdio output tests completed");
    CYG_TEST_FINISH("Finished tests from testcase " __FILE__
                    " for C library stdio output functions");

} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library stdio output functions");
    CYG_TEST_INFO("The output of these tests needs to be manually "
                  "verified.");

    test(0);

    return 0;

} // main()

// EOF stdiooutput.c
