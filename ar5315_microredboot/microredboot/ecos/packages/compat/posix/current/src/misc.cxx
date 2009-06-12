//==========================================================================
//
//      misc.cxx
//
//      POSIX misc function implementations
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
// Date:                2000-07-18
// Purpose:             POSIX misc function implementation
// Description:         This file contains the implementation of miscellaneous POSIX
//                      functions that do not belong elsewhere.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include "pprivate.h"                   // POSIX private header

#include <unistd.h>
#include <sys/utsname.h>                // My header
#include <string.h>                     // strcpy
#include <limits.h>
#include <time.h>

#include <cyg/kernel/sched.hxx>

#include <cyg/kernel/sched.inl>

// -------------------------------------------------------------------------
// Supply some suitable values for constants that may not be present
// in all configurations.

#ifndef MQ_OPEN_MAX
#define MQ_OPEN_MAX 0
#endif
#ifndef MQ_PRIO_MAX
#define MQ_PRIO_MAX 0
#endif

// -------------------------------------------------------------------------

#define __string(_x) #_x
#define __xstring(_x) __string(_x)

// -------------------------------------------------------------------------
// uname()

__externC int uname( struct utsname *name )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );

    strcpy( name->sysname, "eCos" );
    strcpy( name->nodename, "" );       // should use gethostname()
    strcpy( name->release, __xstring( CYGNUM_KERNEL_VERSION_MAJOR ) );
    strcpy( name->version, __xstring( CYGNUM_KERNEL_VERSION_MINOR ) );
    strcpy( name->machine, "" );

    CYG_REPORT_RETVAL(0);
    return 0;
}

// -------------------------------------------------------------------------
// sysconf()

#define SC_CASE( _name, _val ) case _name: return _val

__externC long sysconf( int name )
{

    switch( name )
    {
        SC_CASE( _SC_AIO_LISTIO_MAX,                    AIO_LISTIO_MAX );
        SC_CASE( _SC_AIO_MAX,                           AIO_MAX );
        SC_CASE( _SC_AIO_PRIO_DELTA_MAX,                AIO_PRIO_DELTA_MAX );
        SC_CASE( _SC_ARG_MAX,                           ARG_MAX );
        SC_CASE( _SC_CHILD_MAX,                         CHILD_MAX );
        SC_CASE( _SC_DELAYTIMER_MAX,                    DELAYTIMER_MAX );
        SC_CASE( _SC_GETGR_R_SIZE_MAX,                  0 );
        SC_CASE( _SC_GETPW_R_SIZE_MAX,                  0 );
        SC_CASE( _SC_LOGIN_NAME_MAX,                    LOGIN_NAME_MAX );
        SC_CASE( _SC_MQ_OPEN_MAX,                       MQ_OPEN_MAX );
        SC_CASE( _SC_MQ_PRIO_MAX,                       MQ_PRIO_MAX );
        SC_CASE( _SC_NGROUPS_MAX,                       NGROUPS_MAX );
        SC_CASE( _SC_OPEN_MAX,                          OPEN_MAX );
        SC_CASE( _SC_PAGESIZE,                          PAGESIZE );
        SC_CASE( _SC_RTSIG_MAX,                         RTSIG_MAX );
        SC_CASE( _SC_SEM_NSEMS_MAX,                     SEM_NSEMS_MAX );
        SC_CASE( _SC_SEM_VALUE_MAX,                     SEM_VALUE_MAX );
        SC_CASE( _SC_SIGQUEUE_MAX,                      SIGQUEUE_MAX );
        SC_CASE( _SC_STREAM_MAX,                        STREAM_MAX );
#ifdef CYGPKG_POSIX_PTHREAD
        SC_CASE( _SC_THREAD_DESTRUCTOR_ITERATIONS,      PTHREAD_DESTRUCTOR_ITERATIONS );
        SC_CASE( _SC_THREAD_KEYS_MAX,                   PTHREAD_KEYS_MAX );
        SC_CASE( _SC_THREAD_STACK_MIN,                  PTHREAD_STACK_MIN );
        SC_CASE( _SC_THREAD_THREADS_MAX,                PTHREAD_THREADS_MAX );
#endif
        SC_CASE( _SC_TIMER_MAX,                         TIMER_MAX );
        SC_CASE( _SC_TTY_NAME_MAX,                      TTY_NAME_MAX );
        SC_CASE( _SC_TZNAME_MAX,                        TZNAME_MAX );
        SC_CASE( _SC_VERSION,                           _POSIX_VERSION );

#ifdef CYGPKG_POSIX_TIMERS
    case _SC_CLK_TCK:
    {
        struct timespec ts;
        ts.tv_sec = 1;
        ts.tv_nsec = 0;
        cyg_tick_count ticks = cyg_timespec_to_ticks( &ts );
        return ticks;
    }
#endif

    case _SC_ASYNCHRONOUS_IO:
    #ifdef _POSIX_ASYNCHRONOUS_IO
        return 1;
    #else
        return -1;
    #endif
            
    case _SC_FSYNC:
    #ifdef _POSIX_FSYNC
        return 1;
    #else
        return -1;
    #endif
                
    case _SC_JOB_CONTROL:
    #ifdef _POSIX_JOB_CONTROL
        return 1;
    #else
        return -1;
    #endif
                    
    case _SC_MAPPED_FILES:
    #ifdef _POSIX_MAPPED_FILES
        return 1;
    #else
        return -1;
    #endif
                        
    case _SC_MEMLOCK:
    #ifdef _POSIX_MEMLOCK
        return 1;
    #else
        return -1;
    #endif
                            
    case _SC_MEMLOCK_RANGE:
    #ifdef _POSIX_MEMLOCK_RANGE
        return 1;
    #else
        return -1        ;
    #endif      
                                
    case _SC_MEMORY_PROTECTION:
    #ifdef _POSIX_MEMORY_PROTECTION
        return 1;
    #else
        return -1;
    #endif
            
    case _SC_MESSAGE_PASSING:
    #ifdef _POSIX_MESSAGE_PASSING
        return 1;
    #else
        return -1;
    #endif
                
    case _SC_PRIORITIZED_IO:
    #ifdef _POSIX_PRIORITIZED_IO
        return 1;
    #else
        return -1;
    #endif
                    
    case _SC_PRIORITY_SCHEDULING:
    #ifdef _POSIX_PRIORITY_SCHEDULING
        return 1;
    #else
        return -1;
    #endif
                        
    case _SC_REALTIME_SIGNALS:
    #ifdef _POSIX_REALTIME_SIGNALS
        return 1;
    #else
        return -1;
    #endif
                            
    case _SC_SAVED_IDS:
    #ifdef _POSIX_SAVED_IDS
        return 1;
    #else
        return -1;
    #endif
                                
    case _SC_SEMAPHORES:
    #ifdef _POSIX_SEMAPHORES
        return 1;
    #else
        return -1;
    #endif
                                    
    case _SC_SHARED_MEMORY_OBJECTS:
    #ifdef _POSIX_SHARED_MEMORY_OBJECTS
        return 1;
    #else
        return -1;
    #endif
                                        
    case _SC_SYNCHRONIZED_IO:
    #ifdef _POSIX_SYNCHRONIZED_IO
        return 1;
    #else
        return -1;
    #endif
                                            
    case _SC_THREADS:
    #ifdef _POSIX_THREADS
        return 1;
    #else
        return -1;
    #endif
                                                
    case _SC_THREAD_ATTR_STACKADDR:
    #ifdef _POSIX_THREAD_ATTR_STACKADDR
        return 1;
    #else
        return -1;
    #endif
                                                    
    case _SC_THREAD_ATTR_STACKSIZE:
    #ifdef _POSIX_THREAD_ATTR_STACKSIZE
        return 1;
    #else
        return -1;
    #endif
                                                        
    case _SC_THREAD_PRIO_INHERIT:
    #ifdef _POSIX_THREAD_PRIO_INHERIT
        return 1;
    #else
        return -1;
    #endif
                                                            
    case _SC_THREAD_PRIO_PROTECT:
    #ifdef _POSIX_THREAD_PRIO_PROTECT
        return 1;
    #else
        return -1;
    #endif
                                                                
    case _SC_THREAD_PRIORITY_SCHEDULING:
    #ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
        return 1;
    #else
        return -1;
    #endif
                                                                    
    case _SC_THREAD_PROCESS_SHARED:
    #ifdef _POSIX_THREAD_PROCESS_SHARED
        return 1;
    #else
        return -1;
    #endif
                                                                        
    case _SC_THREAD_SAFE_FUNCTIONS:
    #ifdef _POSIX_THREAD_SAFE_FUNCTIONS
        return 1;
    #else
        return -1;
    #endif
                                                                            
    case _SC_TIMERS:
    #ifdef _POSIX_TIMERS
        return 1;
    #else
        return -1;
    #endif
                                                                                

    default:
        errno = EINVAL;
        return -1;
    }
}

