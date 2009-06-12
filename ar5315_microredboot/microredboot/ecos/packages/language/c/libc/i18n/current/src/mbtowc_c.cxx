//===========================================================================
//
//      mbtowc_c.cxx
//
//      Internal __mbtowc_c() routine 
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
// Author(s):     jjohnstn
// Contributors:  jjohnstn
// Date:          2000-11-24
// Purpose:       Provide internal use __mbtowc_c() routine
// Description:   C locale version of mbtowc()
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//

// CONFIGURATION

#include <pkgconf/libc_i18n.h>     // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <locale.h>
#include <stdlib.h>                // Header for this file
#include <string.h>                // strcmp definition
#include <stddef.h>                // size_t definition
#include "internal.h"              // internal __isxxxx macros

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_STDLIB_MBTOWC_TRACE_LEVEL)
static int mbtowc_trace = CYGNUM_LIBC_STDLIB_MBTOWC_TRACE_LEVEL;
# define TL1 (0 < mbtowc_trace)
#else
# define TL1 (0)
#endif

// STATICS

// FUNCTIONS

int
__mbtowc_c ( wchar_t *pwc, const char *s, size_t n, int *state )
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;
  int retval;

  CYG_REPORT_FUNCNAMETYPE( "__mbtowc_c", "returning %d" );
  CYG_REPORT_FUNCARG4( "pwc=%08x, s=%08x, n=%ud, state=%08x", pwc, s, n, state );
  
  if (pwc != NULL)
    CYG_CHECK_DATA_PTR( pwc, "pwc is not a valid pointer!" );
  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
 
  if (pwc == NULL)
    pwc = &dummy;

  if (s != NULL && n == 0)
    {
      retval = -1;
      CYG_REPORT_RETVAL (retval);
      return retval;
    }

  if (s == NULL)
    retval = 0;  /* not state-dependent */
  else
    {
      if (pwc)
	*pwc = (wchar_t)*t;
      retval = (*t != '\0');
    }
  CYG_REPORT_RETVAL (retval);
  return retval;
} // __mbtowc_c()

// EOF mbtowc_c.cxx
