#ifndef CYGONCE_LIBC_STDIO_IO_HXX
#define CYGONCE_LIBC_STDIO_IO_HXX
//========================================================================
//
//      io.hxx
//
//      Internal C library stdio io interface definitions
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
// Author(s):     nickg
// Contributors:  
// Date:          2000-06-30
// Purpose:     
// Description: 
// Usage:         #include <cyg/libc/stdio/io.hxx>
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdio.h>   // Configuration header

//========================================================================
// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common project-wide type definitions
#include <cyg/infra/cyg_ass.h>     // Get assertion macros, as appropriate
#include <errno.h>                 // Cyg_ErrNo

#ifdef CYGPKG_LIBC_STDIO_FILEIO
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#else
#include <cyg/io/io.h>             // Device I/O support
#include <cyg/io/config_keys.h>    // CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN
#endif

//========================================================================
// Stream handle type

#ifdef CYGPKG_LIBC_STDIO_FILEIO
typedef int cyg_stdio_handle_t;
#define CYG_STDIO_HANDLE_NULL -1
#else
typedef cyg_io_handle_t cyg_stdio_handle_t;
#define CYG_STDIO_HANDLE_NULL 0
#endif

//========================================================================
#endif // CYGONCE_LIBC_STDIO_IO_HXX multiple inclusion protection
// EOF io.hxx