//==========================================================================
// Some trivial compatibility functions.
// These are merely present to permit existing code to be ported a little
// more easily, and to provide adequate standards compatibility.

__externC pid_t getpid    ( void ) { return 42; }
__externC pid_t getppid   ( void ) { return 41; }
__externC uid_t getuid    ( void ) { return 666; }
__externC uid_t geteuid   ( void ) { return 666; }
__externC gid_t getgid    ( void ) { return 88; }
__externC gid_t getegid   ( void ) { return 88; }
__externC int   setuid    ( uid_t uid ) { errno = EPERM; return -1; }
__externC int   setgid    ( uid_t gid ) { errno = EPERM; return -1; }
__externC int   getgroups ( int gidsetsize, gid_t grouplist[] ) { return 0; };
__externC pid_t getpgrp   ( void ) { return 42; }
__externC pid_t setsid    ( void ) { errno = EPERM; return -1; }
__externC int   setpgid   ( pid_t pid, pid_t pgid ) { errno = ENOSYS; return -1; }

//==========================================================================
// Exports to other packages

// -------------------------------------------------------------------------
// POSIX API function entry

__externC void cyg_posix_function_start()
{
    Cyg_Thread *self = Cyg_Scheduler::get_current_thread();

    // Inhibit ASR delivery in this function until it returns.
    
    self->set_asr_inhibit();
}

// -------------------------------------------------------------------------

__externC void cyg_posix_function_finish()
{
    Cyg_Thread *self = Cyg_Scheduler::get_current_thread();

    // Re-allow ASR delivery.
    
    self->clear_asr_inhibit();

    // After clearing the inhibit flag, blip the scheduler lock
    // to get any pending ASRs delivered.
    Cyg_Scheduler::lock();
    Cyg_Scheduler::unlock();
}

// -------------------------------------------------------------------------
// EOF misc.cxx
