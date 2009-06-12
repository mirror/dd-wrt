//==========================================================================
//
//      common/kapi.cxx
//
//      C API Implementation
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Nick Garnett
// Copyright (C) 2003 Jonathan Larmour
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
// Author(s):   nickg, dsm
// Contributors:        nickg
// Date:        1998-03-02
// Purpose:     C API Implementation
// Description: C++ implementation of the C API
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#ifdef CYGFUN_KERNEL_API_C

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation
#include <cyg/kernel/diag.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>       // thread inlines
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/intr.hxx>
#include <cyg/kernel/clock.hxx>

#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/flag.hxx>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/mbox.hxx>

#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/clock.inl>        // clock inlines

#include <cyg/kernel/kapi.h>           // C API

// -------------------------------------------------------------------------
// Magic new function

inline void *operator new(size_t size, void *ptr)
{
    CYG_CHECK_DATA_PTR( ptr, "Bad pointer" );
    return ptr;
}

// -------------------------------------------------------------------------

#ifdef CYGDBG_USE_ASSERTS

#define CYG_ASSERT_SIZES(cstruct, cxxstruct)                      \
CYG_MACRO_START                                                   \
    char *msg = "Size of C struct " #cstruct                      \
                       " != size of C++ struct " #cxxstruct ;     \
    CYG_ASSERT( sizeof(cstruct) == sizeof(cxxstruct) , msg );     \
CYG_MACRO_END

#else

#define CYG_ASSERT_SIZES(cstruct, cxxstruct)

#endif

/*---------------------------------------------------------------------------*/
/* Scheduler operations */

/* Starts scheduler with created threads.  Never returns. */
externC void cyg_scheduler_start(void) __THROW
{
    Cyg_Scheduler::start();
}

/* Lock the scheduler. */
externC void cyg_scheduler_lock(void) __THROW
{
    Cyg_Scheduler::lock();
    // get_sched_lock() is unsigned, see below "cyg_ucount32 lock"
    CYG_ASSERT( (0xff000000 & (Cyg_Scheduler::get_sched_lock())) == 0,
                "Scheduler overlocked" );
}

/* Lock the scheduler, but never more than level=1. */
externC void cyg_scheduler_safe_lock(void) __THROW
{
    Cyg_Scheduler::lock();
    cyg_ucount32 slock = Cyg_Scheduler::get_sched_lock();
    if (slock > 1)
        Cyg_Scheduler::unlock();
    // get_sched_lock() is unsigned, see below "cyg_ucount32 lock"
    CYG_ASSERT( (0xff000000 & (Cyg_Scheduler::get_sched_lock())) == 0,
                "Scheduler overlocked" );
}

/* Unlock the scheduler. */
externC void cyg_scheduler_unlock(void) __THROW
{
    cyg_ucount32 slock = Cyg_Scheduler::get_sched_lock();
    CYG_ASSERT( 0 < slock, "Scheduler not locked" );
    // And program defensively too:
    if ( 0 < slock )
        Cyg_Scheduler::unlock();
}

/* Read the scheduler lock value. */
externC cyg_ucount32 cyg_scheduler_read_lock(void) __THROW
{
    cyg_ucount32 slock = Cyg_Scheduler::get_sched_lock();
    return slock;
}

/*---------------------------------------------------------------------------*/
/* Thread operations */

externC void cyg_thread_create(
    cyg_addrword_t      sched_info,             /* scheduling info (eg pri)  */
    cyg_thread_entry_t  *entry,                 /* entry point function      */
    cyg_addrword_t      entry_data,             /* entry data                */
    char                *name,                  /* optional thread name      */
    void                *stack_base,            /* stack base, NULL = alloc  */
    cyg_ucount32        stack_size,             /* stack size, 0 = default   */
    cyg_handle_t        *handle,                /* returned thread handle    */
    cyg_thread          *thread                 /* put thread here           */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_thread, Cyg_Thread );

    Cyg_Thread *t = new((void *)thread) Cyg_Thread (
        (CYG_ADDRWORD) sched_info,
        (cyg_thread_entry *)entry,
        (CYG_ADDRWORD) entry_data,
        name,
        (CYG_ADDRWORD) stack_base,
        stack_size
        );
    t=t;
    
    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)thread;
}

externC void cyg_thread_exit() __THROW
{
    Cyg_Thread::exit();
}

externC cyg_bool_t cyg_thread_delete( cyg_handle_t thread ) __THROW
{
    Cyg_Thread *th = (Cyg_Thread *)thread;
    if( th->get_state() != Cyg_Thread::EXITED )
        th->kill(); // encourage it to terminate
    if( th->get_state() != Cyg_Thread::EXITED )
        return false; // it didn't run yet, leave it up to the app to fix
    th->~Cyg_Thread();
    return true;
}

