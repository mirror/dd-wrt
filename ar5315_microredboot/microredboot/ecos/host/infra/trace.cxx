//{{{  Banner                                           

//============================================================================
//
//      trace.cxx
//
//      Host side implementation of the infrastructure trace facilities.
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
// Date:        1998/12/07
// Version:     0.01
// Purpose:     To provide a host-side implementation of the eCos tracing
//              facilities.
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                                       

// Make sure that the host-side extensions get prototyped
// as well. Note that the tracing code needs to interact
// with the assertion facilities to set up an appropriate
// callback.
#define CYG_DECLARE_HOST_ASSERTION_SUPPORT
#include "pkgconf/infra.h"
#include "cyg/infra/cyg_type.h"
#include "cyg/infra/cyg_ass.h"

// Without this #define the tracing enums and prototypes are
// not visible.
#define CYGDBG_USE_TRACING
#include "cyg/infra/cyg_trac.h"

// The standard C++ string class is used extensively
#include <string>

// Add a few C headers
#include <cctype>
#include <cstring>
#include <cstdio>

//}}}

//{{{  Description                                      

// -------------------------------------------------------------------------
// The tracing macros end up calling one of the following routines:
//
// void cyg_tracenomsg(cyg_uint32 what, const char* fn, const char* file, cyg_uint32 line)
// void cyg_tracemsg(  ..., const char* msg)
// void cyg_tracemsg2( ..., CYG_ADDRWORD arg0, CYG_ADDRWORD arg1 )
// void cyg_tracemsg4( ..., CYG_ADDRWORD arg0, CYG_ADDRWORD arg1, ... )
// void cyg_tracemsg6( ..., CYG_ADDRWORD arg0, CYG_ADDRWORD arg1, ... )
// void cyg_tracemsg8( ..., CYG_ADDRWORD arg0, CYG_ADDRWORD arg1, ... )
//
// For the 2/4/6/8 variants the msg argument is essentially a printf()
// style format string. However the intention is that the implementation
// of the trace code can delay doing the formatting until the trace
// information is actually needed (with obvious consequences for
// generated strings). Such an implementation would significantly 
// reduce the overheads associated with tracing, and is what is implemented
// here.
//
// CYG_ADDRWORD is likely to be either "int" or the platform-specific
// 64 bit data type: it should be big enough to hold either a pointer
// or any normal integral type. This causes problems on machines which
// have e.g. 32 bit int and 64 bit long: any 32 bit quantities will
// have been converted to 64 bit quantities in the calling code, and
// it is no longer possible to just pass the format string to sprintf().
// Instead what amounts to a re-implementation of sprintf() is needed
// here.
//
// The basic implementation of this trace code is as follows:
//
// 1) a static array of data structures to hold the trace data. The
//    size can be configured. There is a current index into this
//    array.
//
// 2) the various trace functions simply update this array and the
//    counter.
//
// 3) all of the trace functions also check a static to see whether
//    or not it is necessary to install a trace handler. This cannot
//    be done by means of a static object due to constructor priority
//    ordering problems.
//
// 4) the callback function does all the hardware of the formatting
//    etc.

//}}}
//{{{  Types and statics                                

// ----------------------------------------------------------------------------
// A data structure rather than a class is used to hold the trace data.
// This guarantees that the array gets put in the bss section and is properly
// zeroed. A "valid" field in the structure can be checked when dumping the
// array.

typedef struct trace_entry {
    bool            valid;
    cyg_uint32      what;
    cyg_uint32      line;
    const char*     fn;
    const char*     file;
    const char*     msg;
    CYG_ADDRWORD    data[8];
} trace_entry;

#ifndef CYGNUM_INFRA_TRACE_VECTOR_SIZE
# define CYGNUM_INFRA_TRACE_VECTOR_SIZE 2048
#endif

static trace_entry  tracevec[CYGNUM_INFRA_TRACE_VECTOR_SIZE];
static volatile int trace_index = 0;

// Forward declaration of the callback function, for convenience.
static void trace_callback(void (*)(const char*));

// Has the callback been installed yet?
static bool callback_installed = false;

//}}}
//{{{  The trace functions themselves                   

// ----------------------------------------------------------------------------
// The functions that get called by the trace macros. Typically these work
// as follows:
//
// 1) read and increment the trace index. This makes tracing marginally usable
//    in multi-threaded systems.
//
// 2) invalidate the entry that is about to be updated. Again this helps a bit
//    with multi-threaded systems.
//
// 3) fill in all the fields as per the command-line arguments, zeroing
//    unused fields.
//
// 4) set the valid flag to true, which means the contents can now be output.
//
// This is by no means sufficient to guarantee that a call to dump the trace
// vector in some other thread can work safely, but it may help a little bit.

