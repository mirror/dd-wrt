//==========================================================================
//
//      select.cxx
//
//      Fileio select() support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Nick Garnett
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
// Date:                2000-05-25
// Purpose:             Fileio select() support
// Description:         Support for select().
//                      
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <stdarg.h>                    // for fcntl()

#include "fio.h"                       // Private header

#include <sys/select.h>                // select header

#include <cyg/kernel/sched.hxx>        // scheduler definitions
#include <cyg/kernel/thread.hxx>       // thread definitions
#include <cyg/kernel/mutex.hxx>        // mutex definitions
#include <cyg/kernel/clock.hxx>        // clock definitions

#include <cyg/kernel/sched.inl>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/clock.inl>

//==========================================================================
// File object locking

#define LOCK_FILE( fp ) cyg_file_lock( fp )

#define UNLOCK_FILE( fp ) cyg_file_unlock( fp )

//==========================================================================
// Local variables

// Mutex for serializing select processing. This essntially controls
// access to the contents of the selinfo structures embedded in the
// client system data structures.
static Cyg_Mutex select_mutex CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS);

// Condition variable where any thread that is waiting for a select to
// fire is suspended. Note that select is not intended to be a real time
// operation. Whenever any selectable event occurs, all selecting threads
// will be resumed. They must then rescan their selectees and resuspend if
// necessary.
static Cyg_Condition_Variable CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_IO_FS) selwait( select_mutex ) ;

static volatile cyg_uint32 selwake_count = 0;

//==========================================================================
// Timeval to ticks conversion support

// Converters from sec and us to ticks
static struct Cyg_Clock::converter us_converter, sec_converter;

static cyg_bool converters_initialized = false;

externC cyg_tick_count cyg_timeval_to_ticks( const struct timeval *tv )
{
    if( !converters_initialized )
    {
        // Create the converters we need.
        Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1000, &us_converter );
        Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1000000000, &sec_converter );

        converters_initialized = true;
    }
    
    // Short circuit zero timeval
    if( tv->tv_sec == 0 && tv->tv_usec == 0 )
    {
        return 0;
    }
        
    // Convert the seconds field to ticks.
    cyg_tick_count ticks = Cyg_Clock::convert( tv->tv_sec, &sec_converter );

    // Convert the nanoseconds. This will round down to nearest whole tick.
    ticks += Cyg_Clock::convert( (cyg_tick_count)tv->tv_usec, &us_converter );

    return ticks;
}

//==========================================================================
// Select API function

static int
cyg_pselect(int nfd, fd_set *in, fd_set *out, fd_set *ex,
           struct timeval *tv, const sigset_t *mask)
{
    FILEIO_ENTRY();

    int error = ENOERR;
    int fd, mode, num;
    cyg_file *fp;
    fd_set in_res, out_res, ex_res;  // Result sets
    fd_set *selection[3], *result[3];
    cyg_tick_count ticks;
    int mode_type[] = {CYG_FREAD, CYG_FWRITE, 0};
    cyg_uint32 wake_count;
    sigset_t oldmask;

    FD_ZERO(&in_res);
    FD_ZERO(&out_res);
    FD_ZERO(&ex_res);

    // Set up sets
    selection[0] = in;   result[0] = &in_res;
    selection[1] = out;  result[1] = &out_res;
    selection[2] = ex;   result[2] = &ex_res;

    // Compute end time
    if (tv)
        ticks = cyg_timeval_to_ticks( tv );
    else ticks = 0;

    // Lock the mutex
    select_mutex.lock();

