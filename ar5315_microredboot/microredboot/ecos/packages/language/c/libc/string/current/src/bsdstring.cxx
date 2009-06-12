//===========================================================================
//
//      bsdstring.cxx
//
//      BSD string routines
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
// Date:          2001-11-27
// Purpose:       Provide string functions derived from BSD
// Description: 
// Usage:         #include <string.h>
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
#include <stddef.h>                // size_t, NULL etc.
#include <ctype.h>                 // toupper/tolower

// FUNCTIONS

/*---------------------------------------------------------------------*/
/* strcasecmp */

__externC int
__strcasecmp( const char *s1, const char *s2 )
{
    CYG_REPORT_FUNCNAMETYPE( "strcasecmp", "returning %d" );
    CYG_REPORT_FUNCARG2( "s1=%08x, s2=%08x", s1, s2 );

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    while (*s1 != '\0' && tolower(*s1) == tolower(*s2))
    {
        s1++;
        s2++;
    }

    int ret = tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
    CYG_REPORT_RETVAL( ret );
    return ret;
}

__externC int
strcasecmp( const char *s1, const char *s2 ) \
  CYGBLD_ATTRIB_WEAK_ALIAS(__strcasecmp);


/*---------------------------------------------------------------------*/
/* strncasecmp */

__externC int
__strncasecmp( const char *s1, const char *s2, size_t n )
{
    CYG_REPORT_FUNCNAMETYPE( "strncasecmp", "returning %d" );
    CYG_REPORT_FUNCARG3( "s1=%08x, s2=%08x, n=%d", s1, s2, n );

    if (n == 0)
    {
        CYG_REPORT_RETVAL(0);
        return 0;
    }

    CYG_CHECK_DATA_PTR( s1, "s1 is not a valid pointer!" );
    CYG_CHECK_DATA_PTR( s2, "s2 is not a valid pointer!" );

    while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
        if (n == 0 || *s1 == '\0' || *s2 == '\0')
            break;
        s1++;
        s2++;
    }

    int ret = tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
    CYG_REPORT_RETVAL( ret );
    return ret;
}

__externC int
strncasecmp( const char *s1, const char *s2, size_t n ) \
  CYGBLD_ATTRIB_WEAK_ALIAS(__strncasecmp);

/*---------------------------------------------------------------------*/
/* bcmp */

__externC int
__bcmp( const void *s1, const void *s2, size_t n )
{
    // Don't bother tracing - memcmp can do that
    return memcmp (s1, s2, n);
}

__externC int
bcmp( const void *s1, const void *s2, size_t n ) \
  CYGBLD_ATTRIB_WEAK_ALIAS(__bcmp);

/*---------------------------------------------------------------------*/
/* bcopy */

__externC void
__bcopy( const void *src, void *dest, size_t n )
{
    // Don't bother tracing - memmove can do that
    memmove (dest, src, n);
}

__externC void
bcopy( const void *src, void *dest, size_t n ) \
  CYGBLD_ATTRIB_WEAK_ALIAS(__bcopy);

/*---------------------------------------------------------------------*/
/* bzero */

__externC void
__bzero( void *s, size_t n )
{
    // Don't bother tracing - memset can do that
    memset( s, 0, n );
}

__externC void
bzero( void *s, size_t n ) CYGBLD_ATTRIB_WEAK_ALIAS(__bzero);

/*---------------------------------------------------------------------*/
/* index */

__externC char *
__index( const char *s, int c )
{
    // Don't bother tracing - strchr can do that
    return strchr(s, c);
}

__externC char *
index( const char *s, int c ) CYGBLD_ATTRIB_WEAK_ALIAS(__index);

/*---------------------------------------------------------------------*/
/* rindex */

__externC char *
__rindex( const char *s, int c )
{
    // Don't bother tracing - strrchr can do that
    return strrchr(s, c);
}

__externC char *
rindex( const char *s, int c ) CYGBLD_ATTRIB_WEAK_ALIAS(__rindex);

/*---------------------------------------------------------------------*/
/* swab */

__externC void
__swab( const void *from, void *to, size_t n )
{
    const char *f = (const char *)from;
    char *t = (char *)to;
    size_t ptr;

    CYG_REPORT_FUNCNAME( "swab" );
    CYG_REPORT_FUNCARG3( "from=%08x, to=%08x, n=%d", from, to, n );

    if (n) {
        CYG_CHECK_DATA_PTR( from, "from is not a valid pointer!" );
        CYG_CHECK_DATA_PTR( to, "to is not a valid pointer!" );
    }

    for (ptr = 1; ptr < n; ptr += 2)
    {
        char p = f[ptr];
        char q = f[ptr-1];
        t[ptr-1] = p;
        t[ptr  ] = q;
    }
    if (ptr == n)          /* I.e., if n is odd, */
        t[ptr-1] = '\0';   /* then pad with a NUL. */

    CYG_REPORT_RETURN();
}

__externC void
swab( const void *from, void *to, size_t n ) \
  CYGBLD_ATTRIB_WEAK_ALIAS(__swab);

/*---------------------------------------------------------------------*/
// EOF bsdstring.cxx
