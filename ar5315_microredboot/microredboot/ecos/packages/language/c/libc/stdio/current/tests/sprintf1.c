//=================================================================
//
//        sprintf1.c
//
//        Testcase for C library sprintf()
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
// Date:          2000-04-20
// Description:   Contains testcode for C library sprintf() function
//
//
//####DESCRIPTIONEND####


// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header


// INCLUDES

#include <stdio.h>
#include <cyg/infra/testcase.h>

// FUNCTIONS

// Functions to avoid having to use libc strings

static int my_strlen(const char *s)
{
    const char *ptr;

    ptr = s;
    for ( ptr=s ; *ptr != '\0' ; ptr++ )
        ;

    return (int)(ptr-s);
} // my_strlen()


static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


static char *my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()



static void test(CYG_ADDRWORD data)
{
    static char x[500];
    static char y[500];
    int ret;
    int tmp;
    int *ptr;


    // Check 1
    ret = sprintf(x, "%d", 20);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "20")==0, "%d test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%d test return code");

    // Check 2
    my_strcpy(y, "Pigs noses. Get 'em while there 'ot");
    ret = sprintf(x, "%s", y);
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%s test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%s test return code");

    // Check 3
    ret = sprintf(x, "||%7d||", 2378);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "||   2378||")==0, "padding test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "padding test return code");

    // Check 4
    ret = sprintf(x, "%x", 3573);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "df5")==0, "hex conversion (lowercase)");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "hex conv (lowercase) return code");

    // Check 5
    ret = sprintf(x, "%X", 3573);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "DF5")==0, "hex conversion (uppercase)");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "hex conv (upperbase ) return code");

    // Check 6
    ret = sprintf(x, "%c", 65);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "A")==0, "%c test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%c test return code");

    // Check 7
    ret = sprintf(x, "%o",4628);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "11024")==0, "octal conversion");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "octal conversion return code");

    // Check 8
    ret = sprintf(x, "%u", (unsigned int) 4738);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "4738")==0, "%u test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%u test return code");

    // Check 9
    ptr = &tmp;
    ret = sprintf(x, "1234567x%n||", ptr);
    CYG_TEST_PASS_FAIL(tmp==8, "%n test");
    CYG_TEST_PASS_FAIL(ret==10, "%n test return code");

    // Check 10
    ret = sprintf(x, "%%");
    CYG_TEST_PASS_FAIL(my_strcmp(x, "%")==0, "%% test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%% test return code");

    // Check 11
    ret = sprintf(x, "%ld", (long)1<<30);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "1073741824")==0, "%ld test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%ld test return code");

    // Check 12
    ret = sprintf(x, "%lu", (unsigned long)(1<<31) + 100);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "2147483748")==0, "%lu test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%lu test return code");

    // Check 13
    ret = sprintf(x, "%x", 0x789a);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "789a")==0, "%x test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%x test return code");

    // Check 14
    ret = sprintf(x, "%X", 0x789ab2);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "789AB2")==0, "%X test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%x test return code");

    // Check 15
    ret = sprintf(x, "%08x", 0xdea2f2);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "00dea2f2")==0, "%0x test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%0x test return code");

    // Check 16
    ret = sprintf(x, "%09X", 0x12fa1c);
    CYG_TEST_PASS_FAIL(my_strcmp(x, "00012FA1C")==0, "%0X test");
    CYG_TEST_PASS_FAIL(ret==my_strlen(x), "%0X test return code");

    // Check 17
    ptr=&tmp;
    ret = sprintf(x, "%p", (void *)ptr);
    // just check _something_ was returned
    CYG_TEST_PASS_FAIL((ret==my_strlen(x)) && (ret > 0),
                       "%p test return code");

