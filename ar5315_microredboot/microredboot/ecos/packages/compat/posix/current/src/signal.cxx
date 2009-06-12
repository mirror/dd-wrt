//==========================================================================
//
//      signal.cxx
//
//      POSIX signal functions implementation
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
// Date:                2000-03-27
// Purpose:             POSIX signal functions implementation
// Description:         This file contains the implementation of the POSIX signal
//                      functions.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/posix.h>

#ifdef CYGPKG_POSIX_SIGNALS

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/isoinfra.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include "pprivate.h"                   // POSIX private header

#include <signal.h>                     // our header
#include <setjmp.h>
#include <unistd.h>                     // _exit

#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/clock.inl>
#include <cyg/kernel/thread.inl>

// -------------------------------------------------------------------------
// Internal definitions

// Handle entry to a signal package function. 
#define SIGNAL_ENTRY() CYG_REPORT_FUNCTYPE( "returning %d" );

// Do a signal package defined return. This requires the error code
// to be placed in errno, and if it is non-zero, -1 returned as the
// result of the function. This also gives us a place to put any
// generic tidyup handling needed for things like signal delivery and
// cancellation.
#define SIGNAL_RETURN(err)                      \
CYG_MACRO_START                                 \
    int __retval = 0;                           \
    if( err != 0 ) __retval = -1, errno = err;  \
    CYG_REPORT_RETVAL( __retval );              \
    return __retval;                            \
CYG_MACRO_END

// Similarly for functions that have valid non-zero returns
#define SIGNAL_RETURN_VALUE(val)                \
CYG_MACRO_START                                 \
    CYG_REPORT_RETVAL( val );                   \
    return val;                                 \
CYG_MACRO_END

// Range check on a signal value.
#define SIGNAL_VALID(_sig_) (((_sig_) > 0) && ((_sig_) < ((int)sizeof(sigset_t)*8)))

//==========================================================================
// Signal management structures

typedef struct signal_info
{
    struct signal_info          *next;  // link in list of pending signals
    siginfo_t                   si;     // siginfo to pass to handler
} signal_info;

typedef struct
{
    struct sigaction            sa;     // Sigaction defining what to do
    signal_info                 *pending; // List of pending signals - this is
                                          // a circular list with pending pointing
                                          // to the tail element (or NULL if empty).
} signal_state;

//==========================================================================
// Signal management variables

// Lock used to protect signal management structures
Cyg_Mutex signal_mutex CYGBLD_POSIX_INIT;

// Condition variable for all threads in sigsuspend() and sigwait()
// to wait on.
Cyg_Condition_Variable CYGBLD_POSIX_INIT signal_sigwait( signal_mutex ) ;

// Global pending signal set
sigset_t sig_pending;

// Array controlling signal states
static signal_state sigstate[sizeof(sigset_t)*8];

// Array of available signal_info objects for queueing signals
static signal_info siginfo[SIGQUEUE_MAX];

// List of free signal_info objects
static signal_info *siginfo_next = NULL;

//==========================================================================
// Variables used to support alarm()

// Forward def of action function
static void sigalrm_action( Cyg_Alarm *alarm, CYG_ADDRWORD data );

// Kernel alarm object
static Cyg_Alarm CYGBLD_POSIX_INIT sigalrm_alarm( Cyg_Clock::real_time_clock, sigalrm_action, 0 ) ;

// Set true when alarm is armed
volatile cyg_bool sigalrm_armed = false;

// Set true when alarm has fired and is waiting to be delivered
volatile cyg_bool sigalrm_pending = false;

//==========================================================================
// Implementation functions.
// These are where the real work of the signal mechanism gets done.

