#ifndef CYGONCE_KERNEL_THREAD_INL
#define CYGONCE_KERNEL_THREAD_INL

//==========================================================================
//
//      thread.inl
//
//      Thread class inlines
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:        1997-09-09
// Purpose:     Define inlines for thread classes
// Description: Inline implementations of various member functions defined
//              in various Thread classes. 
// Usage:
//              #include <cyg/kernel/thread.hxx>
//              ...
//              #include <cyg/kernel/thread.inl>
//              ...

//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/thread.hxx>
#include <cyg/hal/hal_arch.h>

#include <cyg/kernel/clock.inl>
#include <cyg/infra/diag.h>

#ifndef CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE
#define CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE (0)
#endif

//==========================================================================
// Inlines for Cyg_HardwareThread

// -------------------------------------------------------------------------
// get the size/base of this thread's stack

inline CYG_ADDRESS
Cyg_HardwareThread::get_stack_base()
{
    return stack_base - CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE;
}

inline cyg_uint32
Cyg_HardwareThread::get_stack_size()
{
    return stack_size + 2 * CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE;
}

// -------------------------------------------------------------------------
// Check the stack bounds of this thread:
#ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING
inline void Cyg_HardwareThread::check_stack(void)
{
    cyg_uint32 sig = (cyg_uint32)this;
    cyg_uint32 *base = (cyg_uint32 *)get_stack_base();
    cyg_uint32 *top =  (cyg_uint32 *)(stack_base + stack_size);
    cyg_ucount32 i;

    CYG_INSTRUMENT_THREAD(CHECK_STACK, base, top );
    
    CYG_ASSERT( 0 == ((sizeof(CYG_WORD)-1) & (cyg_uint32)base), "stack base not word aligned" );
    CYG_ASSERT( 0 == ((sizeof(CYG_WORD)-1) & (cyg_uint32)top),  "stack  top not word aligned" );

    CYG_ASSERT( (cyg_uint32)stack_ptr > (cyg_uint32)stack_base,
                "Stack_ptr below base" );
    CYG_ASSERT( (cyg_uint32)stack_ptr <= ((cyg_uint32)stack_base + stack_size),
                "Stack_ptr above top" );

    for ( i = 0;
          i < CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE/sizeof(cyg_uint32);
          i++ ) {
        if ((sig ^ (i * 0x01010101)) != base[i]) {
            char *reason = "Stack base corrupt";
            diag_printf("%s - i: %d\n", reason, i);
            diag_dump_buf(base, CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE);
            CYG_FAIL(reason);
        }
        if ((sig ^ (i * 0x10101010)) != top[i]) {
            char *reason = "Stack top corrupt";
            diag_printf("%s - i: %d\n", reason, i);
            diag_dump_buf(top, CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE);
            CYG_FAIL(reason);
        }
    }            

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    // we won't have added check data above the stack limit if it hasn't
    // been incremented
    if (stack_limit != stack_base) {
        CYG_ADDRESS limit = stack_limit;
        // the limit will be off by the check data size, so lets correct it
        limit -= CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE;
        
        // determine base of check data by rounding up to nearest word aligned
        // address if not already aligned
        cyg_uint32 *p = (cyg_uint32 *)((limit + 3) & ~3);
        // i.e. + sizeof(cyg_uint32)-1) & ~(sizeof(cyg_uint32)-1);
        
        for ( i = 0;
              i < CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE/sizeof(cyg_uint32);
              i++ ) {
            if ((sig ^ (i * 0x01010101)) != p[i]) {
                char *reason = "Gap between stack limit and base corrupt";
                diag_printf("%s - i: %d\n", reason, i);
                diag_dump_buf(p, CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE);
                CYG_FAIL(reason);
            }
        }
    }
#endif
}
#endif

// -------------------------------------------------------------------------
// Measure the stack usage of the thread
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
inline cyg_uint32 Cyg_HardwareThread::measure_stack_usage(void)
{
#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    CYG_WORD *base = (CYG_WORD *)stack_limit;
    cyg_uint32 size = (stack_size - (stack_limit-stack_base))/sizeof(CYG_WORD);
#else
    CYG_WORD *base = (CYG_WORD *)stack_base;
    cyg_uint32 size = stack_size/sizeof(CYG_WORD);
#endif
    cyg_ucount32 i;

    // Work up the stack comparing with the preset value
    // We assume the stack grows downwards, hmm...
    for (i=0; i<size; i++) {
	if (base[i] != 0xDEADBEEF)
	  break;
    }
    return (size - i)*sizeof(CYG_WORD);
}
#endif

