#ifndef CYGONCE_COMPAT_UITRON_UIT_FUNC_INL
#define CYGONCE_COMPAT_UITRON_UIT_FUNC_INL
//===========================================================================
//
//      uit_func.inl
//
//      uITRON compatibility functions
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-03-13
// Purpose:     uITRON compatibility functions
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifdef CYGPKG_UITRON

#ifdef CYGPRI_UITRON_FUNCS_HERE_AND_NOW

#include <cyg/compat/uitron/uit_objs.hxx> // uITRON setup CYGNUM_UITRON_SEMAS

// kernel facilities only needed here
#include <cyg/kernel/intr.hxx>
#include <cyg/kernel/sched.hxx>

// and the implementations of other kernel facilities
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.inl>
#include <cyg/kernel/clock.inl>


// ------------------------------------------------------------------------
// The variable where dis_dsp/ena_dsp state is held:
extern cyg_uint32 cyg_uitron_dis_dsp_old_priority;

// ------------------------------------------------------------------------
// Parameter checking; either check the expression and return an error code
// if not true, or assert the truth with a made-up message.

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
// default: uitron error codes are returned
#define CYG_UIT_PARAMCHECK( _true_, _error_ ) CYG_MACRO_START           \
    if ( ! (_true_) ) return (_error_);                                 \
CYG_MACRO_END
#else
// ...but they are asserted if asserts are on
#define CYG_UIT_PARAMCHECK( _true_, _error_ ) CYG_MACRO_START           \
    CYG_ASSERT( (_true_), "CYG_UIT_PARAMCHECK fail: " #_true_ );        \
CYG_MACRO_END
#endif // else !CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS

// ------------------------------------------------------------------------
// CYG_UITRON_CHECK_AND_GETP
// 
// Macro to rangecheck and do the addressing of a static uitron system
// object; _which_ sort of object is given, and token pasting is used
// horribly to get the static array, limits and the like.
//
// Usage:
//   INT snd_msg( ID mbxid, ... ) {
//      Cyg_Mbox *p;
//      CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
//      p->...(...);

// internal: plain assignment to the object pointer, from static array
#define CYG_UIT_SPTR( _which_, _idx_, _ptr_ ) CYG_MACRO_START           \
        (_ptr_) =  CYG_UITRON_OBJS( _which_ ) + ((_idx_) - 1);          \
CYG_MACRO_END

// internal: plain assignment to the object pointer, from pointer array
// with error checking.
#define CYG_UIT_SPTR_PTR( _which_, _idx_, _ptr_ ) CYG_MACRO_START       \
        (_ptr_) =  CYG_UITRON_PTRS( _which_ )[ ((_idx_) - 1) ];         \
        if ( NULL == (_ptr_) ) return E_NOEXS;                          \
CYG_MACRO_END

#define CYG_UITRON_CHECK_AND_GETP_DIRECT( _which_, _idx_, _ptr_ )       \
CYG_MACRO_START                                                         \
    CYG_UIT_PARAMCHECK( 0 < (_idx_), E_ID );                            \
    CYG_UIT_PARAMCHECK( CYG_UITRON_NUM( _which_ ) >= (_idx_), E_ID );   \
    CYG_UIT_SPTR( _which_, _idx_, _ptr_ );                              \
CYG_MACRO_END

#define CYG_UITRON_CHECK_AND_GETP_INDIRECT( _which_, _idx_, _ptr_ )     \
CYG_MACRO_START                                                         \
    CYG_UIT_PARAMCHECK( 0 < (_idx_), E_ID );                            \
    CYG_UIT_PARAMCHECK( CYG_UITRON_NUM( _which_ ) >= (_idx_), E_ID );   \
    CYG_UIT_SPTR_PTR( _which_, _idx_, _ptr_ );                          \
CYG_MACRO_END

// As above but for handler numbers which return E_PAR when out of range
#define CYG_UITRON_CHECK_AND_GETHDLR( _which_, _num_, _ptr_ )           \
CYG_MACRO_START                                                         \
    CYG_UIT_PARAMCHECK( 0 < (_num_), E_PAR );                           \
    CYG_UIT_PARAMCHECK( CYG_UITRON_NUM( _which_ ) >= (_num_), E_PAR );  \
    CYG_UIT_SPTR( _which_, _num_, _ptr_ );                              \
CYG_MACRO_END

// And a macro to check that creation of an object is OK
#define CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( _which_, _idx_ )            \
CYG_MACRO_START                                                         \
    CYG_UIT_PARAMCHECK( 0 < (_idx_), E_ID );                            \
    CYG_UIT_PARAMCHECK( CYG_UITRON_NUM( _which_ ) >= (_idx_), E_ID );   \
    Cyg_Scheduler::lock();                                              \
    if ( NULL != CYG_UITRON_PTRS( _which_ )[ ((_idx_) - 1) ] ) {        \
        Cyg_Scheduler::unlock();                                        \
        return E_OBJ;                                                   \
    }                                                                   \
CYG_MACRO_END

// define a magic new operator in order to call constructors
#define CYG_UITRON_NEWFUNCTION( _class_ )                               \
inline void *operator new(size_t size, _class_ *ptr)                    \
{                                                                       \
    CYG_CHECK_DATA_PTR( ptr, "Bad pointer" );                           \
    return ptr;                                                         \
}

// now configury to support selectable create/delete support ie. an
// array of pointers to the objects themselves.
#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_TASKS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( TASKS, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_TASKS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( TASKS, _idx_, _ptr_ )
#endif

#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_SEMAS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( SEMAS, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_SEMAS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( SEMAS, _idx_, _ptr_ )
#endif

#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_MBOXES( _idx_, _ptr_ )                \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( MBOXES, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_MBOXES( _idx_, _ptr_ )                \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( MBOXES, _idx_, _ptr_ )
#endif

#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_FLAGS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( FLAGS, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_FLAGS( _idx_, _ptr_ )                 \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( FLAGS, _idx_, _ptr_ )
#endif

#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( _idx_, _ptr_ )          \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( MEMPOOLFIXED, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( _idx_, _ptr_ )          \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( MEMPOOLFIXED, _idx_, _ptr_ )
#endif

#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
#define CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( _idx_, _ptr_ )            \
    CYG_UITRON_CHECK_AND_GETP_INDIRECT( MEMPOOLVAR, _idx_, _ptr_ )
#else
#define CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( _idx_, _ptr_ )            \
    CYG_UITRON_CHECK_AND_GETP_DIRECT( MEMPOOLVAR, _idx_, _ptr_ )
#endif

// ------------------------------------------------------------------------
// Common error checking macros

#if !defined( CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS ) && \
    !defined( CYGDBG_USE_ASSERTS )
// if not checking and not asserted, these are removed to avoid usused
// variable warnings.
#define CYG_UITRON_CHECK_TASK_CONTEXT_SELF( _self_ )     CYG_EMPTY_STATEMENT
#define CYG_UITRON_CHECK_TASK_CONTEXT()                  CYG_EMPTY_STATEMENT
#define CYG_UITRON_CHECK_DISPATCH_ENABLED()              CYG_EMPTY_STATEMENT
#define CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( _tmout_ ) CYG_EMPTY_STATEMENT

#else
// the default:
// Check a task is actually a uITRON task
#define CYG_UITRON_CHECK_TASK_CONTEXT_SELF( _self_ ) CYG_MACRO_START    \
    CYG_UIT_PARAMCHECK(                                                 \
        (&cyg_uitron_TASKS[0] <= (_self_)) &&                           \
        ((_self_) < &cyg_uitron_TASKS[CYGNUM_UITRON_TASKS]),            \
                                  E_CTX );                              \
CYG_MACRO_END

#define CYG_UITRON_CHECK_TASK_CONTEXT() CYG_MACRO_START                 \
    Cyg_Thread *self = Cyg_Thread::self();                              \
    CYG_UITRON_CHECK_TASK_CONTEXT_SELF( self );                         \
CYG_MACRO_END

// Check dispatching is enabled for calls which might wait
#define CYG_UITRON_CHECK_DISPATCH_ENABLED()  CYG_MACRO_START            \
    CYG_UIT_PARAMCHECK( 0 == cyg_uitron_dis_dsp_old_priority, E_CTX );  \
CYG_MACRO_END

#define CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO(_tmout_)  CYG_MACRO_START \
    CYG_UIT_PARAMCHECK( -1 <= (_tmout_), E_PAR );                       \
    if ( TMO_POL != (_tmout_) )                                         \
        CYG_UITRON_CHECK_DISPATCH_ENABLED();                            \
CYG_MACRO_END

#endif

#ifdef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
#define CYG_UIT_PARAMCHECK_PTR( _p_ )   CYG_MACRO_START                 \
        CYG_UIT_PARAMCHECK( NADR != (_p_), E_PAR );                     \
CYG_MACRO_END
#else // do check for NULL
#define CYG_UIT_PARAMCHECK_PTR( _p_ )   CYG_MACRO_START                 \
        CYG_UIT_PARAMCHECK( NADR != (_p_), E_PAR );                     \
        CYG_UIT_PARAMCHECK( NULL != (_p_), E_PAR );                     \
CYG_MACRO_END
#endif // !CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR

// ------------------------------------------------------------------------
// CYG_UITRON_FAIL_RETURN
//
// After a call which waits, it might return with success, or due to a
// timeout or a release wait (a forced escape from the waiting condition).
// This macro examines context and finds out which, then executes a return
// with the correct uITRON condition code.

#define CYG_UITRON_FAIL_RETURN_SELF( _self_ ) CYG_MACRO_START           \
    Cyg_Thread::cyg_reason reason = (_self_)->get_wake_reason();        \
    if ( Cyg_Thread::TIMEOUT  == reason )                               \
        return E_TMOUT;                                                 \
    if ( Cyg_Thread::BREAK    == reason )                               \
        return E_RLWAI;                                                 \
    if ( Cyg_Thread::DESTRUCT == reason )                               \
        return E_DLT;                                                   \
    return E_SYS; /* if no plausible reason was found */                \
CYG_MACRO_END

#define CYG_UITRON_FAIL_RETURN() CYG_MACRO_START                        \
    Cyg_Thread *self = Cyg_Thread::self();                              \
    CYG_UITRON_FAIL_RETURN_SELF( self );                                \
CYG_MACRO_END

// ------------------------------------------------------------------------
// Interrupts disabled?
#define CYG_UITRON_CHECK_CPU_UNLOC()                                    \
    CYG_UIT_PARAMCHECK( (Cyg_Interrupt::interrupts_enabled()), E_CTX )

// ------------------------------------------------------------------------
// Timing: is it in eCos clock ticks or milliSeconds (or something else?)

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

#ifdef CYGSEM_UITRON_TIME_IS_MILLISECONDS
extern Cyg_Clock::converter uit_clock_to_system;
extern Cyg_Clock::converter uit_clock_from_system;

#define CYG_UITRON_TIME_UIT_TO_SYS32( t ) \
Cyg_Clock::convert( (cyg_uint64)(t), &uit_clock_to_system )

#define CYG_UITRON_TIME_SYS_TO_UIT32( t ) \
Cyg_Clock::convert( (cyg_uint64)(t), &uit_clock_from_system )

// long (cyg_uint64) versions:
#define CYG_UITRON_TIME_UIT_TO_SYS64( t ) \
Cyg_Clock::convert( (t), &uit_clock_to_system )

#define CYG_UITRON_TIME_SYS_TO_UIT64( t ) \
Cyg_Clock::convert( (t), &uit_clock_from_system )

#else // Time is whatever the system clock is doing:

// Straight through - int (cyg_int32) argument versions:
#define CYG_UITRON_TIME_UIT_TO_SYS32( t )  ( t )
#define CYG_UITRON_TIME_SYS_TO_UIT32( t )  ( t )
// long (cyg_uint64) versions:
#define CYG_UITRON_TIME_UIT_TO_SYS64( t )  ( t )
#define CYG_UITRON_TIME_SYS_TO_UIT64( t )  ( t )
#endif

#endif // CYGVAR_KERNEL_COUNTERS_CLOCK - otherwise these should not be used.

// ------------------------------------------------------------------------
// the function definitions themselves:

// ******************************************************
// ***    6.5 C Language Interfaces                   ***
// ******************************************************

// - Task Management Functions

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
CYG_UITRON_NEWFUNCTION( Cyg_Thread )

CYG_UIT_FUNC_INLINE
ER
cre_tsk ( ID tskid, T_CTSK *pk_ctsk )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_ctsk );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( TASKS, tskid );

    Cyg_Thread *p = &(CYG_UITRON_OBJS( TASKS )[ tskid - 1 ]);
    cyg_uint32 state = p->get_state();
    if ( 0 == (state & Cyg_Thread::EXITED) )
        ret = E_OBJ; // how did it get to be running?
    else if ( ((INT)p->get_stack_size()) < pk_ctsk->stksz )
        ret = E_NOMEM; // more stack requested than available
    else {
        CYG_UITRON_PTRS( TASKS )[ tskid - 1 ] =
            new( p ) Cyg_Thread(
                (CYG_ADDRWORD)      pk_ctsk->itskpri,
                (cyg_thread_entry *)pk_ctsk->task,
                (CYG_ADDRWORD)      0,
                // preserve the original name and stack:
#ifdef CYGVAR_KERNEL_THREADS_NAME
                p->get_name(),
#else
                NULL,
#endif
                p->get_stack_base(),
                p->get_stack_size() );
        // but ensure the task state is dormant:
        // (it is not constructed dormant, but suspended)
        p->kill();
#ifdef CYGIMP_THREAD_PRIORITY
        // and record the initial priority outside the task too.
        CYG_UITRON_TASK_INITIAL_PRIORITY( tskid ) = pk_ctsk->itskpri;
#endif
    }
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( TASKS )[ tskid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    cyg_uint32 state = p->get_state();
    if ( state & Cyg_Thread::EXITED )
        // just disconnect the pointer from its object
        CYG_UITRON_PTRS( TASKS )[ tskid - 1 ] = NULL;
    else
        ret = E_OBJ;
    Cyg_Scheduler::unlock();
    return ret;
}
#endif // CYGPKG_UITRON_TASKS_CREATE_DELETE