    // Scan sets for possible I/O until something found, timeout or error.
    while (!error)
    {
        wake_count = selwake_count;
        
        num = 0;  // Total file descriptors "ready"
        for (mode = 0;  !error && mode < 3;  mode++)
        {
            if (selection[mode]) {
                for (fd = 0;  !error && fd < nfd;  fd++)
                {
                    if (FD_ISSET(fd, selection[mode]))
                    {
                        fp = cyg_fp_get( fd );
                        if( fp == NULL )
                        {
                            error = EBADF;
                            break;
                        }

                        if ((*fp->f_ops->fo_select)(fp, mode_type[mode], 0))
                        {
                            FD_SET(fd, result[mode]);
                            num++;
                        }

                        cyg_fp_free( fp );
                    }
                }
            }
        }

        if( error )
            break;
        
        if (num)
        {
            // Found something, update user's sets
            if (in)  FD_COPY( &in_res, in );
            if (out) FD_COPY( &out_res, out );
            if (ex)  FD_COPY( &ex_res, ex );
            select_mutex.unlock();
            CYG_FILEIO_DELIVER_SIGNALS( mask );
            FILEIO_RETURN_VALUE(num);
        }

        Cyg_Scheduler::lock();

        // Switch to the supplied signal mask. This will permit delivery
        // of any signals that might terminate this select operation.
        
        CYG_FILEIO_SIGMASK_SET( mask, &oldmask );
    
        do
        {

            // We need to see if any signals have been posted while we
            // were testing all those files. The handlers will not
            // have run because we have ASRs inhibited but the signal
            // will have been set pending.

            if( CYG_FILEIO_SIGPENDING() )
            {
                // There are pending signals so we need to terminate
                // the select operation and return EINTR. Handlers for
                // the pending signals will be called just before we
                // return.

                error = EINTR;
                break;
            }
            
            if( wake_count == selwake_count )
            {
                // Nothing found, see if we want to wait
                if (tv)
                {
                    // Special case of "poll"
                    if (ticks == 0)
                    {
                        error = EAGAIN;
                        break;
                    }

                    ticks += Cyg_Clock::real_time_clock->current_value();
                
                    if( !selwait.wait( ticks ) )
                    {
                        // A non-standard wakeup, if the current time is equal to
                        // or past the timeout, return zero. Otherwise return
                        // EINTR, since we have been released.

                        if( Cyg_Clock::real_time_clock->current_value() >= ticks )
                        {
                            error = EAGAIN;
                            break;
                        }
                        else error = EINTR;
                    }

                    ticks -= Cyg_Clock::real_time_clock->current_value();
                }
                else
                {
                    // Wait forever (until something happens)
            
                    if( !selwait.wait() )
                        error = EINTR;
                }
            }

        } while(0);

        CYG_FILEIO_SIGMASK_SET( &oldmask, NULL );
        
        Cyg_Scheduler::unlock();
        
    } // while(!error)

    select_mutex.unlock();
 
    // If the error code is EAGAIN, this means that a timeout has
    // happened. We return zero in that case, rather than a proper
    // error code.
    // If the error code is EINTR, then a signal may be pending
    // delivery. Call back into the POSIX package to handle it.
    
    if( error == EAGAIN )
        FILEIO_RETURN_VALUE(0);
    else if( error == EINTR )
        CYG_FILEIO_DELIVER_SIGNALS( mask );

    FILEIO_RETURN(error);
}

// -------------------------------------------------------------------------
// Select API function

__externC int
select(int nfd, fd_set *in, fd_set *out, fd_set *ex, struct timeval *tv)
{
	return cyg_pselect(nfd, in, out, ex, tv, NULL);
}

// -------------------------------------------------------------------------
// Pselect API function
//
// This is derived from the POSIX-200X specification.

__externC int
pselect(int nfd, fd_set *in, fd_set *out, fd_set *ex,
	const struct timespec *ts, const sigset_t *sigmask)
{
	struct timeval tv;

#ifndef CYGPKG_POSIX_SIGNALS
        CYG_ASSERT( sigmask == NULL,
                    "pselect called with non-null sigmask without POSIX signal support"
                    );
#endif

	if (ts != NULL)
        {
            tv.tv_sec = ts->tv_sec;
            tv.tv_usec = ts->tv_nsec/1000;
        }

	return cyg_pselect(nfd, in, out, ex, ts ? &tv : NULL, sigmask);
}

//==========================================================================
// Select support functions.

// -------------------------------------------------------------------------
// cyg_selinit() is used to initialize a selinfo structure

void cyg_selinit( struct CYG_SELINFO_TAG *sip )
{
    sip->si_info = 0;
    sip->si_thread = 0;
}

// -------------------------------------------------------------------------
// cyg_selrecord() is called when a client device needs to register
// the current thread for selection.

void cyg_selrecord( CYG_ADDRWORD info, struct CYG_SELINFO_TAG *sip )
{
    sip->si_info = info;
    sip->si_thread = (CYG_ADDRESS)Cyg_Thread::self();
}

// -------------------------------------------------------------------------
// cyg_selwakeup() is called when the client device matches the select
// criterion, and needs to wake up a selector.

void cyg_selwakeup( struct CYG_SELINFO_TAG *sip )
{
    // We don't actually use the si_info field of selinfo at present.
    // A potential use would be to select one of several selwait condition
    // variables to signal. However, that would only be necessary if we
    // end up having lots of threads in select.

    Cyg_Scheduler::lock();
 
    if( sip->si_thread != 0 )
    {
        // If the thread pointer is still present, this selection has
        // not been fired before. We just wake up all threads waiting,
        // regardless of whether they are waiting for this event or
        // not.  This avoids any race conditions, and is consistent
        // with the behaviour of the BSD kernel.
        
        sip->si_thread = 0;
        selwait.broadcast();
        selwake_count++;

    }

    Cyg_Scheduler::unlock();    
}

// -------------------------------------------------------------------------
// EOF select.cxx
