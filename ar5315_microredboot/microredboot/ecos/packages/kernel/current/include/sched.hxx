#ifndef CYGONCE_KERNEL_SCHED_HXX
#define CYGONCE_KERNEL_SCHED_HXX

//==========================================================================
//
//      sched.hxx
//
//      Scheduler class declaration(s)
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
// Purpose:     Define Scheduler class interfaces
// Description: These class definitions supply the internal API
//              used to scheduler threads. 
// Usage:       #include <cyg/kernel/sched.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/smp.hxx>          // SMP support

// -------------------------------------------------------------------------
// Miscellaneous types

#ifdef CYGSEM_KERNEL_SCHED_ASR_SUPPORT

typedef void Cyg_ASR( CYG_ADDRWORD data );      // ASR type signature

#endif

__externC void cyg_scheduler_set_need_reschedule();

// -------------------------------------------------------------------------
// Scheduler base class. This defines stuff that is needed by the
// specific scheduler implementation. Each scheduler comprises three
// classes: Cyg_Scheduler_Base, Cyg_Scheduler_Implementation which
// inherits from it and Cyg_Scheduler which inherits from _it_ in turn.

class Cyg_Scheduler_Base
     : public Cyg_Scheduler_SchedLock
{
    friend class Cyg_HardwareThread;
    friend class Cyg_SchedThread;
    
protected:
    // The following variables are implicit in the API, but are
    // not publically visible.

    // Current running thread    
    static Cyg_Thread * volatile current_thread[CYGNUM_KERNEL_CPU_MAX]
                                                CYGBLD_ANNOTATE_VARIABLE_SCHED; 

    // Set when reschedule needed
    static volatile cyg_bool     need_reschedule[CYGNUM_KERNEL_CPU_MAX]
                                                 CYGBLD_ANNOTATE_VARIABLE_SCHED; 

    // Count of number of thread switches
    static volatile cyg_ucount32 thread_switches[CYGNUM_KERNEL_CPU_MAX]
                                                 CYGBLD_ANNOTATE_VARIABLE_SCHED; 

public:

    // return a pointer to the current thread
    static Cyg_Thread *get_current_thread();

    // Set current thread pointer
    static void set_current_thread(Cyg_Thread *thread);
    static void set_current_thread(Cyg_Thread *thread, HAL_SMP_CPU_TYPE cpu);
    
    // Set need_reschedule flag
    static void set_need_reschedule();
    static void set_need_reschedule(Cyg_Thread *thread);

    // Get need_reschedule flag
    static cyg_bool get_need_reschedule();

    // Return current value of lock
    static cyg_ucount32 get_sched_lock();

    // Clear need_reschedule flag
    static void clear_need_reschedule();
    
    // Return current number of thread switches
    static cyg_ucount32 get_thread_switches();
    
};

// -------------------------------------------------------------------------
// Include the scheduler implementation header

#include CYGPRI_KERNEL_SCHED_IMPL_HXX

// Do some checking that we have a consistent universe.

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL
# ifndef CYGIMP_THREAD_PRIORITY
#  error Priority inversion protocols will not work without priorities!!!
# endif
#endif

// -------------------------------------------------------------------------
// Scheduler class. This is the public scheduler interface seen by the
// rest of the kernel.

class Cyg_Scheduler
    : public Cyg_Scheduler_Implementation
{
    friend class Cyg_Thread;
    
    // This function is the actual implementation of the unlock
    // function.  The unlock() later is an inline shell that deals
    // with the common case.
    
    static void             unlock_inner(cyg_uint32 new_lock = 0);
    
public:

    CYGDBG_DEFINE_CHECK_THIS

    // The following API functions are common to all scheduler
    // implementations.

    // claim the preemption lock
    static void             lock();         

    // release the preemption lock and possibly reschedule
    static void             unlock();

    // release and reclaim the lock atomically, keeping the old
    // value on restart
    static void             reschedule();

    // decrement the lock but also look for a reschedule opportunity
    static void             unlock_reschedule();

    // release the preemption lock without rescheduling
    static void             unlock_simple();
    
    // Start execution of the scheduler
    static void start() CYGBLD_ATTRIB_NORET;

    // Start execution of the scheduler on the current CPU
    static void start_cpu() CYGBLD_ATTRIB_NORET;    
    
    // The only  scheduler instance should be this one...
    static Cyg_Scheduler scheduler CYGBLD_ANNOTATE_VARIABLE_SCHED;

};

// -------------------------------------------------------------------------
// This class encapsulates the scheduling abstractions in a thread.
// Cyg_SchedThread is included as a base class of Cyg_Thread. The actual
// implementation of the abstractions is in Cyg_SchedThread_Implementation
// so this class has little to do.

