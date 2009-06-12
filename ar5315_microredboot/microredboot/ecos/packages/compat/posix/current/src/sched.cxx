//==========================================================================
//
//      sched.cxx
//
//      POSIX scheduler API implementation
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-03-27
// Purpose:             POSIX scheduler API implementation
// Description:         This file contains the implementation of the POSIX scheduler
//                      functions.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include "pprivate.h"                   // POSIX private header

#include <cyg/kernel/sched.hxx>        // scheduler definitions
#include <cyg/kernel/thread.hxx>       // thread definitions

#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/thread.inl>       // thread inlines

//==========================================================================
// Process scheduling functions.

//--------------------------------------------------------------------------
// Set scheduling parameters for given process.

int sched_setparam (pid_t pid, const struct sched_param *param)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( pid != 0 )
    {
        errno = ESRCH;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    errno = ENOSYS;
    CYG_REPORT_RETVAL( -1 );
    return -1;
}

//--------------------------------------------------------------------------
// Get scheduling parameters for given process.

int sched_getparam (pid_t pid, struct sched_param *param)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( pid != 0 )
    {
        errno = ESRCH;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    
    errno = ENOSYS;
    CYG_REPORT_RETVAL( -1 );
    return -1;
}

//--------------------------------------------------------------------------
// Set scheduling policy and/or parameters for given process.
int sched_setscheduler (pid_t pid,
                        int policy,
                        const struct sched_param *param)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( pid != 0 )
    {
        errno = ESRCH;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    errno = ENOSYS;
    CYG_REPORT_RETVAL( -1 );
    return -1;
}
    

//--------------------------------------------------------------------------
// Get scheduling policy for given process.

int sched_getscheduler (pid_t pid)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( pid != 0 )
    {
        errno = ESRCH;
        CYG_REPORT_RETVAL( 0 );
        return -1;
    }
    
    errno = ENOSYS;
    CYG_REPORT_RETVAL( -1 );
    return -1;
}    

//--------------------------------------------------------------------------
// Force current thread to relinquish the processor.

int sched_yield (void)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    Cyg_Thread::yield();
    
    CYG_REPORT_RETVAL( 0 );
    return 0;
}
   

//==========================================================================
// Scheduler parameter limits.

//--------------------------------------------------------------------------
// Get maximum priority value for a policy.

int sched_get_priority_max (int policy)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( policy != SCHED_FIFO &&
        policy != SCHED_RR &&
        policy != SCHED_OTHER )
    {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    
    int pri = PTHREAD_POSIX_PRIORITY( CYG_THREAD_MAX_PRIORITY );
    
    CYG_REPORT_RETVAL( pri );
    return pri;
}    

//--------------------------------------------------------------------------
// Get minimum priority value for a policy.

int sched_get_priority_min (int policy)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    if( policy != SCHED_FIFO &&
        policy != SCHED_RR &&
        policy != SCHED_OTHER )
    {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    // idle thread priority isn't valid for general use, so subtract 1
    int pri = PTHREAD_POSIX_PRIORITY( CYG_THREAD_MIN_PRIORITY-1 );
    
    CYG_REPORT_RETVAL( pri );
    return pri;
}    

//--------------------------------------------------------------------------
// Get the SCHED_RR interval for the given process.

int sched_rr_get_interval (pid_t pid, struct timespec *t)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

#ifdef CYGPKG_POSIX_CLOCKS
    if( pid != 0 )
    {
        errno = ESRCH;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    
    cyg_ticks_to_timespec( CYGNUM_KERNEL_SCHED_TIMESLICE_TICKS, t );
    
    CYG_REPORT_RETVAL( 0 );
    return 0;
#else
    errno = ENOSYS;
    CYG_REPORT_RETVAL( -1 );
    return -1;
#endif
} 

// -------------------------------------------------------------------------
// EOF sched.cxx
