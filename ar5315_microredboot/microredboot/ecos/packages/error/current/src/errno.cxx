//===========================================================================
//
//      errno.cxx
//
//      ISO C and POSIX error code
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
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          2000-04-14
// Purpose:       Provide the errno variable
// Description:   This file either provides the errno variable directly,
//                or if thread-safe, using a kernel per-thread data
//                access function
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/system.h>
#include <pkgconf/error.h>   // Configuration header

#ifdef CYGPKG_ERROR_ERRNO

// INCLUDES

#include <cyg/infra/cyg_type.h>   // Common project-wide type definitions
#include <cyg/infra/cyg_trac.h>   // Common tracing functions
#include <cyg/error/errno.h>      // Header for this file

#ifdef CYGSEM_ERROR_PER_THREAD_ERRNO
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif

// GLOBAL VARIABLES

#ifndef CYGSEM_ERROR_PER_THREAD_ERRNO

// errno is initialised to 0 at program startup - ANSI 7.1.4
Cyg_ErrNo errno = 0;

#else // ifndef CYGSEM_ERROR_PER_THREAD_ERRNO

# if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_ERROR_ERRNO_TRACE_LEVEL)
static int errno_trace = CYGNUM_ERROR_ERRNO_TRACE_LEVEL;
#  define TL1 (0 < errno_trace)
# else
#  define TL1 (0)
# endif

// FUNCTIONS

Cyg_ErrNo *
cyg_error_get_errno_p( void )
{
    Cyg_ErrNo *errno_p;

    CYG_REPORT_FUNCNAMETYPE( "cyg_error_get_errno_p", "&errno is %d");

    // set up the thread data, allocating if necessary (even though the
    // user _shouldn't_ read errno before its set, we can't stop them - and
    // ANSI prescribes it has a sensible value (0) before its set too anyway.

    Cyg_Thread *self = Cyg_Thread::self();

    errno_p = (Cyg_ErrNo *)self->get_data_ptr(CYGNUM_KERNEL_THREADS_DATA_ERRNO);

    CYG_TRACE1( TL1, "errno is %d", *errno_p );

    CYG_REPORT_RETVAL( errno_p );
    
    // return the internal data's errno
    return errno_p;
} // cyg_error_get_errno_p()

#endif // ifdef CYGSEM_ERROR_PER_THREAD_ERRNO

#endif // ifdef CYGPKG_ERROR_ERRNO

// EOF errno.cxx
