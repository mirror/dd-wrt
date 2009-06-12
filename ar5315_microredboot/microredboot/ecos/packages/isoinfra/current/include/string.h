#ifndef CYGONCE_ISO_STRING_H
#define CYGONCE_ISO_STRING_H
/*========================================================================
//
//      string.h
//
//      ISO string functions
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-14
// Purpose:       This file provides the string macros, types and functions
//                required by ISO C and POSIX 1003.1. It may also provide
//                other compatibility functions like BSD-style string
//                functions
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <string.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

/* This is the "standard" way to get NULL and size_t from stddef.h,
 * which is the canonical location of the definitions.
 */
#define __need_NULL
#define __need_size_t
#include <stddef.h>

#ifdef CYGINT_ISO_STRERROR
# include CYGBLD_ISO_STRERROR_HEADER
#endif

/* memcpy() and memset() are special as they are used internally by
 * the compiler, so they *must* exist. So if no-one claims to implement
 * them, prototype them anyway */

#ifdef CYGBLD_ISO_MEMCPY_HEADER
# include CYGBLD_ISO_MEMCPY_HEADER
#else

extern 
# ifdef __cplusplus
"C"
# endif
void *
memcpy( void *, const void *, size_t );

#endif

#ifdef CYGBLD_ISO_MEMSET_HEADER
# include CYGBLD_ISO_MEMSET_HEADER
#else

extern 
# ifdef __cplusplus
"C"
# endif
void *
memset( void *, int, size_t );

#endif

#ifdef CYGINT_ISO_STRTOK_R
# include CYGBLD_ISO_STRTOK_R_HEADER
#endif

#ifdef CYGINT_ISO_STRING_LOCALE_FUNCS
# include CYGBLD_ISO_STRING_LOCALE_FUNCS_HEADER
#endif

#ifdef CYGINT_ISO_STRING_BSD_FUNCS
# include CYGBLD_ISO_STRING_BSD_FUNCS_HEADER
#endif

#ifdef CYGINT_ISO_STRING_MEMFUNCS
# include CYGBLD_ISO_STRING_MEMFUNCS_HEADER
#endif

#ifdef CYGINT_ISO_STRING_STRFUNCS
# include CYGBLD_ISO_STRING_STRFUNCS_HEADER
#endif

#endif /* CYGONCE_ISO_STRING_H multiple inclusion protection */

/* EOF string.h */
