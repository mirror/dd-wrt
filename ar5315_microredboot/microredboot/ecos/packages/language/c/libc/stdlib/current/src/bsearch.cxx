//===========================================================================
//
//      bsearch.cxx
//
//      ANSI standard binary search function defined in section 7.10.5.1
//      of the standard
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
// Date:         2000-04-30
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdlib.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <stdlib.h>                // Header for all stdlib functions
                                   // (like this one)

// TRACING

# if defined(CYGDBG_USE_TRACING) && \
     defined(CYGNUM_LIBC_BSEARCH_TRACE_LEVEL)
static int bsearch_trace = CYGNUM_LIBC_BSEARCH_TRACE_LEVEL;
#  define TL1 (0 < bsearch_trace)
# else
#  define TL1 (0)
# endif


// FUNCTIONS

externC void *
bsearch( const void *key, const void *base, size_t nmemb, size_t size,
         __bsearch_comparison_fn_t compar )
{
    CYG_REPORT_FUNCNAMETYPE( "bsearch", "returning %08x" );

    CYG_REPORT_FUNCARG5( "key=%08x, base=%08x, nmemb=%d, size=%d, "
                         "compar=%08x", key, base, nmemb, size, compar );

    CYG_CHECK_DATA_PTR( key, "key is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( base, "base is not a valid pointer!" );
    CYG_CHECK_FUNC_PTR( compar, "compar is not a valid function pointer!" );

    CYG_ADDRESS current;
    size_t lower = 0;
    size_t upper = nmemb;
    size_t index;
    int result;
    
    if (nmemb == 0 || size == 0)
    {
        CYG_TRACE2( TL1, "Warning! either nmemb (%d) or size (%d) is 0",
                    nmemb, size );
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
    
    while (lower < upper)
    {
        index = (lower + upper) / 2;
        current = (CYG_ADDRESS) (((char *) base) + (index * size));
        
        CYG_TRACE2( TL1, "About to call comparison function with "
                    "key=%08x, current=%08x", key, current );
        result = compar (key, (void *) current);
        CYG_TRACE1( TL1, "Comparison function returned %d", result );
        
        if (result < 0)
            upper = index;
        else if (result > 0)
            lower = index + 1;
        else
        {
            CYG_REPORT_RETVAL( current );
            return (void *)current;
        } // else
    } // while
    
    CYG_REPORT_RETVAL( NULL );
    return NULL;
} // bsearch()

// EOF bsearch.cxx
