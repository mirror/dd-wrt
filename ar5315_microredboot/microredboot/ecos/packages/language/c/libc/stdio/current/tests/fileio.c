//=================================================================
//
//        fileio.c
//
//        Testcase for C library file I/O functions
//
//=================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):       Gary Thomas
// Contributors:    
// Date:            2003-03-06
// Description:     Contains testcode for C library file I/O functions
//
//
//####DESCRIPTIONEND####

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <stdio.h>                 // All the stdio functions
#include <errno.h>                 // errno
#include <cyg/infra/testcase.h>    // Testcase API
#include <unistd.h>
#include <cyg/fileio/fileio.h>

#define NUM_TEST_LINES 100
#define TEST_DIR "/"
#define TEST_FILE "test.file"

static char errmsg[256];

// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    int err, i, lines, len, total_len, expected_len;
    FILE *fp;
    char buf[512], *cp;

    err = mount("", TEST_DIR, "ramfs");
    if (err < 0) {
        CYG_TEST_FAIL_FINISH("Can't mount '/' file system");
    }

    err = chdir(TEST_DIR);
    if (err < 0) {
        CYG_TEST_FAIL_FINISH("Can't chdir('/')");
    }

    fp = fopen(TEST_FILE, "w");
    if (fp == (FILE *)NULL) {
        CYG_TEST_FAIL_FINISH("Can't create test file");
    }

    for (i = 0;  i < NUM_TEST_LINES;  i++) {
        len = fprintf(fp, "This is line: %4d\n", i+1);
        if (len != 19) {
            sprintf(errmsg, "Error writing data - line: %d, len: %d\n", i+1, len);
            CYG_TEST_FAIL_FINISH(errmsg);
        }
    }

    err = fclose(fp);
    if (err) {
        CYG_TEST_FAIL_FINISH("Error closingfile");
    }

    fp = fopen(TEST_FILE, "r");
    if (fp == (FILE *)NULL) {
        CYG_TEST_FAIL_FINISH("Can't open test file");
    }

    lines = 0;
    total_len = 0;
    while ((cp = fgets(buf, sizeof(buf), fp)) != NULL) {
        lines++;
        total_len += strlen(buf);
    }
    sprintf(errmsg, "Read %d lines, %d bytes", lines, total_len);
    CYG_TEST_INFO(errmsg);
    expected_len = total_len;

    if (lines != NUM_TEST_LINES) {        
        sprintf(errmsg, "Read %d lines, not %d", lines, NUM_TEST_LINES);
        CYG_TEST_FAIL_FINISH(errmsg);
    }

    if (ferror(fp)) {
        CYG_TEST_FAIL_FINISH("ferror() set");
    }

    if (!feof(fp)) {
        CYG_TEST_FAIL_FINISH("feof() not set");
    }

    err = fclose(fp);
    if (err) {
        CYG_TEST_FAIL_FINISH("Error closingfile");
    }

    fp = fopen(TEST_FILE, "r");
    if (fp == (FILE *)NULL) {
        CYG_TEST_FAIL_FINISH("Can't open test file");
    }

    // This tests whether or not it is possible to detect EOF separate
    // from an error and recover from it.  The buffer size is [probably]
    // such that fread() will not be able to read the entire file in
    // 512 byte chunks.
    lines = 0;
    total_len = 0;
    while ((len = fread(buf, sizeof(buf), 1, fp)) == 1) {
        lines++;
        total_len += sizeof(buf);
    }
    sprintf(errmsg, "Read %d chunks, %d bytes, err: %d, eof: %d", 
            lines, total_len, ferror(fp), feof(fp));
    CYG_TEST_INFO(errmsg);

    if (ferror(fp)) {
        CYG_TEST_FAIL_FINISH("ferror() set");
    }

    if (!feof(fp)) {
        CYG_TEST_FAIL_FINISH("feof() not set");
    }

    if (expected_len != total_len) {
        if (total_len > expected_len) {
            CYG_TEST_FAIL_FINISH("Too many characters read");
        }
        // There should still be more characters left to read, 
        // but less than sizeof(buf)
        clearerr(fp);
        rewind(fp);
        fseek(fp, total_len, SEEK_SET);
        len = fread(buf, 1, sizeof(buf), fp);
        if (len != (expected_len - total_len)) {
            sprintf(errmsg, "Wrong number of residual bytes - read %d, should be %d", 
                    len, expected_len-total_len);
            CYG_TEST_FAIL_FINISH(errmsg);
        }
    }

    err = fclose(fp);
    if (err) {
        CYG_TEST_FAIL_FINISH("Error closingfile");
    }

    CYG_TEST_PASS("Stdio file I/O tests completed");
    CYG_TEST_FINISH("Finished tests from testcase " __FILE__
                    " for C library stdio file I/O functions");
} // test()

int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C "
                  "library stdio file I/O functions");

    test(0);

    return 0;

} // main()

// EOF fileio.c
