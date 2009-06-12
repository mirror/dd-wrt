#ifndef CYGONCE_LIBC_STDIO_STDIO_INL
#define CYGONCE_LIBC_STDIO_STDIO_INL
//===========================================================================
//
//      stdio.inl
//
//      ANSI standard I/O routines - inlined functions
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
// Date:         2000-04-19
// Purpose:     
// Description: 
// Usage:       Do not include this file directly - use #include <stdio.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <stddef.h>                // NULL and size_t from compiler
#include <stdarg.h>                // va_list
#include <stdio.h>                 // Just be sure it has been included
#include <errno.h>                 // Definition of error codes and errno
#include <string.h>                // Definition of strerror() for perror()
#include <limits.h>                // INT_MAX

// INLINE FUNCTION DEFINITIONS

//===========================================================================

// 7.9.5 File access functions

extern __inline__ void
setbuf( FILE *stream, char *buf ) __THROW
{
    if (buf == NULL)
        setvbuf( stream, NULL, _IONBF, 0 );
    else
        // NB: Should use full buffering by default ordinarily, but in
        // the current system we're always connected to an interactive
        // terminal, so use line buffering
        setvbuf( stream, buf, _IOLBF, BUFSIZ );

} // setbuf()

//===========================================================================

// 7.9.6 Formatted input/output functions

extern __inline__ int
vfprintf( FILE *stream, const char *format, va_list arg ) __THROW
{
    return vfnprintf(stream, INT_MAX, format, arg);
} // vfprintf()


extern __inline__ int
vprintf( const char *format, va_list arg ) __THROW
{
    return vfnprintf(stdout, INT_MAX, format, arg);
} // vprintf()


extern __inline__ int
vsprintf( char *s, const char *format, va_list arg ) __THROW
{
    return vsnprintf(s, INT_MAX, format, arg);
} // vsprintf()


//===========================================================================

// 7.9.7 Character input/output functions

extern __inline__ int
puts( const char *s ) __THROW
{
    int rc;

    rc = fputs( s, stdout );

    if (rc >= 0)
        rc = fputc('\n', stdout );

    return rc;
} // puts()


//===========================================================================

// 7.9.10 Error-handling functions

extern __inline__ void
perror( const char *s ) __THROW
{
    if (s && *s)
        fprintf( stderr, "%s: %s\n", s, strerror(errno) );
    else
        fputs( strerror(errno), stderr );

} // perror()

//===========================================================================

// Other non-ANSI functions

extern __inline__ int
vscanf( const char *format, va_list arg ) __THROW
{
    return vfscanf( stdin, format, arg );
} // vscanf()


#endif // CYGONCE_LIBC_STDIO_STDIO_INL multiple inclusion protection

// EOF stdio.inl