// -------------------------------------------------------------------------
// Attach a stack to this thread. If there is a HAL defined macro to
// do this, then we use that, otherwise assume a falling stack.
inline void Cyg_HardwareThread::attach_stack(CYG_ADDRESS s_base, cyg_uint32 s_size)
{
#ifdef CYGNUM_HAL_STACK_SIZE_MINIMUM
    CYG_ASSERT( s_size >= CYGNUM_HAL_STACK_SIZE_MINIMUM,
                "Stack size too small");
#endif

#ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING
    {
        cyg_uint32 sig = (cyg_uint32)this;
        cyg_uint32 *base = (cyg_uint32 *)s_base;
        cyg_uint32 *top =  (cyg_uint32 *)(s_base + s_size -
            CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE);

        unsigned int i;

        CYG_INSTRUMENT_THREAD(ATTACH_STACK, base, top );
        
        CYG_ASSERT( NULL != base, "stack base non-NULL" );
        CYG_ASSERT( 0 == ((sizeof(CYG_WORD)-1) & (cyg_uint32)base), "stack base alignment" );
        CYG_ASSERT( 0 == ((sizeof(CYG_WORD)-1) & (cyg_uint32)top),  "stack  top alignment" );

        for ( i = 0;
              i < CYGNUM_KERNEL_THREADS_STACK_CHECK_DATA_SIZE/sizeof(cyg_uint32);
              i++ ) {
            base[i] = (sig ^ (i * 0x01010101));
             top[i] = (sig ^ (i * 0x10101010));
        }            
        // This check for overlap of the two signature areas also detects
        // wrap round zero of the size in the unsigned subtraction below.
        CYG_ASSERT( &base[i] < &top[0], "Stack is so small size wrapped" );
        // Use this 'i' expression to round correctly to whole words.
        s_base += i * sizeof(cyg_uint32);
        s_size -= i * sizeof(cyg_uint32) * 2;
        // This is a complete guess, the 256; the point is to assert early that
        // this might go badly wrong.  It would not detect wrap of unsigned size.
        CYG_ASSERT( s_size >= 256,
                    "Stack size too small after allocating checking buffer");
    }
#endif
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
    {
	CYG_WORD *base = (CYG_WORD *)s_base;
	cyg_uint32 size = s_size/sizeof(CYG_WORD);
	cyg_ucount32 i;

	// initialize all of stack with known value - don't choose 0
	// could do with pseudo value as above, but this way, checking
	// is faster
	for (i=0; i<size; i++) {
		base[i] = 0xDEADBEEF;
	}
	// Don't bother about the case when the stack isn't a multiple of
	// CYG_WORD in size. Since it's at the top of the stack, it will
	// almost certainly be overwritten the instant the thread starts
	// anyway.
    }
#endif
    stack_base = s_base;
    stack_size = s_size;
#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    stack_limit = s_base;
#endif
    
#ifdef HAL_THREAD_ATTACH_STACK

    HAL_THREAD_ATTACH_STACK(stack_ptr, stack_base, stack_size);
    
#else

    stack_ptr = stack_base + stack_size;

#endif

#ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING
    check_stack();
#endif
}

// -------------------------------------------------------------------------

inline Cyg_HardwareThread::Cyg_HardwareThread(
    cyg_thread_entry        *e_point,   // entry point function
    CYG_ADDRWORD            e_data,     // entry data
    cyg_ucount32            s_size,     // stack size, 0 = use default
    CYG_ADDRESS             s_base      // stack base, NULL = allocate
)
{
    entry_point = e_point;
    entry_data  = e_data;
#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
    saved_context = 0;
#endif
    
    attach_stack( s_base, s_size );
};

// -------------------------------------------------------------------------

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

// Return the current saved state for this thread.
inline HAL_SavedRegisters *Cyg_HardwareThread::get_saved_context()
{
    HAL_SavedRegisters *regs;
    if( saved_context != 0 ) regs = saved_context;
    else HAL_THREAD_GET_SAVED_REGISTERS( stack_ptr, regs );
    return regs;
}

inline void Cyg_HardwareThread::set_saved_context(HAL_SavedRegisters *ctx)
{
    saved_context = ctx;
}

#endif

// -------------------------------------------------------------------------
// (declare this inline before its first use)

