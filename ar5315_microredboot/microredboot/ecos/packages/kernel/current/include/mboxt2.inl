#ifndef CYGONCE_KERNEL_MBOXT2_INL
#define CYGONCE_KERNEL_MBOXT2_INL
//==========================================================================
//
//      mboxt2.inl
//
//      Mboxt2 mbox template class implementation
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
// Purpose:     Mboxt2 template implementation
// Description: This file contains the implementations of the mboxt2
//              template classes.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation

#include <cyg/kernel/mboxt2.hxx>       // our header

#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/clock.inl>        // clock inlines

// -------------------------------------------------------------------------
// inline function for awakening waiting threads

template <class T, cyg_count32 QUEUE_SIZE>
inline void
Cyg_Mboxt2<T,QUEUE_SIZE>::wakeup_winner( const T &msg )
{
    CYG_ASSERT( !get_threadq.empty(), "Where did the winner go?" );

    // The queue is non-empty, so grab the next thread and wake it up.
    Cyg_Thread *thread = get_threadq.dequeue();

    CYG_ASSERTCLASS( thread, "Bad thread pointer");

    T *msg_ret = (T *)(thread->get_wait_info());
    *msg_ret = msg;

    thread->set_wake_reason( Cyg_Thread::DONE );
    thread->wake();
        
    CYG_INSTRUMENT_MBOXT(WAKE, this, thread);        
}

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
template <class T, cyg_count32 QUEUE_SIZE>
inline void
Cyg_Mboxt2<T,QUEUE_SIZE>::wakeup_putter( void )
{
    if( !put_threadq.empty() ) {
        // The queue is non-empty, so grab the next thread and wake it up.
        Cyg_Thread *thread = put_threadq.dequeue();

        CYG_ASSERTCLASS( thread, "Bad thread pointer");

        T *new_msg = (T *)(thread->get_wait_info());

        cyg_count32 in = base + (count++);
        if ( size <= in )
            in -= size;

        CYG_ASSERT( size > in, "in overflow" );
        CYG_ASSERT( 0 <= in, "in overflow" );
        CYG_ASSERT( size >= count, "count overflow" );

        itemqueue[ in ] = *new_msg;

        thread->set_wake_reason( Cyg_Thread::DONE );
        thread->wake();
        
        CYG_INSTRUMENT_MBOXT(WAKE, this, thread);        
    }
}
#endif

// -------------------------------------------------------------------------
// Constructor

