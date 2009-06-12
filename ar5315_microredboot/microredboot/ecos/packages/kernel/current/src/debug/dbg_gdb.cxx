/*==========================================================================
//
//      dbg_gdb.c
//
//      GDB Debugging Interface
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
// Date:        1998-08-22
// Purpose:     GDB Debugging Interface
// Description: Interface for calls from GDB stubs into the OS. These
//              currently mostly support thread awareness.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>                // CYG_HAL_USE_ROM_MONITOR_CYGMON

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

#include <cyg/kernel/ktypes.h>

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/sched.hxx>

#include <cyg/kernel/thread.inl>
#include <cyg/kernel/sched.inl>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_stub.h>

extern "C"
{
#include <cyg/hal/dbg-threads-api.h>
};

#define USE_ID 1

#if (CYG_BYTEORDER == CYG_LSBFIRST)

unsigned long swap32(unsigned long x)
{
    unsigned long r = 0;

    r |= (x>>24)&0xFF;
    r |= ((x>>16)&0xFF)<<8;
    r |= ((x>>8)&0xFF)<<16;
    r |= ((x)&0xFF)<<24;

    return r;
}

#else

#define swap32(x) ((unsigned long)(x))

#endif

//--------------------------------------------------------------------------

externC int dbg_thread_capabilities(struct dbg_capabilities * cpb)
{
    cpb->mask1 = has_thread_current     |
        has_thread_registers            |
        has_thread_reg_change           |
        has_thread_list                 |
        has_thread_info                 ;
    return 1 ; 
}

//--------------------------------------------------------------------------

static void dbg_make_threadref(Cyg_Thread *thread, threadref *ref )
{
    // The following test tries to avoid accessing uninitialized pointers.
    // This can happen if we take a breakpoint before the data is copied
    // or the BSS zeroed. We currently assume that RAM will reset to zero
    // or 0xff. If it is random, we have no hope.

    if( (CYG_ADDRWORD)thread == 0 || (CYG_ADDRWORD)thread == 0xffffffff )                     
    {
        ((unsigned long *)ref)[0] = 0;
        ((unsigned long *)ref)[1] = 0;
    }
    else
    {
        cyg_uint16 id = thread->get_unique_id();

#if USE_ID
        ((unsigned long *)ref)[0] = (unsigned long)thread;
        ((unsigned long *)ref)[1] = (unsigned long)swap32(id);
#else    
        ((unsigned long *)ref)[1] = (unsigned long)thread;
        ((unsigned long *)ref)[0] = (unsigned long)id;
#endif
    }
}

static Cyg_Thread *dbg_get_thread( threadref *ref)
{
#if USE_ID

    cyg_uint16 id = 0;

    id = (cyg_uint16)swap32(((unsigned long *)ref)[1]);

    Cyg_Thread *th = Cyg_Thread::get_list_head();
    while( th != 0 )
    {
        if( th->get_unique_id() == id ) break;
        th = th->get_list_next();
    }

//    if( thread->get_unique_id() != id ) th = 0;

#else
 
    cyg_uint16 id = 0;

    Cyg_Thread *thread = (Cyg_Thread *)(((unsigned long *)ref)[1]);
    id = (cyg_uint16)(((unsigned long *)ref)[0]);

    // Validate the thread.
    Cyg_Thread *th = Cyg_Thread::get_list_head();
    while( th != 0 )
    {
        if( th == thread ) break;
        th = th->get_list_next();
    }

//    if( thread->get_unique_id() != id ) th = 0;

#endif
 
    return th;
}

//--------------------------------------------------------------------------

externC int dbg_currthread(threadref * varparm)
{
    Cyg_Thread *thread = Cyg_Scheduler::get_current_thread();

    dbg_make_threadref(thread, varparm );
  
    return 1 ; 
}

//--------------------------------------------------------------------------

externC int dbg_thread_id(threadref *threadid)
{
    Cyg_Thread *thread = dbg_get_thread(threadid);
    if( thread == 0 ) return 0;
    return thread->get_unique_id ();
}

//--------------------------------------------------------------------------

externC int dbg_currthread_id(void)
{
    Cyg_Thread *thread = Cyg_Scheduler::get_current_thread();
    return thread->get_unique_id ();
}

//--------------------------------------------------------------------------

externC int dbg_threadlist(int startflag,
                   threadref * lastthreadid,
                   threadref * next_thread)
{
    Cyg_Thread *thread;
    if( startflag )
    {
        thread = Cyg_Thread::get_list_head();
        dbg_make_threadref(thread, next_thread);
    }
    else
    {
        thread = dbg_get_thread(lastthreadid);

        if( thread == 0 ) return 0;
        thread = thread->get_list_next();

        if( thread == 0 ) return 0;
        dbg_make_threadref(thread, next_thread);        
    }
    return 1 ;
}

//--------------------------------------------------------------------------
// Some support routines for manufacturing thread info strings

static char *dbg_addstr(char *s, char *t)
{
    while( (*s++ = *t++) != 0 );

    return s-1;
}

static char *dbg_addint(char *s, int n, int base)
{
    char buf[16];
    char sign = '+';
    cyg_count8 bpos;
    char *digits = "0123456789ABCDEF";

    if( n < 0 ) n = -n, sign = '-';
    
    /* Set pos to start */
    bpos = 0;

    /* construct digits into buffer in reverse order */
    if( n == 0 ) buf[bpos++] = '0';
    else while( n != 0 )
    {
        cyg_ucount8 d = n % base;
        buf[bpos++] = digits[d];
        n /= base;
    }

    /* set sign if negative. */
    if( sign == '-' )
    {
        buf[bpos] = sign;
    }
    else bpos--;

    /* Now write it out in correct order. */
    while( bpos >= 0 )
        *s++ = buf[bpos--];

    *s = 0;
    
    return s;
}

