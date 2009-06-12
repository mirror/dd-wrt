#ifndef CYGONCE_LIBC_CTYPE_INL
#define CYGONCE_LIBC_CTYPE_INL
/*===========================================================================
//
//      ctype.inl
//
//      Inline implementations of ISO standard ctype routines defined in
//      section 7.3 of the standard
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
// Date:         2000-04-14
// Purpose:     
// Description: 
// Usage:        Do not include this file directly - use #include <ctype.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_i18n.h>   /* Configuration header */

/* The outline implementation will override this to prevent inlining */
#ifndef CYGPRI_LIBC_I18N_CTYPE_INLINE
# define CYGPRI_LIBC_I18N_CTYPE_INLINE extern __inline__
#endif

/* FUNCTIONS */

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/

/* 7.3.1 Character testing functions */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isupper( int c )
{
    return (('A' <= c) && (c <= 'Z'));
} /* isupper() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
islower( int c )
{
    return (('a' <= c) && (c <= 'z'));
} /* islower() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isalpha( int c )
{
    return ( islower(c) || isupper(c) );
} /* isalpha() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isdigit( int c )
{
    return ( ('0' <= c) && (c <= '9') );
} /* isdigit() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isalnum( int c )
{
    return ( isalpha(c) || isdigit(c) );
} /* isalnum() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
iscntrl( int c )
{
    /* Simple standard 7-bit ASCII map is assumed */
    return ( ((0 <= c) && (c <= 0x1F)) ||
             (c == 0x7F) );
} /* iscntrl() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isgraph( int c )
{
    // Simple standard 7-bit ASCII map is assumed
    return ( ('!' <= c) && (c <= '~') );
} /* isgraph() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isprint( int c )
{
    /* Simple standard 7-bit ASCII map is assumed */
    return ( (' ' <= c) && (c <= '~') );
} /* isprint() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
ispunct( int c )
{
    /* Simple standard 7-bit ASCII map is assumed */
    return ( (('!' <= c) && (c <= '/')) ||   // ASCII 0x21 - 0x2F
             ((':' <= c) && (c <= '@')) ||   // ASCII 0x3A - 0x40
             (('[' <= c) && (c <= '`')) ||   // ASCII 0x5B - 0x60
             (('{' <= c) && (c <= '~')) );   // ASCII 0x7B - 0x7E
             
} /* ispunct() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isspace( int c )
{
    return ( (c == ' ') || (c == '\f') || (c == '\n') || (c == '\r') ||
             (c == '\t') || (c == '\v') );
} /* isspace() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
isxdigit( int c )
{
    return ( isdigit(c) ||
             (('a' <= c) && (c <= 'f')) ||
             (('A' <= c) && (c <= 'F')) );
} /* isxdigit() */

/*========================================================================*/

/* 7.3.2 Character case mapping functions */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
tolower( int c )
{
    return isupper(c) ? c - 'A' + 'a' : c;
} /* tolower() */


CYGPRI_LIBC_I18N_CTYPE_INLINE int
toupper( int c )
{
    return islower(c) ? c - 'a' + 'A' : c;
} /* toupper() */

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* CYGONCE_LIBC_CTYPE_INL multiple inclusion protection */

/* EOF ctype.inl */
