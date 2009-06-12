//==========================================================================
//
//      sem.cxx
//
//      POSIX semaphore implementation
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-03-27
// Purpose:             POSIX semaphore implementation
// Description:         This file contains the implementation of the POSIX semaphore
//                      functions.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <semaphore.h>                  // our header

#include "pprivate.h"                   // POSIX private header

#include <cyg/kernel/thread.hxx>        // Kernel threads

#include <cyg/kernel/thread.inl>        // Cyg_ThreadQueue::empty()

#include <cyg/kernel/sema.hxx>          // Kernel semaphores

// -------------------------------------------------------------------------
// Internal definitions

// Handle entry to a pthread package function. 
#define SEMA_ENTRY() CYG_REPORT_FUNCTYPE( "returning %d" );

// Do a semaphore package defined return. This requires the error code
// to be placed in errno, and if it is non-zero, -1 returned as the
// result of the function. This also gives us a place to put any
// generic tidyup handling needed for things like signal delivery and
// cancellation.
#define SEMA_RETURN(err)                        \
CYG_MACRO_START                                 \
    int __retval = 0;                           \
    if( err != 0 ) __retval = -1, errno = err;  \
    CYG_REPORT_RETVAL( __retval );              \
    return __retval;                            \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// new operator to allow us to invoke the Cyg_Thread constructor on the
// user's semaphore object.

inline void *operator new(size_t size,  void *ptr) { return (void *)ptr; };

// -------------------------------------------------------------------------
// Initialize semaphore to value.
// pshared is not supported under eCos.

externC int sem_init  (sem_t *sem, int pshared, unsigned int value)
{
    SEMA_ENTRY();

    if( value > SEM_VALUE_MAX )
        SEMA_RETURN(EINVAL);

    Cyg_Counting_Semaphore *sema;

    sema = new((void *)sem) Cyg_Counting_Semaphore(value);

    sema=sema;
    
    SEMA_RETURN(0);
}

// -------------------------------------------------------------------------
// Destroy the semaphore.

externC int sem_destroy  (sem_t *sem)
{
    SEMA_ENTRY();

    Cyg_Counting_Semaphore *sema = (Cyg_Counting_Semaphore *)sem;

    // Check that the semaphore has no waiters
    if( sema->waiting() )
        SEMA_RETURN(EBUSY);

    // Call the destructor
    sema->~Cyg_Counting_Semaphore();
    
    SEMA_RETURN(0);
}

// -------------------------------------------------------------------------
// Decrement value if >0 or wait for a post.

externC int sem_wait  (sem_t *sem)
{
    int retval = 0;
    
    SEMA_ENTRY();

#ifdef CYGPKG_POSIX_PTHREAD
    // check for cancellation first.
    pthread_testcancel();
#endif

    Cyg_Counting_Semaphore *sema = (Cyg_Counting_Semaphore *)sem;

    if( !sema->wait() ) retval = EINTR;
    
#ifdef CYGPKG_POSIX_PTHREAD
    // check if we were woken because we were being cancelled
    pthread_testcancel();
#endif

    SEMA_RETURN(retval);
}

// -------------------------------------------------------------------------
// Decrement value if >0, return -1 if not.

externC int sem_trywait  (sem_t *sem)
{
    int retval = 0;
    
    SEMA_ENTRY();

    Cyg_Counting_Semaphore *sema = (Cyg_Counting_Semaphore *)sem;

    if( !sema->trywait() ) retval = EAGAIN;
    
    SEMA_RETURN(retval);
}

// -------------------------------------------------------------------------
// Increment value and wake a waiter if one is present.

externC int sem_post  (sem_t *sem)
{
    SEMA_ENTRY();

    Cyg_Counting_Semaphore *sema = (Cyg_Counting_Semaphore *)sem;

    sema->post();
    
    SEMA_RETURN(0);
}
    

// -------------------------------------------------------------------------
// Get current value

externC int sem_getvalue  (sem_t *sem, int *sval)
{
    SEMA_ENTRY();

    Cyg_Counting_Semaphore *sema = (Cyg_Counting_Semaphore *)sem;

    *sval = sema->peek();

    CYG_REPORT_RETVAL( 0 );
    return 0;
}

// -------------------------------------------------------------------------
// Open an existing named semaphore, or create it.

externC sem_t *sem_open  (const char *name, int oflag, ...)
{
    SEMA_ENTRY();

    errno = ENOSYS;

    CYG_REPORT_RETVAL( SEM_FAILED );
    return SEM_FAILED;
}

// -------------------------------------------------------------------------
// Close descriptor for semaphore.

externC int sem_close  (sem_t *sem)
{
    SEMA_ENTRY();

    SEMA_RETURN(ENOSYS);
}   

// -------------------------------------------------------------------------
// Remove named semaphore

externC int sem_unlink  (const char *name)
{
    SEMA_ENTRY();

    SEMA_RETURN(ENOSYS);
}    

// -------------------------------------------------------------------------
// EOF sem.cxx
