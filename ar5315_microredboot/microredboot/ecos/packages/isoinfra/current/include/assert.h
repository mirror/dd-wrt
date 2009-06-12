#ifndef CYGONCE_ISO_ASSERT_H
#define CYGONCE_ISO_ASSERT_H
/*========================================================================
//
//      assert.h
//
//      ISO C assertions
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
// Purpose:       This file provides the assert functions required by 
//                ISO C and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <assert.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */
#include <pkgconf/infra.h>             /* CYGDBG_USE_ASSERTS */

/* INCLUDES */

#ifdef CYGBLD_ISO_ASSERT_HEADER
# include CYGBLD_ISO_ASSERT_HEADER
#else

# ifdef NDEBUG
#  define assert( __bool ) ((void)0)
# else /* if NDEBUG is NOT defined */

/* First preference is to be standards compliant */

#if defined(CYGINT_ISO_STDIO_FORMATTED_IO) && defined(CYGINT_ISO_EXIT)

# include <stdio.h>
# include <stdlib.h>

# define assert( __bool )                                                 \
    do {                                                                  \
        if (0 == (__bool)) {                                              \
            fprintf( stderr, "User assertion failed: \"%s\", at %s:%d\n", \
                         #__bool, __FILE__, __LINE__);                    \
            abort();                                                      \
        }                                                                 \
    } while(0)


/* Second preference is to use the common infra assertion support */

#elif defined(CYGDBG_USE_ASSERTS)

# include <cyg/infra/cyg_ass.h>

# define assert( __bool ) \
        CYG_MACRO_START   \
        CYG_ASSERT( __bool, "User assertion failed: \""  #__bool "\"" ); \
        CYG_MACRO_END
#else /* Fallback */

# include <cyg/infra/diag.h>

# define assert( __bool )                                                 \
    do {                                                                  \
        if (0 == (__bool)) {                                              \
            diag_printf( "User assertion failed: \"%s\", at %s:%d\n",     \
                         #__bool, __FILE__, __LINE__);                    \
        for (;;);                                                         \
        }                                                                 \
    } while(0)

#endif

# endif /* NDEBUG not defined */
#endif

#endif /* CYGONCE_ISO_ASSERT_H multiple inclusion protection */

/* EOF assert.h */