externC void cyg_posix_signal_start()
{
    // Chain all free signal_info objects together
    for( int i = 0; i < SIGQUEUE_MAX; i++ )
    {
        siginfo[i].next = siginfo_next;
        siginfo_next = &siginfo[i];
    }
    
    // initialize all signal actions to SIG_DFL
    for ( unsigned int i=0; i<(sizeof(sigstate)/sizeof(signal_state)); i++ )
    {
        sigstate[i].sa.sa_handler = SIG_DFL;
    }

    // Clear the pending signal set
    sigemptyset( &sig_pending );
}

// -------------------------------------------------------------------------
// Generate a signal

cyg_bool cyg_sigqueue( const struct sigevent *sev, int code,
                       pthread_info *thread )
{
    if( sev->sigev_notify == SIGEV_NONE )
    {
        // Do nothing
        return true;
    }

    if( sev->sigev_notify == SIGEV_THREAD )
    {
        // create a thread to run the notification
        // function.
        // FIXME: implement SIGEV_THREAD
        return true;
    }

    // Otherwise we must have a SIGEV_SIGNAL notification

    // Find out whether the current thread already has the mutex
    // locked. This is a distinct possibility if this function is
    // called from the ASR while exiting the signal_sigwait condvar in
    // pause() and sigtimedwait().
    
    pthread_info *self = pthread_self_info();
    cyg_bool locked = (self != NULL) && (signal_mutex.get_owner() == self->thread);
    
    // Lock the mutex only if we do not already own it
    if( !locked ) signal_mutex.lock();
    
    int signo = sev->sigev_signo;
    signal_state *ss = &sigstate[signo];

    if( ss->sa.sa_flags & SA_SIGINFO )
    {
        // We have a queuable signal, allocate a signal_info
        // object and add it to the queue.

        if( siginfo_next == NULL )
        {
            if( !locked ) signal_mutex.unlock();
            return false;
        }

        signal_info *si = siginfo_next;
        siginfo_next = si->next;

        si->si.si_signo = signo;
        si->si.si_code = code;
        si->si.si_value = sev->sigev_value;

        if( ss->pending == NULL )
        {
            si->next = si;
        }
        else
        {
            si->next = ss->pending->next;
            ss->pending->next = si;
        }
            
        ss->pending = si;
    }
    // else A non-queuable signal, just set it pending

    if( thread != NULL )
    {
        sigaddset( &thread->sigpending, signo );
        // just wake the thread up now if it's blocked somewhere
        if ((thread->sigpending & ~thread->sigmask) != 0)
        {
            thread->thread->set_asr_pending();
            thread->thread->release();
        }
    }
    else
    {
        sigaddset( &sig_pending, signo );
        // Wake up any threads in sigsuspend() and sigwait().
        if (!signal_sigwait.get_queue()->empty())
        {
            signal_sigwait.broadcast();
        } 
        else
        {
            cyg_posix_pthread_release_thread( &sig_pending );
        }
    }

    if( !locked ) signal_mutex.unlock();
    
    return true;
}

// -------------------------------------------------------------------------
// Deliver any pending unblocked signals to the current thread
// Returns true if a signal handler was called.

