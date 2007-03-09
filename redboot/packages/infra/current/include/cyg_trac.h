#ifndef CYGONCE_INFRA_CYG_TRAC_H
#define CYGONCE_INFRA_CYG_TRAC_H

//==========================================================================
//
//      cyg_trac.h
//
//      Macros and prototypes for the tracing system
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg from an original by hmt
// Contributors:        nickg
// Date:        1998-04-23
// Purpose:     Use traces to log procedure entry, and "print" stuff
// Description: Runtime logging messages that compile to nothing in
//              release versions of the code, to allow
//              as-you-go tracing of alternate builds.
// Usage:       #include <cyg/infra/cyg_trac.h>
//              ...
//              CYG_TRACE2( PIPE_TRACE, "pipe %x, data %d", ppipe, oword );
//
//      which can result, for example, in a message of the form:
//      "TRACE: pipemgr.cxx:1340, write_pipe(): pipe 0x8004c, data 17"
//
//####DESCRIPTIONEND####
//

/****************************************************************************

Explicit tracing
================

CYG_TRACE0( bool, msg );
CYG_TRACE1( bool, msg, arg1 );
CYG_TRACE2( bool, msg, arg1, arg2 );
....
CYG_TRACE8( bool, msg, .... [with 8 args] );

In general, the bool controls whether or not the tracing occurs for a
particular invocation of the macro.  The msg is a printf-style string,
though exactly which formats are supported depends on the underlying
implementation.  Typically, at least %d, %x, %08x, %c and %s will be
supported.  Of course a most compact implementation might print

  TRACE:z1dbuff.c[92]get_nextdata(): data pointer %x offset %d: 42BD8 1C

or some such, leaving you to work it out for yourself.

It is expected that the boolean would rarely actually be a complex
expression; it is more likely that it would either be "1", tracing being
controlled for the whole compilation unit or subsystem by means of the
CYGDBG_USE_TRACING symbol, or a local symbol for tracing over the whole
file, defined to 0 or to 1.  For runtime control of tracing in a debugging
session, it is typical to use symbols defined to expressions such as:

    static int xxx_trace = 0;
    #define TL1 (0 < xxx_trace)
    #define TL2 (1 < xxx_trace)

so you set xxx_trace to 1 to enable those messages conditioned by TL1
(trace level 1) and so on.

    CYG_TRACE1( TL1, "Major argument is %d", zz );
    CYG_TRACE4( TL2, "...minor details %d %d %d %d", m1, m2, m3 ,m4 );

To assist with the case where the same symbol or expression is used
throughout a compilation unit, the programmer can define the symbol
CYG_TRACE_USER_BOOL as they choose and then use convenience macros with the
suffix 'B' in the obvious manner:

    #define CYG_TRACE_USER_BOOL (xxx_trace > 0)
    CYG_TRACE2B( "Counters are %d, %d", countlo, counthi );

For the case where you just want to print a load of numbers in hex, or
decimal, convenience suffices X, D and Y are provided.  X uses %08x, D %d
and Y an unadorned %x for each argument.

    CYG_TRACE3D( TL2, m1, m2, d );
   
If you want to do something similar but with a little more comment, the
names (strictly spellings) of the variables you are printing can be used by
appending a V to the X, D or Y.

    CYG_TRACE3DV( TL2, m1, m2, d );

might output:

  TRACE:z1dbuff.c[92]get_nextdata(): m1=23 m2=-4 d=55

These conveniences can be combined, and they apply equally to tracing with
up to 8 variables; the B for Bool goes last:

     CYG_TRACE4DVB( i, i*i, i*i*i, i*i*i*i );

might output:

  TRACE:table.c[12]main(): i=3 i*i=9 i*i*i=27 i*i*i*i=81


Function Tracing
================

There are also facities for easily reporting function entry and exit,
printing the function arguments, and detecting returns without logging (or
without a value!).

The basic facility is

        CYG_REPORT_FUNCTION();

In C, place this between the local variable declarations and the first
statement or errors will ensue.  C++ is more flexible; place the macro as
the first line of all functions you wish to trace.  The following
variations are also provided:

  CYG_REPORT_FUNCTYPE( exitmsg )  provide a printf string for the type
                                  of the returned value
  CYG_REPORT_FUNCNAME( name )     supply a function name explicitly, for
                                  if __FUNCTION__ is not supported
  CYG_REPORT_FUNCNAMETYPE( name, exitmsg ) both of the above extensions

These are unconditional; the assumption is that if function reporting is
used at all it will be used for all functions within a compilation unit.
However, it is useful to be able to control function reporting at finer
grain without editing the source files concerned, at compile time or at
runtime.  To support this, conditioned versions (with suffix 'C') are
provided for the above four macros, which only procduce trace output if the
macro CYG_REPORT_USER_BOOL evaluates true.

  CYG_REPORT_FUNCTIONC()
  CYG_REPORT_FUNCNAMEC( name )
  CYG_REPORT_FUNCTYPEC( exitmsg )
  CYG_REPORT_FUNCNAMETYPEC( name, exitmsg )

You can define CYG_REPORT_USER_BOOL to anything you like before invoking
these macros; using a simple -DCYG_REPORT_USER_BOOL=0 or ...=1 on the
compiler command line would do the trick, but there is more flexibility to
be gained by something like:

  #define CYG_REPORT_USER_BOOL (reporting_bool_FOO)
  #ifdef TRACE_FOO
  int reporting_bool_FOO = 1;
  #else
  int reporting_bool_FOO = 0;
  #endif

where FOO relates to the module name.  Thus an external symbol sets the
default, but it can be overridden in a debugging session by setting the
variable reporting_bool_FOO.

Note that the condition applied to the initial CYG_REPORT_FUNC...() macro
controls all function-related reporting (not tracing) from that function;
the underlying mechanisms still operate even if no output is created.  Thus
no conditioned variants of CYG_REPORT_FUNCARG[s] nor of CYG_REPORT_RETURN
are needed.

Examples:
    int myfunction()
    {
        CYG_REPORT_FUNCTYPE( "recode is %d" );

A function return is traced using

    CYG_REPORT_RETURN()         a void return
    CYG_REPORT_RETVAL( value )  returning a value

With the CYG_REPORT_FUNCTYPE example, the latter might produce a message
like:

  TRACE:myprog.c[40]fact(): enter
  TRACE:myprog.c[53]fact(): retcode is 24

It is also useful to trace the values of the arguments to a function:
        CYG_REPORT_FUNCARGVOID          confirms that the function is void
        CYG_REPORT_FUNCARG1( format, arg )              printf-style
                to
        CYG_REPORT_FUNCARG8( format, arg1...arg8 )      printf-style

The CYG_REPORT_FUNCARG[1-8] macros are also offered with the convenience
extensions: D, X, or Y, and V like the explicit tracing macros.  For
example:

    int fact( int number )
    {
        CYG_REPORT_FUNCTYPE( "recode is %d" );
        CYG_REPORT_FUNCARG1DV( number );
        int result = number;
        while ( --number > 1 )  result *= number
        CYG_REPORT_RETVAL( result );
        return result;
    }

might produce:

  TRACE:myprog.c[40]fact(): enter
  TRACE:myprog.c[40]fact(): number=4
  TRACE:myprog.c[53]fact(): retcode is 24

If no exit message is provided, a default of %08x is used.


General Configury
=================

If CYGDBG_INFRA_DEBUG_FUNCTION_PSEUDOMACRO is *not* defined, it is assumed
that __PRETTY_FUNCTION__ or equivalents do not exist, so no function name
tracing is possible; only file and line number.

If CYGDBG_INFRA_DEBUG_TRACE_MESSAGE is *not* defined, the message and
arguments to all tracing macros are not used; only "execution was here"
type information, by file, function and line number, is available.  This
can greatly reduce the size of an image with tracing disabled, which may be
crucial in debugging on actual shipped hardware with limited memory.

If configured for buffered tracing then CYG_TRACE_PRINT() can be used to
output the contents of the trace buffer on demand.

CYG_TRACE_DUMP() outputs a form of "core dump" containing info on the
scheduler and threads at the time. This information will be invalid if
the kernel is not running.

C/C++: in C++ the function reporting is implemented using a class object
with a destructor; this allows reporting of a return which has not been
explicitly reported, and detection of accidental multiple return reports.
This helps you write the function reporting correctly.  In C it is not
possible to be so sophisticated, so the implementation is not so helpful in
detecting errors in the use of the tracing system.

Note that for all of the above variations, the internal API to the
functions which are called in consequence of tracing remains the same, so
these variations can be mixed in the same executable, by configuring the
tracing macros differently in different compilation units or subsystems.


Summary
=======

Explicit tracing
----------------

CYG_TRACE0( bool, msg )                         if bool, print msg
CYG_TRACE1( bool, msg, arg )                    if bool, printf-style
        to
CYG_TRACE8( bool, msg, arg1...arg8 )            if bool, printf-style

CYG_TRACE0B( msg, args... ) to CYG_TRACE8B()    use CYG_TRACE_USER_BOOL

CYG_TRACE1X( bool, args... ) to CYG_TRACE8X()   print args using %08x
CYG_TRACE1Y( bool, args... ) to CYG_TRACE8Y()   print args using %x
CYG_TRACE1D( bool, args... ) to CYG_TRACE8D()   print args using %d

CYG_TRACE1XV( bool, args... ) to CYG_TRACE8XV() print args using "arg=%08x"
CYG_TRACE1YV( bool, args... ) to CYG_TRACE8YV() print args using "arg=%x"
CYG_TRACE1DV( bool, args... ) to CYG_TRACE8DV() print args using "arg=%d"

CYG_TRACE1XB( args... ) to CYG_TRACE8XB()       print using %08x, no bool
CYG_TRACE1YB( args... ) to CYG_TRACE8YB()       print using %x, no bool
CYG_TRACE1DB( args... ) to CYG_TRACE8DB()       print using %d, no bool

CYG_TRACE1XVB( args... ) to CYG_TRACE8XVB()     use "arg=%08x", no bool
CYG_TRACE1YVB( args... ) to CYG_TRACE8YVB()     use "arg=%x", no bool
CYG_TRACE1DVB( args... ) to CYG_TRACE8DVB()     use "arg=%d", no bool

Function tracing
----------------

CYG_REPORT_FUNCTION()                           default function entry
CYG_REPORT_FUNCNAME( name )                     name the function
CYG_REPORT_FUNCTYPE( exitmsg )                  printf for retval
CYG_REPORT_FUNCNAMETYPE( name, exitmsg )        both

CYG_REPORT_FUNCTIONC()                          as above, but conditional
CYG_REPORT_FUNCNAMEC( name )                    on CYG_REPORT_USER_BOOL
CYG_REPORT_FUNCTYPEC( exitmsg )                 however it is defined
CYG_REPORT_FUNCNAMETYPEC( name, exitmsg )       ...

CYG_REPORT_RETURN()                             void function exit
CYG_REPORT_RETVAL( value )                      returning value

CYG_REPORT_FUNCARGVOID()                        void function entry
CYG_REPORT_FUNCARG1( format, arg )              printf-style
        to
CYG_REPORT_FUNCARG8( format, arg1...arg8 )      printf-style

CYG_REPORT_FUNCARG1X( arg )
        to
CYG_REPORT_FUNCARG8X( arg1...arg8 )             use %08x
CYG_REPORT_FUNCARG1Y...                         use %x
CYG_REPORT_FUNCARG1D...                         use %d

CYG_REPORT_FUNCARG1XV...                        use "arg=%08x"
CYG_REPORT_FUNCARG1YV...                        use "arg=%x"
CYG_REPORT_FUNCARG1DV...                        use "arg=%d"

Other
-----

CYG_TRACE_DUMP()                                dumps kernel state
CYG_TRACE_PRINT()                               prints buffered tracing


---------------------------------------------------------------------------

Internal Documentation
======================

The required functions which are used by the tracing macros are

    externC void
    cyg_tracenomsg( const char *psz_func, const char *psz_file, 
                    cyg_uint32 linenum );
    
    externC void
    cyg_tracemsg( cyg_uint32 what, const char *psz_func, const char *psz_file,
                  cyg_uint32 linenum, const char *psz_msg );
    
    externC void
    cyg_tracemsg2( cyg_uint32 what, 
                   const char *psz_func, const char *psz_file,
                   cyg_uint32 linenum, const char *psz_msg,
                   CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1 );
    // extended in the obvious way for 4,6,8 arguments

These functions should expect psz_func and psz_file to possibly be NULL in
case those facilities are not available in the compilation environment, and
do something safe in such cases.  A NULL message should really be dealt
with safely also, just logging "execution here" info like cyg_tracenomsg().

Discussion of possible underlying implementations
-------------------------------------------------

It is intended that the functions that get called can simply print the info
they are given in as fancy a format as you like, or they could do the
printf-type formatting and log the resulting text in a buffer.  They get
told the type of event (function-entry, function-arguments, function-exit
or plain tracing info) and so can perform fancy indenting, for example, to
make call stack inspection more obvious to humans.  It is also intended
that a more compact logging arrangement be possible, for example one which
records, in 32-bit words (CYG_ADDRWORDs), the addresses of the file,
function and msg strings, the line number and the arguments.  This has the
implication that the msg string should not be constructed dynamically but
be static ie. a plain quoted C string.  The number of arguments also must
be recorded, and if it is chosen to save string arguments in the buffer
rather than just their addresses (which could be invalid by the time the
logged information is processed) some flagging of which arguments are
strings must be provided.  The system could also be extended to deal with
floats of whichever size fir in a CYG_ADDRWORD; these would probably
require special treatment also.  With these considerations in mind, the
maximum number of parameters in a single trace message has been set to 8,
so that a byte bitset could be used to indicate which arguments are
strings, another for those which are floats, and the count of arguments
also fits in a byte as number or a bitset.


****************************************************************************/

