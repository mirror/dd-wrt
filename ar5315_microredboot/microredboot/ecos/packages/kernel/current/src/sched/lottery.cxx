//==========================================================================
//
//      sched/lottery.cxx
//
//      Lottery scheduler class implementation
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
// Purpose:     Lottery scheduler class implementation
// Description: This file contains the implementations of
//              Cyg_Scheduler_Implementation and
//              Cyg_SchedThread_Implementation.
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
#include <cyg/kernel/intr.hxx>         // interrupt defines, for Cyg_HAL_Clock

#include <cyg/hal/hal_arch.h>          // Architecture specific definitions


#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.inl>        // scheduler inlines

#ifdef CYGSEM_KERNEL_SCHED_LOTTERY

#define CYG_ENABLE_TRACE 1

//==========================================================================
// Cyg_Scheduler_Implementation class static members

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

cyg_count32 Cyg_Scheduler_Implementation::timeslice_count =
                                        CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;

#endif

//==========================================================================
// Cyg_Scheduler_Implementation class members

// -------------------------------------------------------------------------
// Constructor.

Cyg_Scheduler_Implementation::Cyg_Scheduler_Implementation()
{
    CYG_REPORT_FUNCTION();

    total_tickets = 0;
    rand_seed = 1;
}

// -------------------------------------------------------------------------
// Choose the best thread to run next

Cyg_Thread *Cyg_Scheduler_Implementation::schedule()
{
    CYG_REPORT_FUNCTION();

#ifdef CYGPKG_HAL_POWERPC

    // PowerPc specific version of random number generator.
    register cyg_int32 r1 asm("r4");
    r1 = rand_seed;
    asm(
        "li     7,0;"
        "ori    7,7,33614;"
        "mulhwu 5,7,%0;"
        "mullw  6,7,%0;"
        "srawi  6,6,1;"
        "add    %0,5,6;"
        "cmpwi  %0,0;"
        "bge    1f;"
        "slwi   %0,%0,1;"
        "srwi   %0,%0,1;"
        "addi   %0,%0,1;"
        "1:;"
        : "=r"(r1)
        : "0"(r1)
        : "r5", "r6", "r7"
        );
    rand_seed = r1;    

#else
#if 1
    rand_seed = (rand_seed * 1103515245) + 1234;
    cyg_int32 r1 = rand_seed & 0x7FFFFFFF;
#else    
    // Generic implementation of RNG.
#if( CYG_BYTEORDER == CYG_MSBFIRST )
#define _LO 1    
#define _HI 0
#else
#define _LO 0    
#define _HI 1
#endif    
    union { cyg_int64 r64; cyg_int32 r32[2]; } u;
    u.r64 = (cyg_int64)rand_seed * 33614LL;
    cyg_int32 r1 = u.r32[_HI] + (u.r32[_LO]>>1);
    if( r1 < 0 )
        r1 = (r1 & 0x7FFFFFFF) + 1;
    rand_seed = r1;
#undef _LO
#undef _HI
#endif    
#endif    

    cyg_int32 ticket = r1 % total_tickets;
    cyg_int32 tick = ticket;
    Cyg_Thread *thread = run_queue.highpri();

    // Search the run queue for the thread with the
    // given ticket.
    while( ticket > 0 )
    {
        ticket -= thread->priority;
        if( ticket <= 0 ) break;
        thread = thread->next;
        
        CYG_ASSERT( thread != run_queue.highpri(), "Looping in scheduler");
    }

    CYG_TRACE3( CYG_ENABLE_TRACE,
        "seed %08x ticket %d thread %08x",
        rand_seed, tick, thread);

    // If the thread has any compensation tickets, take them away since
    // it has just won.

    if( thread->compensation_tickets > 0 )
    {
        thread->priority -= thread->compensation_tickets;
        total_tickets -= thread->compensation_tickets;
        thread->compensation_tickets = 0;
    }
 
    // Re-insert thread at head of list. This reduces runtime by
    // putting the large ticket holders at the front of the list.
    
//    run_queue.remove(thread);
//    run_queue.enqueue(thread);

    CYG_CHECK_DATA_PTR( thread, "Invalid next thread pointer");            
    CYG_ASSERTCLASS( thread, "Bad next thread" );
    
    return thread;
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::add_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();

    // If the thread is on some other queue, remove it
    // here.
    if( thread->queue != NULL )
    {
        thread->queue->remove(thread);
        thread->queue = NULL;
    }
    
    total_tickets += thread->priority;

    run_queue.enqueue(thread);
}

// -------------------------------------------------------------------------

void Cyg_Scheduler_Implementation::rem_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();

    run_queue.remove(thread);

    total_tickets -= thread->priority;

    // Compensate the thread for the segment of the quantum that
    // it used. This makes it more likely to win the lottery next time
    // it is scheduled. We only do this for threads that have voluntarily
    // given up the CPU.