cyg_bool cyg_deliver_signals()
{
    cyg_bool res = false;
    
    pthread_info *self = pthread_self_info();

    // If there is no pthread_info pointer for this thread then
    // it is not a POSIX thread and cannot have signals delivered
    // to it.
    
    if( self == NULL ) return false;
    
    // If there are no pending signals our work is done
    if( sig_pending == 0 && self->sigpending == 0 )
        return false;

    // If there are no unmasked pending signals our
    // work is also done
    if( ((sig_pending | self->sigpending) & ~self->sigmask) == 0 )
        return false;

    // As with cyg_sigqueue(), this function can get called from an
    // ASR where the signal_mutex is already locked. Check here to
    // avoid relocking...
    
    cyg_bool locked = signal_mutex.get_owner() == self->thread;
    
    if( !locked ) signal_mutex.lock();
        
    sigset_t todo;

    // Since a signal handler may raise another signal, or unmask an existing
    // signal, we loop here while there are no more unblocked signals pending.
    while( (todo = ((sig_pending | self->sigpending) & ~self->sigmask)) != 0 )
    {
        // Here todo is a mask of the signals available for delivery
        
        int signo = 0;

        // This prioritizes low numbered signals
        HAL_LSBIT_INDEX( signo, todo );

        signal_state *ss = &sigstate[signo];
        sigset_t sigbit = 1L<<signo;

        if( ss->sa.sa_handler != SIG_IGN )
        {
            sigset_t oldmask = self->sigmask;
            siginfo_t lsi;

            if(ss->pending != NULL)
            {
                // There is a queued signal. Dequeue it and copy the
                // siginfo object to a local copy.
                
                signal_info *si = ss->pending->next;
                    
                // Make a local copy of the siginfo object
                lsi = si->si;
                    
                // Remove the head signal_info object from the
                // circular list. 
                if( ss->pending == si )
                    ss->pending = NULL;
                else
                    ss->pending->next = si->next;

                // Return it to the free list
                si->next = siginfo_next;
                siginfo_next = si;
            }
            else
            {
                // There are no signals queued. Set up the local siginfo_t
                // object with default values. 

                lsi.si_signo = signo;
                lsi.si_code = SI_USER;
                lsi.si_value.sival_int = 0;
            }
            
            // Clear the bit from the pending masks. If the pending
            // queue is not empty, leave the bits set, otherwise clear
            // them. Do this now so that if the signal handler longjumps
            // out, the signal subsystem is clean.
        
            if( ss->pending == NULL )
            {
                // Clear the bit in both masks regardless of which
                // one it actually came from. This is cheaper than
                // trying to find out.
                sig_pending &= ~sigbit;
                self->sigpending &= ~sigbit;
            }

            // Add the mask set and the signal itself to the
            // mask while we call the signal handler
            self->sigmask = oldmask | ss->sa.sa_mask | sigbit;
            
            // Unlock now so that a longjmp out of the handler
            // does the right thing. We do this even if we did not
            // lock the mutex since it will only recently have been
            // relocked and thus all data is still consistent.
                
            signal_mutex.unlock();
            
            if( ss->sa.sa_flags & SA_SIGINFO )
            {
                // A sigaction delivery
                CYG_CHECK_FUNC_PTR( ss->sa.sa_sigaction,
                                    "Bad sa_sigaction signal handler" );
                ss->sa.sa_sigaction( signo, &lsi, NULL );
            }
            else if ( ss->sa.sa_handler == SIG_DFL )
            {
                CYG_TRACE2( true,
                            "Unhandled POSIX signal: sig=%d, mask=%08x",
                            signo, oldmask );

                // FIXME: should do something better here
#if CYGINT_ISO_EXIT
                _exit( -signo );
#endif
                CYG_FAIL("Unhandled POSIX signal");
            }            
            else
            {
                // This is a standard signal delivery.
                CYG_CHECK_FUNC_PTR( ss->sa.sa_handler,
                                    "Bad sa_handler signal handler" );

                ss->sa.sa_handler( signo );
            }

            // Relock the mutex 
            signal_mutex.lock();                

            // Restore original signal mask
            self->sigmask = oldmask;

            // return that we have handled a signal
            res = true;
        }
    }

    if( !locked ) signal_mutex.unlock();
    
    return res;
}

// -------------------------------------------------------------------------
// Utility routine to signal any threads waiting in sigwait*().

void cyg_posix_signal_sigwait()
{
    signal_sigwait.broadcast();
}       

// -------------------------------------------------------------------------
// Action routine called from kernel alarm to deliver the SIGALRM signal.
// We cannot call any signal delivery functions directly here, so we simply
// set a flag and schedule an ASR to be called.