#include <pkgconf/infra.h>

#include <cyg/infra/cyg_ass.h>

// -------------------------------------------------------------------------
// CYGDBG_INFRA_DEBUG_FUNCTION_PSEUDOMACRO is dealt with in cyg_ass.h.
// -------------------------------------------------------------------------

#ifdef CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
// We define macros and appropriate prototypes for the trace/fail
// system.  These are:
//      CYG_TRACE0..8     - trace if boolean
//      CYG_TRACEPROC     - default no comment proc entry
//      CYG_TRACEPROCOUT  - default no comment proc exit
//      CYG_TRACE_DUMP    - outputs a form of "core dump", including the state
//                          of the kernel scheduler, threads, etc.
//      CYG_TRACE_PRINT   - Forces manual output of any trace info that has
//                          been buffered up.

// these are executed to deal with tracing - breakpoint?

externC void
cyg_tracenomsg( const char *psz_func, const char *psz_file, cyg_uint32 linenum );

externC void
cyg_trace_dump(void);

#define CYG_TRACE_DUMP() cyg_trace_dump()

#ifdef CYGDBG_INFRA_DEBUG_TRACE_ASSERT_BUFFER

externC void
cyg_trace_print(void);

#define CYG_TRACE_PRINT() cyg_trace_print()

#else
#define CYG_TRACE_PRINT() CYG_EMPTY_STATEMENT
#endif