extern "C" void
cyg_tracenomsg(const char* fn, const char* file, cyg_uint32 line)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = cyg_trace_trace;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = 0;
    tracevec[i].data[0] = 0;
    tracevec[i].data[1] = 0;
    tracevec[i].data[2] = 0;
    tracevec[i].data[3] = 0;
    tracevec[i].data[4] = 0;
    tracevec[i].data[5] = 0;
    tracevec[i].data[6] = 0;
    tracevec[i].data[7] = 0;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

extern "C" void
cyg_tracemsg(cyg_uint32 what, const char* fn, const char* file, cyg_uint32 line, const char* msg)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = what;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = msg;
    tracevec[i].data[0] = 0;
    tracevec[i].data[1] = 0;
    tracevec[i].data[2] = 0;
    tracevec[i].data[3] = 0;
    tracevec[i].data[4] = 0;
    tracevec[i].data[5] = 0;
    tracevec[i].data[6] = 0;
    tracevec[i].data[7] = 0;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

extern "C" void
cyg_tracemsg2(cyg_uint32 what, const char* fn, const char* file, cyg_uint32 line, const char *msg,
              CYG_ADDRWORD arg0, CYG_ADDRWORD arg1)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = what;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = msg;
    tracevec[i].data[0] = arg0;
    tracevec[i].data[1] = arg1;
    tracevec[i].data[2] = 0;
    tracevec[i].data[3] = 0;
    tracevec[i].data[4] = 0;
    tracevec[i].data[5] = 0;
    tracevec[i].data[6] = 0;
    tracevec[i].data[7] = 0;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

extern "C" void
cyg_tracemsg4(cyg_uint32 what, const char *fn, const char* file, cyg_uint32 line, const char *msg,
              CYG_ADDRWORD arg0, CYG_ADDRWORD arg1,
              CYG_ADDRWORD arg2, CYG_ADDRWORD arg3)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = what;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = msg;
    tracevec[i].data[0] = arg0;
    tracevec[i].data[1] = arg1;
    tracevec[i].data[2] = arg2;
    tracevec[i].data[3] = arg3;
    tracevec[i].data[4] = 0;
    tracevec[i].data[5] = 0;
    tracevec[i].data[6] = 0;
    tracevec[i].data[7] = 0;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

extern "C" void
cyg_tracemsg6(cyg_uint32 what, const char *fn, const char* file, cyg_uint32 line, const char *msg,
              CYG_ADDRWORD arg0, CYG_ADDRWORD arg1,
              CYG_ADDRWORD arg2, CYG_ADDRWORD arg3,
              CYG_ADDRWORD arg4, CYG_ADDRWORD arg5)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = what;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = msg;
    tracevec[i].data[0] = arg0;
    tracevec[i].data[1] = arg1;
    tracevec[i].data[2] = arg2;
    tracevec[i].data[3] = arg3;
    tracevec[i].data[4] = arg4;
    tracevec[i].data[5] = arg5;
    tracevec[i].data[6] = 0;
    tracevec[i].data[7] = 0;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

extern "C" void
cyg_tracemsg8(cyg_uint32 what, const char* fn, const char* file, cyg_uint32 line, const char *msg,
              CYG_ADDRWORD arg0, CYG_ADDRWORD arg1,
              CYG_ADDRWORD arg2, CYG_ADDRWORD arg3,
              CYG_ADDRWORD arg4, CYG_ADDRWORD arg5,
              CYG_ADDRWORD arg6, CYG_ADDRWORD arg7)
{
    int i               = trace_index;
    tracevec[i].valid   = false;
    trace_index         = (trace_index + 1) % CYGNUM_INFRA_TRACE_VECTOR_SIZE;

    tracevec[i].what    = what;
    tracevec[i].fn      = fn;
    tracevec[i].file    = file;
    tracevec[i].line    = line;
    tracevec[i].msg     = msg;
    tracevec[i].data[0] = arg0;
    tracevec[i].data[1] = arg1;
    tracevec[i].data[2] = arg2;
    tracevec[i].data[3] = arg3;
    tracevec[i].data[4] = arg4;
    tracevec[i].data[5] = arg5;
    tracevec[i].data[6] = arg6;
    tracevec[i].data[7] = arg7;
    tracevec[i].valid   = true;

    if (!callback_installed) {
        cyg_assert_install_failure_callback("Trace", &trace_callback);
        callback_installed = true;
    }
}

//}}}
//{{{  Output callback                                  

