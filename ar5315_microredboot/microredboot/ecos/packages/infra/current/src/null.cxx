//==========================================================================
//
//      null.cxx
//
//      Null trace and assert functions
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-15
// Purpose:     Trace and assert functions
// Description: The functions in this file are null implementations of the
//              standard trace and assert functions. These can be used to
//              eliminate all trace and assert messages without recompiling.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/infra.h>

#ifndef CYGDBG_USE_ASSERTS
// then you get an empty function regardless, so that other code can link:
// (for example libc assert() calls this whether or not eCos code has
//  asserts enabled - of course if it does, assert() maps to the chosen
//  implementation; if not, it's the same inner function to breakpoint.)

#include <cyg/infra/cyg_type.h>        // base types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

externC void
cyg_assert_fail( const char *psz_func, const char *psz_file,
                 cyg_uint32 linenum, const char *psz_msg ) __THROW
{
    for(;;);
};
#endif // not CYGDBG_USE_ASSERTS

#ifdef CYGDBG_INFRA_DEBUG_TRACE_ASSERT_NULL

#include <cyg/infra/cyg_type.h>        // base types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

// -------------------------------------------------------------------------
// Trace functions:

#ifdef CYGDBG_USE_TRACING

externC void
cyg_tracenomsg( const char *psz_func, const char *psz_file, cyg_uint32 linenum )
{}

// provide every other one of these as a space/caller bloat compromise.

externC void
cyg_tracemsg( cyg_uint32 what, 
              const char *psz_func, const char *psz_file, cyg_uint32 linenum,
              const char *psz_msg )
{}

externC void
cyg_tracemsg2( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1 )
{}
externC void
cyg_tracemsg4( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3 )
{}
externC void
cyg_tracemsg6( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5 )
{}
externC void
cyg_tracemsg8( cyg_uint32 what, 
               const char *psz_func, const char *psz_file, cyg_uint32 linenum,
               const char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5,
               CYG_ADDRWORD arg6,  CYG_ADDRWORD arg7 )
{}

// -------------------------------------------------------------------------

externC void
cyg_trace_dump(void)
{}

#endif // CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
// Assert functions:

#ifdef CYGDBG_USE_ASSERTS

externC void
cyg_assert_fail( const char *psz_func, const char *psz_file,
                 cyg_uint32 linenum, const char *psz_msg ) __THROW
{
    for(;;);
};

extern "C"
{
extern unsigned long _stext;
extern unsigned long _etext;

unsigned long stext_addr = (unsigned long)&_stext;
unsigned long etext_addr = (unsigned long)&_etext;
};

externC cyg_bool cyg_check_data_ptr(const void *ptr)
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p > stext_addr && p < etext_addr ) return false;

    return true;
}

externC cyg_bool cyg_check_func_ptr(const void (*ptr)(void))
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p < stext_addr && p > etext_addr ) return false;

    return true;
}

#endif // CYGDBG_USE_ASSERTS

#endif // CYGDBG_INFRA_DEBUG_TRACE_ASSERT_NULL

// -------------------------------------------------------------------------
// EOF null.cxx