// provide every other one of these as a space/caller bloat compromise.

# ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

enum cyg_trace_what{
    cyg_trace_trace = 0,
    cyg_trace_enter,
    cyg_trace_args,
    cyg_trace_return,
//    cyg_trace_,
//    cyg_trace_,
};

externC void
cyg_tracemsg( cyg_uint32 what, 
              const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
              const char *psz_msg );

externC void
cyg_tracemsg2( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1 );
externC void
cyg_tracemsg4( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3 );
externC void
cyg_tracemsg6( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5 );
externC void
cyg_tracemsg8( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum, 
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5,
               CYG_ADDRWORD arg6,  CYG_ADDRWORD arg7 );

#endif // CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

// -------------------------------------------------------------------------

# ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#  define CYG_TRACE_docall0( _msg_ )                                    \
    cyg_tracemsg( cyg_trace_trace,                                      \
                  __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_ );

#  define CYG_TRACE_docall2( _msg_, _arg0_, _arg1_ )                    \
    cyg_tracemsg2( cyg_trace_trace,                                     \
                   __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_,      \
                 (CYG_ADDRWORD)_arg0_, (CYG_ADDRWORD)_arg1_ );

#  define CYG_TRACE_docall4( _msg_, _arg0_, _arg1_ , _arg2_, _arg3_  )  \
    cyg_tracemsg4( cyg_trace_trace,                                     \
                   __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_,      \
                 (CYG_ADDRWORD)_arg0_, (CYG_ADDRWORD)_arg1_,            \
                 (CYG_ADDRWORD)_arg2_, (CYG_ADDRWORD)_arg3_ );

#  define CYG_TRACE_docall6( _msg_, _arg0_, _arg1_ , _arg2_, _arg3_,    \
                                    _arg4_, _arg5_                   )  \
    cyg_tracemsg6( cyg_trace_trace,                                     \
                   __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_,      \
                 (CYG_ADDRWORD)_arg0_, (CYG_ADDRWORD)_arg1_,            \
                 (CYG_ADDRWORD)_arg2_, (CYG_ADDRWORD)_arg3_,            \
                 (CYG_ADDRWORD)_arg4_, (CYG_ADDRWORD)_arg5_ );

#  define CYG_TRACE_docall8( _msg_, _arg0_, _arg1_ , _arg2_, _arg3_,    \
                                    _arg4_,  _arg5_, _arg6_, _arg7_ )   \
    cyg_tracemsg8( cyg_trace_trace,                                     \
                   __PRETTY_FUNCTION__, __FILE__, __LINE__, _msg_,      \
                 (CYG_ADDRWORD)_arg0_, (CYG_ADDRWORD)_arg1_,            \
                 (CYG_ADDRWORD)_arg2_, (CYG_ADDRWORD)_arg3_,            \
                 (CYG_ADDRWORD)_arg4_, (CYG_ADDRWORD)_arg5_,            \
                 (CYG_ADDRWORD)_arg6_, (CYG_ADDRWORD)_arg7_ );

# else // do not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#  define CYG_TRACE_docall0( _msg_ )                                    \
    cyg_tracenomsg( __PRETTY_FUNCTION__, __FILE__, __LINE__ );

#  define CYG_TRACE_docall2( _msg_, _arg0_, _arg1_ )                    \
    cyg_tracenomsg( __PRETTY_FUNCTION__, __FILE__, __LINE__ );

#  define CYG_TRACE_docall4( _msg_, _arg0_, _arg1_ , _arg2_, _arg3_  )  \
    cyg_tracenomsg( __PRETTY_FUNCTION__, __FILE__, __LINE__ );

#  define CYG_TRACE_docall6( _msg_, _arg0_, _arg1_ , _arg2_, _arg3_,    \
                                    _arg4_, _arg5_                   )  \
    cyg_tracenomsg( __PRETTY_FUNCTION__, __FILE__, __LINE__ );

#  define CYG_TRACE_docall8( _msg_, _arg0_, _arg1_, _arg2_, _arg3_,     \
                                    _arg4_, _arg5_, _arg6_, _arg7_   )  \
    cyg_tracenomsg( __PRETTY_FUNCTION__, __FILE__, __LINE__ );

#endif

// -------------------------------------------------------------------------
// Conditioned trace; if the condition is false, fail.

#define CYG_TRACE0( _bool_, _msg_ )                             \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall0( _msg_ );                             \
    CYG_MACRO_END

#define CYG_TRACE1( _bool_, _msg_, a )                          \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall2( _msg_, a, 0 );                       \
    CYG_MACRO_END
 
#define CYG_TRACE2( _bool_, _msg_, a, b )                       \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall2( _msg_, a, b );                       \
    CYG_MACRO_END

#define CYG_TRACE3( _bool_, _msg_, a, b, c )                    \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall4( _msg_, a, b, c, 0 );                 \
    CYG_MACRO_END
 
#define CYG_TRACE4( _bool_, _msg_, a, b, c, d )                 \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall4( _msg_, a, b, c, d );                 \
    CYG_MACRO_END

#define CYG_TRACE5( _bool_, _msg_, a, b, c, d, e )              \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall6( _msg_, a, b, c, d, e, 0 );           \
    CYG_MACRO_END
 
#define CYG_TRACE6( _bool_, _msg_, a, b, c, d, e, f )           \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall6( _msg_, a, b, c, d, e, f );           \
    CYG_MACRO_END

