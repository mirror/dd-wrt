/*===========================================================================
//
//      memcpy.c
//
//      ANSI standard memcpy() routine
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
// Purpose:     This file implements the ANSI memcpy() function
// Description: This file implements the memcpy() function defined in ANSI para
//              7.11.2.1. This is implemented in the kernel rather than the
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

/* Nonzero if either X or Y is not aligned on a word boundary. */
#define CYG_STR_UNALIGNED(X, Y) \
     (((CYG_WORD)(X) & (sizeof (CYG_WORD) - 1)) | \
      ((CYG_WORD)(Y) & (sizeof (CYG_WORD) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop in the
 * optimised string implementation
 */
#define CYG_STR_OPT_BIGBLOCKSIZE     (sizeof(CYG_WORD) << 2)


/* How many bytes are copied each iteration of the word copy loop in the
 * optimised string implementation
 */
#define CYG_STR_OPT_LITTLEBLOCKSIZE (sizeof (CYG_WORD))

/* EXPORTED SYMBOLS */

externC void *
memcpy( void *s1, const void *s2, size_t n ) __attribute__((weak,
                                                            alias("_memcpy")));

/* FUNCTIONS */

void *
_memcpy( void *s1, const void *s2, size_t n )
{
#if defined(CYGIMP_INFRA_PREFER_SMALL_TO_FAST_MEMCPY) || defined(__OPTIMIZE_SIZE__)
    char *dst = (char *) s1;
    const char *src = (const char *) s2;
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_FUNCNAMETYPE( "_memcpy", "returning %08x" );
    CYG_REPORT_FUNCARG3( "dst=%08x, src=%08x, n=%d", dst, src, n );

    if (n != 0)
    {
        CYG_CHECK_DATA_PTR( dst, "dst is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( src, "src is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( dst+n-1, "dst+n-1 is not a valid address!" );
        CYG_CHECK_DATA_PTR( src+n-1, "src+n-1 is not a valid address!" );
    }
#endif

    while (n--)
    {
        *dst++ = *src++;
    } /* while */
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_RETVAL( s1 );
#endif
    return s1;
#else
    char *dst;
    const char *src;
    CYG_WORD *aligned_dst;
    const CYG_WORD *aligned_src;
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_FUNCNAMETYPE( "_memcpy", "returning %08x" );
#endif

    dst = (char *)s1;
    src = (const char *)s2;

#ifdef CYG_TRACING_FIXED
    CYG_REPORT_FUNCARG3( "dst=%08x, src=%08x, n=%d", dst, src, n );
    
    if (n != 0)
    {
        CYG_CHECK_DATA_PTR( dst, "dst is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( src, "src is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( dst+n-1, "dst+n-1 is not a valid address!" );
        CYG_CHECK_DATA_PTR( src+n-1, "src+n-1 is not a valid address!" );
    }
#endif

    /* If the size is small, or either SRC or DST is unaligned,
     * then punt into the byte copy loop.  This should be rare.
     */
    if (n < sizeof(CYG_WORD) || CYG_STR_UNALIGNED (src, dst))
    {
        while (n--)
            *dst++ = *src++;
#ifdef CYG_TRACING_FIXED
        CYG_REPORT_RETVAL( s1 );
#endif
        return s1;
    } /* if */
    
    aligned_dst = (CYG_WORD *)dst;
    aligned_src = (const CYG_WORD *)src;
    
    /* Copy 4X long words at a time if possible.  */
    while (n >= CYG_STR_OPT_BIGBLOCKSIZE)
    {
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        *aligned_dst++ = *aligned_src++;
        n -= CYG_STR_OPT_BIGBLOCKSIZE;
    } /* while */
    
    /* Copy one long word at a time if possible.  */
    while (n >= CYG_STR_OPT_LITTLEBLOCKSIZE)
    {
        *aligned_dst++ = *aligned_src++;
        n -= CYG_STR_OPT_LITTLEBLOCKSIZE;
    } /* while */
    
    /* Pick up any residual with a byte copier.  */
    dst = (char*)aligned_dst;
    src = (const char*)aligned_src;
    while (n--)
        *dst++ = *src++;
    
#ifdef CYG_TRACING_FIXED
    CYG_REPORT_RETVAL( s1 );
#endif
    return s1;
#endif /* not defined(CYGIMP_PREFER_SMALL_TO_FAST_MEMCPY) ||
        * defined(__OPTIMIZE_SIZE__) */
} /* _memcpy() */

/* EOF memcpy.c */