// ----------------------------------------------------------------------------
// Dumping the output. The assertion code will invoke a single callback
// function, cyg_trace_dummy::trace_callback(), with a function pointer
// that can be used for the actual output.
//
// The trace_callback() function loops through the various entries in the
// vector, ignoring invalid ones, and invoking output_entry() for the
// valid ones.
//
// There are a number of utility routines:
//
//     trim_file() is used to take a full pathname and return just the
//     final part of it as a C++ string. There is an upper bound on the
//     length of this string.
//
//     trim_linenum() formats the linenumber sensibly.
//
//     trim_function() is used to parse a __PRETTY_FUNCTION__ value
//     and produce something more manageable.
//
//     parse_msg() is used to construct the full trace message.
//     Because of possible 32/64 bit confusion it is not possible
//     to just use sprintf() for this.

static std::string
trim_file(const char* file)
{
    // If the output is to look reasonable then the result should be a
    // fixed length. 20 characters is reasonable for now.
    const int max_filename_len = 20;
    
    if (0 == file) {
        return std::string(max_filename_len, ' ');
    }

    // Move to the end of the string, and then back again until
    // a directory separator is found. Given the number of levels
    // in a typical eCos directory hierarchy it is probably not
    // worthwhile outputting any of that information.
    const char * pEnd = file + strlen(file);
    while ((pEnd > file) && ('/' != *pEnd) && ('\\' != *pEnd)) {
        pEnd--;
    }
    if (pEnd != file)
        pEnd++;

    std::string result = "";
    int         i      = 0;
    for ( ;(*pEnd != '\0') && (i < max_filename_len); i++, pEnd++) {
        result += *pEnd;
    }
    for ( ; i < max_filename_len; i++) {
        result += ' ';
    }

    return result;
}

// The linenumber output should be up to four digits, right-padded
// with spaces. sprintf() will do the trick nicely.

static std::string
trim_linenum(cyg_uint32 line)
{
    char buf[32];
    sprintf(buf, "%-4d", (int) line);
    return buf;
}

// Extract a function name. On the target side function names
// are usually obtained via __PRETTY_FUNCTION__, and the resulting
// output is a bit on the large side: return value, arguments, etc
// are all included. On the host side the function name is normally
// supplied explicitly and should not be trimmed at all.
//
// Padding is not appropriate since the function name is likely
// to be followed immediately by the argument list. No maximum
// length is imposed - arguably that is a bad idea.
static std::string
trim_function(const char* fn)
{
    if (0 == fn) {
        return "<unknown>";
    }

#if 1
    return fn;
#else
    // This implements the target-side behaviour.
    //
    // First locate the opening bracket. The function name can
    // be identified by walking backwards from that.
    const char *s;
    for (s = fn; ('\0' != *s) && ('(' != *s); s++);
    for ( ; (s > fn) && (*s != ' '); s--);
    if ( s > fn) s++;

    std::string result = "";
    while ( ('\0' != *s) && ('(' != *s) )
        result += *s++;

    return result;
#endif
}

// The trace format string contained a %s. It is necessary to check
// whether the argument is still valid, and return a suitable
// approximation to the actual data.
static std::string
trim_string(const char * arg)
{
    const int max_string_len = 20;

    std::string result = "";
    if (0 == arg) {
        return result;
    }
    int i;
    for ( i = 0; (i < max_string_len) && ('\0' != *arg) && isprint(*arg); i++, arg++) {
        result += *arg;
    }
    return result;
}

// ----------------------------------------------------------------------------
// Parse a printf() style format string and do the appropriate expansions.
// Because of possible confusion between 32 and 64 bit integers it is not
// possible to use sprintf() itself.
//
// It is assumed that the format string is valid, as are most of the
// arguments. The possible exception is %s arguments where a little bit of
// checking happens first.

