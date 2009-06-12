//===========================================================================
//
//      rand.cxx
//
//      ISO and POSIX 1003.1 standard random number generation functions
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
// Contributors:  
// Date:          2000-04-30
// Purpose:       Provides ISO C rand() and srand() functions, along with
//                POSIX 1003.1 rand_r() function
// Description:   This implements rand() and srand() of section 7.10.2.1 of
//                the ISO C standard. Also rand_r() defined in section 8.3.8
//                of the POSIX 1003.1 standard
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_stdlib.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <stdlib.h>                // Header for all stdlib functions
                                   // (like this one)

#ifdef CYGSEM_LIBC_PER_THREAD_RAND
# include <pkgconf/kernel.h>       // kernel configuration
# include <cyg/kernel/thread.hxx>  // per-thread data
# include <cyg/kernel/thread.inl>  // per-thread data
# include <cyg/kernel/mutex.hxx>   // mutexes
#endif

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_RAND_TRACE_LEVEL)
static int rand_trace = CYGNUM_LIBC_RAND_TRACE_LEVEL;
# define TL1 (0 < rand_trace)
#else
# define TL1 (0)
#endif


// STATICS

#ifdef CYGSEM_LIBC_PER_THREAD_RAND
static Cyg_Thread::cyg_data_index
rand_data_index=CYGNUM_KERNEL_THREADS_DATA_MAX;

static Cyg_Mutex rand_data_mutex CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC);
#else
static unsigned int cyg_libc_rand_seed = CYGNUM_LIBC_RAND_SEED;
#endif

// FUNCTIONS

int
rand( void )
{
    unsigned int *seed_p;
    int retval;

    CYG_REPORT_FUNCNAMETYPE( "rand", "returning %d" );

    // get seed for this thread (if relevant )
#ifdef CYGSEM_LIBC_PER_THREAD_RAND
    Cyg_Thread *self = Cyg_Thread::self();

    // Get a per-thread data slot if we haven't got one already
    // Do a simple test before locking and retrying test, as this is a
    // rare situation
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==rand_data_index) {
        rand_data_mutex.lock();
        if (CYGNUM_KERNEL_THREADS_DATA_MAX==rand_data_index) {

            // FIXME: Should use real CDL to pre-allocate a slot at compile
            // time to ensure there are enough slots
            rand_data_index = self->new_data_index();
            CYG_ASSERT(rand_data_index >= 0, "failed to allocate data index" );
            
            // Initialize seed
            self->set_data(rand_data_index, CYGNUM_LIBC_RAND_SEED);
        }
        rand_data_mutex.unlock();
    } // if

    // we have a valid index now

    seed_p = (unsigned int *)self->get_data_ptr(rand_data_index);
#else
    seed_p = &cyg_libc_rand_seed;
#endif

    CYG_TRACE2( TL1, "Retrieved seed address %08x containing %d",
                seed_p, *seed_p );
    CYG_CHECK_DATA_PTR( seed_p, "Help! Returned address of seed is invalid!" );

    retval = rand_r( seed_p );

    CYG_REPORT_RETVAL( retval );

    return retval;

} // rand()


int 
rand_r( unsigned int *seed )
{
    int retval;
    
    CYG_REPORT_FUNCNAMETYPE( "rand_r", "returning %d" );

    CYG_CHECK_DATA_PTR( seed, "pointer to seed invalid!" );

#if defined(CYGIMP_LIBC_RAND_SIMPLEST)

    // This algorithm sucks in the lower bits

    *seed = (*seed * 1103515245) + 12345; // permutate seed
    
    retval = (int)( *seed & RAND_MAX );

#elif defined(CYGIMP_LIBC_RAND_SIMPLE1)

    // The above algorithm sucks in the lower bits, so we shave them off
    // and repeat a couple of times to make it up

    unsigned int s=*seed;
    unsigned int uret;

    s = (s * 1103515245) + 12345; // permutate seed
    // Only use top 11 bits
    uret = s & 0xffe00000;
    
    s = (s * 1103515245) + 12345; // permutate seed
    // Only use top 14 bits
    uret += (s & 0xfffc0000) >> 11;
    
    s = (s * 1103515245) + 12345; // permutate seed
    // Only use top 7 bits
    uret += (s & 0xfe000000) >> (11+14);
    
    retval = (int)(uret & RAND_MAX);
    *seed = s;

#elif defined(CYGIMP_LIBC_RAND_KNUTH1)

// This is the code supplied in Knuth Vol 2 section 3.6 p.185 bottom

#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ

    *seed = AA*(*seed % QQ) - RR*(unsigned int)(*seed/QQ);
    if (*seed < 0)
        *seed += MM;

    retval = (int)( *seed & RAND_MAX );

#else
# error No valid implementation for rand()!
#endif

    CYG_REPORT_RETVAL( retval );

    return retval;
   
} // rand_r()


void
srand( unsigned int seed )
{
    CYG_REPORT_FUNCNAME( "srand" );

    CYG_REPORT_FUNCARG1DV( (int)seed );

    // get seed for this thread ( if relevant )
#ifdef CYGSEM_LIBC_PER_THREAD_RAND
    Cyg_Thread *self = Cyg_Thread::self();

    // Get a per-thread data slot if we haven't got one already
    // Do a simple test before locking and retrying test, as this is a
    // rare situation
    if (CYGNUM_KERNEL_THREADS_DATA_MAX==rand_data_index) {
        rand_data_mutex.lock();
        if (CYGNUM_KERNEL_THREADS_DATA_MAX==rand_data_index) {

            // FIXME: Should use real CDL to pre-allocate a slot at compile
            // time to ensure there are enough slots
            rand_data_index = self->new_data_index();
            CYG_ASSERT(rand_data_index >= 0, "failed to allocate data index" );
        }
        rand_data_mutex.unlock();
    } // if

    // we have a valid index now

    self->set_data(rand_data_index, (CYG_ADDRWORD) seed);
#else
    cyg_libc_rand_seed = seed;
#endif

    CYG_REPORT_RETURN();

} // srand()

// EOF rand.cxx
