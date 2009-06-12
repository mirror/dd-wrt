#ifndef CYGONCE_ISO_STDIO_H
#define CYGONCE_ISO_STDIO_H
/*========================================================================
//
//      stdio.h
//
//      ISO standard I/O functions
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
// Purpose:       This file provides the stdio functions required by 
//                ISO C and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <stdio.h>
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

/*  Misc functions below use varargs stuff, so pull it in from the compiler
 *  here to save duplication.
 */

#define __need___va_list
#include <stdarg.h>

#ifdef CYGINT_ISO_STDIO_FILETYPES
# ifdef CYGBLD_ISO_STDIO_FILETYPES_HEADER
#  include CYGBLD_ISO_STDIO_FILETYPES_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_STREAMS
# ifdef CYGBLD_ISO_STDIO_STREAMS_HEADER
#  include CYGBLD_ISO_STDIO_STREAMS_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_FILEOPS
# ifdef CYGBLD_ISO_STDIO_FILEOPS_HEADER
#  include CYGBLD_ISO_STDIO_FILEOPS_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_FILEACCESS
# ifdef CYGBLD_ISO_STDIO_FILEACCESS_HEADER
#  include CYGBLD_ISO_STDIO_FILEACCESS_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_FORMATTED_IO
# ifdef CYGBLD_ISO_STDIO_FORMATTED_IO_HEADER
#  include CYGBLD_ISO_STDIO_FORMATTED_IO_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_CHAR_IO
# ifdef CYGBLD_ISO_STDIO_CHAR_IO_HEADER
#  include CYGBLD_ISO_STDIO_CHAR_IO_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_DIRECT_IO
# ifdef CYGBLD_ISO_STDIO_DIRECT_IO_HEADER
#  include CYGBLD_ISO_STDIO_DIRECT_IO_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_FILEPOS
# ifdef CYGBLD_ISO_STDIO_FILEPOS_HEADER
#  include CYGBLD_ISO_STDIO_FILEPOS_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_ERROR
# ifdef CYGBLD_ISO_STDIO_ERROR_HEADER
#  include CYGBLD_ISO_STDIO_ERROR_HEADER
# endif
#endif

#ifdef CYGINT_ISO_STDIO_POSIX_FDFUNCS
# ifdef CYGBLD_ISO_STDIO_POSIX_FDFUNCS_HEADER
#  include CYGBLD_ISO_STDIO_POSIX_FDFUNCS_HEADER
# else

#  ifdef __cplusplus
extern "C" {
#  endif

extern int
fileno( FILE *__stream ) __THROW;

extern FILE *
fdopen( int __fildes, const char *__type ) __THROW;

#  ifdef __cplusplus
} // extern "C"
#  endif
# endif
#endif

#endif /* CYGONCE_ISO_STDIO_H multiple inclusion protection */

/* EOF stdio.h */
