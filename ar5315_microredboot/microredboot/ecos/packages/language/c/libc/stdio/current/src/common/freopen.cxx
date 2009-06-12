//========================================================================
//
//      freopen.cxx
//
//      Implementation of C library freopen() function
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
// Date:          2000-04-20
// Purpose:       Provides ISO C freopen() function
// Description:   Implementation of C library freopen() function as per
//                ISO C standard chap. 7.9.5.4
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

// We only want freopen() if we have fopen()
#if defined(CYGPKG_LIBC_STDIO_OPEN)

// INCLUDES

#include <cyg/infra/cyg_type.h>     // Common project-wide type definitions
#include <stddef.h>                 // NULL and size_t from compiler
#include <stdio.h>                  // header for freopen()

// FUNCTIONS


externC FILE *
freopen( const char *, const char *, FILE * ) __THROW
{
    return NULL;  // Returning NULL is valid! FIXME
} // freopen()

#endif // defined(CYGPKG_LIBC_STDIO_OPEN)

// EOF freopen.cxx
