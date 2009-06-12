//==========================================================================
//
//      sync/mutex.cxx
//
//      Mutex and condition variable implementation
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
// Author(s):    nickg
// Contributors: nickg, jlarmour
// Date:         1999-02-17
// Purpose:      Mutex implementation
// Description:  This file contains the implementations of the mutex
//               and condition variable classes.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation

#include <cyg/kernel/mutex.hxx>        // our header

#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/clock.inl>        // clock inlines

// -------------------------------------------------------------------------
// Mutex protocol test macros.
// If the dynamic protocol option is enabled, then these generate appropriate
// tests on the protocol field. If there is no dynamic choice then they simply
// result in empty statements.

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC

#define IF_PROTOCOL_INHERIT if( protocol == INHERIT )
#define IF_PROTOCOL_CEILING if( protocol == CEILING )
#define IF_PROTOCOL_ACTIVE  if( protocol != NONE )

#else

#define IF_PROTOCOL_INHERIT
#define IF_PROTOCOL_CEILING
#define IF_PROTOCOL_ACTIVE

#endif

// -------------------------------------------------------------------------
// Constructor

Cyg_Mutex::Cyg_Mutex()
{
    CYG_REPORT_FUNCTION();
        
    locked      = false;
    owner       = NULL;

#if defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT) && \
    defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC)

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_INHERIT
    protocol    = INHERIT;
#endif    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_CEILING
    protocol    = CEILING;
    ceiling     = CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY;
#endif    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_NONE
    protocol    = NONE;
#endif

#else // not (DYNAMIC and DEFAULT defined)

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY

    // if there is a default priority ceiling defined, use that to initialize
    // the ceiling.
    ceiling = CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY;    

#else

    // Otherwise set it to zero.
    ceiling = 0;
    
#endif    
#endif

#endif // DYNAMIC and DEFAULT defined
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Construct with defined protocol

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC

Cyg_Mutex::Cyg_Mutex( cyg_protcol protocol_arg )
{
    CYG_REPORT_FUNCTION();
        
    locked      = false;
    owner       = NULL;

    protocol    = protocol_arg;

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY

    // if there is a default priority ceiling defined, use that to initialize
    // the ceiling.
    ceiling = CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DEFAULT_PRIORITY;    

#else

    // Otherwise set it to zero.
    ceiling = 0;
    
#endif    
#endif
    
    CYG_REPORT_RETURN();
}

#endif

// -------------------------------------------------------------------------
// Destructor

Cyg_Mutex::~Cyg_Mutex()
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT( owner == NULL, "Deleting mutex with owner");
    CYG_ASSERT( queue.empty(), "Deleting mutex with waiting threads");
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

#ifdef CYGDBG_USE_ASSERTS

cyg_bool
Cyg_Mutex::check_this( cyg_assert_class_zeal zeal) const
{
//    CYG_REPORT_FUNCTION();
        
    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;
    
    switch( zeal )
    {
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_quick:
    case cyg_trivial:
        if(  locked && owner == NULL ) return false;
        if( !locked && owner != NULL ) return false;        
    case cyg_none:
    default:
        break;
    };

    return true;
}

#endif

// -------------------------------------------------------------------------
// Lock and/or wait