#define CYG_TRACE7( _bool_, _msg_, a, b, c, d, e, f, g )        \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall8( _msg_, a, b, c, d, e, f, g, 0 );     \
    CYG_MACRO_END
 
#define CYG_TRACE8( _bool_, _msg_, a, b, c, d, e, f, g, h )     \
    CYG_MACRO_START                                             \
    if ( ( _bool_ ) )                                           \
        CYG_TRACE_docall8( _msg_, a, b, c, d, e, f, g, h );     \
    CYG_MACRO_END

// -------------------------------------------------------------------------
// Report function entry and exit.
// In C++ the macro CYG_REPORT_FUNCTION should appear as the first line of
// any function. It will generate a message whenever the function is entered
// and when it is exited.
// In C the macro should appear as the first statement after any local variable
// definitions. No exit message will be generated unless CYG_REPORT_RETURN is
// placed just before each return.
// Where a piece of code is to be compiled with both C and C++, the above
// rules for C should be followed.

#ifdef CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS
        
#ifdef __cplusplus

class Cyg_TraceFunction_Report_
{
public:
    int   cond;
    const char *func;
    const char *file;
    cyg_uint32 lnum;
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
    char *exitmsg;
    CYG_ADDRWORD exitvalue;
    enum { UNSET = 0, SET, VOID } exitset;
#endif

    Cyg_TraceFunction_Report_(
        int condition, const char *psz_func, const char *psz_file,
        cyg_uint32 linenum)
    {
        cond = condition;
        func = psz_func;
        file = psz_file;
        lnum = linenum;
        
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
        exitmsg = NULL;
        exitset  = UNSET;
        if ( cond )
            cyg_tracemsg( cyg_trace_enter, func, file, lnum, "enter");
#else
        if ( cond )
            cyg_tracenomsg( func, file, lnum );
#endif
    };

    Cyg_TraceFunction_Report_(
        int condition, const char *psz_func, const char *psz_file, 
        cyg_uint32 linenum, char *psz_exitmsg )
    {
        cond = condition;
        func = psz_func;
        file = psz_file;
        lnum = linenum;
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
        exitmsg = psz_exitmsg;
        exitset  = UNSET;
        if ( cond )
            cyg_tracemsg( cyg_trace_enter, func, file, lnum, "enter");
#else
        CYG_UNUSED_PARAM( char *, psz_exitmsg );
        if ( cond )
            cyg_tracenomsg( func, file, lnum );
#endif
    };

    inline void set_exitvoid( cyg_uint32 linenum )
    {
        lnum = linenum;
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
        CYG_ASSERT( NULL == exitmsg, "exitvoid used in typed function" );
        CYG_ASSERT( UNSET == exitset, "exitvoid used when arg already set" );
        exitset = VOID;
#endif
    }

    inline void set_exitvalue( cyg_uint32 linenum, CYG_ADDRWORD retcode )
    {
        lnum = linenum;
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
        CYG_ASSERT( UNSET == exitset, "exitvalue used when arg already set" );
        exitvalue = retcode;
        exitset = SET;
#else
        CYG_UNUSED_PARAM( CYG_ADDRWORD, retcode );
#endif
    }

    ~Cyg_TraceFunction_Report_()
    {
        if ( cond ) {
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
            if ( VOID == exitset )
                cyg_tracemsg( cyg_trace_return, func, file, lnum,
                              "return void");
            else if ( UNSET == exitset )
                cyg_tracemsg( cyg_trace_return, func, file, lnum,
                              "RETURNING UNSET!");
            else if ( NULL == exitmsg )
                cyg_tracemsg2( cyg_trace_return, func, file, lnum,
                               "return %08x", exitvalue, 0 );
            else
                cyg_tracemsg2( cyg_trace_return, func, file, lnum,
                               exitmsg, exitvalue, 0 );
#else
            cyg_tracenomsg( func, file, lnum );
#endif
        }
    }
};

// These have no CYG_MACRO_START,END around because it is required
// that the scope of the object be the whole function body.  Get it?

// These are the unconditional versions:
#define CYG_REPORT_FUNCTION()                           \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        1, __PRETTY_FUNCTION__,                         \
        __FILE__, __LINE__ )

#define CYG_REPORT_FUNCTYPE( _exitmsg_ )                \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        1, __PRETTY_FUNCTION__,                         \
        __FILE__, __LINE__, _exitmsg_ )

#define CYG_REPORT_FUNCNAME( _name_ )                   \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        1, _name_,                                      \
        __FILE__, __LINE__ )

#define CYG_REPORT_FUNCNAMETYPE( _name_, _exitmsg_  )   \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        1, _name_,                                      \
        __FILE__, __LINE__, _exitmsg_ )

// These are conditioned on macro CYG_REPORT_USER_BOOL
// (which you better have defined)
#define CYG_REPORT_FUNCTIONC()                          \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        CYG_REPORT_USER_BOOL, __PRETTY_FUNCTION__,      \
        __FILE__, __LINE__ )

#define CYG_REPORT_FUNCTYPEC( _exitmsg_ )               \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        CYG_REPORT_USER_BOOL, __PRETTY_FUNCTION__,      \
        __FILE__, __LINE__, _exitmsg_ )

#define CYG_REPORT_FUNCNAMEC( _name_ )                  \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        CYG_REPORT_USER_BOOL, _name_,                   \
        __FILE__, __LINE__ )

#define CYG_REPORT_FUNCNAMETYPEC( _name_, _exitmsg_  )  \
  Cyg_TraceFunction_Report_ cyg_tracefunction_report_(  \
        CYG_REPORT_USER_BOOL, _name_,                   \
        __FILE__, __LINE__, _exitmsg_ )


#define CYG_REPORT_RETURN() CYG_MACRO_START             \
    cyg_tracefunction_report_.set_exitvoid( __LINE__ ); \
CYG_MACRO_END

#define CYG_REPORT_RETVAL( _value_) CYG_MACRO_START     \
    cyg_tracefunction_report_.set_exitvalue(            \
        __LINE__, (CYG_ADDRWORD)(_value_) );            \
CYG_MACRO_END


#else   // not __cplusplus


struct Cyg_TraceFunction_Report_
{
    int   cond;
    char *func;
    char *file; /* not strictly needed in plain 'C' */
    cyg_uint32 lnum; /* nor this */
#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE
    char *exitmsg;
    CYG_ADDRWORD exitvalue;
    int exitset;
#endif

};

#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_FUNCTION_ENTER_INTERNAL() CYG_MACRO_START            \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg( cyg_trace_enter,                                      \
                  cyg_tracefunction_report_.func,                       \
                  cyg_tracefunction_report_.file,                       \
                  cyg_tracefunction_report_.lnum,                       \
                  "enter" );                                            \
CYG_MACRO_END

#define CYG_REPORT_FUNCTION_CONSTRUCT( _c_, _fn_,_fl_,_l_,_xm_,_xv_,_xs_ ) \
        { _c_, _fn_, _fl_, _l_, _xm_, _xv_, _xs_ }

