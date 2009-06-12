//===========================================================================
//
//      compatmode.cxx
//
//      Compatibility mode setting for Math library
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
// Description: Contains the accessor functions to get and set the standards
//              compatibility mode of the Math library
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

#ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE
# include <pkgconf/kernel.h>        // Kernel configuration
# include <cyg/kernel/thread.hxx>   // Kernel thread header
# include <cyg/kernel/thread.inl>   // and its associated inlines
# include <cyg/kernel/mutex.hxx>    // We need mutexes too
#endif

// GLOBAL VARIABLES

#ifndef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

# ifdef CYGSEM_LIBM_COMPAT_IEEE_ONLY
Cyg_libm_compat_t cygvar_libm_compat_mode = CYGNUM_LIBM_COMPAT_IEEE;
# else
Cyg_libm_compat_t cygvar_libm_compat_mode = CYGNUM_LIBM_COMPAT_DEFAULT;
# endif

#else  // ifndef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

static Cyg_Mutex cyg_libm_mode_thread_data_index_mutex;
static cyg_count32 cyg_libm_mode_thread_data_index = CYGNUM_KERNEL_THREADS_DATA_MAX;


# if defined(CYGDBG_USE_TRACING) && \
     defined(CYGNUM_LIBM_COMPATMODE_TRACE_LEVEL)
static int libm_compatmode_trace = CYGNUM_LIBM_COMPATMODE_TRACE_LEVEL;
#  define TL1 (0 < libm_compatmode_trace )
# else
#  define TL1 (0)
# endif

// FUNCTIONS


Cyg_libm_compat_t
cyg_libm_get_compat_mode( void )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_ADDRWORD current_val;

    CYG_REPORT_FUNCNAMETYPE( "Cyg_libm_get_compat_mode", "mode is %d" );

    // First check if this is the first thread to get here, and if so,
    // initialise the thread data index

    if (cyg_libm_mode_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
    {
        CYG_TRACE0( TL1, "Index unset, locking mutex to allocate" );

        // Lock mutex and then check again - less overhead for normal
        // execution path
        cyg_libm_mode_thread_data_index_mutex.lock();

        if (cyg_libm_mode_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
        {
            CYG_TRACE0( TL1, "Allocating index" );
            cyg_libm_mode_thread_data_index = self->new_data_index();

        } // if

        cyg_libm_mode_thread_data_index_mutex.unlock();
      
        self->set_data( cyg_libm_mode_thread_data_index,
                        CYGNUM_LIBM_COMPAT_DEFAULT );

        // But if we've gone through this, then we know what the mode is!
        CYG_REPORT_RETVAL( CYGNUM_LIBM_COMPAT_DEFAULT );
        return CYGNUM_LIBM_COMPAT_DEFAULT;
    } // if

    CYG_TRACE0( TL1, "Index set, now fetching per-thread data" );

    // If we're here, then the index is set up

    current_val = self->get_data( cyg_libm_mode_thread_data_index );

    // If its at the "default" setting, then return the default
    if (current_val == CYGNUM_LIBM_COMPAT_UNINIT)
    {
        CYG_REPORT_RETVAL( CYGNUM_LIBM_COMPAT_DEFAULT );
        return CYGNUM_LIBM_COMPAT_DEFAULT;
    } // if
    
    CYG_REPORT_RETVAL( current_val );
    return (Cyg_libm_compat_t) current_val;
} // cyg_libm_get_compat_mode()


Cyg_libm_compat_t
cyg_libm_set_compat_mode( Cyg_libm_compat_t new_mode )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_ADDRWORD current_val;

    CYG_REPORT_FUNCNAMETYPE( "Cyg_libm_set_compat_mode", "old mode was %d" );
    CYG_REPORT_FUNCARG1DV( new_mode );

    // First check if this is the first thread to get here, and if so,
    // initialise the thread data index

    if (cyg_libm_mode_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
    {
        CYG_TRACE0( TL1, "Index unset, locking mutex to allocate" );

        // Lock mutex and then check again - less overhead for normal
        // execution path
        cyg_libm_mode_thread_data_index_mutex.lock();

        if (cyg_libm_mode_thread_data_index == CYGNUM_KERNEL_THREADS_DATA_MAX)
        {
            CYG_TRACE0( TL1, "Allocating index" );
            cyg_libm_mode_thread_data_index = self->new_data_index();

        } // if

        cyg_libm_mode_thread_data_index_mutex.unlock();
      
        self->set_data( cyg_libm_mode_thread_data_index,
                        new_mode );
        
        // But if we've gone through this, then we know what the old mode was!

        CYG_REPORT_RETVAL( CYGNUM_LIBM_COMPAT_UNINIT );
        return CYGNUM_LIBM_COMPAT_UNINIT;
    
    } // if

    // If we're here, then the index is set up

    CYG_TRACE0( TL1, "Index set, now fetching per-thread data" );

    current_val = self->get_data( cyg_libm_mode_thread_data_index );

    self->set_data( cyg_libm_mode_thread_data_index,
                    new_mode );
    
    CYG_REPORT_RETVAL( current_val );
    return (Cyg_libm_compat_t) current_val;
    
} // cyg_libm_set_compat_mode()

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE


#endif // ifdef CYGPKG_LIBM

// EOF compatmode.cxx
