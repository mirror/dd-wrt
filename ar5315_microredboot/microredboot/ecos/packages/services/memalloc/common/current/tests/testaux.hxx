#ifndef CYGONCE_MEMALLOC_TESTS_TESTAUX_HXX
#define CYGONCE_MEMALLOC_TESTS_TESTAUX_HXX

//==========================================================================
//
//        testaux.hxx
//
//        Auxiliary test header file
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     dsm
// Contributors:  dsm
// Date:          1998-03-09
// Description:
//     Defines some convenience functions to get us going.  In
//     particular this file reserves space for NTHREADS threads,
//     which can be created by calls to aux_new_thread()
//     It also defines a CHECK function.
//
//####DESCRIPTIONEND####


static inline void *operator new(size_t size, void *ptr) { return ptr; };


#include <pkgconf/hal.h>

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
externC void
cyg_hal_invoke_constructors();
#endif

#ifdef NTHREADS

#ifndef STACKSIZE
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL*2
#endif

static Cyg_Thread *thread[NTHREADS];

typedef CYG_WORD64 CYG_ALIGNMENT_TYPE;

static CYG_ALIGNMENT_TYPE thread_obj[NTHREADS] [
   (sizeof(Cyg_Thread)+sizeof(CYG_ALIGNMENT_TYPE)-1)
     / sizeof(CYG_ALIGNMENT_TYPE)                     ];

static CYG_ALIGNMENT_TYPE stack[NTHREADS] [
   (STACKSIZE+sizeof(CYG_ALIGNMENT_TYPE)-1)
     / sizeof(CYG_ALIGNMENT_TYPE)                     ];

static volatile int nthreads = 0;

static Cyg_Thread *new_thread(cyg_thread_entry *entry, CYG_ADDRWORD data)
{
    int _nthreads = nthreads++;

    CYG_ASSERT(_nthreads < NTHREADS, 
               "Attempt to create more than NTHREADS threads");

    thread[_nthreads] = new( (void *)&thread_obj[_nthreads] )
        Cyg_Thread(CYG_SCHED_DEFAULT_INFO,
                   entry, data, 
                   NULL,                // no name
                   (CYG_ADDRESS)stack[_nthreads], STACKSIZE );

    thread[_nthreads]->resume();

    return thread[_nthreads];
}
#endif // defined(NTHREADS)

#define CHECK(b) CYG_TEST_CHECK(b,#b)

#endif // ifndef CYGONCE_KERNEL_TESTS_TESTAUX_HXX

// End of testaux.hxx