#else // do not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_FUNCTION_ENTER_INTERNAL() CYG_MACRO_START            \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracenomsg( cyg_tracefunction_report_.func,                     \
                    cyg_tracefunction_report_.file,                     \
                    cyg_tracefunction_report_.lnum );                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCTION_CONSTRUCT( _c_, _fn_,_fl_,_l_,_xm_,_xv_,_xs_ ) \
        { _c_, _fn_, _fl_, _l_ }

#endif // not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

// These have no CYG_MACRO_START,END around because it is required
// that the scope of the object be the whole function body.  Get it?

// These are the unconditional versions:
#define CYG_REPORT_FUNCTION()                                           \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        1, __PRETTY_FUNCTION__, __FILE__, __LINE__, NULL, 0, 0 );       \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCTYPE( _exitmsg_ )                                \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        1, __PRETTY_FUNCTION__, __FILE__, __LINE__, _exitmsg_, 0, 0 );  \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCNAME( _name_ )                                   \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        1, _name_, __FILE__, __LINE__, NULL, 0, 0 );                    \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCNAMETYPE( _name_, _exitmsg_  )                   \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        1, _name_, __FILE__, __LINE__, _exitmsg_, 0, 0 );               \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

// These are conditioned on macro CYG_REPORT_USER_BOOL
// (which you better have defined)
#define CYG_REPORT_FUNCTIONC()                                          \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        CYG_REPORT_USER_BOOL,                                           \
        __PRETTY_FUNCTION__, __FILE__, __LINE__, NULL, 0, 0 );          \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCTYPEC( _exitmsg_ )                               \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        CYG_REPORT_USER_BOOL,                                           \
        __PRETTY_FUNCTION__, __FILE__, __LINE__, _exitmsg_, 0, 0 );     \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCNAMEC( _name_ )                                  \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        CYG_REPORT_USER_BOOL,                                           \
        _name_, __FILE__, __LINE__, NULL, 0, 0 );                       \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#define CYG_REPORT_FUNCNAMETYPEC( _name_, _exitmsg_  )                  \
    struct Cyg_TraceFunction_Report_ cyg_tracefunction_report_ =        \
    CYG_REPORT_FUNCTION_CONSTRUCT(                                      \
        CYG_REPORT_USER_BOOL,                                           \
        _name_, __FILE__, __LINE__, _exitmsg_, 0, 0 );                  \
    CYG_REPORT_FUNCTION_ENTER_INTERNAL()

#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_RETURN() CYG_MACRO_START                             \
    CYG_ASSERT( NULL == cyg_tracefunction_report_.exitmsg,              \
                "exitvoid used in typed function" );                    \
    CYG_ASSERT( 0 == cyg_tracefunction_report_.exitset,                 \
                "exitvoid used when arg already set" );                 \
    cyg_tracefunction_report_.lnum = __LINE__;                          \
    cyg_tracefunction_report_.exitset = 2;                              \
    if ( cyg_tracefunction_report_.cond )                               \
      cyg_tracemsg( cyg_trace_return,                                   \
                    cyg_tracefunction_report_.func,                     \
                    cyg_tracefunction_report_.file,                     \
                    cyg_tracefunction_report_.lnum,                     \
                    "return void" );                                    \
CYG_MACRO_END

#define CYG_REPORT_RETVAL( _value_ ) CYG_MACRO_START                    \
    CYG_ASSERT( 0 == cyg_tracefunction_report_.exitset,                 \
                "exitvalue used when arg already set" );                \
    cyg_tracefunction_report_.lnum = __LINE__;                          \
    cyg_tracefunction_report_.exitvalue = (CYG_ADDRWORD)(_value_);      \
    cyg_tracefunction_report_.exitset = 1;                              \
    if ( cyg_tracefunction_report_.cond )                               \
      cyg_tracemsg2( cyg_trace_return,                                  \
                     cyg_tracefunction_report_.func,                    \
                     cyg_tracefunction_report_.file,                    \
                     cyg_tracefunction_report_.lnum,                    \
                     cyg_tracefunction_report_.exitmsg ?                \
                        cyg_tracefunction_report_.exitmsg :             \
                        "return %08x",                                  \
                     cyg_tracefunction_report_.exitvalue, 0 );          \
CYG_MACRO_END

#else // do not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_RETURN() CYG_MACRO_START                             \
    cyg_tracefunction_report_.lnum = __LINE__;                          \
    if ( cyg_tracefunction_report_.cond )                               \
      cyg_tracenomsg( cyg_tracefunction_report_.func,                   \
                      cyg_tracefunction_report_.file,                   \
                      cyg_tracefunction_report_.lnum );                 \
CYG_MACRO_END

#define CYG_REPORT_RETVAL( _value_ ) CYG_MACRO_START                    \
    CYG_REPORT_RETURN();                                                \
CYG_MACRO_END

#endif // not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#endif // not __cplusplus

#ifdef CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_FUNCARGVOID() CYG_MACRO_START                        \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg(  cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   "(void)"                                             \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG1( _format_, a ) CYG_MACRO_START              \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg2( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a      , 0                             \
                   );                                                   \
CYG_MACRO_END
    