externC void cyg_thread_suspend(cyg_handle_t thread) __THROW
{
    ((Cyg_Thread *)thread)->suspend();
}

externC void cyg_thread_resume(cyg_handle_t thread) __THROW
{
    Cyg_Thread *th = (Cyg_Thread *)thread;

    // If we are resuming an exited thread then
    // reinitialize it.
    
    if( th->get_state() == Cyg_Thread::EXITED )
        th->reinitialize();

    th->resume();
}

externC void cyg_thread_kill( cyg_handle_t thread) __THROW
{
    ((Cyg_Thread *)thread)->kill();
}

externC void cyg_thread_release( cyg_handle_t thread) __THROW
{
    ((Cyg_Thread *)thread)->release();    
}

externC void cyg_thread_yield() __THROW
{
    Cyg_Thread::yield();
}

externC cyg_handle_t cyg_thread_self() __THROW
{
    return (cyg_handle_t)Cyg_Thread::self();
}

externC cyg_uint16 cyg_thread_get_id( cyg_handle_t thread) __THROW
{
    return ((Cyg_Thread *)thread)->get_unique_id();
}

// idle thread is not really a plain CygThread; danger.
externC cyg_handle_t cyg_thread_idle_thread() __THROW
{
    extern Cyg_Thread idle_thread;
    return (cyg_handle_t)&idle_thread;
}

/* Priority manipulation */
externC void cyg_thread_set_priority(
    cyg_handle_t thread, cyg_priority_t priority ) __THROW
{
#ifdef CYGIMP_THREAD_PRIORITY
    ((Cyg_Thread *)thread)->set_priority(priority);
#endif
}


/* Get the normal priority, ie without any applied mutex inheritance or
 * ceiling protocol. */
externC cyg_priority_t cyg_thread_get_priority(cyg_handle_t thread) __THROW
{
#ifdef CYGIMP_THREAD_PRIORITY
    return ((Cyg_Thread *)thread)->get_priority();
#else
    return 0;
#endif
}


/* Get the current priority, ie any applied mutex inheritance or
 * ceiling protocol. */
externC cyg_priority_t cyg_thread_get_current_priority(cyg_handle_t thread) __THROW
{
#ifdef CYGIMP_THREAD_PRIORITY
    return ((Cyg_Thread *)thread)->get_current_priority();
#else
    return 0;
#endif
}

/* Deadline scheduling control (optional) */

externC void cyg_thread_deadline_wait( 
    cyg_tick_count_t    start_time,             /* abs earliest start time   */
    cyg_tick_count_t    run_time,               /* worst case execution time */
    cyg_tick_count_t    deadline                /* absolute deadline         */
) __THROW
{
    CYG_ASSERT(0,"Not implemented");
} 

externC void cyg_thread_delay(cyg_tick_count_t delay) __THROW
{
    Cyg_Thread::self()->delay(delay);
}

/* Stack information */
externC cyg_addrword_t cyg_thread_get_stack_base(cyg_handle_t thread) __THROW
{
    return ((Cyg_Thread *)thread)->get_stack_base();
}

externC cyg_uint32 cyg_thread_get_stack_size(cyg_handle_t thread) __THROW
{
    return ((Cyg_Thread *)thread)->get_stack_size();
}

#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
externC cyg_uint32 cyg_thread_measure_stack_usage(cyg_handle_t thread) __THROW
{
    return ((Cyg_Thread *)thread)->measure_stack_usage();
}
#endif

/*---------------------------------------------------------------------------*/
/* Thread enumeration and information                                        */

#ifdef CYGVAR_KERNEL_THREADS_LIST

cyg_bool_t cyg_thread_get_next( cyg_handle_t *current, cyg_uint16 *id ) __THROW
{
    cyg_bool_t result = true;

    // There is a minute but finite chance that the thread could have
    // exitted since the previous cyg_thread_get_next() call, and we can't
    // detect the ID mismatch further down. So be quite zealous with checking.

    CYG_CHECK_DATA_PTRC( current );
    CYG_CHECK_DATA_PTRC( id );
    if ( *current != 0 )
        CYG_CHECK_DATA_PTRC( *current );

    Cyg_Scheduler::lock();

    Cyg_Thread *thread = (Cyg_Thread *)*current;
    CYG_ASSERT_ZERO_OR_CLASSC( thread );
    if( *current == 0 )
    {
        thread = Cyg_Thread::get_list_head();
        *current = (cyg_handle_t)thread;
        *id = thread->get_unique_id();
    }
    else if( (thread->get_unique_id() == *id) &&
             (thread = thread->get_list_next()) != NULL )
    {
        CYG_CHECK_DATA_PTRC( thread );
        CYG_ASSERT_CLASSC( thread );
        *current = (cyg_handle_t)thread;
        *id = thread->get_unique_id();
    }
    else
    {
        *current = 0;
        *id = 0;
        result = false;
    }
    
    Cyg_Scheduler::unlock();

    return result;
}