#ifdef CYGSEM_LIBC_STDIO_PRINTF_FLOATING_POINT

    CYG_TEST_INFO("Starting floating point specific tests");

    // Check 18
    ret = sprintf(x, "%f", 2.5);
    my_strcpy( y, "2.500000" );
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "simple %f test #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "simple %f test #1 return code");

    // Check 19
    ret = sprintf(x, "hello %f world", 1.234);
    my_strcpy( y, "hello 1.234000 world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "simple %f test #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "simple %f test #2 return code");

    // Check 20
    ret = sprintf(x, "hello%fworld", 2.3456781);
    my_strcpy( y, "hello2.345678world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "simple %f test #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "simple %f test #3 return code");

    // Check 21
    ret = sprintf(x, "%s%f%d%s", "testing", -0.591, 3, "123");
    my_strcpy( y, "testing-0.5910003123");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%f mixed with others");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),"%f mixed with others return code");

    // Check 22
    ret = sprintf(x, "%s%f%d%s", "testing", -0.591, 3, "123");
    my_strcpy( y, "testing-0.5910003123");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%f mixed with others");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),"%f mixed with others return code");

    // Check 23
    ret = sprintf(x, "hello%fworld", 2.3456786);
    my_strcpy( y, "hello2.345679world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "rounding test #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "rounding test #1 return code");

    // Check 24
    ret = sprintf(x, "hello%fworld", -2.3456786);
    my_strcpy( y, "hello-2.345679world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "rounding test #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "rounding test #2 return code");

    // Check 25
    ret = sprintf(x, "hello%+fworld", -6.54321);
    my_strcpy( y, "hello-6.543210world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "+ modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "+ modifier #1 return code");

    // Check 26
    ret = sprintf(x, "hello%+fworld", 6.54321);
    my_strcpy( y, "hello+6.543210world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "+ modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "+ modifier #2 return code");

    // Check 27
    ret = sprintf(x, "hello%5fworld", 6.5);
    my_strcpy( y, "hello6.500000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "width modifier #1 return code");

    // Check 28
    ret = sprintf(x, "hello%2fworld", 4.3);
    my_strcpy( y, "hello4.300000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "width modifier #2 return code");

    // Check 29
    ret = sprintf(x, "hello%2.1fworld", 5.6);
    my_strcpy( y, "hello5.6world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width and precision modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "width and precision modifier #1 return code");

    // Check 30
    ret = sprintf(x, "hello%5.1fworld", 6.7);
    my_strcpy( y, "hello  6.7world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width and precision modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "width and precision modifier #2 return code");

    // Check 31
    ret = sprintf(x, "hello%3.1fworld", 7.8);
    my_strcpy( y, "hello7.8world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width and precision modifier #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "width and precision modifier #3 return code");

    // Check 32
    ret = sprintf(x, "hello%3.2fworld", 7.8);
    my_strcpy( y, "hello7.80world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "width and precision modifier #4");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "width and precision modifier #4 return code");

    // Check 33
    ret = sprintf(x, "hello%05.1fworld", 6.5);
    my_strcpy( y, "hello006.5world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0 modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0 modifier #1 return code");

    // Check 34
    ret = sprintf(x, "hello%03.0fworld", 6.2);
    my_strcpy( y, "hello006world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0 modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0 modifier #2 return code");

    // Check 35
    ret = sprintf(x, "hello%05.*fworld",2, 7.5);
    my_strcpy( y, "hello07.50world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0 modifier plus *");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "0 modifier plus * return code");

    // Check 36
    ret = sprintf(x, "hello%03.1fworld",-1.232);
    my_strcpy( y, "hello-1.2world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "-ve number with 0 modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "-ve number with 0 modifier #1 return code");

    // Check 37
    ret = sprintf(x, "hello%04.1fworld",-1.232);
    my_strcpy( y, "hello-1.2world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "-ve number with 0 modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "-ve number with 0 modifier #2 return code");

    // Check 38
    ret = sprintf(x, "hello%05.1fworld",-1.232);
    my_strcpy( y, "hello-01.2world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "-ve number with 0 modifier #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "-ve number with 0 modifier #3 return code");

    // Check 39
    ret = sprintf(x, "hello%fworld",0.0);
    my_strcpy( y, "hello0.000000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0.0 test #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0.0 test #1 return code");

    // Check 40
    ret = sprintf(x, "hello%1.0fworld",0.0);
    my_strcpy( y, "hello0world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0.0 test #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0.0 test #2 return code");

    // Check 41
    ret = sprintf(x, "hello%1.1fworld",0.0);
    my_strcpy( y, "hello0.0world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0.0 test #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0.0 test #3 return code");

    // Check 42
    ret = sprintf(x, "hello%5.1fworld",0.0);
    my_strcpy( y, "hello  0.0world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "0.0 test #4");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "0.0 test #4 return code");

    // Check 43
    ret = sprintf(x, "hello%fworld",-0.0);
    my_strcpy( y, "hello-0.000000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "-0.0 test #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "-0.0 test #1 return code");

    // Check 44
    ret = sprintf(x, "hello%fworld",0.234);
    my_strcpy( y, "hello0.234000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "number less than 1 #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "number less than 1 #1 return code");

    // Check 45
    ret = sprintf(x, "hello%02fworld",-0.234);
    my_strcpy( y, "hello-0.234000world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "number less than 1 #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "number less than 1 #2 return code");

    // Check 46
    ret = sprintf(x, "hello%02.2fworld",-0.234);
    my_strcpy( y, "hello-0.23world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "number less than 1 #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "number less than 1 #3 return code");

    // Check 47
    ret = sprintf(x, "hello%06.2fworld",-0.234);
    my_strcpy( y, "hello-00.23world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "number less than 1 #4");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "number less than 1 #4 return code");

    // Check 48
    ret = sprintf(x, "hello%-6.2fworld",2.345);
    my_strcpy( y, "hello2.35  world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "left justification #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "left justification #1 return code");

    // Check 49
    ret = sprintf(x, "hello%-6.2fworld",2.345);
    my_strcpy( y, "hello2.35  world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "left justification #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "left justification #2 return code");

    // Check 50
    ret = sprintf(x, "hello%-3.2fworld",2.345);
    my_strcpy( y, "hello2.35world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "left justification #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "left justification #3 return code");

    // Check 51
    ret = sprintf(x, "hello%#1.0fworld",2.12);
    my_strcpy( y, "hello2.world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "# modifier #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "# modifier #1 return code");

    // Check 52
    ret = sprintf(x, "hello%#1.2fworld",2.0);
    my_strcpy( y, "hello2.00world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "# modifier #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "# modifier #2 return code");

    // Check 53
    ret = sprintf(x, "hello%eworld",2.3456);
    my_strcpy( y, "hello2.345600e+00world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%e #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%e #1 return code");

    // Check 54
    ret = sprintf(x, "hello%4.3eworld",-2.3456e-1);
    my_strcpy( y, "hello-2.346e-01world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%e #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%e #2 return code");

    // Check 55
    ret = sprintf(x, "hello%4.2eworld",2.56e2);
    my_strcpy( y, "hello2.56e+02world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%e #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%e #3 return code");

    // Check 56
    ret = sprintf(x, "hello%09.2eworld",2.56e2);
    my_strcpy( y, "hello02.56e+02world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%e #4");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%e #4 return code");

    // Check 57
    ret = sprintf(x, "hello%09.2Eworld",4.23e19);
    my_strcpy( y, "hello04.23E+19world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%e #5");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%e #5 return code");

    // Check 58
    ret = sprintf(x, "hello%gworld",4.0);
    my_strcpy( y, "hello4world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #1 return code");

    // Check 59
    ret = sprintf(x, "hello%gworld",4.56);
    my_strcpy( y, "hello4.56world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #2 return code");

    // Check 60
    ret = sprintf(x, "hello%5.2gworld",4.56);
    my_strcpy( y, "hello  4.6world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #3");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #3 return code");

    // Check 61
    ret = sprintf(x, "hello%gworld",0.002);
    my_strcpy( y, "hello0.002world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #4");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #4 return code");

    // Check 62
    ret = sprintf(x, "hello%06.1gworld",0.0026);
    my_strcpy( y, "hello00.003world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #5");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #5 return code");

    // Check 63
    ret = sprintf(x, "hello%06gworld",0.000026);
    my_strcpy( y, "hello2.6e-05world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #5");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #5 return code");

    // Check 64
    ret = sprintf(x, "hello%06Gworld",0.000037);
    my_strcpy( y, "hello3.7E-05world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #6");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #6 return code");

    // Check 65
    ret = sprintf(x, "hello%08Gworld",-123456.0);
    my_strcpy( y, "hello-0123456world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #7");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #7 return code");

    // Check 66
    ret = sprintf(x, "hello%07gworld",-1234567.0);
    my_strcpy( y, "hello-1.23457e+06world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #8");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #8 return code");

    // Check 67
    ret = sprintf(x, "hello%013Gworld",-1234567.0);
    my_strcpy( y, "hello-01.23457E+06world");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "%g #9");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y), "%g #9 return code");


#else
    CYG_TEST_PASS("Floating point tests skipped - not configured");
#endif // ifdef CYGSEM_LIBC_STDIO_PRINTF_FLOATING_POINT

    // Long string tests
    ret = sprintf(x, "This is a very long string so I hope this works as "
                  "otherwise I would be very, very, sad. The cat sat on the "
                  "hat, and the mat sat on the rat. Quick brown fax, etc.etc. "
                  "blah, blah and all that jazz. Isn't he finished yet? My "
                  "old man's a dustman, why do I have to think up this "
                  "drivel, isn't that what summer students are for, if "
                  "anything that seems thinking up mindless drivel seems to "
                  "be their occupation in life. Yoof of today, eh? What, "
                  "what? %s So there.",
                  "And this is a middly bit.");
    my_strcpy(y, "This is a very long string so I hope this works as "
                  "otherwise I would be very, very, sad. The cat sat on the "
                  "hat, and the mat sat on the rat. Quick brown fax, etc.etc. "
                  "blah, blah and all that jazz. Isn't he finished yet? My "
                  "old man's a dustman, why do I have to think up this "
                  "drivel, isn't that what summer students are for, if "
                  "anything that seems thinking up mindless drivel seems to "
                  "be their occupation in life. Yoof of today, eh? What, "
                  "what? And this is a middly bit. So there.");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "long (480 char) string output #1");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "long (480 char) string output #1 return code");

    ret = sprintf(x, "Boo! This %s So there.",
                  "is a very long string so I hope this works as "
                  "otherwise I would be very, very, sad. The cat sat on the "
                  "hat, and the mat sat on the rat. Quick brown fax, etc.etc. "
                  "blah, blah and all that jazz. Isn't he finished yet? My "
                  "old man's a dustman, why do I have to think up this "
                  "drivel, isn't that what summer students are for, if "
                  "anything that seems thinking up mindless drivel seems to "
                  "be their occupation in life. Yoof of today, eh? What, "
                  "what? And this is a middly bit.");
    my_strcpy(y, "Boo! This is a very long string so I hope this works as "
                  "otherwise I would be very, very, sad. The cat sat on the "
                  "hat, and the mat sat on the rat. Quick brown fax, etc.etc. "
                  "blah, blah and all that jazz. Isn't he finished yet? My "
                  "old man's a dustman, why do I have to think up this "
                  "drivel, isn't that what summer students are for, if "
                  "anything that seems thinking up mindless drivel seems to "
                  "be their occupation in life. Yoof of today, eh? What, "
                  "what? And this is a middly bit. So there.");
    CYG_TEST_PASS_FAIL(my_strcmp(x, y)==0, "long (485 char) string output #2");
    CYG_TEST_PASS_FAIL(ret == my_strlen(y),
                       "long (485 char) string output #2 return code");




    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "sprintf() function");

} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "sprintf() function");
    CYG_TEST_INFO("These test individual features separately");

    test(0);

    return 0;
} // main()

// EOF sprintf1.c
