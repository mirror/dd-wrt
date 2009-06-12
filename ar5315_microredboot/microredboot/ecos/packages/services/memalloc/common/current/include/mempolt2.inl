#ifndef CYGONCE_MEMALLOC_MEMPOLT2_INL
#define CYGONCE_MEMALLOC_MEMPOLT2_INL

//==========================================================================
//
//      mempolt2.inl
//
//      Mempolt2 (Memory pool template) class declarations
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
// Author(s):    hmt
// Contributors: jlarmour
// Date:         2000-06-12
// Purpose:      Define Mempolt2 class interface
// Description:  The class defined here provides the APIs for thread-safe,
//               kernel-savvy memory managers; make a class with the
//               underlying allocator as the template parameter.
// Usage:        #include <cyg/memalloc/mempolt2.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_ass.h>    // assertion support
#include <cyg/infra/cyg_trac.h>   // tracing support
#include <cyg/kernel/thread.inl>  // implementation eg. Cyg_Thread::self();
#include <cyg/kernel/sched.inl>   // implementation eg. Cyg_Scheduler::lock();

// -------------------------------------------------------------------------
// Constructor; we _require_ these arguments and just pass them through to
// the implementation memory pool in use.
template <class T>
Cyg_Mempolt2<T>::Cyg_Mempolt2(
    cyg_uint8 *base,
    cyg_int32 size,
    CYG_ADDRWORD arg_thru)              // Constructor
    : pool( base, size, arg_thru )
{
}


template <class T>
Cyg_Mempolt2<T>::~Cyg_Mempolt2()  // destructor
{
    // Prevent preemption
    Cyg_Scheduler::lock();
            
    while ( ! queue.empty() ) {
        Cyg_Thread *thread = queue.dequeue();
        thread->set_wake_reason( Cyg_Thread::DESTRUCT );
        thread->wake();
    }

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();    
}
        
// -------------------------------------------------------------------------
// get some memory; wait if none available
template <class T>
inline cyg_uint8 *
Cyg_Mempolt2<T>::alloc( cyg_int32 size )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_uint8 *ret;
    ret = pool.try_alloc( size );
    if ( ret ) {
        Cyg_Scheduler::unlock();
        CYG_ASSERTCLASS( this, "Bad this pointer");
        CYG_REPORT_RETVAL( ret );
        return ret;
    }

    Cyg_Thread *self = Cyg_Thread::self();

    Mempolt2WaitInfo waitinfo( size );

    CYG_MEMALLOC_FAIL(size);

    self->set_wait_info( (CYG_ADDRWORD)&waitinfo );
    self->set_sleep_reason( Cyg_Thread::WAIT );
    self->sleep();
    queue.enqueue( self );

    CYG_ASSERT( 1 == Cyg_Scheduler::get_sched_lock(),
                "Called with non-zero scheduler lock");
        
    // Unlock scheduler and allow other threads to run
    Cyg_Scheduler::unlock();

    cyg_bool result = true; // just used as a flag here
    switch( self->get_wake_reason() )
    {
    case Cyg_Thread::DESTRUCT:
    case Cyg_Thread::BREAK:
        result = false;
        break;
        
    case Cyg_Thread::EXIT:            
        self->exit();
        break;
        
    default:
        break;
    }

    if ( ! result )
        ret = NULL;
    else
        ret = waitinfo.addr;

    CYG_ASSERT( (!result) || (NULL != ret), "Good result but no alloc!" );
    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_REPORT_RETVAL( ret );
    return ret;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
// -------------------------------------------------------------------------
// get some memory with a timeout
template <class T>
inline cyg_uint8 *
Cyg_Mempolt2<T>::alloc( cyg_int32 size, cyg_tick_count abs_timeout )
{
    CYG_REPORT_FUNCTION();
    
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_uint8 *ret;
    ret = pool.try_alloc( size );
    if ( ret ) {
        Cyg_Scheduler::unlock();
        CYG_ASSERTCLASS( this, "Bad this pointer");
        CYG_REPORT_RETVAL( ret );
        return ret;
    }

    Cyg_Thread *self = Cyg_Thread::self();

    Mempolt2WaitInfo waitinfo( size );

    self->set_timer( abs_timeout, Cyg_Thread::TIMEOUT );

    // If the timeout is in the past, the wake reason will have been set to
    // something other than NONE already. If so, skip the wait and go
    // straight to unlock.
    
    if( Cyg_Thread::NONE == self->get_wake_reason() ) {

	CYG_MEMALLOC_FAIL(size);

        self->set_wait_info( (CYG_ADDRWORD)&waitinfo );
        self->sleep();
        queue.enqueue( self );
    }

    CYG_ASSERT( 1 == Cyg_Scheduler::get_sched_lock(),
                "Called with non-zero scheduler lock");
        
    // Unlock scheduler and allow other threads to run
    Cyg_Scheduler::unlock();

    // clear the timer; if it actually fired, no worries.
    self->clear_timer();

    cyg_bool result = true; // just used as a flag here
    switch( self->get_wake_reason() )
    {
    case Cyg_Thread::TIMEOUT:
        result = false;
        break;
            
    case Cyg_Thread::DESTRUCT:
    case Cyg_Thread::BREAK:
        result = false;
        break;
            
    case Cyg_Thread::EXIT:            
        self->exit();
        break;

    default:
        break;
    }

    if ( ! result )
        ret = NULL;
    else
        ret = waitinfo.addr;

    CYG_ASSERT( (!result) || (NULL != ret), "Good result but no alloc!" );
    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_REPORT_RETVAL( ret );
    return ret;
}
#endif 