cyg_handle_t cyg_thread_find( cyg_uint16 id ) __THROW
{
    Cyg_Scheduler::lock();

    Cyg_Thread *thread = Cyg_Thread::get_list_head();

    while( thread != NULL )
    {
        if( thread->get_unique_id() == id )
            break;
        
        thread = thread->get_list_next();
    }

    Cyg_Scheduler::unlock();
    
    return (cyg_handle_t)thread;
}

#endif

cyg_bool_t cyg_thread_get_info( cyg_handle_t threadh,
                                cyg_uint16 id,
                                cyg_thread_info *info ) __THROW
{
    cyg_bool_t result = true;
    Cyg_Thread *thread = (Cyg_Thread *)threadh;
    CYG_CHECK_DATA_PTRC( thread );
    if ( NULL != info )
        CYG_CHECK_DATA_PTRC( info );
    
    Cyg_Scheduler::lock();
    
    if( thread->get_unique_id() == id && info != NULL )
    {
        CYG_ASSERT_CLASSC( thread );
        info->handle = threadh;
        info->id = id;
        info->state = thread->get_state();
#ifdef CYGVAR_KERNEL_THREADS_NAME
        info->name = thread->get_name();
#else
        info->name = NULL;
#endif
        info->set_pri = thread->get_priority();
        info->cur_pri = thread->get_current_priority();
        info->stack_base = thread->get_stack_base();
        info->stack_size = thread->get_stack_size();
        
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
        info->stack_used = thread->measure_stack_usage();
#else
        info->stack_used = 0;
#endif
    }
    else result = false;
    
    Cyg_Scheduler::unlock();

    return result;
}

/*---------------------------------------------------------------------------*/
/* Per-thread data                                                           */

#ifdef CYGVAR_KERNEL_THREADS_DATA

externC cyg_ucount32 cyg_thread_new_data_index() __THROW
{
    Cyg_Thread::cyg_data_index index = Cyg_Thread::new_data_index();
    CYG_ASSERT(index >= 0, "failed to allocate data index" );
    return index;
}

externC void cyg_thread_free_data_index(cyg_ucount32 index) __THROW
{
    Cyg_Thread::free_data_index(index);
}

externC CYG_ADDRWORD cyg_thread_get_data(cyg_ucount32 index) __THROW
{
    return Cyg_Thread::get_data(index);
}

externC CYG_ADDRWORD *cyg_thread_get_data_ptr(cyg_ucount32 index) __THROW
{
    return Cyg_Thread::get_data_ptr(index);
}

externC void cyg_thread_set_data(cyg_ucount32 index, CYG_ADDRWORD 
data) __THROW
{
    Cyg_Thread::self()->set_data(index, data);
}
#endif

/*---------------------------------------------------------------------------*/
/* Thread destructors                                                        */

#ifdef CYGPKG_KERNEL_THREADS_DESTRUCTORS
__externC cyg_bool_t
cyg_thread_add_destructor( cyg_thread_destructor_fn fn,
                           cyg_addrword_t data ) __THROW
{
        return Cyg_Thread::self()->add_destructor( fn, data );
}

__externC cyg_bool_t
cyg_thread_rem_destructor( cyg_thread_destructor_fn fn,
                           cyg_addrword_t data ) __THROW
{
        return Cyg_Thread::self()->rem_destructor( fn, data );
}
#endif

/*---------------------------------------------------------------------------*/
/* Exception handling.                                                       */

#ifdef CYGPKG_KERNEL_EXCEPTIONS    
externC void cyg_exception_set_handler(
    cyg_code_t                  exception_number,
    cyg_exception_handler_t     *new_handler,
    cyg_addrword_t              new_data,
    cyg_exception_handler_t     **old_handler,
    cyg_addrword_t              *old_data
) __THROW
{
    Cyg_Thread::register_exception(
        exception_number,
        (cyg_exception_handler *)new_handler,
        (CYG_ADDRWORD)new_data,
        (cyg_exception_handler **)old_handler,
        (CYG_ADDRWORD *)old_data
        );
}

/* Clear exception handler to default                                        */
externC void cyg_exception_clear_handler(
    cyg_code_t                  exception_number
) __THROW
{
    Cyg_Thread::deregister_exception( exception_number );
}

/* Invoke exception handler                                                  */
externC void cyg_exception_call_handler(
    cyg_handle_t                thread,
    cyg_code_t                  exception_number,
    cyg_addrword_t              error_code
) __THROW
{
    Cyg_Thread *t = (Cyg_Thread *)thread;

    t->deliver_exception( exception_number, error_code );
}
#endif    

