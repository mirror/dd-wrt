//==========================================================================
//
//      tassert3.cxx
//
//      Assertion test case                                                                
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1998-12-21
// Purpose:
// Description:         Check the host extensions.
//
//####DESCRIPTIONEND####
//==========================================================================

// -------------------------------------------------------------------------
// The host-side assertion support has three extensions to the generic
// assertion support:
//
// 1) cyg_assert_install_failure_handler()
//    Provide an alternative handler that gets invoked when an
//    assertion fails.
//
// 2) cyg_assert_install_failure_callback()
//    Provide an additional callback that should get invoked when an
//    assertion triggers to provide additional information.
//
// 3) cyg_assert_failure_invoke_callbacks()
//    Used by (1) to call the functions installed via (2).
//
// The tests make use of setjmp()/longjmp() buffer to avoid having to
// do lots of output parsing in the DejaGnu driver.

#define CYG_DECLARE_HOST_ASSERTION_SUPPORT
#define CYGDBG_USE_ASSERTS
#define CYGDBG_INFRA_DEBUG_PRECONDITIONS
#define CYGDBG_INFRA_DEBUG_POSTCONDITIONS
#define CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS
#define CYGDBG_INFRA_DEBUG_INVARIANTS

#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_ass.h>
#include <cstdlib>
#include <csetjmp>
#include <cstring>

// Forward declarations of some integers to allow the line number to be
// checked.
extern cyg_uint32 main_start, main_end;

// This is used to "recover" from an assertion failure
static jmp_buf setjmp_buffer;

// A constant string useful for comparisons.
static const char message[] = "fail";

// Some state information to make sure that the world is in the expected
// state
bool expecting_failure1         = false;
bool expecting_failure2         = false;
bool expecting_failure3         = false;

// -------------------------------------------------------------------------
// The first assertion handler. This is used to check that the arguments
// make sense.

extern "C"
bool
failure_handler1(const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    if (!expecting_failure1) {
        CYG_TEST_FAIL("assertion failure handler1 invoked unexpectedly");
    }
    expecting_failure1 = false;
    // The function should be 0 or contain main
    if (0 != fn) {
        if (0 == strstr(fn, "main")) {
            CYG_TEST_FAIL("invalid function name passed to assertion failure handler");
        }
    }
    // The file name should always be valid and contain tassert3.cxx.
    // It may contain path information as well.
    if ( (0 == file) || (0 == strstr(file, "tassert3.cxx"))) {
        CYG_TEST_FAIL("invalid file name passed to assertion failure handler");
    }
    // The line number can be validated against some globals.
    if ((line <= main_start) || (line >= main_end)) {
        CYG_TEST_FAIL("invalid line number passed to assertion failure handler");
    }
    // The message passed is known.
    if (0 != strcmp(msg, message)) {
        CYG_TEST_FAIL("invalid message passed to assertion failure handler");
    }
    CYG_TEST_PASS("application-specific assertion failure handler");
    
    // Everything OK, back to main()
    longjmp(setjmp_buffer, 1);
    CYG_TEST_FAIL_FINISH("longjmp() is not functional");
    return false;
}

// -------------------------------------------------------------------------
// A second assertion handler. This is used to make sure that assertion
// failure handlers can be overwritten.

extern "C"
bool
failure_handler2(const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    if (!expecting_failure2) {
        CYG_TEST_FAIL("assertion failure handler2 invoked incorrectly");
    }
    expecting_failure2 = false;
    CYG_TEST_PASS("assertion failure handlers can be replaced");
    longjmp(setjmp_buffer, 1);
    CYG_TEST_FAIL_FINISH("longjmp() is not functional");
    return false;
}
// -------------------------------------------------------------------------
// The third assertion handler. This time a couple of output callbacks are
// installed and the main failure handler has to invoke the output callbacks.
// A number of statics are used to make sure everything works ok.

static const char callback1_title[] = "callback1_header";
static const char callback2_title[] = "callback2_header";
static const char callback2_data[]  = "callback2 data\n";
static bool seen_callback1_title    = false;
static bool seen_callback2_title    = false;
static bool callback1_done          = false;
static bool callback2_done          = false;
static int  number_of_headers       = 0;
static bool callback1_invoked       = false;
static bool callback2_invoked       = false;
static int  number_of_lines_seen    = 0;
const  int  callback2_lines         = 16;
static int  callback2_lines_seen    = 0;
static int  number_of_trailers      = 0;

extern "C"
void
callback1(void (*outputfn)(const char*))
{
    if (callback1_invoked) {
        CYG_TEST_FAIL("callback1 invoked multiple times");
    }
    callback1_invoked = true;

    // This callback does nothing.
}

