//==========================================================================
//
//      sched/mlqueue.cxx
//
//      Multi-level queue scheduler class implementation
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
// Contributors: jlarmour
// Date:         1999-02-17
// Purpose:      Multilevel queue scheduler class implementation
// Description:  This file contains the implementations of
//               Cyg_Scheduler_Implementation and
//               Cyg_SchedThread_Implementation.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/sched.hxx>        // our header

#include <cyg/hal/hal_arch.h>          // Architecture specific definitions

#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines

#ifdef CYGSEM_KERNEL_SCHED_MLQUEUE

//==========================================================================
// Cyg_Scheduler_Implementation class static members

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

cyg_ucount32 Cyg_Scheduler_Implementation::timeslice_count[CYGNUM_KERNEL_CPU_MAX];

#endif


//==========================================================================
// Cyg_Scheduler_Implementation class members

// -------------------------------------------------------------------------
// Constructor.

Cyg_Scheduler_Implementation::Cyg_Scheduler_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    queue_map   = 0;

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    pending_map = 0;
    
    for( int i = 0; i < CYGNUM_KERNEL_SCHED_PRIORITIES; i++ )
        pending[i] = 0;

#endif
    
    for( int i = 0; i < CYGNUM_KERNEL_CPU_MAX; i++ )
    {
#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE        
        timeslice_count[i] = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
#endif        
        need_reschedule[i] = true;
    }
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Choose the best thread to run next

