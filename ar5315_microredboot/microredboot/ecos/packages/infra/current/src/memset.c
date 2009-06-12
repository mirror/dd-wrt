/*===========================================================================
//
//      memset.c
//
//      ANSI standard memset() routine
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-06-04
// Purpose:     This file implements the ANSI memset() function
// Description: This file implements the memset() function defined in ANSI para
//              7.11.6.1. This is implemented in the kernel rather than the
//              C library due to it being required by gcc whether or not the
//              C library has been configured in.
//
//####DESCRIPTIONEND####
//
//==========================================================================*/


/* INCLUDES */

#include <pkgconf/infra.h>      /* Configuration of infra package */

#include <cyg/infra/cyg_type.h> /* Common type definitions */
#include <cyg/infra/cyg_trac.h> /* Tracing support */
#include <cyg/infra/cyg_ass.h>  /* Assertion support */
#include <stddef.h>             /* Compiler defns such as size_t, NULL etc. */

/* MACROS */

/* Nonzero if X is not aligned on a word boundary. */
#define CYG_STR_UNALIGNED(X) ((CYG_WORD)(X) & (sizeof (CYG_WORD) - 1))

/* How many bytes are copied each iteration of the word copy loop in the
 * optimised string implementation
 */
#define CYG_STR_OPT_LITTLEBLOCKSIZE (sizeof (CYG_WORD))

/* EXPORTED SYMBOLS */

externC void *
memset( void *s, int c, size_t n ) __attribute__ ((weak, alias("_memset")));

/* FUNCTIONS */

void *
_memset( void *s, int c, size_t n )
{
#if defined(CYGIMP_INFRA_PREFER_SMALL_TO_FAST_MEMSET) || defined(__OPTIMIZE_SIZE__)
    char *char_ptr = (char *)s;
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_FUNCNAMETYPE( "_memset", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s=%08x, c=%d, n=%d", s, c, n );

    if (n != 0)
    {
        CYG_CHECK_DATA_PTR( char_ptr, "s is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( (&char_ptr[n-1]), "s+n-1 is not a valid address!" );
    }
#endif

    while (n-- != 0)
    {
        *char_ptr++ = (char) c;
    }
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_RETVAL( s );
#endif
    return s;
#else
    char *char_ptr = (char *)s;
    cyg_ucount8 count;
    CYG_WORD buffer;
    CYG_WORD *aligned_addr;
    char *unaligned_addr;
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_FUNCNAMETYPE( "_memset", "returning %08x" );
    CYG_REPORT_FUNCARG3( "s=%08x, c=%d, n=%d", s, c, n );

    if (n != 0)
    {
        CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( (char *)s+n-1, "s+n-1 is not a valid address!" );
    }
#endif

    if (n < sizeof(CYG_WORD) || CYG_STR_UNALIGNED (s))
    {
        while (n-- != 0)
        {
            *char_ptr++ = (char) c;
        }
#ifdef CYG_TRACING_FIXED
        CYG_REPORT_RETVAL( s );
#endif
        return s;
    }
    
    /* If we get this far, we know that n is large and s is word-aligned. */
    
    aligned_addr = (CYG_WORD *)s;
    
    /* Store C into each char sized location in BUFFER so that
     * we can set large blocks quickly.
     */
    c &= 0xff;
    if (CYG_STR_OPT_LITTLEBLOCKSIZE == 4)
    {
        buffer = (c << 8) | c;
        buffer |= (buffer << 16);
    }
    else
    {
        buffer = 0;
        for (count = 0; count < CYG_STR_OPT_LITTLEBLOCKSIZE; count++)
            buffer = (buffer << 8) | c;
    }
    
    while (n >= CYG_STR_OPT_LITTLEBLOCKSIZE*4)
    {
        *aligned_addr++ = buffer;
        *aligned_addr++ = buffer;
        *aligned_addr++ = buffer;
        *aligned_addr++ = buffer;
        n -= 4*CYG_STR_OPT_LITTLEBLOCKSIZE;
    }
    
    while (n >= CYG_STR_OPT_LITTLEBLOCKSIZE)
    {
        *aligned_addr++ = buffer;
        n -= CYG_STR_OPT_LITTLEBLOCKSIZE;
    }
    
    /* Pick up the remainder with a bytewise loop. */
    unaligned_addr = (char*)aligned_addr;
    while (n)
    {
        *unaligned_addr++ = (char)c;
        n--;
    }
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_RETVAL( s );
#endif
    return s;
#endif /* not defined(CYGIMP_PREFER_SMALL_TO_FAST_MEMSET) ||
        * defined(__OPTIMIZE_SIZE__) */
} /* _memset() */

/* EOF memset.c */