static std::string
parse_msg(const char* msg, trace_entry& entry)
{
    if (0 == msg) {
        return "";
    }
    // Keep track of the number of arguments in the trace_entry
    // that have been processed.
    int args_index = 0;

    // A utility buffer for sprintf(), e.g. for integer-> string conversions.
    char util_buf[64];
    
    std::string result = "";
    for ( ; '\0' != *msg; msg++) {
        
        if ('%' != *msg) {
            result += *msg;
            continue;
        }

        // We have a format string. Extract all of it.
        std::string format = "%";
        msg++;

        // The first part of the format string may be one or more flags.
        while ( ('-' == *msg) || ('+' == *msg) || (' ' == *msg) ||
                ('#' == *msg) || ('0' == *msg) ) {
            format += *msg++;
        }

        // Next comes the width. If this is an asterix it is necessary to
        // substitute in an actual argument.
        if ('*' == *msg) {
            int width = (args_index < 8) ? (int) entry.data[args_index++] : 0;
            sprintf(util_buf, "%d", width);
            format += util_buf;
            msg++;
        } else {
            // Otherwise the width should be one or more digits
            while( isdigit(*msg) ) {
                format += *msg++;
            }
        }

        // Look for a precision, again coping with an asterix.
        if ('.' == *msg) {
            format += *msg++;
            if ('*' == *msg) {
                int precision = (args_index < 8) ? (int) entry.data[args_index++] : 0;
                sprintf(util_buf, "%d", precision);
                format += util_buf;
                msg++;
            } else {
                // The precision should be one or more digits, with an optional -
                if ('-' == *msg) {
                    format += *msg++;
                }
                while (isdigit(*msg)) {
                    format += *msg++;
                }
            }
        }

        // Now look for h,l and L. These have to be remembered.
        bool short_version = false;
        bool long_version  = false;
        if ('h' == *msg) {
            format        += *msg++;
            short_version  = true;
        } else if (('l' == *msg) || ('L' == *msg)) {
            format        += *msg++;
            long_version   = true;
        }

        // The end of the format string has been reached.
        int format_ch  = *msg;
        format        += *msg;

        // If we have already formatted too many arguments, there is no point
        // in trying to do the actual formatting.
        if ( 8 <= args_index ) {
            continue;
        }
        CYG_ADDRWORD val = entry.data[args_index++];

        switch( format_ch ) {
          case '%' :
              result += '%';
              break;
              
          case 'd' :
          case 'i' :
          case 'o' :
          case 'u' :
          case 'x' :
          case 'X' : 
              // "format" contains the appropriate format string.
              // Invoke sprintf() using util_buf, doing the
              // appropriate cast, and then append the output
              // of util_buf.
              //
              // This is not totally robust. If a ridiculous
              // precision has been specified then util_buf may
              // overflow.
              if (long_version) {
                  sprintf(util_buf, format.c_str(), (long) val);
              } else {
                  // The implicit cast rules mean that shorts do not
                  // require any special attention.
                  sprintf(util_buf, format.c_str(), (int) val);
              }
              result += util_buf;
              break;

          case 'c' :
              sprintf(util_buf, format.c_str(), (int) val);
              result += util_buf;
              break;
              
          case 'p' :
              sprintf(util_buf, format.c_str(), (void *) val);
              result += util_buf;
              break;
              
          case 's' :
          {
              std::string data = trim_string((char *) val);
              sprintf(util_buf, format.c_str(), data.c_str());
              result += util_buf;
              break;
          }

          default :
              // Any attempt to do floating point conversions would be
              // rather tricky given the casts that have been applied.
              // There is no point in doing anything for unrecognised
              // sequences.
              break;
        }
    }
    return result;
}

// ----------------------------------------------------------------------------


static void
output_entry(void (*pOutputFn)(const char*), trace_entry& entry)
{
    std::string output  = trim_file(entry.file)    + " " +
                          trim_linenum(entry.line) + " " +
                          trim_function(entry.fn)  + " ";
    if (0 != entry.msg) {
        
        switch( entry.what) {
          case cyg_trace_trace  : output += " '"; break;
          case cyg_trace_enter  : output += "{{"; break;
          case cyg_trace_args   : output += "(("; break;
          case cyg_trace_return : output += "}}"; break;
          default               : output += " ?";
        }
        output += parse_msg(entry.msg, entry);
        switch( entry.what) {
          case cyg_trace_trace  : output += "' "; break;
          case cyg_trace_enter  : break;
          case cyg_trace_args   : output += "))"; break;
          case cyg_trace_return : break;
          default               : output += "? ";
        }
    }
    output += "\n";
    (*pOutputFn)(output.c_str());
}

static void
trace_callback( void (*pOutputFn)(const char*))
{
    if ((trace_index < 0) || (trace_index >= CYGNUM_INFRA_TRACE_VECTOR_SIZE))
        return;
    
    // Start at the last entry and work back down to zero, skipping
    // invalid ones. Then go to the top and work back to the current index.
    int i;
    for (i = trace_index - 1; i >= 0; i--) {
        if (tracevec[i].valid) {
            output_entry(pOutputFn, tracevec[i]);
        }
    }
    for (i = (CYGNUM_INFRA_TRACE_VECTOR_SIZE - 1); i >= trace_index; i--) {
        if (tracevec[i].valid) {
            output_entry(pOutputFn, tracevec[i]);
        }
    }
}

//}}}