static void sigalrm_action( Cyg_Alarm *alarm, CYG_ADDRWORD data )
{
    sigset_t mask;
    sigalrm_armed = false;
    sigalrm_pending = true;
    sigemptyset( &mask );
    sigaddset( &mask, SIGALRM );
    // Wake up any threads in sigsuspend() and sigwait() in case they
    // are waiting for an alarm, and would have SIGALRM masked
    signal_sigwait.broadcast();
    
    cyg_posix_pthread_release_thread( &mask );
}

// -------------------------------------------------------------------------
// Check for SIGALRMs. This is called from the ASR and sigtimedwait()
// as alarms need to be handled as a special case.

static __inline__ void check_sigalarm(void)
{
    // If there is a pending SIGALRM, generate it
    if( sigalrm_pending )
    {
        sigalrm_pending = false;
        
        struct sigevent sev;

        sev.sigev_notify           = SIGEV_SIGNAL;
        sev.sigev_signo            = SIGALRM;
        sev.sigev_value.sival_int  = 0;

        // generate the signal
        cyg_sigqueue( &sev, SI_USER );
    }
}

// -------------------------------------------------------------------------
// signal ASR function. This is called from the general POSIX ASR to
// deal with any signal related issues.

externC void cyg_posix_signal_asr(pthread_info *self)
{
    check_sigalarm();

    // Now call cyg_deliver_signals() to see if we can
    // handle any signals now.
    
    cyg_deliver_signals();
}

//==========================================================================
// Per-thread initialization and destruction

externC void cyg_posix_thread_siginit( pthread_info *thread,
                                       pthread_info *parentthread )
{
    // Clear out signal masks
    sigemptyset( &thread->sigpending );
    // but threads inherit signal masks
    if ( NULL == parentthread )
        sigemptyset( &thread->sigmask );
    else
        thread->sigmask = parentthread->sigmask;
    
    cyg_pthread_exception_init( thread );
}

externC void cyg_posix_thread_sigdestroy( pthread_info *thread )
{
    cyg_pthread_exception_destroy( thread );
}

//==========================================================================
// Functions to generate signals

// -------------------------------------------------------------------------
// Deliver sig to a process.
// eCos only supports the value 0 for pid.

externC int kill (pid_t pid, int sig)
{
    SIGNAL_ENTRY();

    if( !SIGNAL_VALID(sig) )
        SIGNAL_RETURN(EINVAL);
    
    if( pid != 0 )
        SIGNAL_RETURN(ESRCH);

    struct sigevent sev;

    sev.sigev_notify           = SIGEV_SIGNAL;
    sev.sigev_signo            = sig;
    sev.sigev_value.sival_int  = 0;
    
    cyg_sigqueue( &sev, SI_USER );

    cyg_deliver_signals();
    
    SIGNAL_RETURN(0);
}

// -------------------------------------------------------------------------

externC int pthread_kill (pthread_t threadid, int sig)
{
    SIGNAL_ENTRY();

    if( !SIGNAL_VALID(sig) )
        SIGNAL_RETURN(EINVAL);
    
    struct sigevent sev;

    pthread_info *thread = pthread_info_id(threadid);

    if( thread == NULL )
        SIGNAL_RETURN(ESRCH);
    
    sev.sigev_notify           = SIGEV_SIGNAL;
    sev.sigev_signo            = sig;
    sev.sigev_value.sival_int  = 0;
    
    cyg_sigqueue( &sev, SI_USER, thread );

    cyg_deliver_signals();
    
    SIGNAL_RETURN(0);
}

//==========================================================================
// Functions to catch signals

// -------------------------------------------------------------------------
// Install signal handler for sig.

externC int sigaction  (int sig, const struct sigaction *act,
                        struct sigaction *oact)
{
    SIGNAL_ENTRY();

    if( !SIGNAL_VALID(sig) )
        SIGNAL_RETURN(EINVAL);
    
    signal_state *ss = &sigstate[sig];
    
    signal_mutex.lock();

    if( oact != NULL )
        *oact = ss->sa;

    ss->sa = *act;