CYG_UIT_FUNC_INLINE
ER
sta_tsk ( ID tskid, INT stacd )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    
    Cyg_Scheduler::lock();
    cyg_uint32 state = p->get_state();
#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
    // there is a race condition with deleting the task
    // so test it now that we have the scheduler locked
    if ( p != CYG_UITRON_PTRS( TASKS )[ tskid - 1 ] )
        ret = E_NOEXS;
    else // NOTE dangling else to the next line:
#endif
    if ( state & Cyg_Thread::EXITED ) {
        p->reinitialize();
#ifdef CYGIMP_THREAD_PRIORITY
        p->set_priority( CYG_UITRON_TASK_INITIAL_PRIORITY( tskid ) );
#endif
        p->set_entry_data( (CYG_ADDRWORD)stacd );
        p->force_resume();
    }
    else
        ret = E_OBJ;
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
void
ext_tsk ( void )
{    
    Cyg_Thread::exit();
}

CYG_UIT_FUNC_INLINE
void
exd_tsk ( void )
{
#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
    Cyg_Thread *p;

    Cyg_Scheduler::lock();
    p = Cyg_Thread::self();
    ID tskid = (p - (&cyg_uitron_TASKS[0])) + 1;    
    // just disconnect the pointer from its object
    CYG_UITRON_PTRS( TASKS )[ tskid - 1 ] = NULL;
    // Any associated storage management, and possibly calling the task
    // destructor, is for future versions.
#else
    // do nothing - deletion not supported so just exit...
#endif
    Cyg_Thread::exit();
    // does not return, does unlock the scheduler for us
}

