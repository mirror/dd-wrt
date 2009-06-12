#ifndef CYGONCE_KERNEL_MEMPOOLT_INL
#define CYGONCE_KERNEL_MEMPOOLT_INL

//==========================================================================
//
//      mempoolt.inl
//
//      Mempoolt (Memory pool template) class declarations
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-02-10
// Purpose:     Define Mempoolt class interface

// Description: The class defined here provides the APIs for thread-safe,
//              kernel-savvy memory managers; make a class with the
//              underlying allocator as the template parameter.
// Usage:       #include <cyg/kernel/mempoolt.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/thread.inl>  // implementation eg. Cyg_Thread::self();
#include <cyg/kernel/sched.inl>   // implementation eg. Cyg_Scheduler::lock();

// -------------------------------------------------------------------------
// Constructor; we _require_ these arguments and just pass them through to
// the implementation memory pool in use.
template <class T>
Cyg_Mempoolt<T>::Cyg_Mempoolt(
    cyg_uint8 *base,
    cyg_int32 size,
    CYG_ADDRWORD arg_thru)              // Constructor
    : pool( base, size, arg_thru )
{
}


template <class T>
Cyg_Mempoolt<T>::~Cyg_Mempoolt()  // destructor
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
Cyg_Mempoolt<T>::alloc( cyg_int32 size )
{
    CYG_REPORT_FUNCTION();
        
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    // Loop while we got no memory, sleeping each time around the
    // loop. This copes with the possibility of a higher priority thread
    // grabbing the freed storage between the wakeup in free() and this
    // thread actually starting.
    cyg_uint8 *ret;
    cyg_bool result = true;
    while( result && (NULL == (ret = pool.alloc( size ))) ) {

	CYG_MEMALLOC_FAIL(size);

        self->set_sleep_reason( Cyg_Thread::WAIT );
        self->sleep();
        queue.enqueue( self );

        CYG_ASSERT( 1 == Cyg_Scheduler::get_sched_lock(),
                    "Called with non-zero scheduler lock");
        
        // Unlock scheduler and allow other threads to run
        Cyg_Scheduler::unlock();
        Cyg_Scheduler::lock();

        CYG_ASSERTCLASS( this, "Bad this pointer");        

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
    }
    CYG_ASSERTCLASS( this, "Bad this pointer");

    if ( ! result )
        ret = NULL;

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( ret );
    return ret;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
// -------------------------------------------------------------------------
// get some memory with a timeout
template <class T>
inline cyg_uint8 *
Cyg_Mempoolt<T>::alloc( cyg_int32 size, cyg_tick_count abs_timeout )
{
    CYG_REPORT_FUNCTION();
        
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    // Loop while we got no memory, sleeping each time around the
    // loop. This copes with the possibility of a higher priority thread
    // grabbing the freed storage between the wakeup in free() and this
    // thread actually starting.
    cyg_uint8 *ret;
    cyg_bool result = true;
    // Set the timer _once_ outside the loop.
    self->set_timer( abs_timeout, Cyg_Thread::TIMEOUT );

    // If the timeout is in the past, the wake reason will have been
    // set to something other than NONE already. Set the result false
    // to force an immediate return.
    
    if( self->get_wake_reason() != Cyg_Thread::NONE )
        result = false;
            
    while( result && (NULL == (ret = pool.alloc( size ))) ) {
	CYG_MEMALLOC_FAIL(size);

        self->set_sleep_reason( Cyg_Thread::TIMEOUT );
        self->sleep();
        queue.enqueue( self );

        CYG_ASSERT( 1 == Cyg_Scheduler::get_sched_lock(),
                    "Called with non-zero scheduler lock");
        
        // Unlock scheduler and allow other threads to run
        Cyg_Scheduler::unlock();
        Cyg_Scheduler::lock();

        CYG_ASSERTCLASS( this, "Bad this pointer");
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
    }

    CYG_ASSERTCLASS( this, "Bad this pointer");

    if ( ! result )
        ret = NULL;

    // clear the timer; if it actually fired, no worries.
    self->clear_timer();

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( ret );
    return ret;
}
#endif 

// -------------------------------------------------------------------------
// get some memory, return NULL if none available
template <class T>
inline cyg_uint8 *
Cyg_Mempoolt<T>::try_alloc( cyg_int32 size )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_uint8 *ret = pool.alloc( size );

    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( ret );

    CYG_MEMALLOC_FAIL_TEST(ret==NULL, size);

    return ret;
}
    
    
// -------------------------------------------------------------------------
// free the memory back to the pool
template <class T>
cyg_bool
Cyg_Mempoolt<T>::free( cyg_uint8 *p, cyg_int32 size )
{
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_int32 ret = pool.free( p, size );

    CYG_ASSERTCLASS( this, "Bad this pointer");

    while ( ret && !queue.empty() ) {
        // we succeeded and there are people waiting
        Cyg_Thread *thread = queue.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");

        // we wake them all up (ie. broadcast) to cope with variable block
        // allocators freeing a big block when lots of small allocs wait.
        thread->set_wake_reason( Cyg_Thread::DONE );
        thread->wake();
        // we cannot yield here; if a higher prio thread can't satisfy its
        // request it would re-queue and we would loop forever
    }
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    return ret;
}

// -------------------------------------------------------------------------
// if applicable: return -1 if not fixed size
template <class T>
inline cyg_int32
Cyg_Mempoolt<T>::get_blocksize()
{
    // there should not be any atomicity issues here
    return pool.get_blocksize();
}

// -------------------------------------------------------------------------
// these two are obvious and generic, but need atomicity protection (maybe)
template <class T>
inline cyg_int32
Cyg_Mempoolt<T>::get_totalmem()
{
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_int32 ret = pool.get_totalmem();

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    return ret;
}

template <class T>
inline cyg_int32
Cyg_Mempoolt<T>::get_freemem()
{
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_int32 ret = pool.get_freemem();

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    return ret;
}

// -------------------------------------------------------------------------
// get information about the construction parameters for external
// freeing after the destruction of the holding object
template <class T>
inline void
Cyg_Mempoolt<T>::get_arena(
    cyg_uint8 * &base, cyg_int32 &size, CYG_ADDRWORD &arg_thru )
{
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    pool.get_arena( base, size, arg_thru );

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
}

// -------------------------------------------------------------------------
// Return the size of the memory allocation (previously returned 
// by alloc() or try_alloc() ) at ptr. Returns -1 if not found
template <class T>
cyg_int32
Cyg_Mempoolt<T>::get_allocation_size( cyg_uint8 *ptr )
{
    cyg_int32 ret;
    
    // Prevent preemption
    Cyg_Scheduler::lock();
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    ret = pool.get_allocation_size( ptr );

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    return ret;
}

// -------------------------------------------------------------------------
// debugging/assert function

#ifdef CYGDBG_USE_ASSERTS

template <class T>
inline cyg_bool
Cyg_Mempoolt<T>::check_this(cyg_assert_class_zeal zeal) const
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
#endif // ifndef CYGONCE_KERNEL_MEMPOOLT_INL
// EOF mempoolt.inl