    if( ss->sa.sa_handler == SIG_IGN )
    {
        // Setting the handler to SIG_IGN causes any pending
        // signals to be discarded and any queued values to also
        // be removed.

        pthread_info *self = pthread_self_info();
        sigset_t sigbit = 1<<sig;

        if( (sig_pending | self->sigpending) & sigbit )
        {
            // This signal is pending, clear it

            sig_pending &= ~sigbit;
            self->sigpending &= ~sigbit;

            // Clean out any queued signal_info objects
            while( ss->pending != NULL )
            {
                signal_info *si = ss->pending->next;
                
                // Remove the head signal_info object from the
                // circular list. 
                if( ss->pending == si )
                    ss->pending = NULL;
                else
                    ss->pending->next = si->next;

                // Return it to the free list
                si->next = siginfo_next;
                siginfo_next = si;
            }
        }
    }
    
    cyg_deliver_signals();
    
    signal_mutex.unlock();
    
    SIGNAL_RETURN(0);
}
    

// -------------------------------------------------------------------------
// Queue signal to process with value.

externC int sigqueue (pid_t pid, int sig, const union sigval value)
{
    SIGNAL_ENTRY();

    if( !SIGNAL_VALID(sig) )
        SIGNAL_RETURN(EINVAL);
    
    struct sigevent sev;

    sev.sigev_notify   = SIGEV_SIGNAL;
    sev.sigev_signo    = sig;
    sev.sigev_value    = value;
    
    cyg_sigqueue( &sev, SI_QUEUE );

    cyg_deliver_signals();
    
    SIGNAL_RETURN(0);
}
    
//==========================================================================
// Functions to deal with current blocked and pending masks

// -------------------------------------------------------------------------
// Set process blocked signal mask
// Map this onto pthread_sigmask().

externC int sigprocmask  (int how, const sigset_t *set, sigset_t *oset)
{
    return pthread_sigmask( how, set, oset);
}
    

// -------------------------------------------------------------------------
// Set calling thread's blocked signal mask

externC int pthread_sigmask (int how, const sigset_t *set, sigset_t *oset)
{
    int err = 0;
    
    SIGNAL_ENTRY();

    pthread_info *self = pthread_self_info();
    // Save old set
    if( oset != NULL )
        *oset = self->sigmask;

    if( set != NULL )
    {
        switch( how )
        {
        case SIG_BLOCK:
            self->sigmask |= *set;
            break;
        
        case SIG_UNBLOCK:
            self->sigmask &= ~*set;
            break;
            
        case SIG_SETMASK:
            self->sigmask = *set;
            break;

        default:
            err = EINVAL;
            break;
        }
    }

    // Deliver any newly unblocked signals
    cyg_deliver_signals();
    
    SIGNAL_RETURN(err);
}

// -------------------------------------------------------------------------
// Exported routine to set calling thread's blocked signal mask
//
// Optionally set and return the current thread's signal mask. This is
// exported to other packages so that they can manipulate the signal
// mask without necessarily having them delivered (as calling
// pthread_sigmask() would). Signals can be delivered by calling
// cyg_posix_deliver_signals().

externC void cyg_pthread_sigmask_set (const sigset_t *set, sigset_t *oset)
{
    pthread_info *self = pthread_self_info();

    if( self != NULL )
    {
        if( oset != NULL )
            *oset = self->sigmask;

        if( set != NULL )
            self->sigmask = *set;
    }
}

// -------------------------------------------------------------------------
// Exported routine to test for any pending signals.
//
// This routine tests for any pending undelivered, unmasked
// signals. If there are any it returns true.  This is exported to
// other packages, such as FILEIO, so that they can detect whether to
// abort a current API call with an EINTR result.

externC cyg_bool cyg_posix_sigpending(void)
{
    pthread_info *self = pthread_self_info();

    if( self == NULL )
        return false;
    
    return ( ((sig_pending | self->sigpending) & ~self->sigmask) != 0 );
}

