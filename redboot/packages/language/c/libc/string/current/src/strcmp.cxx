//===========================================================================
//
//      strcmp.cxx
//
//      ANSI standard strcmp() routine
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

externC int
strcmp( const char *s1, const char *s2 ) CYGBLD_ATTRIB_WEAK_ALIAS(__strcmp);

// FUNCTIONS

int
__strcmp( const char *s1, const char *s2 )
{ 
    CYG_REPORT_FUNCNAMETYPE( "__strcmp", "returning %d" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    int retval;

    CYG_UNUSED_PARAM( int, retval ); // in case tracing is off

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    while (*s1 != '\0' && *s1 == *s2)
    {
        s1++;
        s2++;
    }
    
    retval = (*(unsigned char *) s1) - (*(unsigned char *) s2);

    CYG_REPORT_RETVAL( retval );

    return retval;
#else
    const CYG_WORD *aligned_s1;
    const CYG_WORD *aligned_s2;
    
    // If s1 or s2 are unaligned, then compare bytes.
    if (CYG_LIBC_STR_UNALIGNED2 (s1, s2))
    {  
        while (*s1 != '\0' && *s1 == *s2)
        {
            s1++;
            s2++;
        }
        retval = (*(unsigned char *) s1) - (*(unsigned char *) s2);
        
        CYG_REPORT_RETVAL( retval );
        
        return retval;
    } 
    
    // If s1 and s2 are word-aligned, compare them a word at a time.
    aligned_s1 = (const CYG_WORD *)s1;
    aligned_s2 = (const CYG_WORD *)s2;
    while (*aligned_s1 == *aligned_s2)
    {
        // To get here, *aligned_s1 == *aligned_s2, thus if we find a
        // null in *aligned_s1, then the strings must be equal, so return
        // zero.
        if (CYG_LIBC_STR_DETECTNULL (*aligned_s1))
        {
            CYG_REPORT_RETVAL( 0 );
            return 0;
        } // if
        
        aligned_s1++;
        aligned_s2++;
    }
    
    // A difference was detected in last few bytes of s1, so search bytewise
    s1 = (const char *)aligned_s1;
    s2 = (const char *)aligned_s2;
    
    while (*s1 != '\0' && *s1 == *s2)
    {
        s1++;
        s2++;
    }

    retval = (*(unsigned char *) s1) - (*(unsigned char *) s2);
    
    CYG_REPORT_RETVAL( retval );
        
    return retval;

#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __strcmp()

// EOF strcmp.cxx