/*---------------------------------------------------------------------------*/
/* Interrupt handling                                                        */

externC void cyg_interrupt_create(
    cyg_vector_t        vector,         /* Vector to attach to               */
    cyg_priority_t      priority,       /* Queue priority                    */
    cyg_addrword_t      data,           /* Data pointer                      */
    cyg_ISR_t           *isr,           /* Interrupt Service Routine         */
    cyg_DSR_t           *dsr,           /* Deferred Service Routine          */
    cyg_handle_t        *handle,        /* returned handle                   */
    cyg_interrupt       *intr           /* put interrupt here                */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_interrupt, Cyg_Interrupt );

    Cyg_Interrupt *t = new((void *)intr) Cyg_Interrupt (
        (cyg_vector)vector,
        (cyg_priority)priority,
        (CYG_ADDRWORD)data,
        (cyg_ISR *)isr,
        (cyg_DSR *)dsr );
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)intr;
}

externC void cyg_interrupt_delete( cyg_handle_t interrupt) __THROW
{
    ((Cyg_Interrupt *)interrupt)->~Cyg_Interrupt();
}

void cyg_interrupt_attach( cyg_handle_t interrupt ) __THROW
{
    ((Cyg_Interrupt *)interrupt)->attach();
}

void cyg_interrupt_detach( cyg_handle_t interrupt ) __THROW
{
    ((Cyg_Interrupt *)interrupt)->detach();
}

/* VSR manipulation */

externC void cyg_interrupt_get_vsr(
    cyg_vector_t        vector,         /* vector to get                     */
    cyg_VSR_t           **vsr           /* vsr got                           */
) __THROW
{
    Cyg_Interrupt::get_vsr( (cyg_vector)vector, (cyg_VSR **)vsr);
}

externC void cyg_interrupt_set_vsr(
    cyg_vector_t        vector,         /* vector to set                     */
    cyg_VSR_t           *vsr            /* vsr to set                        */
) __THROW
{
    Cyg_Interrupt::set_vsr( (cyg_vector)vector, (cyg_VSR *)vsr);
}

/* CPU level interrupt mask                                                  */
externC void cyg_interrupt_disable() __THROW
{
    Cyg_Interrupt::disable_interrupts();
}

externC void cyg_interrupt_enable() __THROW
{
    Cyg_Interrupt::enable_interrupts();
}

/* Interrupt controller access                                               */
externC void cyg_interrupt_mask(cyg_vector_t vector) __THROW
{
    Cyg_Interrupt::mask_interrupt( (cyg_vector)vector);
}

externC void cyg_interrupt_mask_intunsafe(cyg_vector_t vector) __THROW
{
    Cyg_Interrupt::mask_interrupt_intunsafe( (cyg_vector)vector);
}

externC void cyg_interrupt_unmask(cyg_vector_t vector) __THROW
{
    Cyg_Interrupt::unmask_interrupt( (cyg_vector)vector);
}

externC void cyg_interrupt_unmask_intunsafe(cyg_vector_t vector) __THROW
{
    Cyg_Interrupt::unmask_interrupt_intunsafe( (cyg_vector)vector);
}

externC void cyg_interrupt_acknowledge(cyg_vector_t vector) __THROW
{
    Cyg_Interrupt::acknowledge_interrupt( (cyg_vector)vector);
}


externC void cyg_interrupt_configure(
    cyg_vector_t        vector,         /* vector to configure               */
    cyg_bool_t          level,          /* level or edge triggered           */
    cyg_bool_t          up              /* rising/faling edge, high/low level*/
) __THROW
{
    Cyg_Interrupt::configure_interrupt( (cyg_vector)vector, level, up );
}

externC void cyg_interrupt_set_cpu(
    cyg_vector_t        vector,         /* vector to control                 */
    cyg_cpu_t           cpu             /* CPU to set                        */
) __THROW
{
#ifdef CYGPKG_KERNEL_SMP_SUPPORT    
    Cyg_Interrupt::set_cpu( vector, cpu );
#endif    
}

externC cyg_cpu_t cyg_interrupt_get_cpu(
    cyg_vector_t        vector          /* vector to control                 */
) __THROW
{
#ifdef CYGPKG_KERNEL_SMP_SUPPORT        
    return Cyg_Interrupt::get_cpu( vector );
#else
    return CYG_KERNEL_CPU_THIS();
#endif    
    
}

/*---------------------------------------------------------------------------*/
/* Counters, Clocks and Alarms                                               */

externC void cyg_counter_create(
    cyg_handle_t        *handle,        /* returned counter handle           */
    cyg_counter         *counter        /* put counter here                  */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_counter, Cyg_Counter );

    Cyg_Counter *t = new((void *)counter) Cyg_Counter ();
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)counter;
}

