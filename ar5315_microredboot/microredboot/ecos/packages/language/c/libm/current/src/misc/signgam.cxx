//===========================================================================
//
//      signgam.cxx
//
//      Support sign of the gamma*() functions in Math library
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
// Author(s):   jlarmour
// Contributors:  jlarmour
// Date:        1998-02-13
// Purpose:     
// Description: Contains the accessor functions to get and set the stored sign
//              of the gamma*() functions in the math library
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Tracing macros

#include <math.h>                  // Main header for math library

#ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS
# include <pkgconf/kernel.h>        // Kernel configuration
# include <cyg/kernel/thread.hxx>   // Kernel thread header
# include <cyg/kernel/thread.inl>   // and its associated inlines
# include <cyg/kernel/mutex.hxx>    // We need mutexes too
#endif


// GLOBAL VARIABLES

#ifndef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

int signgam = 0;

#else  // ifndef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

static Cyg_Mutex cyg_libm_signgam_thread_data_index_mutex;
static cyg_count32 cyg_libm_signgam_thread_data_index = CYGNUM_KERNEL_THREADS_DATA_MAX;

# if defined(CYGDBG_USE_TRACING) && \
     defined(CYGNUM_LIBM_SIGNGAM_TRACE_LEVEL)
static int libm_signgam_trace = CYGNUM_LIBM_SIGNGAM_TRACE_LEVEL;
#  define TL1 (0 < libm_signgam_trace )
# else
#  define TL1 (0)
# endif

#endif // ifndef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

// FUNCTIONS

#ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

int *
cyg_libm_get_signgam_p( void )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_ADDRWORD *current_val_p;

    CYG_REPORT_FUNCNAMETYPE( "Cyg_libm_get_signgam_p", "&signgam is %08x" );

    // First check if this is the first thread to get here, and if so,
    // initialise the thread data index

    if (cyg_libm_signgam_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
    {
        CYG_TRACE0( TL1, "Index unset, locking mutex to allocate" );

        // Lock mutex and then check again - less overhead for normal
        // execution path
        cyg_libm_signgam_thread_data_index_mutex.lock();

        if (cyg_libm_signgam_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
        {
            CYG_TRACE0( TL1, "Allocating index" );
            cyg_libm_signgam_thread_data_index = self->new_data_index();

        } // if

        cyg_libm_signgam_thread_data_index_mutex.unlock();
      
    } // if

    CYG_TRACE0( TL1, "Index set, now fetching per-thread data" );

    // If we're here, then the index is set up

    current_val_p = self->get_data_ptr( cyg_libm_signgam_thread_data_index );

    CYG_REPORT_RETVAL( current_val_p );
    return (int *) current_val_p;
} // cyg_libm_get_signgam_p()

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS


#endif // ifdef CYGPKG_LIBM

// EOF signgam.cxx