static char *dbg_adddec(char *s, int x)
{
    return dbg_addint(s, x, 10);
}

//--------------------------------------------------------------------------

externC int dbg_threadinfo(
                   threadref * threadid,
                   struct cygmon_thread_debug_info * info)
{
    static char statebuf[60];
    
    Cyg_Thread *thread = dbg_get_thread(threadid);
    if( thread == 0 ) return 0;

    info->context_exists        = 1;

    char *sbp = statebuf;
    char *s;

    if( thread->get_state() & Cyg_Thread::SUSPENDED )
    {
        sbp = dbg_addstr( sbp, "suspended+");
    }

    switch( thread->get_state() & ~Cyg_Thread::SUSPENDED )
    {
    case Cyg_Thread::RUNNING:
        if ( Cyg_Scheduler::get_current_thread() == thread ) {
            s = "running";              break;
        }
        else if ( thread->get_state() & Cyg_Thread::SUSPENDED ) {
            s = ""; sbp--; /*kill '+'*/ break;
        }
        else {
            s = "ready";                break;
        }
    case Cyg_Thread::SLEEPING:
        s = "sleeping";                 break;
    case Cyg_Thread::COUNTSLEEP | Cyg_Thread::SLEEPING:
    case Cyg_Thread::COUNTSLEEP:
        s = "counted sleep";            break;
    case Cyg_Thread::CREATING:
        s = "creating"; sbp = statebuf; break;
    case Cyg_Thread::EXITED:
        s = "exited"; sbp = statebuf;   break;
    default:
        s = "unknown state";            break;
    }

    sbp = dbg_addstr( sbp, s );
    sbp = dbg_addstr( sbp, ", Priority: " );
    sbp = dbg_adddec( sbp, thread->get_priority() );
    
    info->thread_display        = statebuf;

#ifdef CYGVAR_KERNEL_THREADS_NAME
    info->unique_thread_name    = thread->get_name();
#else
    info->unique_thread_name    = 0;
#endif

    info->more_display          = 0;

    return 1 ;
}

//--------------------------------------------------------------------------

externC int dbg_getthreadreg(
                     threadref * osthreadid,
                     int regcount, /* count of registers in the array */
                     void * regval)  /* fillin this array */
{
    Cyg_Thread *thread = dbg_get_thread(osthreadid);

    if( thread == 0 ) return 0;

    if( thread == Cyg_Scheduler::get_current_thread() )
    {
#if defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)
        // We have no state for the current thread, Cygmon has
        // got that and we cannot get at it.
        return 0;
#elif defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
        // registers hold the state of the current thread.
        __stub_copy_registers ((target_register_t *)regval, registers);
#else
        return 0;
#endif
    }
    else
    {
        HAL_SavedRegisters *regs = thread->get_saved_context();
        if( regs == 0 ) return 0;

        HAL_GET_GDB_REGISTERS (regval, regs);
    }
    
    return 1 ;
}

//--------------------------------------------------------------------------
                   
externC int dbg_setthreadreg(
                            threadref * osthreadid, 
                            int regcount , /* number of registers */
                            void * regval) 
{
    Cyg_Thread *thread = dbg_get_thread(osthreadid);
    
    if( thread == 0 ) return 0;

    if( thread == Cyg_Scheduler::get_current_thread() )
    {
#if defined(CYG_HAL_USE_ROM_MONITOR_CYGMON)
        // We have no state for the current thread, Cygmon has
        // got that and we cannot get at it.
        return 0;
#elif defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
        // registers hold the state of the current thread.
        __stub_copy_registers (registers, (target_register_t *)regval);
#else
        return 0;
#endif
    }
    else
    {
        HAL_SavedRegisters *regs = thread->get_saved_context();
        if( regs == 0 ) return 0;

        HAL_SET_GDB_REGISTERS (regs, regval);
    }
    
    return 1;
}

//--------------------------------------------------------------------------
// Thread scheduler control for debugger.
// Arguments:
//      osthreadid      : must match currently executing thread.
//                        Future use: change the currently executing thread.
//      lock            : 0 == unlock scheduler, 1 == lock scheduler
//      mode            : 0 == single-instruction step, 1 == free running
//
// Return values:
// 1  == success
// 0  == failure
// -1 == request that the caller handle this itself
//       (eg.by disabling interrupts)
//

externC int dbg_scheduler(
                          threadref * osthreadid,
                          int lock,     /* 0 == unlock, 1 == lock */
                          int mode)     /* 0 == step,   1 == continue */
{
#if 0
    /* Minimal implementation: let stub do the work.  */
    return -1;                          // Stub will disable interrupts
#else
    Cyg_Thread *thread = dbg_get_thread(osthreadid);

    if( thread == 0 ) return 0;         // fail

    if( thread == Cyg_Scheduler::get_current_thread() )
    {
        // OK to proceed

        if (lock)
        {
            Cyg_Scheduler::lock();
        }
        else
        {
            if (Cyg_Scheduler::get_sched_lock() >= 1)
                Cyg_Scheduler::unlock_simple();
        }
        return 1;                       // success
    }
    else
    {
        // Cannot accept any thread other than current one
        return 0;                       // fail
    }
#endif
}


#endif // CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT

//--------------------------------------------------------------------------
// End of dbg_gdb.cxx
