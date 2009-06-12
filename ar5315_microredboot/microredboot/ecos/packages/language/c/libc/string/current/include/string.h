#ifndef CYGONCE_LIBC_STRING_H
#define CYGONCE_LIBC_STRING_H
/*===========================================================================
//
//      string.h
//
//      ANSI standard string and memory area manipulation routines
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
// Purpose:       This file provides various string functions required by
//                ISO C 9899:1990
// Description:   The real contents of this file get filled in from the
//                configuration set by the implementation
// Usage:         Do not include this file directly - use #include <string.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_string.h>   /* Configuration header */

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTION PROTOTYPES */

/*=========================================================================*/

/* 7.11.2 Copying functions */

extern void *
memmove( void *, const void *, size_t );

extern char *
strcpy( char *, const char * );

extern char *
strncpy( char *, const char *, size_t );

/*=========================================================================*/

/* 7.11.3 Concatenation functions */

extern char *
strcat( char *, const char * );

extern char *
strncat( char *, const char *, size_t );


/*=========================================================================*/

/* 7.11.4 Comparison functions */

extern int
memcmp( const void *, const void *, size_t );

extern int
strcmp( const char *, const char * );

extern int
strcoll( const char *, const char * );

extern int
strncmp( const char *, const char *, size_t );

extern size_t
strxfrm( char *, const char *, size_t );


/*=========================================================================*/

/* 7.11.5 Search functions */

extern void *
memchr( const void *, int,  size_t );

extern char *
strchr( const char *, int );

extern size_t
strcspn( const char *, const char * );

extern char *
strpbrk( const char *, const char * );

extern char *
strrchr( const char *, int );

extern size_t
strspn( const char *, const char * );

extern char *
strstr( const char *, const char * );

extern char *
strtok( char *, const char * );

/* POSIX 1003.1 section 8.3.3 strtok_r() */

extern char *
strtok_r( char *, const char *, char ** );


/*=========================================================================*/

/* 7.11.6 Miscellaneous functions */

extern size_t
strlen( const char * );

// NB This is a BSD function
#ifndef __STRICT_ANSI__
extern char *
strdup( const char * );
#endif

#ifdef __cplusplus
}   /* extern "C" */
#endif

/* INLINE FUNCTIONS */

#ifdef CYGIMP_LIBC_STRING_INLINES
#include <cyg/libc/string/string.inl>
#endif

#endif /* CYGONCE_LIBC_STRING_H multiple inclusion protection */

/* EOF string.h */