Cyg_Thread *
Cyg_Scheduler_Implementation::schedule(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");

    // The run queue may _never_ be empty, there is always
    // an idle thread at the lowest priority.

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    Cyg_Thread *current = get_current_thread();
    register cyg_uint32 index;

    CYG_ASSERT( current->cpu != CYG_KERNEL_CPU_NONE, "Current thread does not have CPU set!");

    // If the current thread is still runnable, return it to pending
    // state so that it can be considered alongside any other threads
    // for execution.
    if( current->get_state() == Cyg_Thread::RUNNING )
    {
        current->cpu = CYG_KERNEL_CPU_NONE;
        pending[current->priority]++;
        pending_map |= (1<<current->priority);
    }
    else
    {
        // Otherwise, ensure that the thread is no longer marked as
        // running.
        current->cpu = CYG_KERNEL_CPU_NONE;        
    }

    
    HAL_LSBIT_INDEX(index, pending_map);

    Cyg_RunQueue *queue = &run_queue[index];
    
    CYG_ASSERT( !queue->empty(), "Queue for index empty");
    CYG_ASSERT( pending[index] > 0, "Pending array and map disagree");

    Cyg_Thread *thread = queue->get_head();

    // We know there is a runnable thread in this queue, If the thread
    // we got is not it, scan until we find it. While not constant time,
    // this search has an upper bound of the number of CPUs in the system.
    
    while( thread->cpu != CYG_KERNEL_CPU_NONE )
        thread = thread->get_next();

    // Take newly scheduled thread out of pending map
    thread->cpu = CYG_KERNEL_CPU_THIS();
    if( --pending[index] == 0 )
        pending_map &= ~(1<<index);
    
#else    

    register cyg_uint32 index;

    HAL_LSBIT_INDEX(index, queue_map);

    Cyg_RunQueue *queue = &run_queue[index];
    
    CYG_ASSERT( !queue->empty(), "Queue for index empty");

    Cyg_Thread *thread = queue->get_head();

#endif
    
    CYG_INSTRUMENT_MLQ( SCHEDULE, thread, index);
    
    CYG_ASSERT( thread != NULL , "No threads in run queue");
    CYG_ASSERT( thread->queue == NULL , "Runnable thread on a queue!");
   
    CYG_REPORT_RETVAL(thread);

    return thread;
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::add_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    cyg_priority pri                               = thread->priority;
    Cyg_RunQueue *queue = &run_queue[pri];

    CYG_INSTRUMENT_MLQ( ADD, thread, pri);
    
    CYG_ASSERT((CYG_THREAD_MIN_PRIORITY >= pri) 
               && (CYG_THREAD_MAX_PRIORITY <= pri),
               "Priority out of range!");

    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");

    // If the thread is on some other queue, remove it
    // here.
    if( thread->queue != NULL )
    {
        thread->queue->remove(thread);
    }
    
    if( queue->empty() )
    {
        // set the map bit and ask for a reschedule if this is a
        // new highest priority thread.
      
        queue_map |= (1<<pri);

    }
    // else the queue already has an occupant, queue behind him

    queue->add_tail(thread);

    // If the new thread is higher priority than any
    // current thread, request a reschedule.

    set_need_reschedule(thread);
    
#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    // If the thread is not currently running, increment the pending
    // count for the priority, and if necessary set the bit in the
    // pending map.

    if( thread->cpu == CYG_KERNEL_CPU_NONE )
    {
        if( pending[pri]++ == 0 )
            pending_map |= (1<<pri);
    }
    // Otherwise the pending count will be dealt with in schedule().
    
#endif    
    
    CYG_ASSERT( thread->queue == NULL , "Runnable thread on a queue!");
    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( !run_queue[pri].empty(), "Queue for pri empty");
    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");    
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::rem_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
        
    CYG_ASSERT( queue_map != 0, "Run queue empty");

    cyg_priority pri    = thread->priority;
    Cyg_RunQueue *queue = &run_queue[pri];

    CYG_INSTRUMENT_MLQ( REM, thread, pri);
    
    CYG_ASSERT( pri != CYG_THREAD_MIN_PRIORITY, "Idle thread trying to sleep!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    if( thread->cpu == CYG_KERNEL_CPU_NONE )
    {
        // If the thread is not running, then we need to adjust the
        // pending count array and map if necessary.

        if( --pending[pri] == 0 )
            pending_map &= ~(1<<pri);
    }
    else
    {
        // If the target thread is currently running on a different
        // CPU, send a reschedule interrupt there to deschedule it.        
        if( thread->cpu != CYG_KERNEL_CPU_THIS() )
            CYG_KERNEL_CPU_RESCHEDULE_INTERRUPT( thread->cpu, 0 );
    }
    // If the thread is current running on this CPU, then the pending
    // count will be dealt with in schedule().
    
#endif
        
    CYG_ASSERT( queue_map & (1<<pri), "Queue map bit not set for pri");
    CYG_ASSERT( !run_queue[pri].empty(), "Queue for pri empty");
    
    // remove thread from queue
    queue->remove(thread);

    if( queue->empty() )
    {
        // If this was only thread in
        // queue, clear map.
      
        queue_map &= ~(1<<pri);
    }

    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
    CYG_ASSERT( ((queue_map & (1<<pri))!=0) == ((!run_queue[pri].empty())!=0), "Map and queue disagree");
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Set the need_reschedule flag
// This function overrides the definition in Cyg_Scheduler_Base and tests
// for a reschedule condition based on the priorities of the given thread
// and the current thread(s).

void Cyg_Scheduler_Implementation::set_need_reschedule(Cyg_Thread *thread)
{
#ifndef CYGPKG_KERNEL_SMP_SUPPORT

    if( current_thread[0]->priority > thread->priority ||
        current_thread[0]->get_state() != Cyg_Thread::RUNNING )
        need_reschedule[0] = true;
    
#else

    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();
    HAL_SMP_CPU_TYPE cpu_count = CYG_KERNEL_CPU_COUNT();

    // Start with current CPU. If we can do the job locally then
    // that is most efficient. Only go on to other CPUs if that is
    // not possible.

    for(int i = 0; i < cpu_count; i++)
    {
        HAL_SMP_CPU_TYPE cpu = (i + cpu_this) % cpu_count;
       
        // If a CPU is not already marked for rescheduling, and its
        // current thread is of lower priority than _thread_, then
        // set its need_reschedule flag.

        Cyg_Thread *cur = current_thread[cpu];

        if( (!need_reschedule[cpu]) &&
            (cur->priority > thread->priority)
          )
        {
            need_reschedule[cpu] = true;

            if( cpu != cpu_this )
            {
                // All processors other than this one need to be sent
                // a reschedule interrupt.
                
                CYG_INSTRUMENT_SMP( RESCHED_SEND, cpu, 0 );
                CYG_KERNEL_CPU_RESCHEDULE_INTERRUPT( cpu, 0 );
            }

            // Having notionally rescheduled _thread_ onto the cpu, we
            // now see if we can reschedule the former current thread of
            // that CPU onto another.
            
            thread = cur;
        }
    } 

#endif  
}

// -------------------------------------------------------------------------
// Set up initial idle thread

void Cyg_Scheduler_Implementation::set_idle_thread( Cyg_Thread *thread, HAL_SMP_CPU_TYPE cpu )
{
    // Make the thread the current thread for this CPU.
    
    current_thread[cpu] = thread;

    // This will insert the thread in the run queues and make it
    // available to execute.
    thread->resume();

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    thread->cpu = cpu;
    
    // In SMP, we need to take this thread out of the pending array
    // and map.
    
    cyg_priority pri    = thread->priority;
    if( --pending[pri] == 0 )
        pending_map &= ~(1<<pri);
#endif    
    
}

// -------------------------------------------------------------------------
// register thread with scheduler

void
Cyg_Scheduler_Implementation::register_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
    // No registration necessary in this scheduler
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

// deregister thread
void
Cyg_Scheduler_Implementation::deregister_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);
    // No registration necessary in this scheduler    
    CYG_REPORT_RETURN();
}
    