cyg_bool
Cyg_Mutex::lock(void)
{
    CYG_REPORT_FUNCTYPE("returning %d");

    cyg_bool result = true;
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    CYG_INSTRUMENT_MUTEX(LOCK, this, 0);

    // Loop while the mutex is locked, sleeping each time around
    // the loop. This copes with the possibility of a higher priority
    // thread grabbing the mutex between the wakeup in unlock() and
    // this thread actually starting.
    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL

    IF_PROTOCOL_ACTIVE
	self->count_mutex();

#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
    
    IF_PROTOCOL_CEILING
        self->set_priority_ceiling(ceiling);

#endif        
               
    while( locked && result )
    {
        CYG_ASSERT( self != owner, "Locking mutex I already own");
        
        self->set_sleep_reason( Cyg_Thread::WAIT );
        
        self->sleep();
        
        queue.enqueue( self );

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT

        IF_PROTOCOL_INHERIT
            owner->inherit_priority(self);

#endif

        CYG_INSTRUMENT_MUTEX(WAIT, this, 0);

        // Allow other threads to run
        Cyg_Scheduler::reschedule();
        
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

    if( result )
    {
        locked      = true;
        owner       = self;

        CYG_INSTRUMENT_MUTEX(LOCKED, this, 0);
    }
    else
    {
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL

       IF_PROTOCOL_ACTIVE
           self->uncount_mutex();

#endif    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT

        IF_PROTOCOL_INHERIT
            self->disinherit_priority();

#endif
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

        IF_PROTOCOL_CEILING
            self->clear_priority_ceiling();

#endif
    }
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_ASSERTCLASS( this, "Bad this pointer");    

    CYG_REPORT_RETVAL(result);

    return result;
}

// -------------------------------------------------------------------------
// Try to lock and return success

cyg_bool
Cyg_Mutex::trylock(void)
{
    CYG_REPORT_FUNCTYPE("returning %d");
        
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    cyg_bool result = true;
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    // If the mutex is not locked, grab it
    // for ourself. Otherwise return failure.
    if( !locked )
    {
        Cyg_Thread *self = Cyg_Thread::self();
        
        locked  = true;
        owner   = self;

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL

       IF_PROTOCOL_ACTIVE
            self->count_mutex();

#endif
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

        IF_PROTOCOL_CEILING
            self->set_priority_ceiling(ceiling);
        
#endif        
        
    }
    else result = false;

    CYG_INSTRUMENT_MUTEX(TRY, this, result);
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    
    CYG_REPORT_RETVAL(result);
    return result;    
}

// -------------------------------------------------------------------------
// unlock

void
Cyg_Mutex::unlock(void)
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MUTEX(UNLOCK, this, 0);

    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERT( locked, "Unlock mutex that is not locked");
    CYG_ASSERT( owner == Cyg_Thread::self(), "Unlock mutex I do not own");
        
    if( !queue.empty() ) {

        // The queue is non-empty, so grab the next
        // thread from it and wake it up.

        Cyg_Thread *thread = queue.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT

        // Give the owner-to-be a chance to inherit from the remaining
        // queue or the relinquishing thread:

        IF_PROTOCOL_INHERIT
            thread->relay_priority(owner, &queue);

#endif

        thread->set_wake_reason( Cyg_Thread::DONE );
        
        thread->wake();

        CYG_INSTRUMENT_MUTEX(WAKE, this, thread);
        
    }

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL

    IF_PROTOCOL_ACTIVE
	owner->uncount_mutex();

#endif    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT

    IF_PROTOCOL_INHERIT
        owner->disinherit_priority();
    
#endif
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

    IF_PROTOCOL_CEILING
        owner->clear_priority_ceiling();
        
#endif
    
    locked      = false;
    owner       = NULL;
    
    CYG_ASSERTCLASS( this, "Bad this pointer");    
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Release all waiting threads.

void Cyg_Mutex::release()
{
    CYG_REPORT_FUNCTION();

    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MUTEX(RELEASE, this, 0);

    CYG_ASSERTCLASS( this, "Bad this pointer");
        
    while( !queue.empty() )
    {
        // The queue is non-empty, so grab each
        // thread from it and release it.

        Cyg_Thread *thread = queue.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");

        thread->release();

        CYG_INSTRUMENT_MUTEX(RELEASED, this, thread);
        
    }

    CYG_ASSERTCLASS( this, "Bad this pointer");    
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Set ceiling priority for priority ceiling protocol

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

void Cyg_Mutex::set_ceiling( cyg_priority priority )
{
    CYG_REPORT_FUNCTION();

//    CYG_ASSERT( priority >=  CYG_THREAD_MAX_PRIORITY, "Priority out of range");
//    CYG_ASSERT( priority <=  CYG_THREAD_MIN_PRIORITY, "Priority out of range");
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    ceiling = priority;
    
    // Unlock the scheduler
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();    
}

#endif

// -------------------------------------------------------------------------
// Set priority inversion protocol

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
void Cyg_Mutex::set_protocol( cyg_protcol new_protocol )
{
    CYG_REPORT_FUNCTION();

    // Prevent preemption
    Cyg_Scheduler::lock();
    
    protocol = new_protocol;
    
    // Unlock the scheduler
    Cyg_Scheduler::unlock();
    
    CYG_REPORT_RETURN();    
}

#endif


//==========================================================================
// Condition variables

Cyg_Condition_Variable::Cyg_Condition_Variable(
    Cyg_Mutex   &mx                // linked mutex
    )
{
    CYG_REPORT_FUNCTION();
        
    mutex       = &mx;

    CYG_ASSERTCLASS( mutex, "Invalid mutex argument");

    CYG_REPORT_RETURN();
}

Cyg_Condition_Variable::Cyg_Condition_Variable()
{
    CYG_REPORT_FUNCTION();
        
    mutex       = NULL;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Destructor

Cyg_Condition_Variable::~Cyg_Condition_Variable()
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT( queue.empty(), "Deleting condvar with waiting threads");

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

#ifdef CYGDBG_USE_ASSERTS

cyg_bool
Cyg_Condition_Variable::check_this( cyg_assert_class_zeal zeal) const
{
    bool result = true;

    CYG_REPORT_FUNCTYPE("returning %d");
    CYG_REPORT_FUNCARG1("zeal = %d", zeal);
        
    // check that we have a non-NULL pointer first
    if( this == NULL )
        result = false;
    else {
        
        switch( zeal )
        {
        case cyg_system_test:
        case cyg_extreme:
        case cyg_thorough:
            if( mutex != NULL && !mutex->check_this(zeal) )
                result = false;
        case cyg_quick:
        case cyg_trivial:
        case cyg_none:
        default:
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

#endif

// -------------------------------------------------------------------------
// Wait for condition to be true    
// Note: if this function is entered with the scheduler locked (e.g. to
// suspend DSR processing) then there is no need to take the lock.  Also
// in this case, exit with the scheduler locked, which allows this function
// to be used in a totally thread-safe manner.

cyg_bool
Cyg_Condition_Variable::wait_inner( Cyg_Mutex *mx )
{
    CYG_REPORT_FUNCTION();

    cyg_bool result = true;
    Cyg_Thread *self = Cyg_Thread::self();

    Cyg_Scheduler::lock();

    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");
    CYG_ASSERTCLASS( self, "Bad self thread");

    CYG_INSTRUMENT_CONDVAR(WAIT, this, 0);
    
    mx->unlock();

    self->set_sleep_reason( Cyg_Thread::WAIT );
        
    self->sleep();
        
    queue.enqueue( self );

    // Avoid calling ASRs during the following unlock.
    self->set_asr_inhibit();
    
    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock_reschedule();

    // Allow ASRs again
    self->clear_asr_inhibit();
            
    CYG_INSTRUMENT_CONDVAR(WOKE, this, self->get_wake_reason());

    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");

    switch( self->get_wake_reason() )
    {
    case Cyg_Thread::DESTRUCT:          // which, the cv or the mutex?
    case Cyg_Thread::BREAK:
        result = false;
        break;
            
    case Cyg_Thread::EXIT:            
        self->exit();
        break;

    default:
        break;
    }

    // When we awake, we must re-acquire the mutex.  Note that while
    // it is essential to release the mutex and queue on the CV
    // atomically relative to other threads, to avoid races, it is not
    // necessary for us to re-acquire the mutex in the same atomic
    // action. Hence we can do it after unlocking the scheduler.
    // We need to loop here in case the thread is released while waiting
    // for the mutex. It is essential that we exit this function with the
    // mutex claimed.

    while ( !mx->lock() )
        continue;

    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");
    CYG_ASSERT( mx->owner == self, "Not mutex owner");

    CYG_REPORT_RETURN();

    return result;
}

// -------------------------------------------------------------------------
// Wake one thread

void
Cyg_Condition_Variable::signal(void)
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_CONDVAR(SIGNAL, this, 0);
    
    if( !queue.empty() )
    {
        // The queue is non-empty, so grab the next
        // thread from it and wake it up.

        Cyg_Thread *thread = queue.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");
        
        thread->set_wake_reason( Cyg_Thread::DONE );
        
        thread->wake();

        CYG_INSTRUMENT_CONDVAR(WAKE, this, thread);
        
    }
    
    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Set cond true, wake all threads

void
Cyg_Condition_Variable::broadcast(void)
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERTCLASS( this, "Bad this pointer");

    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_CONDVAR(BROADCAST, this, 0);
    
    // Grab all the threads from the queue and let them
    // go.
    
    while( !queue.empty() )
    {
        Cyg_Thread *thread = queue.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");
        
        thread->set_wake_reason( Cyg_Thread::DONE );
        
        thread->wake();

        CYG_INSTRUMENT_CONDVAR(WAKE, this, thread);        
    }
    
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();    

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Optional timed wait on a CV

#if defined(CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT)

cyg_bool
Cyg_Condition_Variable::wait_inner( Cyg_Mutex *mx, cyg_tick_count timeout )
{
    CYG_REPORT_FUNCTYPE("returning %d");
    CYG_REPORT_FUNCARG1("timeout = %d", timeout);
        
    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");

    cyg_bool result = true;
    
    Cyg_Thread *self = Cyg_Thread::self();

    CYG_ASSERTCLASS( self, "Bad self thread");
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_CONDVAR(TIMED_WAIT, this, 0 );
    
    mx->unlock();

    // The ordering of sleep() and set_timer() here are
    // important. If the timeout is in the past, the thread
    // will be woken up immediately and will not sleep.
    
    self->sleep();
        
    // Set the timer and sleep reason
    self->set_timer( timeout, Cyg_Thread::TIMEOUT );

    // Only enqueue if the timeout has not already fired.
    if( self->get_wake_reason() == Cyg_Thread::NONE )
        queue.enqueue( self );

    // Avoid calling ASRs during the following unlock.
    self->set_asr_inhibit();
        
    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock_reschedule();

    // Allow ASRs again
    self->clear_asr_inhibit();
                
    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");
    
    self->clear_timer();

    CYG_INSTRUMENT_CONDVAR(WOKE, this, self->get_wake_reason());
    
    switch( self->get_wake_reason() )
    {
    case Cyg_Thread::TIMEOUT:            
    case Cyg_Thread::DESTRUCT:          // which, the cv or the mutex?
    case Cyg_Thread::BREAK:
        result = false;
        break;
            
    case Cyg_Thread::EXIT:            
        self->exit();
        break;

    default:
        break;
    }

    
    // When we awake, we must re-acquire the mutex.  Note that while
    // it is essential to release the mutex and queue on the CV
    // atomically relative to other threads, to avoid races, it is not
    // necessary for us to re-acquire the mutex in the same atomic
    // action. Hence we can do it after unlocking the scheduler.

    while ( !mx->lock() )
        continue;
    
    CYG_ASSERTCLASS( this, "Bad this pointer");
    CYG_ASSERTCLASS( mx, "Corrupt mutex");

    CYG_REPORT_RETVAL(result);
    
    return result;
}

#endif


// -------------------------------------------------------------------------
// EOF sync/mutex.cxx