// -------------------------------------------------------------------------
// Exported routine to deliver selected signals
//
// This routine optionally sets the given mask and then tries to
// deliver any pending signals that have been unmasked. This is
// exported to other packages so that they can cause signals to be
// delivered at controlled points during execution.

externC void cyg_posix_deliver_signals( const sigset_t *mask )
{
    sigset_t oldmask;
    pthread_info *self = pthread_self_info();

    if( self != NULL )
    {
        if( mask != NULL )
        {
            oldmask = self->sigmask;
            self->sigmask = *mask;
        }
        else
            oldmask = 0;   // silence warning

        cyg_deliver_signals();

        if( mask != NULL )        
            self->sigmask = oldmask;
    }
}

// -------------------------------------------------------------------------
// Get set of pending signals for this process

externC int sigpending  (sigset_t *set)
{
    SIGNAL_ENTRY();

    if( set == NULL )
        SIGNAL_RETURN(EINVAL);
    
    pthread_info *self = pthread_self_info();
    
    *set = self->sigpending | sig_pending;
    
    SIGNAL_RETURN(0);
}
    

//==========================================================================
// Wait for or accept signals

// -------------------------------------------------------------------------
// Block signals in set and wait for a signal

externC int sigsuspend  (const sigset_t *set)
{
    SIGNAL_ENTRY();

    pthread_info *self = pthread_self_info();

    signal_mutex.lock();

    // Save the old mask and set the current mask to
    // the one supplied.
    sigset_t old = self->sigmask;
    self->sigmask = *set;

    // Loop until a signal gets delivered
    while( !cyg_deliver_signals() )
        signal_sigwait.wait();

    self->sigmask = old;
    
    signal_mutex.unlock();
    
    SIGNAL_RETURN(EINTR);
}
    

// -------------------------------------------------------------------------
// Wait for a signal in set to arrive
// Implement this as a variant on sigtimedwait().

externC int sigwait  (const sigset_t *set, int *sig)
{
    SIGNAL_ENTRY();

    siginfo_t info;
    
    int ret = sigtimedwait( set, &info, NULL );

    if( ret == -1 )
        SIGNAL_RETURN(errno);

    *sig = ret;
    
    SIGNAL_RETURN(0);
}

// -------------------------------------------------------------------------
// Do the same as sigwait() except return a siginfo_t object too.
// Implement this as a variant on sigtimedwait().

externC int sigwaitinfo  (const sigset_t *set, siginfo_t *info)
{
    SIGNAL_ENTRY();

    int ret = sigtimedwait( set, info, NULL );
    
    SIGNAL_RETURN_VALUE(ret);
}

// -------------------------------------------------------------------------
// Wait either for a signal in the given set to become pending, or
// for the timeout to expire. If timeout is NULL, wait for ever.