// -------------------------------------------------------------------------
// Test the given priority for uniqueness

cyg_bool
Cyg_Scheduler_Implementation::unique( cyg_priority priority)
{
    CYG_REPORT_FUNCTYPE("returning %d");
    CYG_REPORT_FUNCARG1("priority=%d", priority);
    // Priorities are not unique
    CYG_REPORT_RETVAL(true);
    return true;
}

//==========================================================================
// Support for timeslicing option

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::timeslice(void)
{
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_FUNCTION();
#endif

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

    HAL_SMP_CPU_TYPE cpu;
    HAL_SMP_CPU_TYPE cpu_count = CYG_KERNEL_CPU_COUNT();
    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();
    
    for( cpu = 0; cpu < cpu_count; cpu++ )
    {
        if( --timeslice_count[cpu] == 0 )
            if( cpu == cpu_this )
                timeslice_cpu();
            else CYG_KERNEL_CPU_TIMESLICE_INTERRUPT( cpu, 0 );
    }

#else    

    if( --timeslice_count[CYG_KERNEL_CPU_THIS()] == 0 )
        timeslice_cpu();
    
#endif

#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_RETURN();
#endif    
}

// -------------------------------------------------------------------------

void
Cyg_Scheduler_Implementation::timeslice_cpu(void)
{
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_FUNCTION();
#endif

    Cyg_Thread *thread = get_current_thread();
    HAL_SMP_CPU_TYPE cpu_this = CYG_KERNEL_CPU_THIS();
    
    CYG_ASSERT( queue_map != 0, "Run queue empty");
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE_ENABLE
    if( thread->timeslice_enabled &&
        timeslice_count[cpu_this] == 0 )
#else    
    if( timeslice_count[cpu_this] == 0 )
#endif
    {
        CYG_INSTRUMENT_SCHED(TIMESLICE,0,0);
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
        CYG_TRACE0( true, "quantum consumed, time to reschedule" );
#endif

        CYG_ASSERT( get_sched_lock() > 0 , "Timeslice called with zero sched_lock");

        // Only try to rotate the run queue if the current thread is running.
        // Otherwise we are going to reschedule anyway.
        if( thread->get_state() == Cyg_Thread::RUNNING )
        {
            Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

            CYG_INSTRUMENT_MLQ( TIMESLICE, thread, 0);
                
            CYG_ASSERTCLASS( thread, "Bad current thread");
            CYG_ASSERTCLASS( sched, "Bad scheduler");
    
            cyg_priority pri    = thread->priority;
            Cyg_RunQueue *queue = &sched->run_queue[pri];

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

            // In SMP systems we set the head of the queue to point to
            // the thread immediately after the current
            // thread. schedule() will then pick that thread, or one
            // after it to run next.
            
            queue->to_head( thread->get_next() );
#else            
            queue->rotate();
#endif
            
            if( queue->get_head() != thread )
                sched->set_need_reschedule();

            timeslice_count[cpu_this] = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
        }
    }

    
    CYG_ASSERT( queue_map & (1<<CYG_THREAD_MIN_PRIORITY), "Idle thread vanished!!!");
    CYG_ASSERT( !run_queue[CYG_THREAD_MIN_PRIORITY].empty(), "Idle thread vanished!!!");
#ifdef CYGDBG_KERNEL_TRACE_TIMESLICE
    CYG_REPORT_RETURN();
#endif
}