extern "C"
void
callback2(void (*outputfn)(const char*))
{
    if (callback2_invoked) {
        CYG_TEST_FAIL("callback2 invoked multiple times");
    }
    callback2_invoked = true;
    for (int i = 0; i < callback2_lines; i++) {
        (*outputfn)(callback2_data);
    }
}

// handle_callback_header() should be invoked at least twice,
// once with callback1_header and once with callback2_header
extern "C"
void
handle_callback_header(const char* name)
{
    number_of_headers++;
    if (0 == strcmp(name, callback1_title)) {
        if (seen_callback1_title) {
            CYG_TEST_FAIL("callback1 title seen multiple times");
        } else {
            seen_callback1_title = true;
        }
    }
    if (0 == strcmp(name, callback2_title)) {
        if (seen_callback2_title) {
            CYG_TEST_FAIL("callback2 title seen multiple times");
        } else {
            seen_callback2_title = true;
        }
    }
}

// The body output function should be invoked zero times for
// callback1 and a fixed number of times for callback2.
extern "C"
void
handle_callback_body(const char* data)
{
    number_of_lines_seen++;
    
    if (seen_callback1_title && !callback1_done) {
        CYG_TEST_FAIL("callback1 should not perform any output");
    }
    if (seen_callback2_title && !callback2_done) {
        callback2_lines_seen++;
        if (0 != strcmp(data, callback2_data)) {
            CYG_TEST_FAIL("callback2 has generated incorrect data");
        }
    }
}

// The trailer output function should be invoked at least twice, once
// for each callback.
extern "C"
void
handle_callback_trailer(void)
{
    if (0 == number_of_headers) {
        CYG_TEST_FAIL("callback trailer seen before header");
    }
    number_of_trailers++;
    if (seen_callback1_title && !callback1_done) {
        callback1_done = true;
    }
    if (seen_callback2_title && !callback2_done) {
        callback2_done = true;
    }
}

// This is the failure handler. It causes the various callbacks to run, then
// checks the resulting values of the statics.
extern "C"
bool
failure_handler3(const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    if (!expecting_failure3) {
        CYG_TEST_FAIL("assertion failure handler3 invoked incorrectly");
    }
    expecting_failure3 = false;

    cyg_assert_failure_invoke_callbacks(
        &handle_callback_header,
        &handle_callback_body,
        &handle_callback_trailer);

    bool all_ok = true;
    if (!seen_callback1_title) {
        CYG_TEST_FAIL("callback1's title not seen");
        all_ok = false;
    }
    if (!seen_callback2_title) {
        CYG_TEST_FAIL("callback2's title not seen");
        all_ok = false;
    }
    if (number_of_headers != number_of_trailers) {
        CYG_TEST_FAIL("headers and trailers do not match up");
        all_ok = false;
    }
    if (!callback1_done) {
        CYG_TEST_FAIL("callback1 did not finish");
        all_ok = false;
    }
    if (!callback2_done) {
        CYG_TEST_FAIL("callback2 did not finish");
        all_ok = false;
    }
    if (number_of_lines_seen < callback2_lines) {
        CYG_TEST_FAIL("total number of output lines is less than expected");
        all_ok = false;
    }
    if (callback2_lines_seen != callback2_lines) {
        CYG_TEST_FAIL("callback2 generated the wrong number of lines");
        all_ok = false;
    }
    
    if (all_ok) {
        CYG_TEST_PASS("assertion callbacks");
    }
        
    longjmp(setjmp_buffer, 1);
    CYG_TEST_FAIL_FINISH("longjmp() is not functional");
    return false;
}

// ----------------------------------------------------------------------------
// main(). Perform the various assertion tests in order.

cyg_uint32 main_start = (cyg_uint32) __LINE__;
int
main(int argc, char** argv)
{
    expecting_failure1 = true;
    // First check, a very basic assertion invocation.
    if (setjmp(setjmp_buffer) == 0) {
        cyg_assert_install_failure_handler(&failure_handler1);
        CYG_FAIL(message);
        CYG_TEST_FAIL("assertion did not trigger");
    }
    expecting_failure1 = false;
    expecting_failure2 = true;

    // Now try installing a different assertion handler.
    if (setjmp(setjmp_buffer) == 0) {
        cyg_assert_install_failure_handler(&failure_handler2);
        CYG_FAIL(message);
        CYG_TEST_FAIL("assertion did not trigger");
    }
    expecting_failure2 = false;
    expecting_failure3 = true;

    if (setjmp(setjmp_buffer) == 0) {
        cyg_assert_install_failure_callback(callback1_title, &callback1);
        cyg_assert_install_failure_callback(callback2_title, &callback2);
        cyg_assert_install_failure_handler(&failure_handler3);
        CYG_FAIL(message);
        CYG_TEST_FAIL("assertion did not trigger");
    }
    expecting_failure3 = false;
    
    return 0;
}
cyg_uint32 main_end = (cyg_uint32) __LINE__;