externC int sigtimedwait  (const sigset_t *set, siginfo_t *info,
                           const struct timespec *timeout)
{
    SIGNAL_ENTRY();

    // check for cancellation first.
    pthread_testcancel();

    int err = 0;
    cyg_tick_count ticks;

    if( timeout == NULL ) ticks = 0;
    else ticks = cyg_timespec_to_ticks( timeout ) +
             Cyg_Clock::real_time_clock->current_value();

    pthread_info *self = pthread_self_info();
    
    signal_mutex.lock();

    sigset_t todo;

    // Wait for a signal in the set to become pending
    while( (todo = (*set & (sig_pending | self->sigpending))) == 0 )
    {
        // If timeout is not NULL, do a timed wait on the
        // sigwait condition variable. If it is NULL - wait
        // until we are woken.
        if( timeout )
        {
            if( ticks == 0 || !signal_sigwait.wait(ticks) )
            {
                // If the timeout is actually zero, or we have waited and
                // timed out, then we must quit with an error.
                err = EAGAIN;
                break;
            }
        }
        else {
            if ( !signal_sigwait.wait() ) {
                // check we weren't woken up forcibly (e.g. to be cancelled)
                // if so, pretend it's an error
                err = EAGAIN;
                break;
            }
        }
        
        // Special case check for SIGALRM since the fact SIGALRM is masked
        // would have prevented it being set pending in the alarm handler.
        check_sigalarm();

        cyg_posix_timer_asr(self);
    }

    if( err == 0 )
    {
        // There is a signal in the set that is pending: deliver
        // it. todo contains a mask of all the signals that could be
        // delivered now, but we only want to deliver one of them.

        int signo = 0;

        // Select the lowest numbered signal from the todo mask
        HAL_LSBIT_INDEX( signo, todo );

        signal_state *ss = &sigstate[signo];
        sigset_t sigbit = 1L<<signo;

        if( (ss->sa.sa_flags & SA_SIGINFO) && (ss->pending != NULL) )
        {
            // If the SA_SIGINFO bit is set, then there
            // will be a signal_info object queued on the
            // pending field.

            signal_info *si = ss->pending->next;
            *info = si->si;

            // Remove the head signal_info object from the
            // circular list. 
            if( ss->pending == si )
                ss->pending = NULL;
            else
                ss->pending->next = si->next;
                
            si->next = siginfo_next;
            siginfo_next = si;
                
        }
        else
        {
            // Not a queued signal, or there is no signal_info object
            // on the pending queue: fill in info structure with
            // default values.
            info->si_signo           = signo;
            info->si_code            = SI_USER;
            info->si_value.sival_int = 0;
        }

        // Clear the bit from the pending masks. If the pending
        // queue is not empty, leave the bits set, otherwise clear
        // them.
        
        if( ss->pending == NULL )
        {
            // Clear the bit in both masks regardless of which
            // one it actually came from. This is cheaper than
            // trying to find out.
            sig_pending &= ~sigbit;
            self->sigpending &= ~sigbit;
        }

        // all done
    }
    
    signal_mutex.unlock();

    pthread_testcancel();

    if (err)
        SIGNAL_RETURN(err);
    else
        SIGNAL_RETURN_VALUE( info->si_signo );
}

//==========================================================================
// alarm, pause and sleep

// -------------------------------------------------------------------------
// Generate SIGALRM after some number of seconds

externC unsigned int alarm( unsigned int seconds )
{
    int res = 0;
    struct timespec tv;
    cyg_tick_count trigger, interval;

    SIGNAL_ENTRY();

    signal_mutex.lock();

    if( sigalrm_armed )
    {
        sigalrm_alarm.disable();

        sigalrm_alarm.get_times( &trigger, &interval );

        // Convert trigger time back to interval
        trigger -= Cyg_Clock::real_time_clock->current_value();
        
        cyg_ticks_to_timespec( trigger, &tv );

        res = tv.tv_sec;
        
        sigalrm_armed = false;
    }

    if( seconds != 0 )
    {
        // Here we know that the sigalrm_alarm is unarmed, set it up
        // to trigger in the required number of seconds.

        tv.tv_sec = seconds;
        tv.tv_nsec = 0;

        trigger = cyg_timespec_to_ticks( &tv );

        // Convert trigger interval to absolute time
        trigger += Cyg_Clock::real_time_clock->current_value();
        
        sigalrm_alarm.initialize( trigger, 0 );

        sigalrm_armed = true;
    }
    
    signal_mutex.unlock();

    CYG_REPORT_RETVAL(res);
    
    return res;
}

// -------------------------------------------------------------------------
// Wait for a signal to be delivered.

externC int pause( void )
{
    SIGNAL_ENTRY();

    signal_mutex.lock();

    // Check for any pending signals that can be delivered and
    // if there are none, wait for a signal to be generated
    if( !cyg_deliver_signals() )
        signal_sigwait.wait();

    // Now check again for some signals to deliver
    cyg_deliver_signals();
    
    signal_mutex.unlock();
    
    SIGNAL_RETURN(EINTR);
}