#define CYG_REPORT_FUNCARG2( _format_, a,b ) CYG_MACRO_START            \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg2( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b                     \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG3( _format_, a,b,c ) CYG_MACRO_START          \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg4( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c      , 0                             \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG4( _format_, a,b,c,d ) CYG_MACRO_START        \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg4( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c, (CYG_ADDRWORD)d                     \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG5( _format_, a,b,c,d,e ) CYG_MACRO_START      \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg6( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c, (CYG_ADDRWORD)d,                    \
                   (CYG_ADDRWORD)e      , 0                             \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG6( _format_, a,b,c,d,e,f ) CYG_MACRO_START    \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg6( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c, (CYG_ADDRWORD)d,                    \
                   (CYG_ADDRWORD)e, (CYG_ADDRWORD)f                     \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG7( _format_, a,b,c,d,e,f,g ) CYG_MACRO_START  \
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg8( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c, (CYG_ADDRWORD)d,                    \
                   (CYG_ADDRWORD)e, (CYG_ADDRWORD)f,                    \
                   (CYG_ADDRWORD)g      , 0                             \
                   );                                                   \
CYG_MACRO_END

#define CYG_REPORT_FUNCARG8( _format_, a,b,c,d,e,f,g,h ) CYG_MACRO_START\
  if ( cyg_tracefunction_report_.cond )                                 \
    cyg_tracemsg8( cyg_trace_args,                                      \
                   cyg_tracefunction_report_.func,                      \
                   cyg_tracefunction_report_.file,                      \
                   cyg_tracefunction_report_.lnum,                      \
                   (_format_),                                          \
                   (CYG_ADDRWORD)a, (CYG_ADDRWORD)b,                    \
                   (CYG_ADDRWORD)c, (CYG_ADDRWORD)d,                    \
                   (CYG_ADDRWORD)e, (CYG_ADDRWORD)f,                    \
                   (CYG_ADDRWORD)g, (CYG_ADDRWORD)h                     \
                   );                                                   \
CYG_MACRO_END


#else // do not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#define CYG_REPORT_FUNCARGVOID() CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG1( _format_, a ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG2( _format_, a,b ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG3( _format_, a,b,c ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG4( _format_, a,b,c,d ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG5( _format_, a,b,c,d,e ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG6( _format_, a,b,c,d,e,f ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG7( _format_, a,b,c,d,e,f,g ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG8( _format_, a,b,c,d,e,f,g,h ) CYG_EMPTY_STATEMENT

#endif  // not CYGDBG_INFRA_DEBUG_TRACE_MESSAGE

#else   // no CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS

#define CYG_REPORT_FUNCTION()                           CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCTYPE( _exitmsg_ )                CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAME( _name_ )                   CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMETYPE( _name_, _exitmsg_  )   CYG_EMPTY_STATEMENT

#define CYG_REPORT_FUNCTIONC()                          CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCTYPEC( _exitmsg_ )               CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMEC( _name_ )                  CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMETYPEC( _name_, _exitmsg_  )  CYG_EMPTY_STATEMENT

#define CYG_REPORT_FUNCARGVOID() CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG1( _format_, a ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG2( _format_, a,b ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG3( _format_, a,b,c ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG4( _format_, a,b,c,d ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG5( _format_, a,b,c,d,e ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG6( _format_, a,b,c,d,e,f ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG7( _format_, a,b,c,d,e,f,g ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG8( _format_, a,b,c,d,e,f,g,h ) CYG_EMPTY_STATEMENT

#define CYG_REPORT_RETURN()                             CYG_EMPTY_STATEMENT
#define CYG_REPORT_RETVAL( _value_ )                    CYG_EMPTY_STATEMENT
    
#endif  // CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS
    
#else   // ! CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
// No traces: we define empty statements for trace macros.

#define CYG_TRACE0( _bool_, _msg_  ) CYG_EMPTY_STATEMENT
#define CYG_TRACE1( _bool_, _msg_, a ) CYG_EMPTY_STATEMENT
#define CYG_TRACE2( _bool_, _msg_, a,b ) CYG_EMPTY_STATEMENT
#define CYG_TRACE3( _bool_, _msg_, a,b,c ) CYG_EMPTY_STATEMENT
#define CYG_TRACE4( _bool_, _msg_, a,b,c,d ) CYG_EMPTY_STATEMENT
#define CYG_TRACE5( _bool_, _msg_, a,b,c,d,e ) CYG_EMPTY_STATEMENT
#define CYG_TRACE6( _bool_, _msg_, a,b,c,d,e,f ) CYG_EMPTY_STATEMENT
#define CYG_TRACE7( _bool_, _msg_, a,b,c,d,e,f,g ) CYG_EMPTY_STATEMENT
#define CYG_TRACE8( _bool_, _msg_, a,b,c,d,e,f,g,h ) CYG_EMPTY_STATEMENT

#define CYG_REPORT_FUNCTION()                           CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCTYPE( _exitmsg_ )                CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAME( _name_ )                   CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMETYPE( _name_, _exitmsg_  )   CYG_EMPTY_STATEMENT

#define CYG_REPORT_FUNCTIONC()                          CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCTYPEC( _exitmsg_ )               CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMEC( _name_ )                  CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCNAMETYPEC( _name_, _exitmsg_  )  CYG_EMPTY_STATEMENT

#define CYG_REPORT_FUNCARGVOID() CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG1( _format_, a ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG2( _format_, a,b ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG3( _format_, a,b,c ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG4( _format_, a,b,c,d ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG5( _format_, a,b,c,d,e ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG6( _format_, a,b,c,d,e,f ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG7( _format_, a,b,c,d,e,f,g ) CYG_EMPTY_STATEMENT
#define CYG_REPORT_FUNCARG8( _format_, a,b,c,d,e,f,g,h ) CYG_EMPTY_STATEMENT

#define CYG_REPORT_RETURN()                             CYG_EMPTY_STATEMENT
#define CYG_REPORT_RETVAL( _value_ )                    CYG_EMPTY_STATEMENT

#define CYG_TRACE_PRINT() CYG_EMPTY_STATEMENT
#define CYG_TRACE_DUMP()  CYG_EMPTY_STATEMENT

#endif // ! CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
//
// CYG_TRACEn{[XDY]{V}}{B}
//
// Convenience macros: these fall into a few dimensions, with suffix letters:
// First option:
//     X: user need not supply a format string, %08x is used
//     D: ditto but signed decimal, %d
//     Y: ditto but just plain %x
// Second option, only meaningful with one of XDY:
//     V: "<var> = %..." is used, by stringifying the argument
// Third option:
//     B: user need not supply a bool; the symbol CYG_TRACE_USER_BOOL is
//        used (which we do not define, user must do this)

#define CYG_TRACE0B( _msg_  ) \
        CYG_TRACE0( CYG_TRACE_USER_BOOL, _msg_  ) 
#define CYG_TRACE1B( _msg_, a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, _msg_, a ) 
#define CYG_TRACE2B( _msg_, a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, _msg_, a,b ) 
#define CYG_TRACE3B( _msg_, a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, _msg_, a,b,c ) 
#define CYG_TRACE4B( _msg_, a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, _msg_, a,b,c,d ) 
#define CYG_TRACE5B( _msg_, a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, _msg_, a,b,c,d,e ) 
#define CYG_TRACE6B( _msg_, a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, _msg_, a,b,c,d,e,f ) 
#define CYG_TRACE7B( _msg_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, _msg_, a,b,c,d,e,f,g ) 
#define CYG_TRACE8B( _msg_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, _msg_, a,b,c,d,e,f,g,h ) 

// long hex versions

#define CYG_TRACE1X( _bool_, a ) \
        CYG_TRACE1( _bool_, "%08x", a ) 
#define CYG_TRACE2X( _bool_, a,b ) \
        CYG_TRACE2( _bool_, "%08x %08x", a,b ) 
#define CYG_TRACE3X( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, "%08x %08x %08x", a,b,c ) 
#define CYG_TRACE4X( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, "%08x %08x %08x %08x", a,b,c,d ) 
#define CYG_TRACE5X( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, "%08x %08x %08x %08x %08x", a,b,c,d,e ) 
#define CYG_TRACE6X( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, "%08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7X( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, "%08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8X( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, "%08x %08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1XV( _bool_, a ) \
        CYG_TRACE1( _bool_, # a "=%08x ", a ) 
#define CYG_TRACE2XV( _bool_, a,b ) \
        CYG_TRACE2( _bool_, \
                    # a "=%08x " # b "=%08x " , a,b )
#define CYG_TRACE3XV( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " , a,b,c )
#define CYG_TRACE4XV( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    , a,b,c,d )
#define CYG_TRACE5XV( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " \
                    , a,b,c,d,e )
#define CYG_TRACE6XV( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7XV( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8XV( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " # h "=%08x " \
                    , a,b,c,d,e,f,g,h )

#define CYG_TRACE1XB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, "%08x", a ) 
#define CYG_TRACE2XB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, "%08x %08x", a,b ) 
#define CYG_TRACE3XB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, "%08x %08x %08x", a,b,c ) 
#define CYG_TRACE4XB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, "%08x %08x %08x %08x", a,b,c,d ) 
#define CYG_TRACE5XB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, "%08x %08x %08x %08x %08x", a,b,c,d,e ) 
#define CYG_TRACE6XB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, "%08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7XB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, "%08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8XB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, "%08x %08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1XVB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, # a "=%08x ", a ) 
#define CYG_TRACE2XVB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " , a,b )
#define CYG_TRACE3XVB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " , a,b,c )
#define CYG_TRACE4XVB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    , a,b,c,d )
#define CYG_TRACE5XVB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " \
                    , a,b,c,d,e )
