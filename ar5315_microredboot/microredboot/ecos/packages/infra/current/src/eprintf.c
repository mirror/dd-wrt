//========================================================================
//
//      eprintf.c
//
//      __eprintf() used by libgcc
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
// Date:          2001-08-21
// Purpose:       Provide __eprintf() as used by libgcc.
// Description:   libgcc calls __eprintf() to display errors and abort.
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/system.h>

#ifdef CYGPKG_ISOINFRA 
# include <pkgconf/isoinfra.h>     // Configuration header
#endif

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_ass.h>     // Default assertion
#include <cyg/infra/diag.h>

#ifdef CYGPKG_ISOINFRA
# if defined(CYGINT_ISO_STDIO_FORMATTED_IO) || \
     defined(CYGINT_ISO_STDIO_FILEACCESS)
#  include <stdio.h>
# endif
# if CYGINT_ISO_EXIT
#  include <stdlib.h>
# endif
#endif

// FUNCTIONS

__externC void
__eprintf (const char *string, const char *expression,
           unsigned int line, const char *filename)
{
#ifdef CYGINT_ISO_STDIO_FORMATTED_IO
    fprintf(stderr, string, expression, line, filename);
#else
    diag_printf(string, expression, line, filename);
#endif
#ifdef CYGINT_ISO_STDIO_FILEACCESS
    fflush (stderr);
#endif
#if CYGINT_ISO_EXIT
    abort();
#else
    CYG_FAIL( "Aborting" );
    for (;;);
#endif
} // __eprintf()

// EOF eprintf.c
