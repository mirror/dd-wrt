//==========================================================================
//
//      sched/bitmap.cxx
//
//      Bitmap scheduler class implementation
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-16
// Purpose:     Bitmap scheduler class implementation
// Description: This file contains the implementations of
//              Cyg_Scheduler_Implementation and Cyg_SchedThread_Implementation.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>             // base kernel types
#include <cyg/infra/cyg_trac.h>           // tracing macros
#include <cyg/infra/cyg_ass.h>            // assertion macros

#include <cyg/kernel/sched.hxx>            // our header

#include <cyg/hal/hal_arch.h>           // Architecture specific definitions

#include <cyg/kernel/thread.inl>           // thread inlines
#include <cyg/kernel/sched.inl>            // scheduler inlines

#ifdef CYGSEM_KERNEL_SCHED_BITMAP

//==========================================================================
// Cyg_Scheduler_Implementation class members

// -------------------------------------------------------------------------
// Constructor.

Cyg_Scheduler_Implementation::Cyg_Scheduler_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    // At present we cannot init run_queue here because the absence of
    // ordering of static constructors means that we could do this
    // after the static idle thread has been created. (Guess how I
    // found this out!)    
//    run_queue = 0;

}

// -------------------------------------------------------------------------
// Choose the best thread to run next

Cyg_Thread *Cyg_Scheduler_Implementation::schedule()
{
    CYG_REPORT_FUNCTION();
        
    // The run queue may _never_ be empty, there is always
    // an idle thread at the lowest priority.
    
    CYG_ASSERT(run_queue != 0, "Run queue empty");

    cyg_uint32 index;

    HAL_LSBIT_INDEX(index, run_queue);
    
    return thread_table[index];
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::add_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT((CYG_THREAD_MIN_PRIORITY >= thread->priority) 
               && (CYG_THREAD_MAX_PRIORITY <= thread->priority),
               "Priority out of range!");

    CYG_ASSERT( thread_table[thread->priority] == NULL ||
                thread_table[thread->priority] == thread,
                "Duplicate thread priorities" );

    CYG_ASSERT( (run_queue & (1<<thread->priority)) == 0,
                "Run queue bit already set" );

    // If the thread is on some other queue, remove it
    // here.
    if( thread->queue != NULL )
    {
        thread->queue->remove(thread);
        thread->queue = NULL;
    }
    
    run_queue |= 1<<thread->priority;

    // If the new thread is higher priority than the
    // current thread, request a reschedule.

    if( thread->priority < Cyg_Scheduler::get_current_thread()->priority )
        set_need_reschedule();
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::rem_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    CYG_ASSERT( thread_table[thread->priority] == thread,
                "Invalid thread priority" );
    
    CYG_ASSERT( (run_queue & (1<<thread->priority)) != 0,
                "Run queue bit not set" );

    run_queue &= ~(1<<thread->priority);

    if( thread == Cyg_Scheduler::get_current_thread() )
        set_need_reschedule();
}

// -------------------------------------------------------------------------
// Set up initial idle thread

void Cyg_Scheduler_Implementation::set_idle_thread( Cyg_Thread *thread, HAL_SMP_CPU_TYPE cpu )
{
    CYG_REPORT_FUNCTION();

    // Make the thread the current thread for this CPU.
    
    current_thread[cpu] = thread;
    
    // This will insert the thread in the run queues and make it
    // available to execute.
    thread->resume();
}

// -------------------------------------------------------------------------
// register thread with scheduler

void Cyg_Scheduler_Implementation::register_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    thread_table[thread->priority] = thread;
}

// -------------------------------------------------------------------------

// deregister thread
void Cyg_Scheduler_Implementation::deregister_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    thread_table[thread->priority] = NULL;
}
    
// -------------------------------------------------------------------------
// Test the given priority for uniqueness

cyg_bool Cyg_Scheduler_Implementation::unique( cyg_priority priority)
{
    CYG_REPORT_FUNCTION();
        
    return thread_table[priority] == NULL;
}


//==========================================================================
// Cyg_Cyg_SchedThread_Implementation class members

Cyg_SchedThread_Implementation::Cyg_SchedThread_Implementation
(
    CYG_ADDRWORD sched_info
)
{
    CYG_REPORT_FUNCTION();

#if 1
    // Assign this thread's priority to the supplied sched_info
    // or the next highest priority available.
    
    priority = cyg_priority(sched_info);

    while( !Cyg_Scheduler::scheduler.unique(priority) )
        priority++;
    
#else    
    // Assign initial priorities to threads in descending order of
    // creation.

    static cyg_priority init_priority = 0;
    
    priority = init_priority++;
#endif
    
}

// -------------------------------------------------------------------------

void Cyg_SchedThread_Implementation::yield()
{
    CYG_REPORT_FUNCTION();
        
    // We cannot yield in this scheduler
}

//==========================================================================
// Cyg_ThreadQueue_Implementation class members

Cyg_ThreadQueue_Implementation::Cyg_ThreadQueue_Implementation()
{
    CYG_REPORT_FUNCTION();
        
    wait_queue = 0;                       // empty queue

    CYG_REPORT_RETURN();
}


void Cyg_ThreadQueue_Implementation::enqueue(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    wait_queue |= 1<<thread->priority;
    thread->queue = CYG_CLASSFROMBASE(Cyg_ThreadQueue,
                                      Cyg_ThreadQueue_Implementation,
                                      this);
}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::dequeue()
{
    CYG_REPORT_FUNCTION();
        
    // Isolate ls bit in run_queue.
    cyg_sched_bitmap next_thread = wait_queue & -wait_queue;

    if( next_thread == 0 ) return NULL;

    wait_queue &= ~next_thread;

    cyg_uint32 index;

    HAL_LSBIT_INDEX(index, next_thread);
    
    Cyg_Thread *thread = Cyg_Scheduler::scheduler.thread_table[index];

    thread->queue = NULL;

    return thread;
}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::highpri()
{
    CYG_REPORT_FUNCTION();
        
    // Isolate ls bit in run_queue.
    cyg_sched_bitmap next_thread = wait_queue & -wait_queue;

    if( next_thread == 0 ) return NULL;

    cyg_uint32 index;

    HAL_LSBIT_INDEX(index, next_thread);
    
    return Cyg_Scheduler::scheduler.thread_table[index];
}

// -------------------------------------------------------------------------

void Cyg_ThreadQueue_Implementation::remove(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    wait_queue &= ~(1<<thread->priority);
    thread->queue = NULL;
}

#endif

// -------------------------------------------------------------------------
// EOF sched/bitmap.cxx
