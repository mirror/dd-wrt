//===========================================================================
//
//      wctomb.cxx
//
//      ISO standard wctomb() routine 
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
// Purpose:       Provide ISO C wctomb() routine
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code was based on newlib/libc/stdlib/wctomb.c
// The following is modified from the original newlib description:
//
/*
FUNCTION
<<wctomb>>---wide char to multibyte converter

INDEX
	wctomb

ANSI_SYNOPSIS
	#include <stdlib.h>
	int wctomb(char *<[s]>, wchar_t <[wchar]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int wctomb(<[s]>, <[wchar]>)
	char *<[s]>;
	wchar_t <[wchar]>;

DESCRIPTION
When CYG_LIBC_MB_CAPABLE is not defined, this is a minimal ANSI-conforming 
implementation of <<wctomb>>.  The
only ``wide characters'' recognized are single bytes,
and they are ``converted'' to themselves.  

When CYG_LIBC_MB_CAPABLE is defined, this routine calls the LC_CTYPE locale wctomb_fn to perform
the conversion, passing a state variable to allow state dependent
decoding.  The result is based on the locale setting which may
be restricted to a defined set of locales.

Each call to <<wctomb>> modifies <<*<[s]>>> unless <[s]> is a null
pointer or CYG_LIBC_MB_CAPABLE is defined and <[wchar]> is invalid.

RETURNS
This implementation of <<wctomb>> returns <<0>> if
<[s]> is <<NULL>>; it returns <<-1>> if CYG_LIBC_MB_CAPABLE is enabled
and the wchar is not a valid multi-byte character, it returns <<1>>
if CYG_LIBC_MB_CAPABLE is not defined or the wchar is in reality a single
byte character, otherwise it returns the number of bytes in the
multi-byte character.

PORTABILITY
<<wctomb>> is required in the ANSI C standard.  However, the precise
effects vary with the locale.

<<wctomb>> requires no supporting OS subroutines.
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

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_I18N_WCTOMB_TRACE_LEVEL)
static int wctomb_trace = CYGNUM_LIBC_I18N_WCTOMB_TRACE_LEVEL;
# define TL1 (0 < wctomb_trace)
#else
# define TL1 (0)
#endif

// STATICS

#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
# ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
static volatile Cyg_Thread::cyg_data_index
wctomb_data_index=CYGNUM_KERNEL_THREADS_DATA_MAX;
static Cyg_Mutex wctomb_data_mutex CYG_INIT_PRIORITY(LIBC);
# else
static int cyg_libc_wctomb_last;
# endif
#endif

int
wctomb ( char *s, const wchar_t wchar ) 
{
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
  int  *state;
#endif
  int   retval;
  
  CYG_REPORT_FUNCNAMETYPE( "wctomb", "returning %d" );
  CYG_REPORT_FUNCARG2( "s=%08x, wchar=%08x", s, wchar );
  
  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
  
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED

#ifdef CYGSEM_LIBC_I18N_PER_THREAD_MB
  Cyg_Thread *self = Cyg_Thread::self();
  
  // Get a per-thread data slot if we haven't got one already
  // Do a simple test before locking and retrying test, as this is a
  // rare situation
  if (CYGNUM_KERNEL_THREADS_DATA_MAX==wctomb_data_index) {
    wctomb_data_mutex.lock();
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==wctomb_data_index) {
      
      // FIXME: Should use real CDL to pre-allocate a slot at compile
      // time to ensure there are enough slots
      wctomb_data_index = self->new_data_index();
      
      CYG_ASSERT(wctomb_data_index >= 0, "failed to allocate data index" );
    }
    wctomb_data_mutex.unlock();
  } // if
  
  // we have a valid index now
  
  state = (int *)self->get_data_ptr(wctomb_data_index);
#else  /* not CYGSEM_LIBC_I18N_PER_THREAD_MB */
  state = &cyg_libc_wctomb_last;
#endif /* not CYGSEM_LIBC_I18N_PER_THREAD_MB */
  
  CYG_TRACE2( TL1, "Retrieved wctomb_last address %08x containing %d",
	      state, *state );

  if (__current_ctype_locale->wctomb_fn)
    {
      retval = __current_ctype_locale->wctomb_fn (s, wchar, state);
      CYG_REPORT_RETVAL( retval );
      return retval;
    }
#endif /* CYGINT_LIBC_I18N_MB_REQUIRED */

  if (s == NULL)
    retval = 0;
  else
    {
      *s = (char) wchar;
      retval = 1;
    }

  CYG_REPORT_RETVAL( retval );
  return retval;

} // wctomb()

// EOF wctomb.cxx
