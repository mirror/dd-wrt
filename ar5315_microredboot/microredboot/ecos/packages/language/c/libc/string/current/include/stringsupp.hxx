#ifndef CYGONCE_LIBC_STRING_STRINGSUPP_HXX
#define CYGONCE_LIBC_STRING_STRINGSUPP_HXX
//===========================================================================
//
//      stringsupp.hxx
//
//      Support for the ANSI standard string functions
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
// Contributors:  jlarmour
// Date:          2000-04-14
// Purpose:       Internal support for the libc string function implementations
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_string.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h> // Common type definitions
#include <stddef.h>             // Compiler definitions such as
                                // size_t, NULL etc.

// CONSTANTS

#ifndef CYGIMP_LIBC_STRING_PREFER_SMALL_TO_FAST

// Masks for CYG_LIBC_STR_DETECTNULL below
__externC const cyg_uint64 Cyg_libc_str_null_mask_1;
__externC const cyg_uint64 Cyg_libc_str_null_mask_2;

#endif

// MACROS

// Nonzero if X is not aligned on a word boundary.
#define CYG_LIBC_STR_UNALIGNED(X) ((CYG_WORD)(X) & (sizeof (CYG_WORD) - 1))

// Nonzero if either X or Y is not aligned on a word boundary.
#define CYG_LIBC_STR_UNALIGNED2(X , Y) \
    (((CYG_WORD)(X) & (sizeof (CYG_WORD) - 1)) | \
     ((CYG_WORD)(Y) & (sizeof (CYG_WORD) - 1)))

// Nonzero if any byte of X contains a NULL.
#define CYG_LIBC_STR_DETECTNULL(X) \
    (((X) - (CYG_WORD)Cyg_libc_str_null_mask_1) & \
     ~(X) & (CYG_WORD)Cyg_libc_str_null_mask_2)

// How many bytes are copied each iteration of the 4X unrolled loop in the
// optimised string implementations
#define CYG_LIBC_STR_OPT_BIGBLOCKSIZE     (sizeof(CYG_WORD) << 2)

// How many bytes are copied each iteration of the word copy loop in the
// optimised string implementations
#define CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE  (sizeof (CYG_WORD))

// Threshold for punting to the byte copier in the optimised string
// implementations
#define CYG_LIBC_STR_OPT_TOO_SMALL(LEN) \
    ((LEN) < CYG_LIBC_STR_OPT_LITTLEBLOCKSIZE)


// FUNCTION PROTOTYPES

// These are function prototypes for the aliased functions that actually
// implement the string functions

//===========================================================================

// 7.11.2 Copying functions

__externC void *
__memmove( void *, const void *, size_t );


__externC char *
__strcpy( char *, const char * );


__externC char *
__strncpy( char *, const char *, size_t );


//===========================================================================

// 7.11.3 Concatenation functions


__externC char *
__strcat( char *, const char * );


__externC char *
__strncat( char *, const char *, size_t );


//===========================================================================

// 7.11.4 Comparison functions

__externC int
__memcmp( const void *, const void *, size_t );


__externC int
__strcmp( const char *, const char * );


__externC int
__strcoll( const char *, const char * );


__externC int
__strncmp( const char *, const char *, size_t );


__externC size_t
__strxfrm( char *, const char *, size_t );


//===========================================================================

// 7.11.5 Search functions


__externC void *
__memchr( const void *, int , size_t );


__externC char *
__strchr( const char *, int );


__externC size_t
__strcspn( const char *, const char * );


__externC char *
__strpbrk( const char *, const char * );


__externC char *
__strrchr( const char *, int );


__externC size_t
__strspn( const char *, const char * );


__externC char *
__strstr( const char *, const char * );


__externC char *
__strtok( char *, const char * );

// For POSIX 1003.1 section 8.3.3 strtok_r()

__externC char *
__strtok_r( char *, const char *, char ** );


//===========================================================================

// 7.11.6 Miscellaneous functions

__externC size_t
__strlen( const char * );

// NB This is a BSD function
__externC char *
__strdup( const char * );

#endif // CYGONCE_LIBC_STRING_STRINGSUPP_HXX multiple inclusion protection

// EOF stringsupp.hxx