externC void cyg_counter_delete(cyg_handle_t counter) __THROW
{
    ((Cyg_Counter *)counter)->~Cyg_Counter();
}

/* Return current value of counter                                           */
externC cyg_tick_count_t cyg_counter_current_value(cyg_handle_t counter) __THROW
{
    return ((Cyg_Counter *)counter)->current_value();
}

/* Set new current value                                                     */
externC void cyg_counter_set_value(
    cyg_handle_t        counter,
    cyg_tick_count_t new_value
) __THROW
{
    ((Cyg_Counter *)counter)->set_value( new_value );
}

/* Advance counter by one tick                                               */
externC void cyg_counter_tick(cyg_handle_t counter) __THROW
{
    ((Cyg_Counter *)counter)->tick(); 
}

/* Advance counter by multiple ticks                                         */
externC void cyg_counter_multi_tick(cyg_handle_t counter, cyg_tick_count_t ticks) __THROW
{
    ((Cyg_Counter *)counter)->tick(ticks); 
}

/* Create a clock object                */
externC void cyg_clock_create(
    cyg_resolution_t    resolution,     /* Initial resolution                */
    cyg_handle_t        *handle,        /* Returned clock handle             */
    cyg_clock           *clock          /* put clock here                    */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_clock, Cyg_Clock );

    Cyg_Clock::cyg_resolution res;

    res.dividend = resolution.dividend;
    res.divisor  = resolution.divisor;

    Cyg_Clock *t = new((void *)clock) Cyg_Clock ( res );
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)clock;
}

externC void cyg_clock_delete(cyg_handle_t clock) __THROW
{
    ((Cyg_Clock *)clock)->~Cyg_Clock();
}

/* convert a clock handle to a counter handle so we can use the              */
/* counter API on it.                                                        */
externC void cyg_clock_to_counter(
    cyg_handle_t        clock,
    cyg_handle_t        *counter
) __THROW
{
    CYG_CHECK_DATA_PTR( counter, "Bad counter handle pointer" );
    *counter = (cyg_handle_t)(Cyg_Counter *)clock;
}

externC void cyg_clock_set_resolution(
    cyg_handle_t        clock,
    cyg_resolution_t    resolution      /* New resolution                    */
) __THROW
{
    Cyg_Clock::cyg_resolution res;

    res.dividend = resolution.dividend;
    res.divisor  = resolution.divisor;

    ((Cyg_Clock *)clock)->set_resolution( res );
}

externC cyg_resolution_t cyg_clock_get_resolution(cyg_handle_t clock) __THROW
{
    Cyg_Clock::cyg_resolution res =
        ((Cyg_Clock *)clock)->get_resolution();    

    cyg_resolution_t resolution;

    resolution.dividend = res.dividend;
    resolution.divisor  = res.divisor;

    return resolution;
}

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
externC cyg_handle_t cyg_real_time_clock(void) __THROW
{
    return (cyg_handle_t)Cyg_Clock::real_time_clock;
}

externC cyg_tick_count_t cyg_current_time(void) __THROW
{
    return Cyg_Clock::real_time_clock->current_value();
}
#endif

externC void cyg_alarm_create(
    cyg_handle_t        counter,        /* Attached to this counter          */
    cyg_alarm_t         *alarmfn,       /* Call-back function                */
    cyg_addrword_t      data,           /* Call-back data                    */
    cyg_handle_t        *handle,        /* Returned alarm object             */
    cyg_alarm           *alarm          /* put alarm here                    */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_alarm, Cyg_Alarm );

    Cyg_Alarm *t = new((void *)alarm) Cyg_Alarm (
        (Cyg_Counter *)counter,
        (cyg_alarm_fn *)alarmfn,
        (CYG_ADDRWORD)data
    );
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)alarm;
}

/* Disable alarm, detach from counter and invalidate handles                 */
externC void cyg_alarm_delete( cyg_handle_t alarm) __THROW
{
    ((Cyg_Alarm *)alarm)->~Cyg_Alarm();
}

externC void cyg_alarm_initialize(
    cyg_handle_t        alarm,
    cyg_tick_count_t    trigger,        /* Absolute trigger time             */
    cyg_tick_count_t    interval        /* Relative retrigger interval       */
) __THROW
{
    ((Cyg_Alarm *)alarm)->initialize(
        (cyg_tick_count)trigger,
        (cyg_tick_count)interval);
}

externC void cyg_alarm_get_times(
    cyg_handle_t        alarm,
    cyg_tick_count_t    *trigger,       /* Next trigger time                 */
    cyg_tick_count_t    *interval       /* Current interval                  */
) __THROW
{
    ((Cyg_Alarm *)alarm)->get_times(
        (cyg_tick_count*)trigger,
        (cyg_tick_count*)interval);
}

