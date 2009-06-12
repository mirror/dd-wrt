//===========================================================================
//
//      strtok.cxx
//
//      ISO standard strtok() routine 
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          2000-04-14
// Purpose:       Provide ISO C strtok() and POSIX strtok_r() routines
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code is based on original code with the following copyright:
//
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


// CONFIGURATION

#include <pkgconf/libc_string.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <string.h>                // Header for this file
#include <stddef.h>         // Compiler definitions such as size_t, NULL etc.
#include <cyg/libc/string/stringsupp.hxx> // Useful string function support and
                                          // prototypes

#ifdef CYGSEM_LIBC_STRING_PER_THREAD_STRTOK
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_STRING_STRTOK_TRACE_LEVEL)
static int strtok_trace = CYGNUM_LIBC_STRING_STRTOK_TRACE_LEVEL;
# define TL1 (0 < strtok_trace)
#else
# define TL1 (0)
#endif

// STATICS

#ifdef CYGSEM_LIBC_STRING_PER_THREAD_STRTOK
static Cyg_Thread::cyg_data_index
strtok_data_index=CYGNUM_KERNEL_THREADS_DATA_MAX;

static Cyg_Mutex strtok_data_mutex CYG_INIT_PRIORITY(LIBC);
#else
static char *cyg_libc_strtok_last;
#endif

// FUNCTIONS

char *
strtok( char *s1, const char *s2 )
{
    char **lasts;
    char *retval;

    CYG_REPORT_FUNCNAMETYPE( "strtok", "returning %08x" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );
    
    if (s1 != NULL)
        CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

#ifdef CYGSEM_LIBC_STRING_PER_THREAD_STRTOK
    Cyg_Thread *self = Cyg_Thread::self();

    // Get a per-thread data slot if we haven't got one already
    // Do a simple test before locking and retrying test, as this is a
    // rare situation
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==strtok_data_index) {
        strtok_data_mutex.lock();
        if (CYGNUM_KERNEL_THREADS_DATA_MAX==strtok_data_index) {

            // FIXME: Should use real CDL to pre-allocate a slot at compile
            // time to ensure there are enough slots
            strtok_data_index = self->new_data_index();
            CYG_ASSERT(strtok_data_index >= 0,
                       "failed to allocate data index" );
        }
        strtok_data_mutex.unlock();
    } // if

    // we have a valid index now

    lasts = (char **)self->get_data_ptr(strtok_data_index);
#else
    lasts = &cyg_libc_strtok_last;
#endif

    CYG_TRACE2( TL1, "Retrieved strtok_last address %08x containing %s",
                lasts, *lasts );

    retval = strtok_r( s1, s2, lasts );
    
    CYG_REPORT_RETVAL( retval );

    return retval;
} // strtok()

char *
strtok_r( char *s1, const char *s2, char **lasts )
{
    char *spanp;
    int c, sc;
    char *tok;
    
    CYG_REPORT_FUNCNAMETYPE( "strtok_r", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, lasts=%08x", s1, s2, lasts );
    
    if (s1 != NULL)
        CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( lasts, "lasts is not a valid pointer!" );


    if (s1 == NULL && (s1 = *lasts) == NULL)
    {
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
    
    //
    // Skip (span) leading delimiters (s += strspn(s, delim), sort of).
    //
cont:
    c = *s1++;
    for (spanp = (char *)s2; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    } // for
    
    if (c == 0) {               // no non-delimiter characters
        *lasts = NULL;

        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
    tok = s1 - 1;
    
    //
    // Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
    // Note that delim must have one NUL; we stop if we see that, too.
    //
    for (;;) {
        c = *s1++;
        spanp = (char *)s2;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s1 = NULL;
                else
                    s1[-1] = 0;
                *lasts = s1;

                CYG_REPORT_RETVAL( tok );

                return (tok);
            } // if
        } while (sc != 0);
    } // for
    // NOTREACHED
} // strtok_r()

// EOF strtok.cxx