// -------------------------------------------------------------------------

__externC void cyg_scheduler_timeslice_cpu(void)
{
    Cyg_Scheduler::scheduler.timeslice_cpu();
}

#endif

//==========================================================================
// Cyg_SchedThread_Implementation class members

Cyg_SchedThread_Implementation::Cyg_SchedThread_Implementation
(
    CYG_ADDRWORD sched_info
)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("sched_info=%08x", sched_info);
        
    // Set priority to the supplied value.
    priority = (cyg_priority)sched_info;

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE_ENABLE
    // If timeslice_enabled exists, set it true by default
    timeslice_enabled = true;
#endif
#ifdef CYGPKG_KERNEL_SMP_SUPPORT
    cpu = CYG_KERNEL_CPU_NONE;
#endif
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Yield the processor to another thread

void
Cyg_SchedThread_Implementation::yield(void)
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Thread *thread  = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);

    // Only do this if this thread is running. If it is not, there
    // is no point.
    
    if( thread->get_state() == Cyg_Thread::RUNNING )
    {
        // To yield we simply rotate the appropriate
        // run queue to the next thread and reschedule.

        CYG_INSTRUMENT_MLQ( YIELD, thread, 0);
        
        CYG_ASSERTCLASS( thread, "Bad current thread");
    
        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

        CYG_ASSERTCLASS( sched, "Bad scheduler");
    
        cyg_priority pri    = thread->priority;
        Cyg_RunQueue *queue = &sched->run_queue[pri];

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

            // In SMP systems we set the head of the queue to point to
            // the thread immediately after the current
            // thread. schedule() will then pick that thread, or one
            // after it to run next.
            
            queue->to_head( thread->get_next() );
#else            
            queue->rotate();
#endif
        
        if( queue->get_head() != thread )
            sched->set_need_reschedule();

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
            // Reset the timeslice counter so that this thread gets a full
            // quantum. 
        else Cyg_Scheduler::reset_timeslice_count();
#endif
    }
    
    // Unlock the scheduler and switch threads
#ifdef CYGDBG_USE_ASSERTS
    // This test keeps the assertions in unlock_inner() happy if
    // need_reschedule was not set above.
    if( !Cyg_Scheduler::get_need_reschedule() )
        Cyg_Scheduler::unlock();
    else 
#endif    
    Cyg_Scheduler::unlock_reschedule();

    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Rotate the run queue at a specified priority.
// (pri is the decider, not this, so the routine is static)

void
Cyg_SchedThread_Implementation::rotate_queue( cyg_priority pri )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("priority=%d", pri);
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;

    CYG_ASSERTCLASS( sched, "Bad scheduler");
    
    Cyg_RunQueue *queue = &sched->run_queue[pri];

    if ( !queue->empty() ) {
        queue->rotate();
        sched->set_need_reschedule();
    }

    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Move this thread to the head of its queue
// (not necessarily a scheduler queue)