inline cyg_uint16 Cyg_Thread::get_unique_id()
{
    return unique_id;
}

// -------------------------------------------------------------------------
// Initialize the context of this thread.

inline void Cyg_HardwareThread::init_context(Cyg_Thread *thread)
{
#ifdef CYGPKG_INFRA_DEBUG
    cyg_uint32 threadid = thread->get_unique_id()*0x01010000;
#else
    cyg_uint32 threadid = 0x11110000;
#endif
    HAL_THREAD_INIT_CONTEXT( stack_ptr, thread, thread_entry, threadid );
}



// -------------------------------------------------------------------------
// Save current thread's context and load that of the given next thread.
// This function is only really here for completeness, the
// kernel generally calls the HAL macros directly.

inline void Cyg_HardwareThread::switch_context(Cyg_HardwareThread *next)
{
    HAL_THREAD_SWITCH_CONTEXT( &stack_ptr, &next->stack_ptr );
}

// -------------------------------------------------------------------------
// Get and set entry_data.

inline void Cyg_HardwareThread::set_entry_data( CYG_ADDRWORD data )
{
    entry_data = data;
}

inline CYG_ADDRWORD Cyg_HardwareThread::get_entry_data()
{
    return entry_data;
}

// -------------------------------------------------------------------------
// Allocate some memory at the lower end of the stack
// by moving the stack limit pointer.

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT

#ifndef CYGFUN_KERNEL_THREADS_STACK_CHECKING
// if stack checking, implementation is in thread.cxx
inline void *Cyg_HardwareThread::increment_stack_limit( cyg_ucount32 size )
{
    void *ret = (void *)stack_limit;
    stack_limit += size;
    return ret;
}
#endif
    
inline CYG_ADDRESS
Cyg_HardwareThread::get_stack_limit()
{
    return stack_limit;
}

#endif    

//==========================================================================
// Inlines for Cyg_Thread class

