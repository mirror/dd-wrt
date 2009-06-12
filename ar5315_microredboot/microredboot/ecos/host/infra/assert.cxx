//{{{  Banner                                           

//============================================================================
//
//      assert.cxx
//
//      Host side implementation of the infrastructure assertions.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
// Copyright (C) 1998, 1999, 2000, 2001 Red Hat, Inc.
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
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        1998/11/27
// Version:     0.01
// Purpose:     To provide a host-side implementation of the eCos assertion
//              facilities.
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                                       

#include "pkgconf/infra.h"
#include "cyg/infra/cyg_type.h"
// Without this symbol the header file has no effect
#define CYGDBG_USE_TRACING
// Make sure that the host-side extensions get prototyped
// as well.
#define CYG_DECLARE_HOST_ASSERTION_SUPPORT
#include "cyg/infra/cyg_ass.h"

// STDIO is needed for the default assertion handler.
// STDLIB is needed for exit() and the status codes.
#include <cstdio>
#include <cstdlib>

#if defined(__unix__) || defined(__CYGWIN32__)
extern "C" {
#include <unistd.h>		// Needed for _exit()
}
#endif

// These are needed for the table of callbacks.
#include <utility>
#include <iterator>
#include <vector>

//}}}

// -------------------------------------------------------------------------
// Statics. The host-side assertion code requires two bits of data.
//
// The first identifies the function that should actually get invoked
// when an assertion is triggered. A default implementation is defined
// in this module, but applications may install a replacement.
//
// The second is a table of callback functions that various libraries
// or bits of application code may install. Each such callback gets invoked
// when an assertion triggers.

// VC++ bogosity. Using a full function pointer prototype in a template
// confuses the compiler. It is still possible to declare the callbacks vector,
// but not any iterators for that vector. A typedef makes the problem go
// away.
typedef void (*cyg_callback_fn)(void (*)(const char*));

                                        // The current assertion handler
static bool (*current_handler)( const char*, const char*, cyg_uint32, const char*) = 0;

                                        // The callback table.
static std::vector<std::pair<const char*, cyg_callback_fn> > callbacks;

// ----------------------------------------------------------------------------
// Many applications will want to handle assertion failures differently
// from the default, for example pipe the output into an emacs buffer
// rather than just generate a file. This routine allows a suitable
// function to be installed.

extern "C" void
cyg_assert_install_failure_handler( bool(*fn)(const char*, const char*, cyg_uint32, const char*) )
{
    current_handler = fn;
}

// ----------------------------------------------------------------------------
// Various different bits of the system may want to register callback functions
// that get invoked during an assertion failure and that output useful
// data. Typically this might happen in the constructor for a static object.
// A good example of such a callback is the implementation of the trace code.
//
// The implementation requires creating a new entry in the static vector.
// A memory exhaustion exception could occur but there is no sensible way of
// handling it at this level.
//
// Multiple callbacks with the same name are legal. Multiple callbacks with
// the same function are unlikely, but it is probably not worthwhile raising
// an exception (especially since this code may be called from C).
extern "C" void
cyg_assert_install_failure_callback( const char* name, void (*fn)(void (*)(const char*)) )
{
    callbacks.push_back(std::make_pair(name, fn));
}

// -------------------------------------------------------------------------
// Once an assertion has triggered either the default handler or the
// installed handler will want to invoke all the callbacks. Rather than
// provide direct access to the callback table and require the calling
// code to be in C++, a functional interface is provided instead.
extern "C" void
cyg_assert_failure_invoke_callbacks(
    void (*first_fn)(const char*),
    void (*data_fn)(const char*),
    void (*final_fn)(void) )
{
    std::vector<std::pair<const char*, cyg_callback_fn> >::const_iterator i;

    for ( i = callbacks.begin(); i != callbacks.end(); i++ ) {

        if (0 != first_fn) {
            (*first_fn)(i->first);
        }
        if (0 != data_fn) {
            (*(i->second))(data_fn);
        }
        if (0 != final_fn) {
            (*final_fn)();
        }
    }
}

// ----------------------------------------------------------------------------
// The default assertion handler. This assumes that the application is 
// a console application with a sensible stderr stream.
//
// First some initial diagnostics are output immediately, in case
// subsequent attempts to output more data cause additional failures. It
// is worthwhile detecting recursive assertion failures.
//
// Assuming the table of callbacks is not empty it is possible to
// output some more data to a file. If possible mkstemp() is used to
// create this file. If mkstemp() is not available then tmpnam() is
// used instead. That function has security problems, albeit not ones
// likely to affect dump files. Once the file is opened the callbacks
// are invoked. Three utilities have to be provided to do the real
// work, and a static is used to keep track of the FILE * pointer.
//
// The testcase tassert8, and in particular the associated Tcl proc
// tassert8_filter in testsuite/cyginfra/assert.exp, has detailed
// knowledge of the output format. Any changes here may need to be
// reflected in that test case. There are also support routines in
// hosttest.exp which may need to be updated.

