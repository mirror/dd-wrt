//===========================================================================
//
//      memchr.cxx
//
//      ANSI standard memchr() routine
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
#include <stddef.h>          // Compiler definitions such as size_t, NULL etc.
#include <cyg/libc/string/stringsupp.hxx> // Useful string function support and
                                          // prototypes

// EXPORTED SYMBOLS

externC void *
memchr( const void *s, int c, size_t n )  CYGBLD_ATTRIB_WEAK_ALIAS(__memchr);

// FUNCTIONS

void *
__memchr( const void *s, int c, size_t n )
{
    CYG_REPORT_FUNCNAMETYPE( "__memchr", "returning addr %08x" );
    CYG_REPORT_FUNCARG3( "s=%08x, c=%d, n=%d", s, c, n );

    if (n)
    {
        CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
    }

#if defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) || defined(__OPTIMIZE_SIZE__)
    const unsigned char *src = (const unsigned char *) s;
    
    c &= 0xff;

    while (n--)
    {
        if (*src == c)
        {
            CYG_REPORT_RETVAL( src );
            return (void *) src;
        } // if
        src++;
    }
    CYG_REPORT_RETVAL( NULL );
    return NULL;
#else
    const unsigned char *src = (const unsigned char *) s;
    CYG_WORD *aligned_src;
    CYG_WORD buffer;
    CYG_WORD mask;
    cyg_ucount8 i;

    c &= 0xff;
    
    // If the size is small, or src is unaligned, then 
    // use the bytewise loop.  We can hope this is rare.
    if (CYG_LIBC_STR_OPT_TOO_SMALL (n) || CYG_LIBC_STR_UNALIGNED (src)) 
    {
        while (n--)
        {
            if (*src == c)
            {
                CYG_REPORT_RETVAL( src );
                return (void *) src;
            } // if
            src++;
        }
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    }
    
    // The fast code reads the ASCII one word at a time and only 
    // performs the bytewise search on word-sized segments if they
    // contain the search character, which is detected by XORing 
    // the word-sized segment with a word-sized block of the search
    // character and then detecting for the presence of NULL in the
    // result.

    aligned_src = (CYG_WORD *) src;
    mask = 0;
    for (i = 0; i < CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE; i++)
        mask = (mask << 8) + c;
    
    while (n > CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE)
    {
        buffer = *aligned_src;
        buffer ^=  mask;
        if (CYG_LIBC_STR_DETECTNULL (buffer))
        {
            src = (unsigned char*) aligned_src;
            for ( i = 0; i < CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE; i++ )
            {
                if (*src == c)
                {
                    CYG_REPORT_RETVAL( src );
                    return (void *) src;
                } // if
                src++;
            }
        }
        n -= CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE;
        aligned_src++;
    }
    
    // If there are fewer than CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE characters
    // left, then we resort to the bytewise loop.
    
    src = (const unsigned char *) aligned_src;
    while (n--)
    {
        if (*src == c)
        {
            CYG_REPORT_RETVAL( src );
            return (void *) src;
        } // if
        src++;
    } 

    CYG_REPORT_RETVAL( NULL );
    return NULL;
#endif // not defined(CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST) ||
       //     defined(__OPTIMIZE_SIZE__)
} // __memchr()

// EOF memchr.cxx
