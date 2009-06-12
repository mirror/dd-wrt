#ifndef CYGONCE_KERNEL_SCHED_INL
#define CYGONCE_KERNEL_SCHED_INL

//==========================================================================
//
//      sched.inl
//
//      Scheduler class inlines
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
// Purpose:     Define inlines for scheduler classes
// Description: Inline functions for the scheduler classes. These are
//              not defined in the header so that we have the option
//              of making them non-inline.
// Usage:
//              #include <cyg/kernel/sched.hxx>
//              ...
//              #include <cyg/kernel/sched.inl>
//              ...
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/instrmnt.h>
#include <cyg/hal/hal_arch.h>

// -------------------------------------------------------------------------
// Inlines for Cyg_Scheduler class

inline void Cyg_Scheduler::lock()
{
    // We do not need to do a read-modify-write sequence here because
    // the scheduler lock is strictly nesting. Even if we are interrupted
    // partway through the increment, the lock will be returned to the same
    // value before we are resumed/rescheduled.

    HAL_REORDER_BARRIER();

    inc_sched_lock();

    HAL_REORDER_BARRIER();

    CYG_INSTRUMENT_SCHED(LOCK,get_sched_lock(),0);
};

inline void Cyg_Scheduler::unlock()
{
    // This is an inline wrapper for the real scheduler unlock function in
    // Cyg_Scheduler::unlock_inner().
    
    // Only do anything if the lock is about to go zero, otherwise we simply
    // decrement and return. As with lock() we do not need any special code
    // to decrement the lock counter.

    CYG_INSTRUMENT_SCHED(UNLOCK,get_sched_lock(),0);
    
    HAL_REORDER_BARRIER();
    
    cyg_ucount32 __lock = get_sched_lock() - 1;
    
    if( __lock == 0 ) unlock_inner(0);
    else set_sched_lock(__lock);

    HAL_REORDER_BARRIER();
}

inline void Cyg_Scheduler::reschedule()
{
    // This function performs the equivalent of calling unlock() and
    // lock() is succession. Unlike that pair, however, it does not
    // leave a brief window between the calls when the lock is unclaimed
    // by the current thread.
    
    CYG_INSTRUMENT_SCHED(RESCHEDULE,get_sched_lock(),0);
    
    unlock_inner( get_sched_lock() );
}

inline void Cyg_Scheduler:: unlock_reschedule()
{
    // This function decrements the scheduler lock and also looks for
    // a reschedule opportunity. When the lock is being decremented
    // from 1 to zero this function is equivalent to unlock. When the
    // lock is being decremented to a non-zero value, it is more or less
    // equivalent to reschedule() followed by unlock().
    
    CYG_INSTRUMENT_SCHED(UNLOCK,get_sched_lock(),0);
    
    unlock_inner( get_sched_lock() - 1 );
}

inline void Cyg_Scheduler::unlock_simple()
{
    // This function decrements the lock, but does not call unlock_inner().
    // Therefore does not immediately allow another thread to run:
    // merely makes it possible for some other thread to run at some
    // indeterminate future time.  This is mainly for use by
    // debuggers, it should not normally be used anywhere else.

    CYG_INSTRUMENT_SCHED(UNLOCK,get_sched_lock(),0);

    HAL_REORDER_BARRIER();
        
    if (get_sched_lock() > 1)
        set_sched_lock(get_sched_lock() - 1);
    else zero_sched_lock();

    HAL_REORDER_BARRIER();
}


// -------------------------------------------------------------------------
// Inlines for Cyg_SchedThread class

#include <cyg/kernel/thread.inl>   // we use some thread inlines here

inline void Cyg_SchedThread::remove()
{
    if( queue != NULL )
    {
        queue->remove((Cyg_Thread *)this);
        queue = NULL;
    }
}

// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_SCHED_INL
// EOF sched.inl