static FILE * default_handler_output_file = 0;
static bool   body_contains_data          = false;

                                        // output the callback name
static void
default_handler_first_fn(const char* name)
{
    if (0 != default_handler_output_file) {
        fprintf(default_handler_output_file, "# {{{  %s\n\n", name);
    }
    body_contains_data = false;
}

                                        // output some actual text.
static void
default_handler_second_fn(const char* data)
{
    body_contains_data = true;
    if (0 != default_handler_output_file) {
        fputs(data, default_handler_output_file);
    }
}

                                        // the end of a callback.
static void
default_handler_final_fn( void )
{
    
    if (0 != default_handler_output_file) {
        if (body_contains_data) {
            fputs("\n", default_handler_output_file);
        }
        fputs("# }}}\n", default_handler_output_file);
    }
}


static void
default_handler(const char* fn, const char* file, cyg_uint32 lineno, const char* msg)
{
    static int invoke_count = 0;
    if (2 == invoke_count) {
        // The fprintf() immediately below causes an assertion failure
    } else if (1 == invoke_count) {
        invoke_count++;
        fprintf(stderr, "Recursive assertion failure.\n");
        return;
    } else {
        invoke_count = 1;
    }
    
    // There is an argument for using write() rather than fprintf() here,
    // in case the C library has been corrupted. For now this has not been
    // attempted.
    if (0 == msg)
        msg ="<unknown>";
    if (0 == file)
        file = "<unknown>";
    
    fprintf(stderr, "Assertion failure: %s\n", msg);
    fprintf(stderr, "File %s, line number %lu\n", file, (unsigned long) lineno);
    if (0 != fn)
        fprintf(stderr, "Function %s\n", fn);
    
    // Only create a logfile if more information is available.
    if (0 != callbacks.size() ) {

        // Use mkstemp() if possible, but only when running on a platform where /tmp
        // is likely to be available.
#if defined(HAVE_MKSTEMP) && !defined(_MSC_VER)
        char filename[32];
        int  fd;
        strcpy(filename, "/tmp/ecosdump.XXXXXX");
        fd = mkstemp(filename);
        if (-1 == fd) {
            fprintf(stderr, "Unable to create a suitable output file for additional data.\n");
        } else {
            default_handler_output_file = fdopen(fd, "w");
            if (0 == default_handler_output_file) {
                close(fd);
            }
        }
#else
        char filename[L_tmpnam];
        if (0 == tmpnam(filename)) {
            fprintf(stderr, "Unable to create a suitable output file for additional data.\n");
        } else {

            // No attempt is made to ensure that the file does not already
            // exist. This would require POSIX calls rather than ISO C ones.
            // The probability of a problem is considered to be too small
            // to worry about.
            default_handler_output_file = fopen(filename, "w");
        }
#endif
        if (0 == default_handler_output_file) {
            fprintf(stderr, "Unable to open output file %s\n", filename);
            fputs("No further assertion information is available.\n", stderr);
        } else {
            fprintf(stderr, "Writing additional output to %s\n", filename);
                
            // Repeat the information about the assertion itself.
            fprintf(default_handler_output_file, "Assertion failure: %s\n", msg);
            fprintf(default_handler_output_file, "File %s, line number %lu\n", file, (unsigned long) lineno);
            if (0 != fn)
                fprintf(default_handler_output_file, "Function %s\n", fn);
            fputs("\n", default_handler_output_file);

            // Now for the various callbacks.
            cyg_assert_failure_invoke_callbacks( &default_handler_first_fn,
                                                 &default_handler_second_fn, &default_handler_final_fn );

            // And close the file.
            fputs("\nEnd of assertion data.\n", default_handler_output_file);
            fclose(default_handler_output_file);
        }
    }
    fflush(stderr);
}

// ----------------------------------------------------------------------------
// The assertion handler. This is the function that gets invoked when
// an assertion triggers. If a special assertion handler has been installed
// then this gets called. If it returns false or if no special handler is
// available then the default handler gets called instead. Typically the
// user will now have a lot of information about what happened to cause the
// assertion failure. The next stage is to invoke abort() which should
// terminate the program and generate a core dump for subsequent inspection
// (unless of course the application is already running in a debugger session).
// A final call to _exit() should be completely redundant.

extern "C" void
cyg_assert_fail( const char* fn, const char* file, cyg_uint32 lineno, const char* msg )
{

    if ((0 == current_handler) || !(*current_handler)(fn, file, lineno, msg)) {
        default_handler(fn, file, lineno, msg);
    }
    abort();
    _exit(0);
}

// ----------------------------------------------------------------------------
// A utility function, primarily intended to be called from inside gdb.
extern "C" void
cyg_assert_quickfail(void)
{
    cyg_assert_fail("gdb", "<no file>", 0, "manual call");
}