#define CYG_TRACE6XVB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7XVB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8XVB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " # h "=%08x " \
                    , a,b,c,d,e,f,g,h )

// decimal versions

#define CYG_TRACE1D( _bool_, a ) \
        CYG_TRACE1( _bool_, "%d", a ) 
#define CYG_TRACE2D( _bool_, a,b ) \
        CYG_TRACE2( _bool_, "%d %d", a,b ) 
#define CYG_TRACE3D( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, "%d %d %d", a,b,c ) 
#define CYG_TRACE4D( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, "%d %d %d %d", a,b,c,d ) 
#define CYG_TRACE5D( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, "%d %d %d %d %d", a,b,c,d,e ) 
#define CYG_TRACE6D( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, "%d %d %d %d %d %d", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7D( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, "%d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8D( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, "%d %d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1DV( _bool_, a ) \
        CYG_TRACE1( _bool_, # a "=%d ", a ) 
#define CYG_TRACE2DV( _bool_, a,b ) \
        CYG_TRACE2( _bool_, \
                    # a "=%d " # b "=%d " , a,b )
#define CYG_TRACE3DV( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " , a,b,c )
#define CYG_TRACE4DV( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    , a,b,c,d )
#define CYG_TRACE5DV( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " \
                    , a,b,c,d,e )
#define CYG_TRACE6DV( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7DV( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8DV( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " # h "=%d " \
                    , a,b,c,d,e,f,g,h )

#define CYG_TRACE1DB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, "%d", a ) 
#define CYG_TRACE2DB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, "%d %d", a,b ) 
#define CYG_TRACE3DB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, "%d %d %d", a,b,c ) 
#define CYG_TRACE4DB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, "%d %d %d %d", a,b,c,d ) 
#define CYG_TRACE5DB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, "%d %d %d %d %d", a,b,c,d,e ) 
#define CYG_TRACE6DB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, "%d %d %d %d %d %d", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7DB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, "%d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8DB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, "%d %d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1DVB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, # a "=%d ", a ) 
#define CYG_TRACE2DVB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " , a,b )
#define CYG_TRACE3DVB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " , a,b,c )
#define CYG_TRACE4DVB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    , a,b,c,d )
#define CYG_TRACE5DVB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " \
                    , a,b,c,d,e )
#define CYG_TRACE6DVB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7DVB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8DVB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " # h "=%d " \
                    , a,b,c,d,e,f,g,h )

// short hex versions

#define CYG_TRACE1Y( _bool_, a ) \
        CYG_TRACE1( _bool_, "%x", a ) 
#define CYG_TRACE2Y( _bool_, a,b ) \
        CYG_TRACE2( _bool_, "%x %x", a,b ) 