// -------------------------------------------------------------------------
// get some memory, return NULL if none available
template <class T>
inline cyg_uint8 *
Cyg_Mempolt2<T>::try_alloc( cyg_int32 size )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_uint8 *ret = pool.try_alloc( size );

    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_MEMALLOC_FAIL_TEST(ret==NULL, size);

    return ret;
}
    
    
// -------------------------------------------------------------------------
// resize existing allocation, if oldsize is non-NULL, previous
// allocation size is placed into it. If previous size not available,
// it is set to 0. NB previous allocation size may have been rounded up.
// Occasionally the allocation can be adjusted *backwards* as well as,
// or instead of forwards, therefore the address of the resized
// allocation is returned, or NULL if no resizing was possible.
// Note that this differs from ::realloc() in that no attempt is
// made to call malloc() if resizing is not possible - that is left
// to higher layers. The data is copied from old to new though.
// The effects of alloc_ptr==NULL or newsize==0 are undefined
template <class T>
cyg_uint8 *
Cyg_Mempolt2<T>::resize_alloc( cyg_uint8 *alloc_ptr, cyg_int32 newsize,
                               cyg_int32 *oldsize )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_uint8 *ret = pool.resize_alloc( alloc_ptr, newsize, oldsize );

    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_MEMALLOC_FAIL_TEST(ret==NULL, newsize);

    return ret;
}
    
    
// -------------------------------------------------------------------------
// free the memory back to the pool
template <class T>
cyg_bool
Cyg_Mempolt2<T>::free( cyg_uint8 *p, cyg_int32 size )
{
    CYG_REPORT_FUNCTION();
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_int32 ret = pool.free( p, size );

    // anyone waiting?
    if ( !(queue.empty()) ) {
        Mempolt2WaitInfo *p;
        Cyg_Thread     *thread;

#ifdef CYGIMP_MEM_T_ONEFREE_TO_ONEALLOC
        thread = queue.dequeue();
        p = (Mempolt2WaitInfo *)(thread->get_wait_info());
        CYG_ASSERT( NULL == p->addr, "Thread already awoken?" );

        cyg_uint8 *mem;
        mem = pool.try_alloc( p->size );
        CYG_ASSERT( NULL != mem, "That should have succeeded" );
        thread->set_wake_reason( Cyg_Thread::DONE );
        thread->wake();
        // return the successful value to it
        p->addr = mem;
#else
        Cyg_ThreadQueue holding;
        do {
            thread = queue.dequeue();
            p = (Mempolt2WaitInfo *)(thread->get_wait_info());
            CYG_ASSERT( NULL == p->addr, "Thread already awoken?" );

            cyg_uint8 *mem;
            if ( NULL != (mem = pool.try_alloc( p->size )) ) {
                // success!  awaken the thread
                thread->set_wake_reason( Cyg_Thread::DONE );
                thread->wake();
                // return the successful value to it
                p->addr = mem;
            }
            else {
                // preserve the entry on the holding queue
                holding.enqueue( thread );
            }
        } while ( !(queue.empty()) );
            
        // Now re-queue the unaffected threads back into the pool queue
        // (no pun intended)
        while ( !(holding.empty()) ) {
            queue.enqueue( holding.dequeue() );
        }
#endif // CYGIMP_MEM_T_ONEFREE_TO_ONEALLOC
    }
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( ret );
    return ret;
}

// -------------------------------------------------------------------------
// Get memory pool status
// Needs atomicity protection (maybe)
template <class T>
inline void
Cyg_Mempolt2<T>::get_status( cyg_mempool_status_flag_t flags,
                             Cyg_Mempool_Status &status )
{
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    if (0 != (flags & CYG_MEMPOOL_STAT_WAITING)) {
        status.waiting = (0 == queue.empty());
    }
    pool.get_status(flags, status);

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
}

// -------------------------------------------------------------------------
// debugging/assert function

#ifdef CYGDBG_USE_ASSERTS

template <class T>
inline cyg_bool
Cyg_Mempolt2<T>::check_this(cyg_assert_class_zeal zeal) const
{
    CYG_REPORT_FUNCTION();
        
    if ( Cyg_Thread::DESTRUCT == Cyg_Thread::self()->get_wake_reason() )
        // then the whole thing is invalid, and we know it.
        // so return OK, since this check should NOT make an error.
        return true;

    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;

    return true;
}
#endif

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_MEMALLOC_MEMPOLT2_INL
// EOF mempolt2.inl