//==========================================================================
// Signal sets

// -------------------------------------------------------------------------
// Clear all signals from set.

externC int sigemptyset  (sigset_t *set)
{
    SIGNAL_ENTRY();

    *set = 0;
    
    SIGNAL_RETURN(0);
}
    

// -------------------------------------------------------------------------
// Set all signals in set.

externC int sigfillset  (sigset_t *set)
{
    SIGNAL_ENTRY();

    *set = ~0;
    
    SIGNAL_RETURN(0);
}
    

// -------------------------------------------------------------------------
// Add signo to set.

externC int sigaddset  (sigset_t *set, int signo)
{
    SIGNAL_ENTRY();

    int err = 0;
    
    if( !SIGNAL_VALID(signo) )
        err = EINVAL;
    else *set |= 1<<signo;
    
    SIGNAL_RETURN(err);
}
    

// -------------------------------------------------------------------------
// Remove signo from set.

externC int sigdelset  (sigset_t *set, int signo)
{
    SIGNAL_ENTRY();

    int err = 0;
    
    if( !SIGNAL_VALID(signo) )
        err = EINVAL;
    else *set &= ~(1<<signo);
    
    SIGNAL_RETURN(err);
}
    

// -------------------------------------------------------------------------
// Test whether signo is in set

externC int sigismember  (const sigset_t *set, int signo)
{
    SIGNAL_ENTRY();

    int ret = 0;
    
    if( !SIGNAL_VALID(signo) )
        SIGNAL_RETURN(EINVAL);

    if( *set & (1<<signo) ) ret = 1;

    CYG_REPORT_RETVAL( ret );
    return ret;
}

//==========================================================================
// ISO C compatibility functions

// -------------------------------------------------------------------------
// Installs a new signal handler for the specified signal, and returns
// the old handler

externC sa_sighandler_t signal(int sig, sa_sighandler_t handler)
{
    SIGNAL_ENTRY();

    int err;
    sa_sighandler_t ret;
    struct sigaction new_action;
    struct sigaction old_action;

    sigemptyset( &new_action.sa_mask );
    new_action.sa_flags = 0;
    new_action.sa_handler = handler;

    err = sigaction( sig, &new_action, &old_action );

    if( err < 0 )
        ret = SIG_ERR;
    else ret = old_action.sa_handler;
    
    CYG_REPORT_RETVAL( ret );
    return ret;    
}

// -------------------------------------------------------------------------
// raise() - ISO C 7.7.2 //
//
// Raises the signal, which will cause the current signal handler for
// that signal to be called

externC int raise(int sig)
{
    return kill( 0, sig );
}

// -------------------------------------------------------------------------
// siglongjmp()
// Restores signal mask and longjumps.

__externC void siglongjmp( sigjmp_buf env, int val )
{
    CYG_REPORT_FUNCNAME( "siglongjmp" );
    CYG_REPORT_FUNCARG2( "&env=%08x, val=%d", &env, val );

    // ISO C says that if we are passed val == 0, then we change it to 1
    if( val == 0 )
        val = 1;

    if( env[0].__savemask )
        pthread_sigmask( SIG_SETMASK, &env[0].__sigsavemask, NULL );
    
    HAL_REORDER_BARRIER(); // prevent any chance of optimisation re-ordering
    hal_longjmp( env[0].__jmp_buf, val );
    HAL_REORDER_BARRIER(); // prevent any chance of optimisation re-ordering

#ifdef CYGDBG_USE_ASSERTS
    CYG_ASSERT( 0, "siglongjmp should not have reached this point!" );
#else
    for (;;)
        CYG_EMPTY_STATEMENT;
#endif
    
}

#endif // ifdef CYGPKG_POSIX_SIGNALS

// -------------------------------------------------------------------------
// EOF signal.cxx
