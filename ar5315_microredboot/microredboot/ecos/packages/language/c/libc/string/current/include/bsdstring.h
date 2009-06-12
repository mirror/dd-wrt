#ifndef CYGONCE_LIBC_BSDSTRING_H
#define CYGONCE_LIBC_BSDSTRING_H
/*===========================================================================
//
//      bsdstring.h
//
//      BSD standard string and memory area manipulation routines
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
// Purpose:       This file provides various string functions normally
//                provided in the BSD UNIX operating system.
// Description:   
// Usage:         Do not include this file directly - use #include <string.h>
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

/* CONFIGURATION */

#include <pkgconf/libc_string.h>   /* Configuration header */

#ifdef CYGFUN_LIBC_STRING_BSD_FUNCS

#define __need_size_t
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/

/* FUNCTION PROTOTYPES */


extern int
strcasecmp( const char * /* s1 */, const char * /* s2 */ );

extern int
strncasecmp( const char * /* s1 */, const char * /* s2 */, size_t /* n */ );

extern int
bcmp( const void * /* s1 */, const void * /* s2 */, size_t /* n */ );

extern void
bcopy( const void * /* src */, void * /* dest */, size_t /* n */ );

extern void
bzero( void * /* s */, size_t /* n */ );

extern char *
index( const char * /* s */, int /* c */ );

extern char *
rindex( const char * /* s */, int /* c */ );

extern void
swab( const void * /* from */, void * /* to */, size_t /* n */ );

/*=========================================================================*/

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* ifdef CYGFUN_LIBC_STRING_BSD_FUNCS */

#endif /* CYGONCE_LIBC_BSDSTRING_H multiple inclusion protection */

/* EOF bsdstring.h */