class Cyg_SchedThread
    : public Cyg_SchedThread_Implementation
{
    friend class Cyg_ThreadQueue_Implementation;
    friend class Cyg_Scheduler_Implementation;
    friend class Cyg_Scheduler;
    
    Cyg_ThreadQueue     *queue;


public:

    Cyg_SchedThread(Cyg_Thread *thread, CYG_ADDRWORD sched_info);

    // Return current queue pointer

    Cyg_ThreadQueue     *get_current_queue();
    
    // Remove this thread from current queue
    void remove();

#ifdef CYGSEM_KERNEL_SCHED_ASR_SUPPORT

    // ASR support.
    // An ASR is an Asynchronous Service Routine. When set pending it
    // is called when the thread exits the scheduler. ASRs are mainly
    // used by compatibility subsystems, such as POSIX, to implement
    // such things as thread cancellation and signal delivery.

private:

    volatile cyg_ucount32       asr_inhibit;    // If > 0, blocks calls to ASRs

    volatile cyg_bool           asr_pending;    // If true, this thread's ASR should be called.

#ifdef CYGSEM_KERNEL_SCHED_ASR_GLOBAL
    static
#endif    
    Cyg_ASR             *asr;            // ASR function
#ifdef CYGSEM_KERNEL_SCHED_ASR_DATA_GLOBAL
    static
#endif    
    CYG_ADDRWORD        asr_data;       // ASR data pointer

    // Default ASR function
    static void         asr_default(CYG_ADDRWORD data);

public:

    // Public interface to ASR mechanism

    // Set, clear and get inhibit flag.
    inline void set_asr_inhibit() { asr_inhibit++; }
    inline void clear_asr_inhibit() { asr_inhibit--; }
    inline cyg_ucount32 get_asr_inhibit() { return asr_inhibit; }

    // Set and get pending flag. The flag is only cleared when the
    // ASR is called.
    inline void set_asr_pending() { asr_pending = true; }
    inline cyg_bool get_asr_pending() { return asr_pending; }

    // Set a new ASR, returning the old one. 
    void set_asr( Cyg_ASR  *new_asr, CYG_ADDRWORD  new_data,
                  Cyg_ASR **old_asr, CYG_ADDRWORD *old_data);

    // Clear the ASR function back to the default.
    void clear_asr();

#else

public:
    
    // Even when we do not have ASRs enabled, we keep these functions
    // available. This avoids excessive ifdefs in the rest of the
    // kernel code.
    inline void set_asr_inhibit() { }
    inline void clear_asr_inhibit() { }
    
#endif    
    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL

private:

    // For all priority inversion protocols we need to keep track of how
    // many mutexes we have locked, including one which we are waiting to
    // lock, because we can inherit priority while sleeping just prior to
    // wakeup.
    
    cyg_count32         mutex_count;

protected:
    // These are implementation functions that are common to all protocols.
        
    // Inherit the given priority. If thread is non-NULL the priority is
    // being inherited from it, otherwise it has come from the mutex.
    void set_inherited_priority( cyg_priority pri, Cyg_Thread *thread = 0 );

    // Relay the priority of the ex-owner thread or from the queue if it
    // has a higher priority than ours.
    void relay_inherited_priority( Cyg_Thread *ex_owner, Cyg_ThreadQueue *pqueue);

    // Lose priority inheritance
    void clear_inherited_priority();
    
public:    
    // Count and uncount the number of mutexes held by
    // this thread.
    void count_mutex() { mutex_count++; };
    void uncount_mutex() { mutex_count--; };

#if defined(CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_SIMPLE)
    
protected:    

    // The simple priority inversion protocols simply needs
    // somewhere to store the base priority of the current thread.
    
    cyg_priority        original_priority;      // our original priority

    cyg_bool            priority_inherited;     // have we inherited?

#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_INHERIT
    
public:

    // Inherit the priority of the provided thread if it
    // has higher priority than this.
    void inherit_priority( Cyg_Thread *thread);

    // Relay the priority of the ex-owner thread or from the queue if it
    // has a higher priority than ours.
    void relay_priority( Cyg_Thread *ex_owner, Cyg_ThreadQueue *pqueue);

    // Lose priority inheritance
    void disinherit_priority();
    
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING

public:

    // Set the priority of this thread to the given ceiling.
    void set_priority_ceiling( cyg_priority pri );

    // Clear the ceiling, if necessary.
    void clear_priority_ceiling();
    
#endif    

#endif
    
};

// -------------------------------------------------------------------------
// Simple inline accessor functions

inline Cyg_Thread *Cyg_Scheduler_Base::get_current_thread()
{
    return current_thread[CYG_KERNEL_CPU_THIS()];
}

inline void Cyg_Scheduler_Base::set_current_thread(Cyg_Thread *thread )
{
    current_thread[CYG_KERNEL_CPU_THIS()] = thread;
}

inline void Cyg_Scheduler_Base::set_current_thread(Cyg_Thread *thread,
                                                   HAL_SMP_CPU_TYPE cpu)
{
    current_thread[cpu] = thread;
}

inline cyg_bool Cyg_Scheduler_Base::get_need_reschedule()
{
    return need_reschedule[CYG_KERNEL_CPU_THIS()];
}

inline void Cyg_Scheduler_Base::set_need_reschedule()
{
    need_reschedule[CYG_KERNEL_CPU_THIS()] = true;
}

inline void Cyg_Scheduler_Base::set_need_reschedule(Cyg_Thread *thread)
{
    need_reschedule[CYG_KERNEL_CPU_THIS()] = true;
}

inline void Cyg_Scheduler_Base::clear_need_reschedule()
{
    need_reschedule[CYG_KERNEL_CPU_THIS()] = false;
}

inline cyg_ucount32 Cyg_Scheduler_Base::get_sched_lock()
{
    return Cyg_Scheduler_SchedLock::get_sched_lock();
}

// Return current number of thread switches
inline cyg_ucount32 Cyg_Scheduler_Base::get_thread_switches()
{
    return thread_switches[CYG_KERNEL_CPU_THIS()];
}

// Return current queue pointer
inline Cyg_ThreadQueue *Cyg_SchedThread::get_current_queue()
{
    return queue;
}

// -------------------------------------------------------------------------
#endif // ifndef __SCHED_HXX__
// EOF sched.hxx
