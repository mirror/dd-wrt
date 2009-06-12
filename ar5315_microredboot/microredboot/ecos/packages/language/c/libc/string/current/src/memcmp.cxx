//===========================================================================
//
//      memcmp.cxx
//
//      ANSI standard memcmp() routine
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
#include <stddef.h>           // Compiler definitions such as size_t, NULL etc.
#include <cyg/libc/string/stringsupp.hxx> // Useful string function support and
                                          // prototypes


// EXPORTED SYMBOLS

externC int
memcmp( const void *s1, const void *s2, size_t n ) \
    CYGBLD_ATTRIB_WEAK_ALIAS(__memcmp);

// FUNCTIONS

int
__memcmp( const void *s1, const void *s2, size_t n )
{
    int retval;

    CYG_REPORT_FUNCNAMETYPE( "__memcmp", "returning %d" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    if (n)
    {
        CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );
    }

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    const unsigned char *m1 = (const unsigned char *) s1;
    const unsigned char *m2 = (const unsigned char *) s2;
    
    while (n--)
    {
        if (*m1 != *m2)
        {
            retval = *m1 - *m2;
            CYG_REPORT_RETVAL( retval );
            return retval;
        }
        m1++;
        m2++;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;
#else  
    const unsigned char *m1 = (const unsigned char *) s1;
    const unsigned char *m2 = (const unsigned char *) s2;
    const CYG_WORD *aligned_m1;
    const CYG_WORD *aligned_m2;
    
    // If the size is too small, or either pointer is unaligned,
    // then we punt to the byte compare loop.  Hopefully this will
    // not turn up in inner loops.
    if (CYG_LIBC_STR_OPT_TOO_SMALL(n) || CYG_LIBC_STR_UNALIGNED2(m1,m2))
    {
        while (n--)
        {
            if (*m1 != *m2)
            {
                retval = *m1 - *m2;
                CYG_REPORT_RETVAL( retval );
                return retval;
            }
            m1++;
            m2++;
        }
        CYG_REPORT_RETVAL( 0 );
        return 0;
    }
    
    // Otherwise, load and compare the blocks of memory one 
    // word at a time.
    aligned_m1 = (const CYG_WORD *) m1;
    aligned_m2 = (const CYG_WORD *) m2;
    while (n > CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE)
    {
        if (*aligned_m1 != *aligned_m2) 
        {
            // This block of characters has a mismatch, somewhere.
            // xoring them together and then testing for null would
            // be fastest, but this is clearer code.
            m1 = (const unsigned char *)aligned_m1;
            m2 = (const unsigned char *)aligned_m2;
            while (n--)
            {
                if (*m1 != *m2)
                {
                    retval = *m1 - *m2;
                    CYG_REPORT_RETVAL( retval );
                    return retval;
                }
                m1++;
                m2++;
            }
            // NOT REACHED
        }
        aligned_m1++;
        aligned_m2++;
        n -= CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE;
    }
    
    // checking the last few characters
    
    m1 = (const unsigned char *)aligned_m1;
    m2 = (const unsigned char *)aligned_m2;
    
    while (n--)
    {
        if (*m1 != *m2)
        {
            retval = *m1 - *m2;
            CYG_REPORT_RETVAL( retval );
            return retval;
        }
        m1++;
        m2++;
    }
    
    CYG_REPORT_RETVAL( 0 );
    return 0;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __memcmp()

// EOF memcmp.cxx
