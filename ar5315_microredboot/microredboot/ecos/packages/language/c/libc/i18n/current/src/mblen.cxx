//===========================================================================
//
//      mblen.cxx
//
//      ISO standard mblen() routine 
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
// Date:          2000-11-01
// Purpose:       Provide ISO C mblen()
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code was based on newlib/libc/stdlib/mblen.c
// The following is modified from the original newlib description:
//
/*
FUNCTION
<<mblen>>---multibyte length function

INDEX
	mblen

ANSI_SYNOPSIS
	#include <stdlib.h>
	int mblen(const char *<[s]>, size_t <[n]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int mblen(<[s]>, <[n]>)
	const char *<[s]>;
	size_t <[n]>;

DESCRIPTION
When CYGINT_LIBC_I18N_MB_REQUIRED is not defined, this is a minimal ANSI-conforming 
implementation of <<mblen>>.  In this case, the
only ``multi-byte character sequences'' recognized are single bytes,
and thus <<1>> is returned unless <[s]> is the null pointer or
has a length of 0 or is the empty string.

When CYGINT_LIBC_I18N_MB_REQUIRED is defined, this routine calls the locale's LC_CTYPE mbtowc_fn to perform
the conversion, passing a state variable to allow state dependent
decoding.  The result is based on the locale setting which may
be restricted to a defined set of locales.

RETURNS
This implementation of <<mblen>> returns <<0>> if
<[s]> is <<NULL>> or the empty string; it returns <<1>> if not CYGINT_LIBC_I18N_MB_REQUIRED or
the character is a single-byte character; it returns <<-1>>
if the multi-byte character is invalid; otherwise it returns
the number of bytes in the multibyte character.

PORTABILITY
<<mblen>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<mblen>> requires no supporting OS subroutines.
*/

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
#include "internal.h"              // __current_ctype_locale definition

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif /* CYGSEM_LIBC_I18N_PER_THREAD_MB */

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_I18N_MBLEN_TRACE_LEVEL)
static int mblen_trace = CYGNUM_LIBC_I18N_MBLEN_TRACE_LEVEL;
# define TL1 (0 < mblen_trace)
#else
# define TL1 (0)
#endif

// STATICS

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
# ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
static volatile Cyg_Thread::cyg_data_index 
mblen_data_index=CYGNUM_KERNEL_THREADS_DATA_MAX;

static Cyg_Mutex mblen_data_mutex CYG_INIT_PRIORITY(LIBC);
# else
static int cyg_libc_mblen_last;
# endif
#endif

// FUNCTIONS

int
mblen ( const char *s, size_t n ) 
{
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
  int  *state;
#endif
  int   retval;
  
  CYG_REPORT_FUNCNAMETYPE( "mblen", "returning %d" );
  CYG_REPORT_FUNCARG2( "s=%08x, n=%ud", s, n );
  
  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
  Cyg_Thread *self = Cyg_Thread::self();
  
  // Get a per-thread data slot if we haven't got one already
  // Do a simple test before locking and retrying test, as this is a
  // rare situation
  if (CYGNUM_KERNEL_THREADS_DATA_MAX==mblen_data_index) {
    mblen_data_mutex.lock();
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==mblen_data_index) {
      
      // FIXME: Should use real CDL to pre-allocate a slot at compile
      // time to ensure there are enough slots
      mblen_data_index = self->new_data_index();
      
      CYG_ASSERT(mblen_data_index >= 0, "failed to allocate data index" );
    }
    mblen_data_mutex.unlock();
  } // if
  
  // we have a valid index now
  
  state = (int *)self->get_data_ptr(mblen_data_index);
#else  /* not CYGSEM_LIBC_I18N_PER_THREAD_MB */
  state = &cyg_libc_mblen_last;
#endif /* not CYGSEM_LIBC_I18N_PER_THREAD_MB */
  
  CYG_TRACE2( TL1, "Retrieved mblen_last address %08x containing %d",
	      state, *state );

  if (__current_ctype_locale->mbtowc_fn)
    {
      retval = __current_ctype_locale->mbtowc_fn (NULL, s, n, state);
      CYG_REPORT_RETVAL( retval );
      return retval;
    }
#endif /* CYGINT_LIBC_I18N_MB_REQUIRED */

  if (s == NULL || *s == '\0')
    retval = 0;
  else if (n == 0)
    retval = -1;
  else
    retval = 1;
  
  CYG_REPORT_RETVAL( retval );
  return retval;
} // mblen()

// EOF mblen.cxx