externC void cyg_alarm_enable( cyg_handle_t alarm ) __THROW
{
    ((Cyg_Alarm *)alarm)->enable();
}

externC void cyg_alarm_disable( cyg_handle_t alarm ) __THROW
{
    ((Cyg_Alarm *)alarm)->disable();
}

/*---------------------------------------------------------------------------*/
/* Mail boxes                                                                */

externC void cyg_mbox_create(
    cyg_handle_t        *handle,
    cyg_mbox            *mbox
) __THROW
{
    CYG_ASSERT_SIZES( cyg_mbox, Cyg_Mbox );
    
    Cyg_Mbox *t = new((void *)mbox) Cyg_Mbox();
    t=t;

    CYG_CHECK_DATA_PTR( handle, "Bad handle pointer" );
    *handle = (cyg_handle_t)mbox;
}

externC void cyg_mbox_delete(cyg_handle_t mbox) __THROW
{
    ((Cyg_Mbox *)mbox)->~Cyg_Mbox();
}

externC void *cyg_mbox_get(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->get();
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
void *cyg_mbox_timed_get(
    cyg_handle_t mbox,
    cyg_tick_count_t abstime
    ) __THROW
{
    return ((Cyg_Mbox *)mbox)->get(abstime);
}
#endif

externC void *cyg_mbox_tryget(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->tryget();
}

externC void *cyg_mbox_peek_item(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->peek_item();
}

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
externC cyg_bool_t cyg_mbox_put(cyg_handle_t mbox, void *item) __THROW
{
    return ((Cyg_Mbox *)mbox)->put(item);
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
externC cyg_bool_t cyg_mbox_timed_put(
    cyg_handle_t mbox,
    void *item,
    cyg_tick_count_t abstime
    ) __THROW
{
    return ((Cyg_Mbox *)mbox)->put(item, abstime);
}
#endif
#endif

externC cyg_bool_t cyg_mbox_tryput(cyg_handle_t mbox, void *item) __THROW
{
    return ((Cyg_Mbox *)mbox)->tryput(item);
}

externC cyg_count32 cyg_mbox_peek(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->peek();
}

externC cyg_bool_t cyg_mbox_waiting_to_get(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->waiting_to_get();
}

externC cyg_bool_t cyg_mbox_waiting_to_put(cyg_handle_t mbox) __THROW
{
    return ((Cyg_Mbox *)mbox)->waiting_to_put();
}


/*---------------------------------------------------------------------------*/
/* Semaphores                                                                */

externC void      cyg_semaphore_init(
    cyg_sem_t           *sem,            /* Semaphore to init                */
    cyg_count32         val              /* Initial semaphore value          */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_sem_t, Cyg_Counting_Semaphore );
    
    Cyg_Counting_Semaphore *t = new((void *)sem) Cyg_Counting_Semaphore(val);
    t=t;
}

externC void cyg_semaphore_destroy( cyg_sem_t *sem ) __THROW
{
    ((Cyg_Counting_Semaphore *)sem)->~Cyg_Counting_Semaphore();
}

externC cyg_bool_t cyg_semaphore_wait( cyg_sem_t *sem ) __THROW
{
    return ((Cyg_Counting_Semaphore *)sem)->wait();
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
externC cyg_bool_t cyg_semaphore_timed_wait(
    cyg_sem_t          *sem,
    cyg_tick_count_t   abstime
    ) __THROW
{
    return ((Cyg_Counting_Semaphore *)sem)->wait(abstime);
}
#endif


externC int cyg_semaphore_trywait( cyg_sem_t *sem ) __THROW
{
    return ((Cyg_Counting_Semaphore *)sem)->trywait();
}

externC void cyg_semaphore_post( cyg_sem_t *sem ) __THROW
{
    ((Cyg_Counting_Semaphore *)sem)->post();
}

externC void cyg_semaphore_peek( cyg_sem_t *sem, cyg_count32 *val ) __THROW
{
    CYG_CHECK_DATA_PTR( val, "Bad val parameter" );

    *val = ((Cyg_Counting_Semaphore *)sem)->peek();
}


/*---------------------------------------------------------------------------*/
/* Flags                                                                     */

void cyg_flag_init(
    cyg_flag_t        *flag             /* Flag to init                      */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_flag_t, Cyg_Flag );
    CYG_ASSERT(
        ( Cyg_Flag::AND == CYG_FLAG_WAITMODE_AND ) &&
        ( Cyg_Flag::OR  == CYG_FLAG_WAITMODE_OR  ) &&
        ( Cyg_Flag::CLR == CYG_FLAG_WAITMODE_CLR ),
        "CYG_FLAG_WAITMODE_xxx definition != C++ Cyg_Flag::xxx" );
    
    Cyg_Flag *t = new((void *)flag) Cyg_Flag();
    t=t;
}

void cyg_flag_destroy( cyg_flag_t *flag ) __THROW
{
    ((Cyg_Flag *)flag)->~Cyg_Flag();
}

void cyg_flag_setbits( cyg_flag_t *flag, cyg_flag_value_t value) __THROW
{
    ((Cyg_Flag *)flag)->setbits( value ); 
}

void cyg_flag_maskbits( cyg_flag_t *flag, cyg_flag_value_t value) __THROW
{
    ((Cyg_Flag *)flag)->maskbits( value ); 
}

cyg_flag_value_t cyg_flag_wait( cyg_flag_t        *flag,
                                cyg_flag_value_t   pattern, 
                                cyg_flag_mode_t    mode ) __THROW
{
    if ( 0 == pattern || 0 != (mode & ~3) )
        return 0;
    return ((Cyg_Flag *)flag)->wait( pattern, mode );

}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
cyg_flag_value_t cyg_flag_timed_wait( cyg_flag_t        *flag,
                                      cyg_flag_value_t   pattern, 
                                      cyg_flag_mode_t    mode,
                                      cyg_tick_count_t   abstime ) __THROW
{
    if ( 0 == pattern || 0 != (mode & ~3) )
        return 0;
    return ((Cyg_Flag *)flag)->wait( pattern, mode, abstime );

}
#endif

cyg_flag_value_t cyg_flag_poll( cyg_flag_t         *flag,
                                cyg_flag_value_t    pattern, 
                                cyg_flag_mode_t     mode ) __THROW
{
    if ( 0 == pattern || 0 != (mode & ~3) )
        return 0;
    return ((Cyg_Flag *)flag)->poll( pattern, mode );

}

cyg_flag_value_t cyg_flag_peek( cyg_flag_t *flag ) __THROW
{
    return ((Cyg_Flag *)flag)->peek();
}

cyg_bool_t cyg_flag_waiting( cyg_flag_t *flag ) __THROW
{
    return ((Cyg_Flag *)flag)->waiting();
}

/*---------------------------------------------------------------------------*/
/* Mutex                                                                     */

externC void cyg_mutex_init(
    cyg_mutex_t        *mutex          /* Mutex to init                      */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_mutex_t, Cyg_Mutex );
    
    Cyg_Mutex *m = new((void *)mutex) Cyg_Mutex;

    m=m;
}

