#ifndef CYGONCE_KERNEL_THREAD_HXX
#define CYGONCE_KERNEL_THREAD_HXX

//==========================================================================
//
//      thread.hxx
//
//      Thread class declarations
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
// Date:        1997-09-09
// Purpose:     Define Thread class interfaces
// Description: The classes defined here collectively implement the
//              internal API used to create, configure and manage threads.
// Usage:       #include <cyg/kernel/thread.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/except.hxx>

#include <cyg/hal/hal_arch.h>

// -------------------------------------------------------------------------
// Miscellaneous types

typedef void cyg_thread_entry(CYG_ADDRWORD data);// Thread entry point function

// -------------------------------------------------------------------------
// Hardware thread interface.
// The implementation of this class is provided by the HAL.

class Cyg_HardwareThread
{
    friend class Cyg_Scheduler;

protected:

    CYG_ADDRESS         stack_base;     // pointer to base of stack area

    cyg_uint32          stack_size;     // size of stack area in bytes

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
    CYG_ADDRESS         stack_limit;    // movable stack limit
#endif    

    CYG_ADDRESS         stack_ptr;      // pointer to saved state on stack

    cyg_thread_entry    *entry_point;   // main entry point (code pointer!)

    CYG_ADDRWORD        entry_data;     // entry point argument

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

    HAL_SavedRegisters  *saved_context; // If non-zero, this points at a more
                                        // interesting context than stack_ptr.
#endif
    
    Cyg_HardwareThread(
        cyg_thread_entry        *entry_point,   // entry point function
        CYG_ADDRWORD            entry_data,     // entry data
        cyg_ucount32            stack_size = 0, // stack size, 0 = use default
        CYG_ADDRESS             stack_base = 0  // stack base, NULL = allocate
    );

    // Thread entry point. This is where all threads begin execution.
    // This routine does a little housekeeping and then call the main
    // entry_point specified above.
    static void thread_entry(Cyg_Thread *thread);

    // Initialize the context of the thread to start execution at thread_entry
    void    init_context( Cyg_Thread *thread );
    
    // Save current thread's context and load that of the given next thread.
    void    switch_context(Cyg_HardwareThread *next);

    // attach a stack to this thread
    void    attach_stack(CYG_ADDRESS stack, cyg_uint32 stack_size);

    // detach the stack from this thread
    CYG_ADDRESS detach_stack();

    // Adjust the thread's saved state to call the exception
    // handler when next executed.
    void    prepare_exception (
        cyg_exception_handler   *exception_handler,
        CYG_ADDRWORD            exception_data,
        cyg_code                exception_number,
        CYG_ADDRWORD            exception_info
        );

public:

    CYGDBG_DEFINE_CHECK_THIS    

    // Get and set entry_data.

    void set_entry_data( CYG_ADDRWORD data );

    CYG_ADDRWORD get_entry_data();

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT    
    // Return the current saved state for this thread.
    HAL_SavedRegisters *get_saved_context();

    // Set the saved context pointer.
    void set_saved_context(HAL_SavedRegisters *ctx);
#endif

    // get the size/base of this thread's stack
    CYG_ADDRESS get_stack_base();

    cyg_uint32 get_stack_size();

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT    
    // Allocate some memory at the lower end of the stack
    // by moving the stack limit pointer.

    void *increment_stack_limit( cyg_ucount32 size);
    
    CYG_ADDRESS get_stack_limit();
#endif    

#ifdef CYGFUN_KERNEL_THREADS_STACK_CHECKING

    inline void check_stack(void);

#endif
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT

    inline cyg_uint32 measure_stack_usage(void);

#endif
};

// -------------------------------------------------------------------------
// Per-thread timer support class.
// This is only included when required.

#ifdef CYGFUN_KERNEL_THREADS_TIMER

class Cyg_ThreadTimer
    : public Cyg_Alarm
{
    friend class Cyg_Thread;

    // Pointer to current thread
    Cyg_Thread          *thread;

    // Constructor
    Cyg_ThreadTimer(
        Cyg_Thread      *thread
        );

    // Alarm function
    static void alarm( Cyg_Alarm *alarm, CYG_ADDRWORD data);

    CYGDBG_DEFINE_CHECK_THIS

};

#endif

// -------------------------------------------------------------------------
// Main Thread class.
// This provides the public API for controlling threads.

