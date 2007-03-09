//===========================================================================
//
//      strncpy.cxx
//
//      ISO C standard strncpy() routine
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
// Contributors:  
// Date:          2000-04-14
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

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

// EXPORTED SYMBOLS

externC char *
strncpy( char *s1, const char *s2, size_t n ) \
    CYGBLD_ATTRIB_WEAK_ALIAS(__strncpy);

// FUNCTIONS

char *
__strncpy( char *s1, const char *s2, size_t n)
{
    CYG_REPORT_FUNCNAMETYPE( "__strncpy", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    if (n)
    {
        CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );
    }

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    char *dscan;
    const char *sscan;
    
    dscan = s1;
    sscan = s2;
    while (n > 0)
    {
        --n;
        if ((*dscan++ = *sscan++) == '\0')
            break;
    }
    while (n-- > 0)
        *dscan++ = '\0';

    CYG_REPORT_RETVAL( s1 );
    
    return s1;
#else
    char *dst = s1;
    const char *src = s2;
    CYG_WORD *aligned_dst;
    const CYG_WORD *aligned_src;
    
    // If SRC and DEST is aligned and count large enough, then copy words.
    if (!CYG_LIBC_STR_UNALIGNED2 (src, dst) &&
        !CYG_LIBC_STR_OPT_TOO_SMALL (n)) {

        aligned_dst = (CYG_WORD *)dst;
        aligned_src = (CYG_WORD *)src;

        // SRC and DEST are both "CYG_WORD" aligned, try to do "CYG_WORD"
        // sized copies.
        while (n >= sizeof (CYG_WORD) && 
               !CYG_LIBC_STR_DETECTNULL(*aligned_src)) {

            n -= sizeof (CYG_WORD);
            *aligned_dst++ = *aligned_src++;
        }

        dst = (char *)aligned_dst;
        src = (const char *)aligned_src;
    } // if
    
    while (n > 0)
    {
        --n;
        if ((*dst++ = *src++) == '\0')
            break;
    }
    
    while (n-- > 0)
        *dst++ = '\0';
    
    CYG_REPORT_RETVAL( s1 );

    return s1;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __strncpy()

// EOF strncpy.cxx