externC void cyg_mutex_destroy( cyg_mutex_t *mutex ) __THROW
{
    ((Cyg_Mutex *)mutex)->~Cyg_Mutex();
}

externC cyg_bool_t cyg_mutex_lock( cyg_mutex_t *mutex ) __THROW
{
    return ((Cyg_Mutex *)mutex)->lock();
}

externC cyg_bool_t cyg_mutex_trylock( cyg_mutex_t *mutex ) __THROW
{
    return ((Cyg_Mutex *)mutex)->trylock();
}

externC void cyg_mutex_unlock( cyg_mutex_t *mutex ) __THROW
{
    ((Cyg_Mutex *)mutex)->unlock();
}

externC void cyg_mutex_release( cyg_mutex_t *mutex ) __THROW
{
    ((Cyg_Mutex *)mutex)->release();
}

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
externC void cyg_mutex_set_ceiling( 
    cyg_mutex_t *mutex, 
    cyg_priority_t priority ) __THROW
{
    ((Cyg_Mutex *)mutex)->set_ceiling(priority);
}
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
externC void cyg_mutex_set_protocol( 
    cyg_mutex_t *mutex,
    enum cyg_mutex_protocol protocol ) __THROW
{
    ((Cyg_Mutex *)mutex)->set_protocol((Cyg_Mutex::cyg_protcol)protocol);
}
#endif

/*---------------------------------------------------------------------------*/
/* Condition Variables                                                       */

externC void cyg_cond_init(
    cyg_cond_t          *cond,          /* condition variable to init        */
    cyg_mutex_t         *mutex          /* associated mutex                  */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_cond_t, Cyg_Condition_Variable );
    
    Cyg_Condition_Variable *t = new((void *)cond) Cyg_Condition_Variable(
        *(Cyg_Mutex *)mutex);
    t=t;
}

externC void cyg_cond_destroy( cyg_cond_t *cond ) __THROW
{
    ((Cyg_Condition_Variable *)cond)->~Cyg_Condition_Variable();
}

externC cyg_bool_t cyg_cond_wait( cyg_cond_t *cond ) __THROW
{
    return ((Cyg_Condition_Variable *)cond)->wait();
}

externC void cyg_cond_signal( cyg_cond_t *cond ) __THROW
{
    ((Cyg_Condition_Variable *)cond)->signal();
}