template <class T, cyg_count32 QUEUE_SIZE>
Cyg_Mboxt2<T,QUEUE_SIZE>::Cyg_Mboxt2()
{
    CYG_REPORT_FUNCTION();
    base = 0;
    count = 0;
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Destructor

template <class T, cyg_count32 QUEUE_SIZE>
Cyg_Mboxt2<T,QUEUE_SIZE>::~Cyg_Mboxt2()
{
    CYG_REPORT_FUNCTION();
#if 0
    CYG_ASSERT( 0 == count, "Deleting mboxt2 with messages");
    CYG_ASSERT( get_threadq.empty(), "Deleting mboxt2 with threads waiting to get");
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    CYG_ASSERT( put_threadq.empty(), "Deleting mboxt2 with threads waiting to put");
#endif
#endif
    // Prevent preemption
    Cyg_Scheduler::lock();

    while ( ! get_threadq.empty() ) {
        Cyg_Thread *thread = get_threadq.dequeue();
        thread->set_wake_reason( Cyg_Thread::DESTRUCT );
        thread->wake();
    }
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    while ( ! put_threadq.empty() ) {
        Cyg_Thread *thread = put_threadq.dequeue();
        thread->set_wake_reason( Cyg_Thread::DESTRUCT );
        thread->wake();
    }
#endif

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// debugging/assert function

#ifdef CYGDBG_USE_ASSERTS

template <class T, cyg_count32 QUEUE_SIZE>
cyg_bool 
Cyg_Mboxt2<T,QUEUE_SIZE>::check_this(cyg_assert_class_zeal zeal) const
{
    if ( Cyg_Thread::DESTRUCT == Cyg_Thread::self()->get_wake_reason() )
        // then the whole thing is invalid, and we know it.
        // so return OK, since this check should NOT make an error.
        return true;

    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;

#if 0 // thread queues do not have checking funcs.
    if ( ! get_threadq.check_this( zeal ) ) return false;
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    if ( ! put_threadq.check_this( zeal ) ) return false;
#endif
#endif

    switch( zeal )
    {
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_quick:
    case cyg_trivial:
        // plenty of scope for fencepost problems here
        if ( size < count ) return false;
        if ( size <= base ) return false;
        if ( 0 > count ) return false;
        if ( 0 > base  ) return false;

        // Comments about needing 2 queues elided; they're not true in this
        // immediate-dispatch model.  I think we could get away with only
        // one queue now, biut is it worth it?  4 bytes of redundant info
        // buys a lot of correctness.
      
    case cyg_none:
    default:
        break;
    };

    return true;
}

#endif


// -------------------------------------------------------------------------
// From here downwards, these are the major functions of the template; if
// being genuinely used as a template they should probably not be inlined.
// If being used to construct a specific class, with explicit functions,
// then they should be.  This is controlled by:

#ifdef CYGIMP_MBOXT_INLINE
#define CYG_MBOXT_INLINE inline
#else
#define CYG_MBOXT_INLINE
#endif

// -------------------------------------------------------------------------
// Get an item, or wait for one to arrive

template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::get( T &ritem )
{
    CYG_REPORT_FUNCTION();
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    CYG_INSTRUMENT_MBOXT(GET, this, count);

    if ( 0 < count ) {
        CYG_INSTRUMENT_MBOXT(GOT, this, count);
    
        ritem = itemqueue[ (count--, base++) ];
        CYG_ASSERT( 0 <= count, "Count went -ve" );
        CYG_ASSERT( size >= base, "Base overflow" );

        if ( size <= base )
            base = 0;

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
        wakeup_putter();
#endif

        // Unlock the scheduler and definitely switch threads
        Cyg_Scheduler::unlock();

        CYG_ASSERTCLASS( this, "Bad this pointer");
        CYG_REPORT_RETVAL( true );
        return true;
    }

    self->set_wait_info( (CYG_ADDRWORD)&ritem );
    self->set_sleep_reason( Cyg_Thread::WAIT );
    self->sleep();
    get_threadq.enqueue( self );

    CYG_INSTRUMENT_MBOXT(WAIT, this, count);
        
    // Unlock scheduler and allow other threads to run
    Cyg_Scheduler::unlock_reschedule();

    cyg_bool result = true;
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
    CYG_ASSERTCLASS( this, "Bad this pointer");    
    CYG_REPORT_RETVAL( result );
    return result;
}


// -------------------------------------------------------------------------
// Try to get an item with an absolute timeout and return success.

#ifdef CYGFUN_KERNEL_THREADS_TIMER
template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::get( T &ritem, cyg_tick_count abs_timeout )
{
    CYG_REPORT_FUNCTION();
        
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    CYG_INSTRUMENT_MBOXT(GET, this, count);

    if ( 0 < count ) {
        CYG_INSTRUMENT_MBOXT(GOT, this, count);
    
        ritem = itemqueue[ (count--, base++) ];
        CYG_ASSERT( 0 <= count, "Count went -ve" );
        CYG_ASSERT( size >= base, "Base overflow" );

        if ( size <= base )
            base = 0;

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
        wakeup_putter();
#endif

        // Unlock the scheduler and maybe switch threads
        Cyg_Scheduler::unlock();

        CYG_ASSERTCLASS( this, "Bad this pointer");
        CYG_REPORT_RETVAL( true );
        return true;
    }

    // Set the timer
    self->set_timer( abs_timeout, Cyg_Thread::TIMEOUT );

    // If the timeout is in the past, the wake reason will have been set to
    // something other than NONE already. If so, skip the wait and go
    // straight to unlock.
    
    if( Cyg_Thread::NONE == self->get_wake_reason() ) {
        self->set_wait_info( (CYG_ADDRWORD)&ritem );
        self->sleep();
        get_threadq.enqueue( self );

        CYG_INSTRUMENT_MBOXT(WAIT, this, count);
    }

    // Unlock scheduler and allow other threads to run
    Cyg_Scheduler::unlock_reschedule();

    // clear the timer; if it actually fired, no worries.
    self->clear_timer();

    CYG_ASSERTCLASS( this, "Bad this pointer");        

    cyg_bool result = true;
    switch( self->get_wake_reason() )
    {
    case Cyg_Thread::TIMEOUT:
        result = false;
        CYG_INSTRUMENT_MBOXT(TIMEOUT, this, count);
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

    CYG_REPORT_RETVAL( result );
    return result;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

// -------------------------------------------------------------------------
// Try to get an item and return success.

template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::tryget( T &ritem )
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MBOXT(TRY, this, count);
    
    cyg_bool result = ( 0 < count );
    // If the mboxt2 is not empty, grab an item and return it.
    if ( result ) {
        ritem = itemqueue[ (count--, base++) ];
        CYG_ASSERT( 0 <= count, "Count went -ve" );
        CYG_ASSERT( size >= base, "Base overflow" );
        if ( size <= base )
            base = 0;

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
        wakeup_putter();
#endif
    }

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( result );
    return result;    
}

// -------------------------------------------------------------------------
// get next item without removing it
template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::peek_item( T &ritem )
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERTCLASS( this, "Bad this pointer");
    
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MBOXT(TRY, this, count);
    
    cyg_bool result = ( 0 < count );
    // If the mboxt2 is not empty, grab an item and return it.
    if ( result )
        ritem = itemqueue[ base ];

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( result );
    return result;    
}

// -------------------------------------------------------------------------
// Put an item in the queue; wait if full.

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::put( const T item )
{
    CYG_REPORT_FUNCTION();
        
    Cyg_Thread *self = Cyg_Thread::self();

    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MBOXT(PUT, this, count);
    CYG_ASSERTCLASS( this, "Bad this pointer");

    if ( size == count ) {
        CYG_ASSERT( get_threadq.empty(), "Threads waiting AND queue full?" );

        self->set_wait_info( (CYG_ADDRWORD)&item );
        self->set_sleep_reason( Cyg_Thread::WAIT );
        self->sleep();
        put_threadq.enqueue( self );

        CYG_INSTRUMENT_MBOXT(WAIT, this, count);
        
        // when this returns, our item is in the queue.
        Cyg_Scheduler::unlock_reschedule();        // unlock, switch threads

        CYG_ASSERTCLASS( this, "Bad this pointer");    

        cyg_bool result = true;
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
        CYG_REPORT_RETVAL( result );
        return result;
    }

    if ( !get_threadq.empty() ) {
        wakeup_winner( item );
        Cyg_Scheduler::unlock();        // unlock, maybe switch threads
        CYG_ASSERTCLASS( this, "Bad this pointer");    
        CYG_REPORT_RETVAL( true );
        return true;
    }

    cyg_count32 in = base + (count++);
    if ( size <= in )
        in -= size;

    CYG_ASSERT( size > in, "in overflow" );
    CYG_ASSERT( 0 <= in, "in overflow" );
    CYG_ASSERT( size >= count, "count overflow" );

    itemqueue[ in ] = item;

    CYG_ASSERTCLASS( this, "Bad this pointer");    
    
    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_REPORT_RETVAL( true );
    return true;
}

// -------------------------------------------------------------------------
// Put an item in the queue; wait if full, with an absolute timeout;
// return success.

#ifdef CYGFUN_KERNEL_THREADS_TIMER
template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::put( const T item, cyg_tick_count abs_timeout )
{
    CYG_REPORT_FUNCTION();
        
    Cyg_Thread *self = Cyg_Thread::self();

    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MBOXT(PUT, this, count);
    CYG_ASSERTCLASS( this, "Bad this pointer");

    if ( size == count ) {

        CYG_ASSERT( get_threadq.empty(), "Threads waiting AND queue full?" );

        // Set the timer
        self->set_timer( abs_timeout, Cyg_Thread::TIMEOUT );

        // If the timeout is in the past, the wake reason will have been set to
        // something other than NONE already. If so, skip the wait and go
        // straight to unlock.
    
        if( Cyg_Thread::NONE == self->get_wake_reason() ) {
            self->set_wait_info( (CYG_ADDRWORD)&item );
            self->sleep();
            put_threadq.enqueue( self );

            CYG_INSTRUMENT_MBOXT(WAIT, this, count);
        }

        // when this returns, our item is in the queue.
        Cyg_Scheduler::unlock_reschedule();        // unlock, switch threads

        // clear the timer; if it actually fired, no worries.
        self->clear_timer();

        cyg_bool result = true;
        switch( self->get_wake_reason() )
        {
        case Cyg_Thread::TIMEOUT:
            result = false;
            CYG_INSTRUMENT_MBOXT(TIMEOUT, this, count);
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

        CYG_ASSERTCLASS( this, "Bad this pointer");    
        CYG_REPORT_RETVAL( result );
        return result;
    }


    if ( !get_threadq.empty() ) {
        wakeup_winner( item );
        Cyg_Scheduler::unlock();        // unlock, maybe switch threads
        CYG_ASSERTCLASS( this, "Bad this pointer");    
        CYG_REPORT_RETVAL( true );
        return true;
    }

    cyg_count32 in = base + (count++);
    if ( size <= in )
        in -= size;

    CYG_ASSERT( size > in, "in overflow" );
    CYG_ASSERT( 0 <= in, "in overflow" );
    CYG_ASSERT( size >= count, "count overflow" );

    itemqueue[ in ] = item;

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();
    CYG_ASSERTCLASS( this, "Bad this pointer");    
    CYG_REPORT_RETVAL( true );
    return true;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER
#endif // CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT

// -------------------------------------------------------------------------
// Try to put an item in the queue and return success; queue may be full.

template <class T, cyg_count32 QUEUE_SIZE>
CYG_MBOXT_INLINE cyg_bool
Cyg_Mboxt2<T,QUEUE_SIZE>::tryput( const T item )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    CYG_INSTRUMENT_MBOXT(PUT, this, count);
    CYG_ASSERTCLASS( this, "Bad this pointer");

    if ( size == count ) {
        CYG_ASSERT( get_threadq.empty(), "Threads waiting AND queue full?" );
        Cyg_Scheduler::unlock();        // unlock, maybe switch threads
        CYG_REPORT_RETVAL( false );
        return false;                   // the mboxt2 is full
    }

    if ( !get_threadq.empty() ) {
        CYG_ASSERT( 0 == count, "Threads waiting AND queue not empty" );
        wakeup_winner( item );
        Cyg_Scheduler::unlock();        // unlock, maybe switch threads
        CYG_REPORT_RETVAL( true );
        return true;
    }

    cyg_count32 in = base + (count++);
    if ( size <= in )
        in -= size;

    CYG_ASSERT( size > in, "in overflow" );
    CYG_ASSERT( 0 <= in, "in overflow" );
    CYG_ASSERT( size >= count, "count overflow" );

    itemqueue[ in ] = item;

    CYG_ASSERTCLASS( this, "Bad this pointer");    

    // Unlock the scheduler and maybe switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETVAL( true );
    return true;
}


// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_MBOXT2_INL
// EOF mboxt2.inl