#define CYG_TRACE3Y( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, "%x %x %x", a,b,c ) 
#define CYG_TRACE4Y( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, "%x %x %x %x", a,b,c,d ) 
#define CYG_TRACE5Y( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, "%x %x %x %x %x", a,b,c,d,e ) 
#define CYG_TRACE6Y( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, "%x %x %x %x %x %x", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7Y( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, "%x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8Y( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, "%x %x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1YV( _bool_, a ) \
        CYG_TRACE1( _bool_, # a "=%x ", a ) 
#define CYG_TRACE2YV( _bool_, a,b ) \
        CYG_TRACE2( _bool_, \
                    # a "=%x " # b "=%x " , a,b )
#define CYG_TRACE3YV( _bool_, a,b,c ) \
        CYG_TRACE3( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " , a,b,c )
#define CYG_TRACE4YV( _bool_, a,b,c,d ) \
        CYG_TRACE4( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    , a,b,c,d )
#define CYG_TRACE5YV( _bool_, a,b,c,d,e ) \
        CYG_TRACE5( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " \
                    , a,b,c,d,e )
#define CYG_TRACE6YV( _bool_, a,b,c,d,e,f ) \
        CYG_TRACE6( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7YV( _bool_, a,b,c,d,e,f,g ) \
        CYG_TRACE7( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8YV( _bool_, a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( _bool_, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " # h "=%x " \
                    , a,b,c,d,e,f,g,h )

#define CYG_TRACE1YB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, "%x", a ) 
#define CYG_TRACE2YB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, "%x %x", a,b ) 
#define CYG_TRACE3YB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, "%x %x %x", a,b,c ) 
#define CYG_TRACE4YB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, "%x %x %x %x", a,b,c,d ) 
#define CYG_TRACE5YB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, "%x %x %x %x %x", a,b,c,d,e ) 
#define CYG_TRACE6YB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, "%x %x %x %x %x %x", \
                    a,b,c,d,e,f ) 
#define CYG_TRACE7YB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, "%x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g ) 
#define CYG_TRACE8YB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, "%x %x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g,h )

#define CYG_TRACE1YVB( a ) \
        CYG_TRACE1( CYG_TRACE_USER_BOOL, # a "=%x ", a ) 
#define CYG_TRACE2YVB( a,b ) \
        CYG_TRACE2( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " , a,b )
#define CYG_TRACE3YVB( a,b,c ) \
        CYG_TRACE3( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " , a,b,c )
#define CYG_TRACE4YVB( a,b,c,d ) \
        CYG_TRACE4( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    , a,b,c,d )
#define CYG_TRACE5YVB( a,b,c,d,e ) \
        CYG_TRACE5( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " \
                    , a,b,c,d,e )
#define CYG_TRACE6YVB( a,b,c,d,e,f ) \
        CYG_TRACE6( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " \
                    , a,b,c,d,e,f ) 
#define CYG_TRACE7YVB( a,b,c,d,e,f,g ) \
        CYG_TRACE7( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_TRACE8YVB( a,b,c,d,e,f,g,h ) \
        CYG_TRACE8( CYG_TRACE_USER_BOOL, \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " # h "=%x " \
                    , a,b,c,d,e,f,g,h )

// -------------------------------------------------------------------------
//
// CYG_REPORT_FUNCARGn{[XDY]{V}}
//
// Convenience macros two: these fall into a few dimensions, with suffix letters:
// First option:
//     X: user need not supply a format string, %08x is used
//     D: ditto but signed decimal, %d
//     Y: ditto but just plain %x
// Second option, only meaningful with one of XDY:
//     V: "<var> = %..." is used, by stringifying the argument

// long hex versions

#define CYG_REPORT_FUNCARG1X( a ) \
        CYG_REPORT_FUNCARG1( "%08x", a ) 
#define CYG_REPORT_FUNCARG2X( a,b ) \
        CYG_REPORT_FUNCARG2( "%08x %08x", a,b ) 
#define CYG_REPORT_FUNCARG3X( a,b,c ) \
        CYG_REPORT_FUNCARG3( "%08x %08x %08x", a,b,c ) 
#define CYG_REPORT_FUNCARG4X( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( "%08x %08x %08x %08x", a,b,c,d ) 
#define CYG_REPORT_FUNCARG5X( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( "%08x %08x %08x %08x %08x", a,b,c,d,e ) 
#define CYG_REPORT_FUNCARG6X( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( "%08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7X( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( "%08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8X( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( "%08x %08x %08x %08x %08x %08x %08x %08x", \
                    a,b,c,d,e,f,g,h )

#define CYG_REPORT_FUNCARG1XV( a ) \
        CYG_REPORT_FUNCARG1( # a "=%08x ", a ) 
#define CYG_REPORT_FUNCARG2XV( a,b ) \
        CYG_REPORT_FUNCARG2( \
                    # a "=%08x " # b "=%08x " , a,b )
#define CYG_REPORT_FUNCARG3XV( a,b,c ) \
        CYG_REPORT_FUNCARG3( \
                    # a "=%08x " # b "=%08x " # c "=%08x " , a,b,c )
#define CYG_REPORT_FUNCARG4XV( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    , a,b,c,d )
#define CYG_REPORT_FUNCARG5XV( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " \
                    , a,b,c,d,e )
#define CYG_REPORT_FUNCARG6XV( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " \
                    , a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7XV( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8XV( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( \
                    # a "=%08x " # b "=%08x " # c "=%08x " # d "=%08x " \
                    # e "=%08x " # f "=%08x " # g "=%08x " # h "=%08x " \
                    , a,b,c,d,e,f,g,h )

// decimal versions


#define CYG_REPORT_FUNCARG1D( a ) \
        CYG_REPORT_FUNCARG1( "%d", a ) 
#define CYG_REPORT_FUNCARG2D( a,b ) \
        CYG_REPORT_FUNCARG2( "%d %d", a,b ) 
#define CYG_REPORT_FUNCARG3D( a,b,c ) \
        CYG_REPORT_FUNCARG3( "%d %d %d", a,b,c ) 
#define CYG_REPORT_FUNCARG4D( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( "%d %d %d %d", a,b,c,d ) 
#define CYG_REPORT_FUNCARG5D( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( "%d %d %d %d %d", a,b,c,d,e ) 
#define CYG_REPORT_FUNCARG6D( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( "%d %d %d %d %d %d", \
                    a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7D( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( "%d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8D( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( "%d %d %d %d %d %d %d %d", \
                    a,b,c,d,e,f,g,h )

#define CYG_REPORT_FUNCARG1DV( a ) \
        CYG_REPORT_FUNCARG1( # a "=%d ", a ) 
#define CYG_REPORT_FUNCARG2DV( a,b ) \
        CYG_REPORT_FUNCARG2( \
                    # a "=%d " # b "=%d " , a,b )
#define CYG_REPORT_FUNCARG3DV( a,b,c ) \
        CYG_REPORT_FUNCARG3( \
                    # a "=%d " # b "=%d " # c "=%d " , a,b,c )
#define CYG_REPORT_FUNCARG4DV( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    , a,b,c,d )
#define CYG_REPORT_FUNCARG5DV( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " \
                    , a,b,c,d,e )
#define CYG_REPORT_FUNCARG6DV( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " \
                    , a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7DV( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " \
                    , a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8DV( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( \
                    # a "=%d " # b "=%d " # c "=%d " # d "=%d " \
                    # e "=%d " # f "=%d " # g "=%d " # h "=%d " \
                    , a,b,c,d,e,f,g,h )

// short hex versions

#define CYG_REPORT_FUNCARG1Y( a ) \
        CYG_REPORT_FUNCARG1( "%x", a ) 
#define CYG_REPORT_FUNCARG2Y( a,b ) \
        CYG_REPORT_FUNCARG2( "%x %x", a,b ) 
#define CYG_REPORT_FUNCARG3Y( a,b,c ) \
        CYG_REPORT_FUNCARG3( "%x %x %x", a,b,c ) 
#define CYG_REPORT_FUNCARG4Y( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( "%x %x %x %x", a,b,c,d ) 
#define CYG_REPORT_FUNCARG5Y( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( "%x %x %x %x %x", a,b,c,d,e ) 
#define CYG_REPORT_FUNCARG6Y( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( "%x %x %x %x %x %x", \
                    a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7Y( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( "%x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8Y( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( "%x %x %x %x %x %x %x %x", \
                    a,b,c,d,e,f,g,h )

#define CYG_REPORT_FUNCARG1YV( a ) \
        CYG_REPORT_FUNCARG1( # a "=%x ", a ) 
#define CYG_REPORT_FUNCARG2YV( a,b ) \
        CYG_REPORT_FUNCARG2( \
                    # a "=%x " # b "=%x " , a,b )
#define CYG_REPORT_FUNCARG3YV( a,b,c ) \
        CYG_REPORT_FUNCARG3( \
                    # a "=%x " # b "=%x " # c "=%x " , a,b,c )
#define CYG_REPORT_FUNCARG4YV( a,b,c,d ) \
        CYG_REPORT_FUNCARG4( \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    , a,b,c,d )
#define CYG_REPORT_FUNCARG5YV( a,b,c,d,e ) \
        CYG_REPORT_FUNCARG5( \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " \
                    , a,b,c,d,e )
#define CYG_REPORT_FUNCARG6YV( a,b,c,d,e,f ) \
        CYG_REPORT_FUNCARG6( \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " \
                    , a,b,c,d,e,f ) 
#define CYG_REPORT_FUNCARG7YV( a,b,c,d,e,f,g ) \
        CYG_REPORT_FUNCARG7( \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " \
                    , a,b,c,d,e,f,g ) 
#define CYG_REPORT_FUNCARG8YV( a,b,c,d,e,f,g,h ) \
        CYG_REPORT_FUNCARG8( \
                    # a "=%x " # b "=%x " # c "=%x " # d "=%x " \
                    # e "=%x " # f "=%x " # g "=%x " # h "=%x " \
                    , a,b,c,d,e,f,g,h )


#endif // CYGONCE_INFRA_CYG_TRAC_H multiple inclusion protection
// EOF cyg_trac.h
