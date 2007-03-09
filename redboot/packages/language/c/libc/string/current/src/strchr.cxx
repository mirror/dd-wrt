//===========================================================================
//
//      strchr.cxx
//
//      ANSI standard strchr() routine
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
// Author(s):    jlarmour
// Contributors:  
// Date:         2000-04-14
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
strchr( const char *s, int c ) CYGBLD_ATTRIB_WEAK_ALIAS(__strchr);

// MACROS

// DETECTCHAR returns nonzero if X contains the byte used 
// to fill MASK.
#define DETECTCHAR(X,MASK) (CYG_LIBC_STR_DETECTNULL( (X) ^ (MASK) ))

// FUNCTIONS

char *
__strchr( const char *s, int c )
{
    CYG_REPORT_FUNCNAMETYPE( "__strchr", "returning %08x" );
    CYG_REPORT_FUNCARG2( "s=%08x, c=%d", s, c );

    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    char charc = c;
    
    while (*s && *s != charc)
    {
        s++;
    }
    
    if (*s != charc)
    {
        s = NULL;
    }
    
    CYG_REPORT_RETVAL( s );

    return (char *) s;
#else
    char charc = c;
    CYG_WORD mask;
    CYG_WORD j;
    CYG_WORD *aligned_addr;
    
    if (CYG_LIBC_STR_UNALIGNED (s))
    {
        while (*s && *s != charc)
            s++;
        if (*s != charc)
            s = NULL;

        CYG_REPORT_RETVAL( s );

        return (char *) s;
    }
    
    for (j = 0, mask = 0; j < sizeof(mask); j++)
        mask = (mask << 8) + charc;
    
    aligned_addr = (CYG_WORD *)s;
    while (!CYG_LIBC_STR_DETECTNULL (*aligned_addr) &&
           !DETECTCHAR (*aligned_addr, mask))
        aligned_addr++;
    
    // The block of bytes currently pointed to by aligned_addr
    // contains either a null or the target char, or both.  We
    // catch it using the bytewise search.
    
    s = (char *)aligned_addr;
    while (*s && *s != charc) 
        s++;
    if (*s != charc)
        s = NULL;

    CYG_REPORT_RETVAL( s );

    return (char *)s;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __strchr()

// EOF strchr.cxx