inline Cyg_Thread *Cyg_Thread::self()
{
    return Cyg_Scheduler::get_current_thread();
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::yield()
{
    self()->Cyg_SchedThread::yield();
}

// -------------------------------------------------------------------------

inline void
Cyg_Thread::rotate_queue( cyg_priority pri )
{
    self()->Cyg_SchedThread::rotate_queue( pri );
}

// -------------------------------------------------------------------------

inline void
Cyg_Thread::to_queue_head( void )
{
    this->Cyg_SchedThread::to_queue_head();
}

// -------------------------------------------------------------------------

#ifdef CYGIMP_THREAD_PRIORITY

inline cyg_priority Cyg_Thread::get_priority()
{
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_SIMPLE

    // If we have an inherited priority, return our original
    // priority rather than the current one.
    
    if( priority_inherited ) return original_priority;

#endif

    return priority;
}

// Return the actual dispatching priority of the thread
// regardless of inheritance or scheduling concerns.
inline cyg_priority Cyg_Thread::get_current_priority()
{
    return priority;
}

#endif

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_sleep_reason( cyg_reason reason)
{
    self()->sleep_reason = reason;
    self()->wake_reason = NONE;
}

// -------------------------------------------------------------------------

inline Cyg_Thread::cyg_reason Cyg_Thread::get_sleep_reason()
{
    return sleep_reason;
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_wake_reason( cyg_reason reason )
{
    sleep_reason = NONE;
    wake_reason = reason;
}

// -------------------------------------------------------------------------

inline Cyg_Thread::cyg_reason Cyg_Thread::get_wake_reason()
{
    return wake_reason;
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::set_timer(
    cyg_tick_count      trigger,
    cyg_reason          reason
)
{
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    self()->sleep_reason = reason;
    self()->wake_reason = NONE;
    self()->timer.initialize( trigger);
#endif
}

// -------------------------------------------------------------------------

inline void Cyg_Thread::clear_timer()
{
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    self()->timer.disable();
#endif
}

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_DATA

inline CYG_ADDRWORD Cyg_Thread::get_data( Cyg_Thread::cyg_data_index index )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");
    
    return self()->thread_data[index];
}

inline CYG_ADDRWORD *Cyg_Thread::get_data_ptr( Cyg_Thread::cyg_data_index index )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");
    
    return &(self()->thread_data[index]);
}

inline void Cyg_Thread::set_data( Cyg_Thread::cyg_data_index index,
                                  CYG_ADDRWORD data )
{
    CYG_ASSERT( index < CYGNUM_KERNEL_THREADS_DATA_MAX,
                "Per thread data index out of bounds");
    CYG_ASSERT( (thread_data_map & (1<<index)) == 0,
                "Unallocated index used");

    thread_data[index] = data;
}

#endif

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_NAME

inline char *Cyg_Thread::get_name()
{
    return name;
}

#endif

// -------------------------------------------------------------------------

#ifdef CYGVAR_KERNEL_THREADS_LIST

inline Cyg_Thread *Cyg_Thread::get_list_head()
{
    return thread_list?thread_list->list_next:0;
}
    
inline Cyg_Thread *Cyg_Thread::get_list_next()
{
    return (this==thread_list)?0:list_next;
}

#endif


// -------------------------------------------------------------------------

#ifdef CYGPKG_KERNEL_EXCEPTIONS

inline void Cyg_Thread::register_exception(
    cyg_code                exception_number,       // exception number
    cyg_exception_handler   handler,                // handler function
    CYG_ADDRWORD            data,                   // data argument
    cyg_exception_handler   **old_handler,          // handler function
    CYG_ADDRWORD            *old_data               // data argument
    )
{
    self()->exception_control.register_exception(
        exception_number,
        handler,
        data,
        old_handler,
        old_data
        );
}

inline void Cyg_Thread::deregister_exception(
    cyg_code                exception_number        // exception number
    )
{
    self()->exception_control.deregister_exception(
        exception_number
        );
}

#endif

//==========================================================================
// Inlines for Cyg_ThreadTimer class

// -------------------------------------------------------------------------
#if defined(CYGFUN_KERNEL_THREADS_TIMER) && defined(CYGVAR_KERNEL_COUNTERS_CLOCK)

inline Cyg_ThreadTimer::Cyg_ThreadTimer(
    Cyg_Thread  *th
    )
    : Cyg_Alarm(Cyg_Clock::real_time_clock,
                &alarm,
                CYG_ADDRWORD(this)
                )
{
    thread = th;
}

#endif

//==========================================================================
// Inlines for Cyg_ThreadQueue class


inline void Cyg_ThreadQueue::enqueue(Cyg_Thread *thread)
{
    Cyg_ThreadQueue_Implementation::enqueue(thread);
}

// -------------------------------------------------------------------------

inline Cyg_Thread *Cyg_ThreadQueue::highpri()
{
    return Cyg_ThreadQueue_Implementation::highpri();
}

// -------------------------------------------------------------------------

inline Cyg_Thread *Cyg_ThreadQueue::dequeue()
{
    return Cyg_ThreadQueue_Implementation::dequeue();
}

// -------------------------------------------------------------------------

inline void Cyg_ThreadQueue::remove(Cyg_Thread *thread)
{
    Cyg_ThreadQueue_Implementation::remove(thread);
}

// -------------------------------------------------------------------------

inline cyg_bool Cyg_ThreadQueue::empty()
{
    return Cyg_ThreadQueue_Implementation::empty();
}

// -------------------------------------------------------------------------

#ifdef CYGPKG_KERNEL_THREADS_DESTRUCTORS

#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
# include <cyg/kernel/sched.inl>
#endif

// Add and remove destructors. Returns true on success, false on failure.
inline cyg_bool
Cyg_Thread::add_destructor( destructor_fn fn, CYG_ADDRWORD data )
{
    cyg_ucount16 i;
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    Cyg_Scheduler::lock();
#endif
    for (i=0; i<CYGNUM_KERNEL_THREADS_DESTRUCTORS; i++) {
        if (NULL == destructors[i].fn) {
            destructors[i].data = data;
            destructors[i].fn = fn;
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
            Cyg_Scheduler::unlock();
#endif
            return true;
        }
    }
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    Cyg_Scheduler::unlock();
#endif
    return false;
}

inline cyg_bool
Cyg_Thread::rem_destructor( destructor_fn fn, CYG_ADDRWORD data )
{
    cyg_ucount16 i;
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    Cyg_Scheduler::lock();
#endif
    for (i=0; i<CYGNUM_KERNEL_THREADS_DESTRUCTORS; i++) {
        if (destructors[i].fn == fn && destructors[i].data == data) {
            destructors[i].fn = NULL;
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
            Cyg_Scheduler::unlock();
#endif
            return true;
        }
    }
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    Cyg_Scheduler::unlock();
#endif
    return false;
}
#endif

// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_THREAD_INL
// EOF thread.inl