CYG_UIT_FUNC_INLINE
ER
ter_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();
    if ( (0 != (Cyg_Thread::EXITED & p->get_state())) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        // already dormant
        ret = E_OBJ;
    else {
        p->force_resume(); // let it run
        p->kill(); // and set prio high so it runs RIGHT NOW!!
#ifdef CYGIMP_THREAD_PRIORITY
#if CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES != 0
        // see if we are already at prio 0:
        if ( 0 == cyg_uitron_dis_dsp_old_priority )
            // then dispatch is enabled, we are not at prio 0
#endif
            p->set_priority( (cyg_priority) 0 );
        // if we do not do this, then we are not running a strictly
        // uITRON compatible scheduler - so just hope for the best.
#endif
    }
    Cyg_Scheduler::unlock();
#ifdef CYGIMP_THREAD_PRIORITY
#if CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES == 0
    if ( (E_OK == ret) && (0 != cyg_uitron_dis_dsp_old_priority) ) {
        // then dispatching is disabled, so our prio is 0 too
        Cyg_Thread::yield(); // so let the dying thread run;
        Cyg_Thread::yield(); // no cost here of making sure.
    }
#endif
#endif
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
dis_dsp ( void )
{
    CYG_UITRON_CHECK_TASK_CONTEXT();
    CYG_UITRON_CHECK_CPU_UNLOC();
    Cyg_Scheduler::lock();
    // Prevent preemption by going up to prio 0
    if ( 0 == cyg_uitron_dis_dsp_old_priority ) {
#ifdef CYGIMP_THREAD_PRIORITY
        Cyg_Thread *p = Cyg_Thread::self();
        cyg_uitron_dis_dsp_old_priority = p->get_priority();
        p->set_priority( 0 );
#else
        cyg_uitron_dis_dsp_old_priority = 1;
#endif
    }
    Cyg_Scheduler::unlock();
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ena_dsp ( void )
{
    CYG_UITRON_CHECK_TASK_CONTEXT();
    CYG_UITRON_CHECK_CPU_UNLOC();
    Cyg_Scheduler::lock();
    // Enable dispatching (if disabled) and maybe switch threads
    if ( 0 != cyg_uitron_dis_dsp_old_priority ) {
        // We had prevented preemption by going up to prio 0
#ifdef CYGIMP_THREAD_PRIORITY
        Cyg_Thread *p = Cyg_Thread::self();
        p->set_priority( cyg_uitron_dis_dsp_old_priority );
        p->to_queue_head(); // to ensure we continue to run
                            // if nobody higher pri
#endif
        cyg_uitron_dis_dsp_old_priority = 0;
    }
    Cyg_Scheduler::unlock();
    CYG_UITRON_CHECK_DISPATCH_ENABLED(); // NB: afterwards!
    return E_OK;
}


CYG_UIT_FUNC_INLINE
ER
chg_pri ( ID tskid, PRI tskpri )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    if ( 0 == tskid ) {
        p = Cyg_Thread::self();
        CYG_UITRON_CHECK_TASK_CONTEXT_SELF( p );
    }
    else
        CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );

#ifdef CYGIMP_THREAD_PRIORITY
    if ( 0 == tskpri )
        // then use the initial priority [Level X]
        tskpri = CYG_UITRON_TASK_INITIAL_PRIORITY( tskid );
#endif
    CYG_UIT_PARAMCHECK( 0 < tskpri, E_PAR );
#ifdef CYGIMP_THREAD_PRIORITY
#if CYG_THREAD_MAX_PRIORITY < CYG_THREAD_MIN_PRIORITY
    CYG_UIT_PARAMCHECK( CYG_THREAD_MAX_PRIORITY <= tskpri &&
                        tskpri <= CYG_THREAD_MIN_PRIORITY, E_PAR );
#else    
    CYG_UIT_PARAMCHECK( CYG_THREAD_MAX_PRIORITY >= tskpri &&
                        tskpri >= CYG_THREAD_MIN_PRIORITY, E_PAR );
#endif
    // Handle changing our own prio specially, if dispatch disabled:
    if ( 0 != cyg_uitron_dis_dsp_old_priority ) {
        // our actual prio is 0 now and must remain so:
        if ( Cyg_Thread::self() == p ) {  // by whichever route p was set
            // set the priority we will return to when dispatch is enabled:
            cyg_uitron_dis_dsp_old_priority = (cyg_uint32)tskpri;
            return E_OK;
        }
    }
    Cyg_Scheduler::lock();
    if ( (p->get_state() & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        ret = E_OBJ; // task is dormant
    else
        p->set_priority( (cyg_priority)tskpri );
    Cyg_Scheduler::unlock();
#endif // CYGIMP_THREAD_PRIORITY got priorities at all?
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
rot_rdq ( PRI tskpri )
{
    // zero means our level; easiet way is to yield() the CPU.
    if ( 0 == tskpri ) {
        Cyg_Thread::yield();
        return E_OK;
    }
#ifdef CYGIMP_THREAD_PRIORITY
#if CYG_THREAD_MAX_PRIORITY < CYG_THREAD_MIN_PRIORITY
    CYG_UIT_PARAMCHECK( CYG_THREAD_MAX_PRIORITY <= tskpri &&
                        tskpri <= CYG_THREAD_MIN_PRIORITY, E_PAR );
#else    
    CYG_UIT_PARAMCHECK( CYG_THREAD_MAX_PRIORITY >= tskpri &&
                        tskpri >= CYG_THREAD_MIN_PRIORITY, E_PAR );
#endif
    Cyg_Thread::rotate_queue( tskpri );
#endif // CYGIMP_THREAD_PRIORITY got priorities at all?
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
rel_wai ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    if ( (p->get_state() & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        ret = E_OBJ;                    // task is dormant
    else {
        p->release();
        // return E_OBJ if the thread was not sleeping
        if ( Cyg_Thread::BREAK != p->get_wake_reason() )
            ret = E_OBJ;
    }
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
get_tid ( ID *p_tskid )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_UIT_PARAMCHECK_PTR( p_tskid );
    if ( (&cyg_uitron_TASKS[0] <= (self)) &&
        ((self) < &cyg_uitron_TASKS[CYGNUM_UITRON_TASKS]) &&
        (0 == Cyg_Scheduler::get_sched_lock()) )
        // then I am a uITRON task and not in an interrupt or DSR
        *p_tskid = (self - (&cyg_uitron_TASKS[0])) + 1;
    else
        *p_tskid = 0; // Otherwise, non-task portion
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_tsk ( T_RTSK *pk_rtsk, ID tskid )
{
    Cyg_Thread *p;
    if ( 0 == tskid ) {
        p = Cyg_Thread::self();
        CYG_UITRON_CHECK_TASK_CONTEXT_SELF( p );
        tskid = (p - (&cyg_uitron_TASKS[0])) + 1; // it gets used below
    }
    else
        CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );

    CYG_UIT_PARAMCHECK_PTR( pk_rtsk );
    pk_rtsk->exinf  = NADR;
    Cyg_Scheduler::lock();              // get an atomic view of the task
    cyg_uint32 state = p->get_state();
    if ( (state & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        pk_rtsk->tskstat = TTS_DMT;
    else if ( state == Cyg_Thread::RUNNING )
        // If it's us, it's running, else it's ready
        pk_rtsk->tskstat = (Cyg_Thread::self() == p)
            ? TTS_RUN                   // RUN state (we are it)
            : TTS_RDY;                  // READY state
    else if ( state & Cyg_Thread::SUSPENDED )
        pk_rtsk->tskstat =
            (state & (Cyg_Thread::COUNTSLEEP | Cyg_Thread::SLEEPING))
            ? TTS_WAS                   // WAIT-SUSPEND state
            : TTS_SUS;                  // SUSPEND state
    else
        pk_rtsk->tskstat =
            (state & (Cyg_Thread::COUNTSLEEP | Cyg_Thread::SLEEPING))
            ? TTS_WAI                   // WAIT state
            : 0;                        // Not sure what's happening here!
#ifdef CYGIMP_THREAD_PRIORITY
    if ( TTS_DMT == pk_rtsk->tskstat )
        pk_rtsk->tskpri = CYG_UITRON_TASK_INITIAL_PRIORITY( tskid );
    else if ( (TTS_RUN == pk_rtsk->tskstat) && 
              (0 != cyg_uitron_dis_dsp_old_priority) )
        // then we are it and dispatching is disabled, so
        // report our "real" priority - it is 0 in the kernel at the moment
        pk_rtsk->tskpri = cyg_uitron_dis_dsp_old_priority;
    else
        pk_rtsk->tskpri = p->get_priority();
#else
    pk_rtsk->tskpri = -1;  // Not applicable
#endif
    Cyg_Scheduler::unlock();
    return E_OK;
}
        
// - Task-Dependent Synchronization Functions
        
CYG_UIT_FUNC_INLINE
ER
sus_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    if ( (p->get_state() & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        ret = E_OBJ;                    // task is dormant
    else
        p->suspend();
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
rsm_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    cyg_uint32 state = p->get_state();
    if ( 0 == (Cyg_Thread::SUSPENDED & state) )
        ret = E_OBJ;                    // thread is not suspended
    else
        p->resume();
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
frsm_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    cyg_uint32 state = p->get_state();
    if ( 0 == (Cyg_Thread::SUSPENDED & state) )
        ret = E_OBJ;                    // thread is not suspended
    else
        p->force_resume();
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
slp_tsk ( void )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_UITRON_CHECK_TASK_CONTEXT_SELF( self );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    // do this now for the case when no sleeping actually occurs
    self->set_wake_reason( Cyg_Thread::DONE );
    Cyg_Thread::counted_sleep();
    if ( Cyg_Thread::DONE != self->get_wake_reason() )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
tslp_tsk ( TMO tmout )
{
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_UITRON_CHECK_TASK_CONTEXT_SELF( self );
    CYG_UIT_PARAMCHECK( -1 <= tmout, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    // do this now for the case when no sleeping actually occurs
    self->set_wake_reason( Cyg_Thread::DONE );
    // note that TMO_POL is not treated specially, though it
    // happens to work almost as a poll (some sleeping may occur)
    if ( TMO_FEVR == tmout )
        Cyg_Thread::counted_sleep();
    else
        Cyg_Thread::counted_sleep(
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( Cyg_Thread::DONE != self->get_wake_reason() )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
wup_tsk ( ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK( Cyg_Thread::self() != p, E_OBJ );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    if ( (p->get_state() & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        ret = E_OBJ;                    // task is dormant
    else
        p->counted_wake();
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
can_wup ( INT *p_wupcnt, ID tskid )
{
    Cyg_Thread *p;
    ER ret = E_OK;
    if ( 0 == tskid ) {
        p = Cyg_Thread::self();
        CYG_UITRON_CHECK_TASK_CONTEXT_SELF( p );
    }
    else
        CYG_UITRON_CHECK_AND_GETP_TASKS( tskid, p );
    CYG_UIT_PARAMCHECK_PTR( p_wupcnt );
    Cyg_Scheduler::lock();              // get an atomic view of the task
    if ( (p->get_state() & (Cyg_Thread::EXITED | Cyg_Thread::CREATING)) ||
         (Cyg_Thread::EXIT == p->get_wake_reason()) )
        ret = E_OBJ;                    // task is dormant
    else {
        cyg_uint32 result = p->cancel_counted_wake();
        *p_wupcnt = result;
    }
    Cyg_Scheduler::unlock();
    return ret;
}
        
// - Synchronization and Communication Functions
        
#ifdef CYGPKG_UITRON_SEMAS
#if 0 < CYG_UITRON_NUM( SEMAS )

#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE

CYG_UITRON_NEWFUNCTION( Cyg_Counting_Semaphore2 )

CYG_UIT_FUNC_INLINE
ER
cre_sem ( ID semid, T_CSEM *pk_csem )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_csem );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( SEMAS, semid );
    if ( TA_TFIFO != pk_csem->sematr )
        ret = E_RSATR;
    else
        CYG_UITRON_PTRS( SEMAS )[ semid - 1 ] =
            new( &(CYG_UITRON_OBJS( SEMAS )[ semid - 1 ]) )
            Cyg_Counting_Semaphore2( (cyg_count32)pk_csem->isemcnt );
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_sem ( ID semid )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( SEMAS )[ semid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    CYG_UITRON_PTRS( SEMAS )[ semid - 1 ] = NULL;
    p->~Cyg_Counting_Semaphore2();
    Cyg_Scheduler::unlock();
    return E_OK;
}
#endif // CYGPKG_UITRON_SEMAS_CREATE_DELETE

CYG_UIT_FUNC_INLINE
ER
sig_sem( ID semid )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    p->post();
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
wai_sem( ID semid )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    cyg_bool result = p->wait();
    if ( !result )
        CYG_UITRON_FAIL_RETURN();
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
preq_sem ( ID semid )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    cyg_bool result = p->trywait();
    if ( !result )
        return E_TMOUT;
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
twai_sem ( ID semid, TMO tmout )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( tmout );
    // do this now for the case when no sleeping actually occurs
    Cyg_Thread *self = Cyg_Thread::self();
    self->set_wake_reason( Cyg_Thread::TIMEOUT );
    cyg_bool result;
    if ( TMO_FEVR == tmout )
        result = p->wait();
    else if ( TMO_POL == tmout )
        result = p->trywait();
    else
        result = p->wait(
            Cyg_Clock::real_time_clock->current_value() +
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    return E_OK;

}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
ref_sem ( T_RSEM *pk_rsem, ID semid )
{
    Cyg_Counting_Semaphore2 *p;
    CYG_UITRON_CHECK_AND_GETP_SEMAS( semid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_rsem );
    pk_rsem->exinf  = NADR;
    pk_rsem->wtsk   = p->waiting();
    pk_rsem->semcnt = p->peek();
    return E_OK;
}

#endif // 0 < CYG_UITRON_NUM( SEMAS )
#endif // CYGPKG_UITRON_SEMAS

#ifdef CYGPKG_UITRON_FLAGS
#if 0 < CYG_UITRON_NUM( FLAGS )

#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE

CYG_UITRON_NEWFUNCTION( Cyg_Flag )

CYG_UIT_FUNC_INLINE
ER
cre_flg ( ID flgid, T_CFLG *pk_cflg )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_cflg );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( FLAGS, flgid );
    if ( 0 != ((~(TA_WMUL | TA_WSGL)) & pk_cflg->flgatr) )
        ret = E_RSATR;
    else
        CYG_UITRON_PTRS( FLAGS )[ flgid - 1 ] =
            new( &(CYG_UITRON_OBJS( FLAGS )[ flgid - 1 ]) )
            Cyg_Flag( (Cyg_FlagValue) pk_cflg->iflgptn );
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_flg ( ID flgid )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( FLAGS )[ flgid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    CYG_UITRON_PTRS( FLAGS )[ flgid - 1 ] = NULL;
    p->~Cyg_Flag();
    Cyg_Scheduler::unlock();
    return E_OK;
}
#endif // CYGPKG_UITRON_FLAGS_CREATE_DELETE

CYG_UIT_FUNC_INLINE
ER
set_flg ( ID flgid, UINT setptn )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    p->setbits( setptn );
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
clr_flg ( ID flgid, UINT clrptn )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    p->maskbits( clrptn );
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
wai_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    CYG_UIT_PARAMCHECK_PTR( p_flgptn );
    CYG_UIT_PARAMCHECK( 0 == (wfmode & ~Cyg_Flag::MASK), E_PAR );
    CYG_UIT_PARAMCHECK( 0 != waiptn, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    // check we can use the wfmode value unchanged
    CYG_ASSERT( Cyg_Flag::AND == TWF_ANDW, "Flag AND value bad" );
    CYG_ASSERT( Cyg_Flag::OR  == TWF_ORW,  "Flag OR value bad" );
    CYG_ASSERT( Cyg_Flag::CLR == TWF_CLR,  "Flag CLR value bad" );

    UINT result = p->wait( waiptn, wfmode );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN();
    *p_flgptn  = result;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
pol_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    CYG_UIT_PARAMCHECK_PTR( p_flgptn );
    CYG_UIT_PARAMCHECK( 0 == (wfmode & ~Cyg_Flag::MASK), E_PAR );
    CYG_UIT_PARAMCHECK( 0 != waiptn, E_PAR );
    // check we can use the wfmode value unchanged
    CYG_ASSERT( Cyg_Flag::AND == TWF_ANDW, "Flag AND value bad" );
    CYG_ASSERT( Cyg_Flag::OR  == TWF_ORW,  "Flag OR value bad" );
    CYG_ASSERT( Cyg_Flag::CLR == TWF_CLR,  "Flag CLR value bad" );

    UINT result = p->poll( waiptn, wfmode );
    if ( ! result )
        return E_TMOUT;
    *p_flgptn  = result;
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
twai_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode,
              TMO tmout )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    CYG_UIT_PARAMCHECK_PTR( p_flgptn );
    CYG_UIT_PARAMCHECK( 0 == (wfmode & ~Cyg_Flag::MASK), E_PAR );
    CYG_UIT_PARAMCHECK( 0 != waiptn, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( tmout );
    // check we can use the wfmode value unchanged
    CYG_ASSERT( Cyg_Flag::AND == TWF_ANDW, "Flag AND value bad" );
    CYG_ASSERT( Cyg_Flag::OR  == TWF_ORW,  "Flag OR value bad" );
    CYG_ASSERT( Cyg_Flag::CLR == TWF_CLR,  "Flag CLR value bad" );

    // do this now for the case when no sleeping actually occurs
    Cyg_Thread *self = Cyg_Thread::self();
    self->set_wake_reason( Cyg_Thread::TIMEOUT );
    UINT result;
    if ( TMO_FEVR == tmout )
        result = p->wait( waiptn, wfmode );
    else if ( TMO_POL == tmout )
        result = p->poll( waiptn, wfmode );
    else
        result = p->wait( waiptn, wfmode,
            Cyg_Clock::real_time_clock->current_value() +
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    *p_flgptn  = result;
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
ref_flg ( T_RFLG *pk_rflg, ID flgid )
{
    Cyg_Flag *p;
    CYG_UITRON_CHECK_AND_GETP_FLAGS( flgid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_rflg );
    pk_rflg->exinf  = NADR;
    pk_rflg->wtsk   = p->waiting();
    pk_rflg->flgptn = p->peek();
    return E_OK;
}

#endif // 0 < CYG_UITRON_NUM( FLAGS )
#endif // CYGPKG_UITRON_FLAGS

#ifdef CYGPKG_UITRON_MBOXES
#if 0 < CYG_UITRON_NUM( MBOXES )

#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
CYG_UITRON_NEWFUNCTION( Cyg_Mbox )

CYG_UIT_FUNC_INLINE
ER
cre_mbx ( ID mbxid, T_CMBX* pk_cmbx )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_cmbx );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( MBOXES, mbxid );
    if ( ((ATR)(TA_TFIFO + TA_MFIFO)) != pk_cmbx->mbxatr )
        ret = E_RSATR;
    else
        CYG_UITRON_PTRS( MBOXES )[ mbxid - 1 ] =
            new( &(CYG_UITRON_OBJS( MBOXES )[ mbxid - 1 ]) )
            Cyg_Mbox();
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_mbx ( ID mbxid )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( MBOXES )[ mbxid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    CYG_UITRON_PTRS( MBOXES )[ mbxid - 1 ] = NULL;
    p->~Cyg_Mbox();
    Cyg_Scheduler::unlock();
    return E_OK;
}
#endif // CYGPKG_UITRON_MBOXES_CREATE_DELETE

// This bit of unpleasantness is to allow uITRON programs to send a NULL
// message - if permitted by the parameter checking.
// 
// NULL is used internally to mean no message; but -1 is fine.  So we send
// a NULL as a NADR and if we see a NULL coming back, change it to a NADR.
//
// One hopes that often this will be optimized out, since the one or both
// of these being true has been detected and errored out just above.

#ifdef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
// represent a NULL as NADR internally
#define CYG_UIT_TMSG_FIXUP_IN( _p_ )    CYG_MACRO_START                 \
    if ( NULL == (_p_) )                                                \
        (_p_) = (T_MSG *)NADR;                                          \
CYG_MACRO_END

// we get a NADR back sometimes, meaning NULL
#define CYG_UIT_TMSG_FIXUP_OUT( _p_ )   CYG_MACRO_START                 \
    if ( NADR == (_p_) )                                                \
        (_p_) = (T_MSG *)NULL;                                          \
CYG_MACRO_END

#else
// NULL is checked for and makes an error
#define CYG_UIT_TMSG_FIXUP_IN( _p_ )    CYG_EMPTY_STATEMENT
#define CYG_UIT_TMSG_FIXUP_OUT( _p_ )   CYG_EMPTY_STATEMENT
#endif

// and sometimes either in status enquiries
#define CYG_UIT_TMSG_FIXUP_ALL( _p_ )   CYG_MACRO_START                 \
    if ( NULL == (_p_) )                                                \
        (_p_) = (T_MSG *)NADR;                                          \
    else if ( NADR == (_p_) )                                           \
        (_p_) = (T_MSG *)NULL;                                          \
CYG_MACRO_END

CYG_UIT_FUNC_INLINE
ER
snd_msg ( ID mbxid, T_MSG *pk_msg )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_msg );
    CYG_UIT_TMSG_FIXUP_IN( pk_msg );
    cyg_bool result = p->tryput( (void *)pk_msg );
    if ( ! result )
        return E_QOVR;
    return E_OK;
}


CYG_UIT_FUNC_INLINE
ER
rcv_msg ( T_MSG **ppk_msg, ID mbxid )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    CYG_UIT_PARAMCHECK_PTR( ppk_msg );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    T_MSG *result = (T_MSG *)p->get();
    if ( ! result )
        CYG_UITRON_FAIL_RETURN();
    CYG_UIT_TMSG_FIXUP_OUT( result );
    *ppk_msg = result;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
prcv_msg ( T_MSG **ppk_msg, ID mbxid )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    CYG_UIT_PARAMCHECK_PTR( ppk_msg );
    T_MSG *result = (T_MSG *)p->tryget();
    if ( ! result )
        return E_TMOUT;
    CYG_UIT_TMSG_FIXUP_OUT( result );
    *ppk_msg = result;
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
trcv_msg ( T_MSG **ppk_msg, ID mbxid, TMO tmout )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    CYG_UIT_PARAMCHECK_PTR( ppk_msg );
    CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( tmout );
    // do this now for the case when no sleeping actually occurs
    Cyg_Thread *self = Cyg_Thread::self();
    self->set_wake_reason( Cyg_Thread::TIMEOUT );
    T_MSG *result;
    if ( TMO_FEVR == tmout )
        result = (T_MSG *)p->get();
    else if ( TMO_POL == tmout )
        result = (T_MSG *)p->tryget();
    else
        result = (T_MSG *)p->get(
            Cyg_Clock::real_time_clock->current_value() +
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    CYG_UIT_TMSG_FIXUP_OUT( result );
    *ppk_msg = result;
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
ref_mbx ( T_RMBX *pk_rmbx, ID mbxid )
{
    Cyg_Mbox *p;
    CYG_UITRON_CHECK_AND_GETP_MBOXES( mbxid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_rmbx );
    pk_rmbx->exinf  = NADR;
    pk_rmbx->wtsk   = p->waiting_to_get();
    pk_rmbx->pk_msg = (T_MSG *)p->peek_item();
    CYG_UIT_TMSG_FIXUP_ALL( pk_rmbx->pk_msg );
    return E_OK;
}

#undef CYG_UIT_TMSG_FIXUP_IN
#undef CYG_UIT_TMSG_FIXUP_OUT
#undef CYG_UIT_TMSG_FIXUP_ALL
        
#endif // 0 < CYG_UITRON_NUM( MBOXES )
#endif // CYGPKG_UITRON_MBOXES

// - Extended Synchronization and Communication Functions
        
#if 0 // NOT SUPPORTED
ER      cre_mbf ( ID mbfid, T_CMBF *pk_cmbf );
ER      del_mbf ( ID mbfid );
ER      snd_mbf ( ID mbfid, VP msg, INT msgsz );
ER      psnd_mbf ( ID mbfid, VP msg, INT msgsz );
ER      tsnd_mbf ( ID mbfid, VP msg, INT msgsz, TMO tmout );
ER      rcv_mbf ( VP msg, INT *p_msgsz, ID mbfid );
ER      prcv_mbf ( VP msg, INT *p_msgsz, ID mbfid );
ER      trcv_mbf ( VP msg, INT *p_msgsz, ID mbfid, TMO tmout );
ER      ref_mbf ( T_RMBF *pk_rmbf, ID mbfid );
ER      cre_por ( ID porid, T_CPOR *pk_cpor );
ER      del_por ( ID porid );
ER      cal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz );
ER      pcal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz );
ER      tcal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz, TMO tmout );
ER      acp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn );
ER      pacp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn );
ER      tacp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn, TMO tmout );
ER      fwd_por ( ID porid, UINT calptn, RNO rdvno, VP msg, INT cmsgsz
              );
ER      rpl_rdv ( RNO rdvno, VP msg, INT rmsgsz );
ER      ref_por ( T_RPOR *pk_rpor, ID porid );
#endif
        
// - Interrupt Management Functions
        
#if 0 // NOT SUPPORTED
ER      def_int ( UINT dintno, T_DINT *pk_dint );
void    ret_wup ( ID tskid );
#endif

CYG_UIT_FUNC_INLINE
ER
loc_cpu ( void )
{
    CYG_UITRON_CHECK_TASK_CONTEXT();
    Cyg_Scheduler::lock();
    // Prevent preemption by going up to prio 0
    if ( 0 == cyg_uitron_dis_dsp_old_priority ) {
#ifdef CYGIMP_THREAD_PRIORITY
        Cyg_Thread *p = Cyg_Thread::self();
        cyg_uitron_dis_dsp_old_priority = p->get_priority();
        p->set_priority( 0 );
#else
        cyg_uitron_dis_dsp_old_priority = 1;
#endif
    }
    Cyg_Interrupt::disable_interrupts();
    Cyg_Scheduler::unlock();
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
unl_cpu ( void )
{
    CYG_UITRON_CHECK_TASK_CONTEXT();
    Cyg_Scheduler::lock();
    // Enable dispatching (if disabled) and maybe switch threads
    if ( 0 != cyg_uitron_dis_dsp_old_priority ) {
        // We had prevented preemption by going up to prio 0
#ifdef CYGIMP_THREAD_PRIORITY
        Cyg_Thread *p = Cyg_Thread::self();
        p->set_priority( cyg_uitron_dis_dsp_old_priority );
#endif
        cyg_uitron_dis_dsp_old_priority = 0;
    }
    Cyg_Interrupt::enable_interrupts();
    Cyg_Scheduler::unlock();
    CYG_UITRON_CHECK_DISPATCH_ENABLED(); // NB: afterwards!
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
dis_int ( UINT eintno )
{
    CYG_INTERRUPT_STATE old_ints;
    
#if 0 < CYGNUM_HAL_ISR_MIN
    CYG_UIT_PARAMCHECK( CYGNUM_HAL_ISR_MIN <= eintno, E_PAR );
#endif
    CYG_UIT_PARAMCHECK( CYGNUM_HAL_ISR_MAX >= eintno, E_PAR );
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_MASK( eintno );
    HAL_RESTORE_INTERRUPTS(old_ints);
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ena_int ( UINT eintno )
{
    CYG_INTERRUPT_STATE old_ints;

#if 0 < CYGNUM_HAL_ISR_MIN
    CYG_UIT_PARAMCHECK( CYGNUM_HAL_ISR_MIN <= eintno, E_PAR );
#endif
    CYG_UIT_PARAMCHECK( CYGNUM_HAL_ISR_MAX >= eintno, E_PAR );
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_UNMASK( eintno );
    HAL_RESTORE_INTERRUPTS(old_ints);
    return E_OK;
}

#if 0 // NOT SUPPORTED
ER      chg_iXX ( UINT iXXXX );
ER      ref_iXX ( UINT *p_iXXXX );
#endif
        
// - Memorypool Management Functions
#ifdef CYGPKG_UITRON_MEMPOOLVAR
#if 0 < CYG_UITRON_NUM( MEMPOOLVAR )

#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE

CYG_UITRON_NEWFUNCTION( Cyg_Mempool_Variable )

CYG_UIT_FUNC_INLINE
ER
cre_mpl ( ID mplid, T_CMPL *pk_cmpl )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_cmpl );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( MEMPOOLVAR, mplid );
    Cyg_Mempool_Variable *p = &(CYG_UITRON_OBJS( MEMPOOLVAR )[ mplid - 1 ]);
    Cyg_Mempool_Status stat;

    // preserve the original memory area to use
    p->get_status( CYG_MEMPOOL_STAT_ORIGBASE|CYG_MEMPOOL_STAT_ORIGSIZE, stat );

    if ( stat.origsize < pk_cmpl->mplsz )
        ret = E_NOMEM;
    else if ( TA_TFIFO != pk_cmpl->mplatr )
        ret = E_RSATR;
    else
        CYG_UITRON_PTRS( MEMPOOLVAR )[ mplid - 1 ] =
            new( p ) Cyg_Mempool_Variable( 
                const_cast<cyg_uint8 *>(stat.origbase), stat.origsize );
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_mpl ( ID mplid )
{
    Cyg_Mempool_Variable *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( MEMPOOLVAR )[ mplid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    CYG_UITRON_PTRS( MEMPOOLVAR )[ mplid - 1 ] = NULL;
    p->~Cyg_Mempool_Variable();
    Cyg_Scheduler::unlock();
    return E_OK;
}
#endif // CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE

CYG_UIT_FUNC_INLINE
ER
get_blk ( VP *p_blk, ID mplid, INT blksz )
{
    Cyg_Mempool_Variable *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blk );
    CYG_UIT_PARAMCHECK( blksz > 0, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    VP result = (VP)p->alloc(blksz);
    if ( ! result )
        CYG_UITRON_FAIL_RETURN();
    *p_blk = result;
    return E_OK;
}


CYG_UIT_FUNC_INLINE
ER
pget_blk ( VP *p_blk, ID mplid, INT blksz )
{
    Cyg_Mempool_Variable *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blk );
    CYG_UIT_PARAMCHECK( blksz > 0, E_PAR );
    VP result = (VP)p->try_alloc(blksz);
    if ( ! result )
        return E_TMOUT;
    *p_blk = result;
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
tget_blk ( VP *p_blk, ID mplid, INT blksz, TMO tmout )
{
    Cyg_Mempool_Variable *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blk );
    CYG_UIT_PARAMCHECK( blksz > 0, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( tmout );
    // do this now for the case when no sleeping actually occurs
    Cyg_Thread *self = Cyg_Thread::self();
    self->set_wake_reason( Cyg_Thread::TIMEOUT );
    VP result;
    if ( TMO_FEVR == tmout )
        result = p->alloc(blksz);
    else if ( TMO_POL == tmout )
        result = p->try_alloc(blksz);
    else
        result = p->alloc( blksz,
            Cyg_Clock::real_time_clock->current_value() +
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    *p_blk = result;
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
rel_blk ( ID mplid, VP blk )
{
    Cyg_Mempool_Variable *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    CYG_UIT_PARAMCHECK_PTR( blk );
    cyg_bool result = p->free( (cyg_uint8 *)blk );
    if ( ! result )
        return E_PAR;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_mpl ( T_RMPL *pk_rmpl, ID mplid )
{
    Cyg_Mempool_Variable *p;
    Cyg_Mempool_Status stat;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLVAR( mplid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_rmpl );
    p->get_status( CYG_MEMPOOL_STAT_WAITING|
                   CYG_MEMPOOL_STAT_TOTALFREE|
                   CYG_MEMPOOL_STAT_MAXFREE, stat );

    pk_rmpl->exinf = NADR;
    pk_rmpl->wtsk = stat.waiting;
    pk_rmpl->frsz = stat.totalfree;
    pk_rmpl->maxsz = stat.maxfree;

    return E_OK;
}

#endif // 0 < CYG_UITRON_NUM( MEMPOOLVAR )
#endif // CYGPKG_UITRON_MEMPOOLVAR

#ifdef CYGPKG_UITRON_MEMPOOLFIXED
#if 0 < CYG_UITRON_NUM( MEMPOOLFIXED )

#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE

CYG_UITRON_NEWFUNCTION( Cyg_Mempool_Fixed )

CYG_UIT_FUNC_INLINE
ER
cre_mpf ( ID mpfid, T_CMPF *pk_cmpf )
{
    ER ret = E_OK;
    CYG_UIT_PARAMCHECK_PTR( pk_cmpf );
    CYG_UITRON_CHECK_NO_OBJ_LOCK_SCHED( MEMPOOLFIXED, mpfid );
    Cyg_Mempool_Fixed *p = &(CYG_UITRON_OBJS( MEMPOOLFIXED )[ mpfid - 1 ]);
    Cyg_Mempool_Status stat;

    // preserve the original memory area to use
    p->get_status( CYG_MEMPOOL_STAT_ORIGBASE|CYG_MEMPOOL_STAT_ORIGSIZE, stat );

    if ( stat.origsize < (pk_cmpf->blfsz * (pk_cmpf->mpfcnt + 1)) )
        ret = E_NOMEM;
    else if ( TA_TFIFO != pk_cmpf->mpfatr )
        ret = E_RSATR;
    else
        CYG_UITRON_PTRS( MEMPOOLFIXED )[ mpfid - 1 ] =
            new( p )
            Cyg_Mempool_Fixed( const_cast<cyg_uint8 *>(stat.origbase),
                               stat.origsize, (CYG_ADDRWORD)pk_cmpf->blfsz );
    Cyg_Scheduler::unlock();
    return ret;
}

CYG_UIT_FUNC_INLINE
ER
del_mpf ( ID mpfid )
{
    Cyg_Mempool_Fixed *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    Cyg_Scheduler::lock();
    // deal with the race condition here
    if ( p != CYG_UITRON_PTRS( MEMPOOLFIXED )[ mpfid - 1 ] ) {
        Cyg_Scheduler::unlock();
        return E_NOEXS;
    }
    CYG_UITRON_PTRS( MEMPOOLFIXED )[ mpfid - 1 ] = NULL;
    p->~Cyg_Mempool_Fixed();
    Cyg_Scheduler::unlock();
    return E_OK;
}
#endif // CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE

CYG_UIT_FUNC_INLINE
ER
get_blf ( VP *p_blf, ID mpfid )
{
    Cyg_Mempool_Fixed *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blf );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    VP result = (VP)p->alloc();
    if ( ! result )
        CYG_UITRON_FAIL_RETURN();
    *p_blf = result;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
pget_blf ( VP *p_blf, ID mpfid )
{
    Cyg_Mempool_Fixed *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blf );
    VP result = (VP)p->try_alloc();
    if ( ! result )
        return E_TMOUT;
    *p_blf = result;
    return E_OK;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
tget_blf ( VP *p_blf, ID mpfid, TMO tmout )
{
    Cyg_Mempool_Fixed *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    CYG_UIT_PARAMCHECK_PTR( p_blf );
    CYG_UITRON_CHECK_DISPATCH_ENABLED_TMO( tmout );
    // do this now for the case when no sleeping actually occurs
    Cyg_Thread *self = Cyg_Thread::self();
    self->set_wake_reason( Cyg_Thread::TIMEOUT );
    VP result;
    if ( TMO_FEVR == tmout )
        result = p->alloc();
    else if ( TMO_POL == tmout )
        result = p->try_alloc();
    else
        result = p->alloc(
            Cyg_Clock::real_time_clock->current_value() +
            (cyg_tick_count)CYG_UITRON_TIME_UIT_TO_SYS32( tmout ) );
    if ( ! result )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    *p_blf = result;
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

CYG_UIT_FUNC_INLINE
ER
rel_blf ( ID mpfid, VP blf )
{
    Cyg_Mempool_Fixed *p;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    CYG_UIT_PARAMCHECK_PTR( blf );
    cyg_bool result = p->free( (cyg_uint8 *)blf );
    if ( ! result )
        return E_PAR;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_mpf ( T_RMPF *pk_rmpf, ID mpfid )
{
    Cyg_Mempool_Fixed *p;
    Cyg_Mempool_Status stat;
    CYG_UITRON_CHECK_AND_GETP_MEMPOOLFIXED( mpfid, p );
    CYG_UIT_PARAMCHECK_PTR( pk_rmpf );

    p->get_status( CYG_MEMPOOL_STAT_WAITING|
                   CYG_MEMPOOL_STAT_TOTALFREE|
                   CYG_MEMPOOL_STAT_TOTALALLOCATED|
                   CYG_MEMPOOL_STAT_BLOCKSIZE, stat );

    pk_rmpf->exinf = NADR;
    pk_rmpf->wtsk = stat.waiting;

    pk_rmpf->frbcnt = stat.totalfree / stat.blocksize;
    // these two are "implementation dependent" ie. eCos only
    pk_rmpf->numbcnt = stat.totalallocated / stat.blocksize;
    pk_rmpf->bsize = stat.blocksize;

    return E_OK;
}
        
#endif // 0 < CYG_UITRON_NUM( MEMPOOLFIXED )
#endif // CYGPKG_UITRON_MEMPOOLFIXED

// - Time Management Functions
        
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
CYG_UIT_FUNC_INLINE
ER
set_tim ( SYSTIME *pk_tim )
{
    CYG_UIT_PARAMCHECK_PTR( pk_tim );
    Cyg_Clock::real_time_clock->set_value(
        CYG_UITRON_TIME_UIT_TO_SYS64( *pk_tim ) );
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
get_tim ( SYSTIME *pk_tim )
{
    CYG_UIT_PARAMCHECK_PTR( pk_tim );
    *pk_tim = CYG_UITRON_TIME_SYS_TO_UIT64(
        Cyg_Clock::real_time_clock->current_value() );
    return E_OK;
}
#endif // CYGVAR_KERNEL_COUNTERS_CLOCK


#ifdef CYGFUN_KERNEL_THREADS_TIMER
CYG_UIT_FUNC_INLINE
ER
dly_tsk ( DLYTIME dlytim )
{
    CYG_UIT_PARAMCHECK( 0 <= dlytim, E_PAR );
    CYG_UITRON_CHECK_DISPATCH_ENABLED();
    if ( 0 >= dlytim )
        return E_OK;
    Cyg_Thread *self = Cyg_Thread::self();
    CYG_UITRON_CHECK_TASK_CONTEXT_SELF( self );
    self->delay( CYG_UITRON_TIME_UIT_TO_SYS64( dlytim ) );
    if ( Cyg_Thread::DONE != self->get_wake_reason() )
        CYG_UITRON_FAIL_RETURN_SELF( self );
    return E_OK;
}
#endif // CYGFUN_KERNEL_THREADS_TIMER

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#ifdef CYGPKG_UITRON_CYCLICS
#if 0 < CYG_UITRON_NUM( CYCLICS )
CYG_UIT_FUNC_INLINE
ER
def_cyc ( HNO cycno, T_DCYC *pk_dcyc )
{
    // pk_dcyc->cycatr is ignored
    // The only relevant attribute is TA_HLNG/TA_ASM.
    // This can be ignored as assembler routines are defined to be
    // more conservative with registers than the procedure call standard.
    cyg_tick_count t;
    Cyg_Timer *p;
    CYG_UITRON_CHECK_AND_GETHDLR( CYCLICS, cycno, p );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    CYG_UIT_PARAMCHECK( NULL != pk_dcyc, E_PAR );
#endif
    if( NADR == pk_dcyc ) {
        p->~Cyg_Timer();
        return E_OK;
    }
    CYG_UIT_PARAMCHECK( 0 == (pk_dcyc->cycact & ~TCY_ON), E_PAR );
    CYG_UIT_PARAMCHECK( 0 < pk_dcyc->cyctim, E_PAR );
    t = CYG_UITRON_TIME_UIT_TO_SYS64( pk_dcyc->cyctim );
    p->initialize(
        Cyg_Clock::real_time_clock,
        (cyg_alarm_fn *)pk_dcyc->cychdr,
        (CYG_ADDRWORD)pk_dcyc->exinf,
        Cyg_Clock::real_time_clock->current_value() + t,
        t,
        pk_dcyc->cycact);
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
act_cyc ( HNO cycno, UINT cycact )
{
    Cyg_Timer *p;
    CYG_UITRON_CHECK_AND_GETHDLR( CYCLICS, cycno, p );
    CYG_UIT_PARAMCHECK( p->is_initialized(), E_NOEXS);
    CYG_UIT_PARAMCHECK( 0 == (cycact & ~(TCY_ON | TCY_INI)), E_PAR );
    p->activate(cycact);
    return E_OK;
}


CYG_UIT_FUNC_INLINE
ER
ref_cyc ( T_RCYC *pk_rcyc, HNO cycno )
{
    Cyg_Timer *p;
    cyg_tick_count t;
    CYG_UITRON_CHECK_AND_GETHDLR( CYCLICS, cycno, p );
    CYG_UIT_PARAMCHECK( p->is_initialized(), E_NOEXS);
    CYG_UIT_PARAMCHECK_PTR( pk_rcyc );

    pk_rcyc->exinf = (VP)p->get_data();
    Cyg_Scheduler::lock();
    t = p->get_trigger() - Cyg_Clock::real_time_clock->current_value();
    Cyg_Scheduler::unlock();
    pk_rcyc->lfttim = CYG_UITRON_TIME_SYS_TO_UIT64( t );
    pk_rcyc->cycact = (UINT)p->is_enabled();
    return E_OK;
}
#endif // 0 < CYG_UITRON_NUM( CYCLICS )
#endif // CYGPKG_UITRON_CYCLICS

#ifdef CYGPKG_UITRON_ALARMS
#if 0 < CYG_UITRON_NUM( ALARMS )
CYG_UIT_FUNC_INLINE
ER
def_alm ( HNO almno, T_DALM *pk_dalm )
{
    Cyg_Timer *p;
    cyg_tick_count t, now;
    CYG_UITRON_CHECK_AND_GETHDLR( ALARMS, almno, p );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    CYG_UIT_PARAMCHECK( NULL != pk_dalm, E_PAR );
#endif
    if( NADR == pk_dalm ) {
        p->~Cyg_Timer();
        return E_OK;
    }

    CYG_UIT_PARAMCHECK( 0 == (pk_dalm->tmmode & ~TTM_REL), E_PAR );
    CYG_UIT_PARAMCHECK( 0 < pk_dalm->almtim, E_PAR );

    // make the time arithmetic safe without locking
    now = Cyg_Clock::real_time_clock->current_value();
    t = CYG_UITRON_TIME_UIT_TO_SYS64( pk_dalm->almtim );
    if( TTM_REL & pk_dalm->tmmode )
        t += now;

    CYG_UIT_PARAMCHECK( now < t, E_PAR );

    p->initialize(Cyg_Clock::real_time_clock,
                  (cyg_alarm_fn *)pk_dalm->almhdr,
                  (CYG_ADDRWORD)pk_dalm->exinf,
                  t, 0, Cyg_Timer::ENABLE);

    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_alm ( T_RALM *pk_ralm, HNO almno )
{
    Cyg_Timer *p;
    cyg_tick_count t;

    CYG_UITRON_CHECK_AND_GETHDLR( ALARMS, almno, p );
    CYG_UIT_PARAMCHECK_PTR( pk_ralm );
    CYG_UIT_PARAMCHECK( p->is_initialized(), E_NOEXS);

    Cyg_Scheduler::lock();
    t = p->get_trigger() - Cyg_Clock::real_time_clock->current_value();
    Cyg_Scheduler::unlock();
    pk_ralm->exinf  = (VP)p->get_data();
    pk_ralm->lfttim = CYG_UITRON_TIME_SYS_TO_UIT64( t );
    return E_OK;
}
#endif // 0 < CYG_UITRON_NUM( ALARMS )
#endif // CYGPKG_UITRON_ALARMS

#endif // CYGVAR_KERNEL_COUNTERS_CLOCK
        
// - System Management Functions
        
CYG_UIT_FUNC_INLINE
ER
get_ver ( T_VER *pk_ver )
{
    CYG_UIT_PARAMCHECK_PTR( pk_ver );

    pk_ver->maker       = CYGNUM_UITRON_VER_MAKER;
    pk_ver->id          = CYGNUM_UITRON_VER_ID;
    pk_ver->spver       = CYGNUM_UITRON_VER_SPVER;
    pk_ver->prver       = CYGNUM_UITRON_VER_PRVER;
    pk_ver->prno[0]     = CYGNUM_UITRON_VER_PRNO_0;
    pk_ver->prno[1]     = CYGNUM_UITRON_VER_PRNO_1;
    pk_ver->prno[2]     = CYGNUM_UITRON_VER_PRNO_2;
    pk_ver->prno[3]     = CYGNUM_UITRON_VER_PRNO_3;
    pk_ver->cpu         = CYGNUM_UITRON_VER_CPU;
    pk_ver->var         = CYGNUM_UITRON_VER_VAR;
   
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_sys ( T_RSYS *pk_rsys )
{
    CYG_UIT_PARAMCHECK_PTR( pk_rsys );
    if ( ! Cyg_Interrupt::interrupts_enabled() )
        // CPU is locked
        pk_rsys->sysstat = TSS_LOC;
    else
        pk_rsys->sysstat =
            (0 == cyg_uitron_dis_dsp_old_priority) ? TSS_TSK : TSS_DDSP;
    return E_OK;
}

CYG_UIT_FUNC_INLINE
ER
ref_cfg ( T_RCFG *pk_rcfg )
{
    CYG_UIT_PARAMCHECK_PTR( pk_rcfg );
    // no details here yet
    return E_OK;
}

#if 0 // NOT SUPPORTED
ER      def_svc ( FN s_fncd, T_DSVC *pk_dsvc );
ER      def_exc ( UINT exckind, T_DEXC *pk_dexc );
#endif
        
// - Network Support Functions
        
#if 0 // NOT SUPPORTED
ER      nrea_dat ( INT *p_reasz, VP dstadr, NODE srcnode, VP srcadr,
               INT datsz );
ER      nwri_dat ( INT *p_wrisz, NODE dstnode, VP dstadr, VP srcadr,
               INT datsz );
ER      nget_nod ( NODE *p_node );
ER      nget_ver ( T_VER *pk_ver, NODE node );
#endif

// ========================================================================

#endif // CYGPKG_UITRON

#endif // CYGPRI_UITRON_FUNCS_HERE_AND_NOW

#endif // CYGONCE_COMPAT_UITRON_UIT_FUNC_INL
//EOF uit_func.inl
