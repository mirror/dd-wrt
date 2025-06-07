/*
 * libproc2 - Library to read proc filesystem
 * Tests for escape library calls
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tests.h"

#include "library/escape.c"

#define MAXTBL(t) (int)( sizeof(t) / sizeof(t[0]) )
/*
 * ASCII characters are always a single character
 */
int check_size_ascii(void *data)
{
    int i;
    const char test_chars[][5] = {
        "\x09", "A", "Z", "a", "z",
        "]", "~" };

    testname = "escape: check u8charlen ascii";
    for (i=0; i < MAXTBL(test_chars); i++) {
            //printf("t %s\n", test_chars[i]);
        if (1 != u8charlen((const unsigned char *)test_chars[i], 5)) {
            //asprintf(&testname, "escape: check u8charlen(%s) ascii", test_chars[i]);
            return 0;
        }
    }
    return 1;
}

/*
 * Check that characters that are the string size are the u8charlen
 */
int check_size_strlen(void *data)
{
    int i;
    const char test_chars[][5] = {
        //"\x00",
        "\x44", "\x7f", "\xc2\x80", "\u0188", "\u07ff",
        "\u0800", "\u8888", "\ud7ff", "\uffff",
        "\U00010000", "\U00018888", "\U0010ffff"};

    testname = "escape: check u8charlen == strlen";
    for (i=0; i < MAXTBL(test_chars); i++) {
            //printf("t %s\n", test_chars[i]);
        if (strlen(test_chars[i]) != u8charlen((const unsigned char *)test_chars[i], 5)) {
            //asprintf(&testname, "escape: check u8charlen(%s) == strlen", test_chars[i]);
            return 0;
        }
    }
    return 1;
}
/*
 * All of these characters will return -1 for u8charlen
 */
int check_size_negative(void *data)
{
    int i;
    const char test_chars[][5] = {
        "\x80", "\xbf", "\xc0", "\xc1", "\xc1\xbf",
        "\xe0\x9f\xbf", "\xed\xa0\x80", "\xed\xbf\xbf",
        "\xee\x00\x80","\xee\x80\x40", "\xdf\x7f",
        "\xfc\x8f\xbf\xbf", "\xfc\xbf\xbf", "\xf0\x80\xa0\xa0",
        "\xf0\x8f\xbf\xbf", "\xf4\x90\x80\x80",
        "\ue000", "\uf8ff",                    // main PUA    begin/end
        "\U000F0000", "\U000FFFFD",            // supp PUA-A  begin/end
        "\U00100000", "\U0010FFFD" };          // supp PUA-B  begin/end

    testname = "escape: check u8charlen == -1";
    for (i=0; i < MAXTBL(test_chars); i++) {
            //printf("t %s\n", test_chars[i]);
        if (-1 != u8charlen((const unsigned char *)test_chars[i], 5)) {
            // Leaks memory but process will exit
            //asprintf(&testname, "escape: check u8charlen(%s) == -1", test_chars[i]);
            return 0;
        }
    }
    return 1;
}

int check_esc_ctl(void *data)
{
    int i;

    // Array of input,expected_output pairs
    char test_strs[][2][20] = {
        { "A", "A" },                               // A
        { "\x7f B", "? B" },                        // DEL
        { "\xe2\x82\xac C", "\u20ac C"},            // Euro symbol
        { "\x90\x20\x70 D", "?\x20\x70 D"},         // C1 controls
        { "\xF0\x9F\x98\x8A E",  "\U0001f60a E" },  // smilie
        { "\x90\x24\x71\x30\x76\xc2\x9c\xc2\x24\x71\x20\x69 F",
            "?$q0v???$q i F"}, //C1 control from perl example
        { "\e[1;31m G", "?[1;31m G"}, // ANSI color sequence
        { "\xdf\xbf H", "\u07ff H"}, // end boundary
        { "\x80\xbf\x41 I", "??A I"} // wrong continuation bytes
    };

    testname = "esc_ctl()";
    for (i=0; i < MAXTBL(test_strs); i++) {
        esc_ctl(test_strs[i][0],strlen(test_strs[i][0]));
        //printf("Is: \"%s\"==\"%s\"\n", test_strs[i][0], test_strs[i][1]);
        if (strcmp(test_strs[i][0], test_strs[i][1]) != 0) {
            // Leaks memory but process will exit
            asprintf(&testname, "escape: check esc_ctl(%s) not %s", test_strs[i][0], test_strs[i][1]);
            return 0;
        }
    }
    return 1;
};

TestFunction test_funcs[] = {
    check_size_ascii,
    check_size_strlen,
    check_size_negative,
    check_esc_ctl,
    NULL
};

int main(int argc, char *argv[])
{
    return run_tests(test_funcs, NULL);
}