class Cyg_Thread
    : public Cyg_HardwareThread,       // provides hardware abstractions
      public Cyg_SchedThread           // provides scheduling abstractions
{
    friend class Cyg_Scheduler;
    friend void deliver_exception( CYG_WORD code, CYG_ADDRWORD data );
    
    // The following definitions are used by all variants of the
    // basic thread object.

public:    
    enum {                       // Thread state values
        
        RUNNING    = 0,          // Thread is runnable or running
        SLEEPING   = 1,          // Thread is waiting for something to happen
        COUNTSLEEP = 2,          // Sleep in counted manner
        SUSPENDED  = 4,          // Suspend count is non-zero
        CREATING   = 8,          // Thread is being created
        EXITED     = 16,         // Thread has exited

        // This is the set of bits that must be cleared by a generic
        // wake() or release().
        SLEEPSET   = (SLEEPING | COUNTSLEEP)
    };
    
private:
    // Current thread state, a logical OR of the above values.
    // Only if this word is zero can the thread execute.
    cyg_uint32                  state;      

    // Suspension counter, if > 0, the thread is suspended
    cyg_ucount32                suspend_count;

    // Wakeup counter, if > 0, sleep will not sleep, just decrement
    cyg_ucount32                wakeup_count;

    // A word of data used in syncronization object to communicate
    // information between sleepers and wakers.
    CYG_ADDRWORD                wait_info;
    
    // Unique thread id assigned on creation
    cyg_uint16                  unique_id;

#ifdef CYGPKG_KERNEL_EXCEPTIONS

    // If exceptions are supported, define an exception control
    // object that will be used to manage and deliver them. If
    // exceptions are global there is a single static instance
    // of this object, if they are per-thread then there is one
    // for each thread.
private:

#ifdef CYGSEM_KERNEL_EXCEPTIONS_GLOBAL
    static
#endif
    Cyg_Exception_Control       exception_control;

public:

    static void register_exception(
        cyg_code                exception_number,       // exception number
        cyg_exception_handler   handler,                // handler function
        CYG_ADDRWORD            data,                   // data argument
        cyg_exception_handler   **old_handler,          // handler function
        CYG_ADDRWORD            *old_data               // data argument
        );

    static void deregister_exception(
        cyg_code                exception_number        // exception number
        );
    
    void deliver_exception(
        cyg_code            exception_number,       // exception being raised
        CYG_ADDRWORD        exception_info          // exception specific info
        );

#endif

    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    // Constructor, Initialize the thread structure. The thread is
    // created in suspended state, and needs to be resumed to execute.
    // It is also started at some (configurable) default priority, which
    // may need to be changed before calling resume.
    
    Cyg_Thread (
        cyg_thread_entry        *entry,         // entry point function
        CYG_ADDRWORD            entry_data,     // entry data
        cyg_ucount32            stack_size = 0, // stack size, 0 = use default
        CYG_ADDRESS             stack_base = 0  // stack base, NULL = allocate
        );

    Cyg_Thread (
        CYG_ADDRWORD            sched_info,     // Scheduling parameter(s)
        cyg_thread_entry        *entry,         // entry point function
        CYG_ADDRWORD            entry_data,     // entry data
        char                    *name,          // thread name
        CYG_ADDRESS             stack_base = 0, // stack base, NULL = allocate
        cyg_ucount32            stack_size = 0  // stack size, 0 = use default
        );

    // Re-initialize the thread back to it's initial state.
    void Cyg_Thread::reinitialize();
    
    ~Cyg_Thread();
    
    // The following are invoked implicitly on the current thread,
    // hence they are static member functions.

    static void         sleep();        // Put thread to sleep

    static void         counted_sleep();// Decrement counter or put
                                        // thread to sleep
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    static void         counted_sleep( cyg_tick_count delay );
                                        // ...for delay ticks
#endif
    
    static void         exit();         // Terminate thread

    static void         yield();        // Yield CPU to another thread

    static void         rotate_queue( cyg_priority pri );
                                        // Rotate that run queue

    void                to_queue_head( void );
                                        // Move to the head of its queue
                                        // (not necessarily a scheduler q)

    static Cyg_Thread   *self();        // Return current thread

        
    // The following are called on threads other than the current one.

    void                wake();         // Wake this thread from sleep.

    void                counted_wake(); // Increment counter or wake thread
    cyg_uint32          cancel_counted_wake();
                                        // Cancel counted wakeups for this
                                        // thread and return how many were
                                        // pending

    void                suspend();      // Suspend this thread: increment counter and
                                        // deschedule.
    
    void                resume();       // Resume this thread: decrement counter and
                                        // reschedule if counter is zero.

    void                release();      // Release thread from sleep with BREAK
                                        // wake_reason.
    
    void                kill();         // Kill this thread
    
    void                force_resume(); // Resume this thread: set counter to zero.

    cyg_uint32          get_state();    // Return current thread state.


    // Accessor functions to set and get wait_info.
    
    void                set_wait_info(CYG_ADDRWORD data);

    CYG_ADDRWORD        get_wait_info();
    
    // This part of the API is used if we have a clock and want
    // per-thread timers for doing delays and timeouts.

    // delay the given number of ticks
    void delay( cyg_tick_count delay );
        

    enum cyg_reason                     // sleep/wakeup reason codes
    {
        NONE,                           // No recorded reason
        WAIT,                           // Wait with no timeout
        DELAY,                          // Simple time delay
        TIMEOUT,                        // Wait with timeout/timeout expired
        BREAK,                          // forced break out of sleep
        DESTRUCT,                       // wait object destroyed[note]
        EXIT,                           // forced termination
        DONE                            // Wait/delay complete
    };
    // [note] NOT the thread, some object it was waiting on.
    //        Thread destruction would first involve EXITing it.
    
private:

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    Cyg_ThreadTimer     timer;          // per-thread timer
#endif

    cyg_reason          sleep_reason;   // reason for sleeping

    cyg_reason          wake_reason;    // reason for waking
    
#ifdef CYGIMP_THREAD_PRIORITY

public:

    // If the scheduler implements priorities, provide
    // functions to set and get it.
    
    void set_priority( cyg_priority pri );

    cyg_priority get_priority();

    // This returns the current dispatching priority of the
    // thread. This may differ from the result of get_priority()
    // in the presence of priority inheritance or certain
    // scheduling algorithms.
    cyg_priority get_current_priority();    
    
#endif

#ifdef CYGVAR_KERNEL_THREADS_DATA

private:
    // Array of single word entries for each index. 
    CYG_ADDRWORD        thread_data[CYGNUM_KERNEL_THREADS_DATA_MAX];

    // Map of free thread_data indexes. Each bit represents an index
    // and is 1 if that index is free, and 0 if it is in use.
    static cyg_ucount32        thread_data_map;

public:
    
    typedef cyg_count32 cyg_data_index;

    static CYG_ADDRWORD get_data( cyg_data_index index );

    static CYG_ADDRWORD *get_data_ptr( cyg_data_index index );

    void                set_data( cyg_data_index index, CYG_ADDRWORD data );

    // returns -1 if no more indexes available
    static cyg_data_index new_data_index();

    static void         free_data_index( cyg_data_index index );

#endif

#ifdef CYGPKG_KERNEL_THREADS_DESTRUCTORS

    // thread destructors, called on thread exit.
private:
    typedef void (*destructor_fn)(CYG_ADDRWORD);
    struct Cyg_Destructor_Entry {
        destructor_fn fn;
        CYG_ADDRWORD data;
    };
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    static 
#endif
    Cyg_Destructor_Entry destructors[ CYGNUM_KERNEL_THREADS_DESTRUCTORS ];
public:
 
    // Add and remove destructors. Returns true on success, false on failure.
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    static 
#endif
    cyg_bool     add_destructor( destructor_fn fn, CYG_ADDRWORD data );
#ifndef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
    static 
#endif
    cyg_bool     rem_destructor( destructor_fn fn, CYG_ADDRWORD data );
#endif

#ifdef CYGVAR_KERNEL_THREADS_NAME

private:
    // An optional thread name string, for humans to read
    char                        *name;

public:    
    // function to get the name string
    char                        *get_name();
    
#endif
    

#ifdef CYGVAR_KERNEL_THREADS_LIST

        // Housekeeping list that tracks all threads
private:
    Cyg_Thread                  *list_next;
    static Cyg_Thread           *thread_list;

    void                        add_to_list(      void );
    void                        remove_from_list( void );
public:

    static Cyg_Thread           *get_list_head();
    
    Cyg_Thread                  *get_list_next();
    
#endif
    
public:
    
    // Set sleep reason to reason and wake reason to NONE
    static void set_sleep_reason( cyg_reason reason = WAIT);

    cyg_reason get_sleep_reason();
    
    // Set the wakeup reason to the given value
    void set_wake_reason( cyg_reason reason = DONE);

    // Get current wake reason
    cyg_reason get_wake_reason();

    static void set_timer(              // Set timeout and sleep reason
        cyg_tick_count  trigger,        // Absolute wakeup time
        cyg_reason      sleep_reason    // reason for sleeping
        );

    static void clear_timer();          // disable thread timer

    // Get a 16 bit unique id for this thread. This is
    // used in tracing and instrumentation to identify the
    // current thread.
    
    cyg_uint16 get_unique_id();
        
};

// -------------------------------------------------------------------------
// Thread Queue class.
// This defines the main API for manipulating queues of threads.

class Cyg_ThreadQueue
    : public Cyg_ThreadQueue_Implementation
{
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    // API used by rest of kernel.
    
                        // Add thread to queue
    void                enqueue(Cyg_Thread *thread);

                        // return first thread on queue
    Cyg_Thread          *highpri();

                        // remove first thread on queue    
    Cyg_Thread          *dequeue();

                        // remove specified thread from queue    
    void                remove(Cyg_Thread *thread);

                        // test if queue is empty
    inline cyg_bool     empty();
    
};

// -------------------------------------------------------------------------
// Thread inlines

// Return current thread state.
inline cyg_uint32 Cyg_Thread::get_state()
{
    return state;
}

inline void Cyg_Thread::set_wait_info(CYG_ADDRWORD data)
{
    wait_info = data;
}

inline CYG_ADDRWORD Cyg_Thread::get_wait_info()
{
    return wait_info;
}

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_THREAD_HXX
// EOF thread.hxx
