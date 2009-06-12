//===========================================================================
//
//      memmove.cxx
//
//      ANSI standard memmove() routine
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

externC void *
memmove( void *s1, const void *s2, size_t n ) \
    CYGBLD_ATTRIB_WEAK_ALIAS(__memmove);

// FUNCTIONS

void *
__memmove( void *s1, const void *s2, size_t n )
{
    char *dst = (char *)s1;
    const char *src = (const char *)s2;
    
    CYG_REPORT_FUNCNAMETYPE( "__memmove", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    if (n)
    {
        CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );
    }

    if ((src < dst) && (dst < (src + n)))
    {
        // Have to copy backwards
        src += n;
        dst += n;
        while (n--)
        {
            *--dst = *--src;
        }
    }
    else
    {
        while (n--)
        {
            *dst++ = *src++;
        }
    }

    CYG_REPORT_RETVAL( s1 );
    
    return s1;
} // __memmove()

// EOF memmove.cxx
