//=================================================================
//
//        ctype.c
//
//        General testcase for C library ctype functions
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
// Date:          2000-04-14
// Description:   Contains general testcode for C library ctype functions
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <ctype.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

int
main( int argc, char *argv[] )
{
    int c; // character we use as a parameter

    CYG_TEST_INIT();

    CYG_TEST_INFO( "Starting tests from testcase " __FILE__ " for C library "
                   "<ctype.h> functions" );

    // Check isalnum
    c = 't';
    CYG_TEST_PASS_FAIL( isalnum(c), "isalnum('t')");
    c = '2';
    CYG_TEST_PASS_FAIL( isalnum(c), "isalnum('2')");
    c = 2;
    CYG_TEST_PASS_FAIL( !isalnum(c), "!isalnum(2)");
    c = 127;
    CYG_TEST_PASS_FAIL( !isalnum(c), "!isalnum(127)");

    // Check isalpha
    c = 'A';
    CYG_TEST_PASS_FAIL( isalpha(c), "isalpha('A')");
    c = 'a';
    CYG_TEST_PASS_FAIL( isalpha(c), "isalpha('a')");
    c = '2';
    CYG_TEST_PASS_FAIL( !isalpha(c), "!isalpha('2')");
    c = '\n';
    CYG_TEST_PASS_FAIL( !isalpha(c), "!isalpha('newline')");

    // Check iscntrl
    c = 'a';
    CYG_TEST_PASS_FAIL( !iscntrl(c), "!iscntrl('a')");
    c = 7;
    CYG_TEST_PASS_FAIL( iscntrl(c), "iscntrl('7')");
    c = '\n';
    CYG_TEST_PASS_FAIL( iscntrl(c), "iscntrl(newline)");
    c = 0x7F;
    CYG_TEST_PASS_FAIL( iscntrl(c), "iscntrl(0x7F)");

    // Check isdigit
    c = '2';
    CYG_TEST_PASS_FAIL( isdigit(c), "isdigit('2')");
    c = '0';
    CYG_TEST_PASS_FAIL( isdigit(c), "isdigit('0')");
    c = 't';
    CYG_TEST_PASS_FAIL( !isdigit(c), "!isdigit('t')");

    // Check isgraph
    c = ')';
    CYG_TEST_PASS_FAIL( isgraph(c), "isgraph(')')");
    c = '~';
    CYG_TEST_PASS_FAIL( isgraph(c), "isgraph('~')");
    c = '9';
    CYG_TEST_PASS_FAIL( isgraph(c), "isgraph('9')");
    c = 9;
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(9)");
    c = ' ';
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(' ')");
    c = '\t';
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(tab)");
    c = '\n';
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(newline)");
    c = 0x7F;
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(DEL)");
    c = 200;
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(200)");
    c = '\0';
    CYG_TEST_PASS_FAIL( !isgraph(c), "!isgraph(NUL)");

    // Check islower
    c = 'J';
    CYG_TEST_PASS_FAIL( !islower(c), "islower('J')");
    c = 'j';
    CYG_TEST_PASS_FAIL( islower(c), "islower('j')");
    c = '5';
    CYG_TEST_PASS_FAIL( !islower(c), "!islower(5)");

    // Check isprint
    c = ' ';
    CYG_TEST_PASS_FAIL( isprint(c), "isprint(' ')");
    c = 'x';
    CYG_TEST_PASS_FAIL( isprint(c), "isprint('x')");
    c = '\b';
    CYG_TEST_PASS_FAIL( !isprint(c), "!isprint(backspace)");

    // Check ispunct
    c = '.';
    CYG_TEST_PASS_FAIL( ispunct(c), "ispunct('.')");
    c = '#';
    CYG_TEST_PASS_FAIL( ispunct(c), "ispunct('#')");
    c = '@';
    CYG_TEST_PASS_FAIL( ispunct(c), "ispunct('@')");
    c = 'f';
    CYG_TEST_PASS_FAIL( !ispunct(c), "!ispunct('f')");
    c = '7';
    CYG_TEST_PASS_FAIL( !ispunct(c), "!ispunct('7')");
    c = '\n';
    CYG_TEST_PASS_FAIL( !ispunct(c), "!ispunct('newline')");

    // Check isspace
    c = ' ';
    CYG_TEST_PASS_FAIL( isspace(c), "isspace(' ')");
    c = '\t';
    CYG_TEST_PASS_FAIL( isspace(c), "isspace(tab)");
    c = '\r';
    CYG_TEST_PASS_FAIL( isspace(c), "isspace(return)");
    c = '\v';
    CYG_TEST_PASS_FAIL( isspace(c), "isspace(vertical tab)");
    c = '\n';
    CYG_TEST_PASS_FAIL( isspace(c), "isspace(newline)");
    c = 'd';
    CYG_TEST_PASS_FAIL( !isspace(c), "!isspace('d')");
    c = ',';
    CYG_TEST_PASS_FAIL( !isspace(c), "!isspace(',')");

    // Check isupper
    c = 'A';
    CYG_TEST_PASS_FAIL( isupper(c), "isupper('A')");
    c = 'a';
    CYG_TEST_PASS_FAIL( !isupper(c), "!isupper('a')");
    c = '2';
    CYG_TEST_PASS_FAIL( !isupper(c), "!isupper('2')");
    c = '\b';
    CYG_TEST_PASS_FAIL( !isupper(c), "!isupper(backspace)");

    // Check isxdigit
    c = 'f';
    CYG_TEST_PASS_FAIL( isxdigit(c), "isxdigit('f')");
    c = 'D';
    CYG_TEST_PASS_FAIL( isxdigit(c), "isxdigit('D')");
    c = '1';
    CYG_TEST_PASS_FAIL( isxdigit(c), "isxdigit('1')");
    c = '0';
    CYG_TEST_PASS_FAIL( isxdigit(c), "isxdigit('0')");
    c = 'g';
    CYG_TEST_PASS_FAIL( !isxdigit(c), "!isxdigit('g')");
    c = 'x';
    CYG_TEST_PASS_FAIL( !isxdigit(c), "!isxdigit('x')");

    // Check tolower
    c = 'F';
    CYG_TEST_PASS_FAIL( tolower(c) == 'f', "tolower('F')");
    c = 'g';
    CYG_TEST_PASS_FAIL( tolower(c) == 'g', "tolower('g')");
    c = '3';
    CYG_TEST_PASS_FAIL( tolower(c) == '3', "tolower('3')");

    // Check toupper
    c = 'f';
    CYG_TEST_PASS_FAIL( toupper(c) == 'F', "toupper('f')");
    c = 'G';
    CYG_TEST_PASS_FAIL( toupper(c) == 'G', "toupper('G')");
    c = ',';
    CYG_TEST_PASS_FAIL( toupper(c) == ',', "toupper(',')");

    CYG_TEST_FINISH( "Finished tests from testcase " __FILE__ " for C library "
                     "<ctype.h> functions" );
} // main()

// EOF ctype.c