//    if( thread->get_state() != Cyg_Thread::RUNNING )
    {
#if 0        
        cyg_uint32 hal_ticks;
        HAL_CLOCK_READ( &hal_ticks );
        thread->compensation_tickets = thread->priority *
            CYGNUM_KERNEL_COUNTERS_RTC_PERIOD / hal_ticks;
#else
        thread->compensation_tickets = (thread->priority *
            CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS) / timeslice_count;
        
#endif        
        thread->priority += thread->compensation_tickets;
    }
}

// -------------------------------------------------------------------------
// register thread with scheduler

void Cyg_Scheduler_Implementation::register_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // No registration necessary in this scheduler
}

// -------------------------------------------------------------------------

// deregister thread
void Cyg_Scheduler_Implementation::deregister_thread(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // No registration necessary in this scheduler    
}
    
// -------------------------------------------------------------------------
// Test the given priority for uniqueness

cyg_bool Cyg_Scheduler_Implementation::unique( cyg_priority priority)
{
    CYG_REPORT_FUNCTION();
        
    // Priorities are not unique
    return true;
}

//==========================================================================
// Support for timeslicing option

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE

void Cyg_Scheduler_Implementation::timeslice()
{
    CYG_REPORT_FUNCTION();

    if( --timeslice_count <= 0 )
    {
        CYG_INSTRUMENT_SCHED(TIMESLICE,0,0);
        
        // Force a reschedule on each timeslice
        need_reschedule = true;
        timeslice_count = CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS;
    }
}

#endif

//==========================================================================
// Cyg_Cyg_SchedThread_Implementation class members

Cyg_SchedThread_Implementation::Cyg_SchedThread_Implementation
(
    CYG_ADDRWORD sched_info
)
{
    CYG_REPORT_FUNCTION();

    priority = cyg_priority(sched_info);
    
    // point the next and prev field at this thread.
    
    next = prev = CYG_CLASSFROMBASE(Cyg_Thread,
                                    Cyg_SchedThread_Implementation,
                                    this);
}

// -------------------------------------------------------------------------
// Insert thread in front of this

void Cyg_SchedThread_Implementation::insert( Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    thread->next        = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);
    thread->prev        = prev;
    prev->next          = thread;
    prev                = thread;    
}

// -------------------------------------------------------------------------
// remove this from queue

void Cyg_SchedThread_Implementation::remove()
{
    CYG_REPORT_FUNCTION();
        
    next->prev          = prev;
    prev->next          = next;
    next = prev         = CYG_CLASSFROMBASE(Cyg_Thread,
                                            Cyg_SchedThread_Implementation,
                                            this);
}

// -------------------------------------------------------------------------
// Yield the processor to another thread

void Cyg_SchedThread_Implementation::yield()
{
    CYG_REPORT_FUNCTION();
        

}

//==========================================================================
// Cyg_ThreadQueue_Implementation class members

void Cyg_ThreadQueue_Implementation::enqueue(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();

    // Always put thread at head of queue
    if( queue == NULL ) queue = thread;
    else
    {
        queue->insert(thread);
//        queue->next->insert(thread);
//        queue = thread;
    }
    
    thread->queue = CYG_CLASSFROMBASE(Cyg_ThreadQueue,
                                      Cyg_ThreadQueue_Implementation,
                                      this);    
}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::dequeue()
{
    CYG_REPORT_FUNCTION();
        
    if( queue == NULL ) return NULL;
    
    Cyg_Thread *thread = queue;
    
    if( thread->next == thread )
    {
        // sole thread on list, NULL out ptr
        queue = NULL;
    }
    else
    {
        // advance to next and remove thread
        queue = thread->next;
        thread->remove();
    }

    thread->queue = NULL;

    return thread;
}

// -------------------------------------------------------------------------

Cyg_Thread *Cyg_ThreadQueue_Implementation::highpri()
{
    CYG_REPORT_FUNCTION();
        
    return queue;
}

// -------------------------------------------------------------------------

void Cyg_ThreadQueue_Implementation::remove(Cyg_Thread *thread)
{
    CYG_REPORT_FUNCTION();
        
    // If the thread we want is the at the head
    // of the list, and is on its own, clear the
    // list and return. Otherwise advance to the
    // next thread and remove ours. If the thread
    // is not at the head of the list, just dequeue
    // it.

    thread->queue = NULL;
    
    if( queue == thread )
    {
        if( thread->next == thread )
        {
            queue = NULL;
            return;
        }
        else queue = thread->next;
    }

    thread->Cyg_SchedThread_Implementation::remove();

}

// -------------------------------------------------------------------------
// Rotate the front thread on the queue to the back.

void Cyg_ThreadQueue_Implementation::rotate()
{
    CYG_REPORT_FUNCTION();
        
    queue = queue->next;
}

// -------------------------------------------------------------------------

#endif

// -------------------------------------------------------------------------
// EOF sched/lottery.cxx
