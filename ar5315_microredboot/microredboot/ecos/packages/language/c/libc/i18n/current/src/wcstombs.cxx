//===========================================================================
//
//      wcstombs.cxx
//
//      ISO standard wcstombs() routine 
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
// Date:          2000-11-02
// Purpose:       Provide ISO C wcstombs()
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code was based on newlib/libc/stdlib/wcstombs.c and newlib/libc/stdlib/wcstombs_r.c
// The following is modified from the original newlib description:
//
/*
FUNCTION
<<wcstombs>>---wide char string to multibyte string converter

INDEX
	wcstombs

ANSI_SYNOPSIS
	#include <stdlib.h>
	int wcstombs(const char *<[s]>, wchar_t *<[pwc]>, size_t <[n]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int wcstombs(<[s]>, <[pwc]>, <[n]>)
	const char *<[s]>;
	wchar_t *<[pwc]>;
	size_t <[n]>;

DESCRIPTION
When CYGINT_LIBC_I18N_MB_REQUIRED is not defined, this is a minimal ANSI-conforming 
implementation of <<wcstombs>>.  In this case,
all wide-characters are expected to represent single bytes and so
are converted simply by casting to char.

When CYGINT_LIBC_I18N_MB_REQUIRED is defined, this routine calls the LC_CTYPE locale wcstomb_fn 
repeatedly to perform the conversion, passing a state variable to allow state dependent
decoding.  The result is based on the locale setting which may
be restricted to a defined set of locales.

RETURNS
This implementation of <<wcstombs>> returns <<0>> if
<[s]> is <<NULL>> or is the empty string; 
it returns <<-1>> if CYGINT_LIBC_I18N_MB_REQUIRED and one of the
wide-char characters does not represent a valid multi-byte character;
otherwise it returns the minimum of: <<n>> or the
number of bytes that are transferred to <<s>>, not including the
nul terminator.

If the return value is -1, the state of the <<pwc>> string is
indeterminate.  If the input has a length of 0, the output
string will be modified to contain a wchar_t nul terminator if
<<n>> > 0.

PORTABILITY
<<wcstombs>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<wcstombs>> requires no supporting OS subroutines.
*/

// CONFIGURATION

#include <pkgconf/libc_i18n.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <locale.h>
#include <stdlib.h>                // Header for this file
#include <string.h>                // strcmp definition
#include <stddef.h>                // size_t definition
#include "internal.h"              // __current_ctype_locale 

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_WCSTOMBS
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_I18N_WCSTOMBS_TRACE_LEVEL)
static int wcstombs_trace = CYGNUM_LIBC_I18N_WCSTOMBS_TRACE_LEVEL;
# define TL1 (0 < wcstombs_trace)
#else
# define TL1 (0)
#endif

// STATICS

// FUNCTIONS

size_t 
wcstombs ( char *s, const wchar_t *pwcs, size_t n )
{
  size_t  retval;
  
  CYG_REPORT_FUNCNAMETYPE( "wcstombs", "returning %ud" );
  CYG_REPORT_FUNCARG3( "s=%08x, pwcs=%08x, n=%ud", s, pwcs, n );

  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
  if (pwcs != NULL)
    CYG_CHECK_DATA_PTR( pwcs, "pwcs is not a valid pointer!" );
  
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED  

  char *ptr = s;
  size_t max = n;
  int state = 0;
  char buff[8];
  int i, num_to_copy;
  int (*wctomb_fn)(char *, wchar_t, int *) = __current_ctype_locale->wctomb_fn;

  if (wctomb_fn)
    {
      while (n > 0)
	{
	  int bytes = (size_t)(wctomb_fn (buff, *pwcs, &state));
	  if (bytes == -1)
	    {
	      retval = (size_t)-1;
	      CYG_REPORT_RETVAL( retval );
	      return retval;
	    }
	  num_to_copy = ((int)n > bytes ? bytes : (int)n);
	  for (i = 0; i < num_to_copy; ++i)
	    *ptr++ = buff[i];
	  
	  if (*pwcs == 0x00)
	    {
	      retval = ptr - s - ((int)n >= bytes);
	      CYG_REPORT_RETVAL( retval );
	      return retval;
	    }
	  ++pwcs;
	  n -= num_to_copy;
	}

      retval = max;
      CYG_REPORT_RETVAL( retval );
      return retval;
    }
#endif /* CYGINT_LIBC_I18N_MB_REQUIRED */
  
  int count = 0;

  if (n != 0) {
    do {
      if ((*s++ = (char) *pwcs++) == 0)
	break;
      count++;
    } while (--n != 0);
  }
  
  retval = count;
  CYG_REPORT_RETVAL( retval );
  return retval;
} // wcstombs()

// EOF wcstombs.cxx