externC void cyg_cond_broadcast( cyg_cond_t *cond ) __THROW
{
    ((Cyg_Condition_Variable *)cond)->broadcast();
}

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT
externC cyg_bool_t cyg_cond_timed_wait(
    cyg_cond_t        *cond,
    cyg_tick_count_t  abstime
    ) __THROW
{
    return ((Cyg_Condition_Variable *)cond)->wait(abstime);
}

#endif

/*---------------------------------------------------------------------------*/
/* Spinlocks                                                                 */

externC void cyg_spinlock_init(
    cyg_spinlock_t      *lock,          /* spinlock to initialize            */
    cyg_bool_t          locked          /* init locked or unlocked           */
) __THROW
{
    CYG_ASSERT_SIZES( cyg_spinlock_t, Cyg_SpinLock );

    // Create the spinlock in cleared state
    Cyg_SpinLock *t = new((void *)lock) Cyg_SpinLock();

    // If the lock is to start locked, then lock it now.
    if( locked )
        t->spin();
}

externC void cyg_spinlock_destroy( cyg_spinlock_t *lock ) __THROW
{
    ((Cyg_SpinLock *)lock)->~Cyg_SpinLock();
}

externC void cyg_spinlock_spin( cyg_spinlock_t *lock ) __THROW
{
    ((Cyg_SpinLock *)lock)->spin();
}

externC void cyg_spinlock_clear( cyg_spinlock_t *lock ) __THROW
{
    ((Cyg_SpinLock *)lock)->clear();
}

externC cyg_bool_t cyg_spinlock_try( cyg_spinlock_t *lock ) __THROW
{
    return ((Cyg_SpinLock *)lock)->trylock();
}

externC cyg_bool_t cyg_spinlock_test( cyg_spinlock_t *lock ) __THROW
{
    return ((Cyg_SpinLock *)lock)->test();
}

externC void cyg_spinlock_spin_intsave( cyg_spinlock_t *lock,
                                cyg_addrword_t *istate ) __THROW
{
    ((Cyg_SpinLock *)lock)->spin_intsave((CYG_INTERRUPT_STATE *)istate);
}
    
externC void cyg_spinlock_clear_intsave( cyg_spinlock_t *lock,
                                 cyg_addrword_t istate ) __THROW
{
    ((Cyg_SpinLock *)lock)->clear_intsave((CYG_INTERRUPT_STATE)istate);
}
    

// -------------------------------------------------------------------------
// Check structure sizes.
// This class and constructor get run automatically in debug versions
// of the kernel and check that the structures configured in the C
// code are the same size as the C++ classes they should match.

#ifdef CYGPKG_INFRA_DEBUG

class Cyg_Check_Structure_Sizes
{
    int dummy;
public:    
    Cyg_Check_Structure_Sizes( int x ) __THROW;

};

#define CYG_CHECK_SIZES(cstruct, cxxstruct)                               \
if( sizeof(cstruct) != sizeof(cxxstruct) )                                \
{                                                                         \
    char *fmt = "Size of C struct " #cstruct                              \
                " != size of C++ struct " #cxxstruct ;                    \
    CYG_TRACE2(1, fmt, sizeof(cstruct) , sizeof(cxxstruct) );             \
    fail = true;                                                          \
    fmt = fmt;                                                            \
}

Cyg_Check_Structure_Sizes::Cyg_Check_Structure_Sizes(int x) __THROW
{
    cyg_bool fail = false;

    dummy = x+1;
    
    CYG_CHECK_SIZES( cyg_thread, Cyg_Thread );
    CYG_CHECK_SIZES( cyg_interrupt, Cyg_Interrupt );
    CYG_CHECK_SIZES( cyg_counter, Cyg_Counter );
    CYG_CHECK_SIZES( cyg_clock, Cyg_Clock );
    CYG_CHECK_SIZES( cyg_alarm, Cyg_Alarm );
    CYG_CHECK_SIZES( cyg_mbox, Cyg_Mbox );
    CYG_CHECK_SIZES( cyg_sem_t, Cyg_Counting_Semaphore );
    CYG_CHECK_SIZES( cyg_flag_t, Cyg_Flag );
    CYG_CHECK_SIZES( cyg_mutex_t, Cyg_Mutex );
    CYG_CHECK_SIZES( cyg_cond_t, Cyg_Condition_Variable );
    CYG_CHECK_SIZES( cyg_spinlock_t, Cyg_SpinLock );
    
    CYG_ASSERT( !fail, "Size checks failed");
}

static Cyg_Check_Structure_Sizes cyg_kapi_check_structure_sizes(1);

#endif


// -------------------------------------------------------------------------

#endif
// EOF common/kapi.cxx