void
Cyg_SchedThread_Implementation::to_queue_head( void )
{
    CYG_REPORT_FUNCTION();
        
    // Prevent preemption
    Cyg_Scheduler::lock();

    Cyg_Thread *thread  = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);

    CYG_ASSERTCLASS( thread, "Bad current thread");
    
    Cyg_ThreadQueue *q = thread->get_current_queue();
    if( q != NULL )
        q->to_head( thread );
    else if( thread->in_list() )
    {
        // If the queue pointer is NULL then it is on a run
        // queue. Move the thread to the head of it's priority list
        // and force a reschedule.
        
        Cyg_Scheduler *sched = &Cyg_Scheduler::scheduler;
        sched->run_queue[thread->priority].to_head( thread );
        sched->set_need_reschedule( thread );
    }

    // Unlock the scheduler and switch threads
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETURN();
}

//==========================================================================
// Cyg_ThreadQueue_Implementation class members

// -------------------------------------------------------------------------        

void
Cyg_ThreadQueue_Implementation::enqueue(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    CYG_INSTRUMENT_MLQ( ENQUEUE, this, thread );
    
#ifdef CYGIMP_KERNEL_SCHED_SORTED_QUEUES

    // Insert the thread into the queue in priority order.

    Cyg_Thread *qhead = get_head();

    if( qhead == NULL ) add_tail( thread );
    else if( qhead == qhead->get_next() )
    {
        // There is currently only one thread in the queue, join it
        // and adjust the queue pointer to point to the highest
        // priority of the two. If they are the same priority,
        // leave the pointer pointing to the oldest.

        qhead->insert( thread );

        if( thread->priority < qhead->priority )
            to_head(thread);
    }
    else
    {
        // There is more than one thread in the queue. First check
        // whether we are of higher priority than the head and if
        // so just jump in at the front. Also check whether we are
        // lower priority than the tail and jump onto the end.
        // Otherwise we really have to search the queue to find
        // our place.

        if( thread->priority < qhead->priority )
        {
            qhead->insert( thread );
            to_head(thread);
        }
        else if( thread->priority > get_tail()->priority )
        {
            // We are lower priority than any thread in the queue,
            // go in at the end.

            add_tail( thread );
        }
        else
        {
            // Search the queue. We do this backwards so that we
            // always add new threads after any that have the same
            // priority.

            // Because of the previous tests we know that this
            // search will terminate before we hit the head of the
            // queue, hence we do not need to check for that
            // condition.
                
            Cyg_Thread *qtmp = get_tail();

            // Scan the queue until we find a higher or equal
            // priority thread.

            while( qtmp->priority > thread->priority )
                qtmp = qtmp->get_prev();

            // Append ourself after the node pointed to by qtmp.
                
            qtmp->append( thread );
        }
    }
#else
    // Just add the thread to the tail of the list
    add_tail( thread );
#endif
    
    thread->queue = CYG_CLASSFROMBASE(Cyg_ThreadQueue,
                                      Cyg_ThreadQueue_Implementation,
                                      this);
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

Cyg_Thread *
Cyg_ThreadQueue_Implementation::dequeue(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");
        
    Cyg_Thread *thread = rem_head();

    CYG_INSTRUMENT_MLQ( DEQUEUE, this, thread );
    
    if( thread != NULL )
        thread->queue = NULL;

    CYG_REPORT_RETVAL(thread);
    return thread;
}

// -------------------------------------------------------------------------

void
Cyg_ThreadQueue_Implementation::remove( Cyg_Thread *thread )
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("thread=%08x", thread);

    CYG_INSTRUMENT_MLQ( REMOVE, this, thread );
    
    thread->queue = NULL;

    Cyg_CList_T<Cyg_Thread>::remove( thread );

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------

Cyg_Thread *
Cyg_ThreadQueue_Implementation::highpri(void)
{
    CYG_REPORT_FUNCTYPE("returning thread %08x");
    CYG_REPORT_RETVAL(get_head());
    return get_head();
}

// -------------------------------------------------------------------------

inline void
Cyg_ThreadQueue_Implementation::set_thread_queue(Cyg_Thread *thread,
                                                 Cyg_ThreadQueue *tq )

{
    thread->queue = tq;
}

// -------------------------------------------------------------------------

#endif

// -------------------------------------------------------------------------
// EOF sched/mlqueue.cxx
