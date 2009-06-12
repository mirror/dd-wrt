#ifndef CYGONCE_SIGNAL_H
#define CYGONCE_SIGNAL_H
//=============================================================================
//
//      signal.h
//
//      POSIX signal header
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg, jlarmour
// Contributors:  
// Date:          2000-03-17
// Purpose:       POSIX signal header
// Description:   This header contains all the definitions needed to support
//                the POSIX signal API under eCos.
//              
// Usage:         This file can either be included directly, or indirectly via
//                the C library signal.h header.
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/posix.h>

#ifdef CYGPKG_POSIX_SIGNALS

#include <stddef.h>             // NULL, size_t

#include <limits.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
// POSIX feature test macros

// We do not support job control
#undef _POSIX_JOB_CONTROL

//-----------------------------------------------------------------------------
// Manifest constants

#ifdef _POSIX_REALTIME_SIGNALS
// For now we define the topmost 8 signals as realtime
#define SIGRTMIN                24
#define SIGRTMAX                31
#endif

//-----------------------------------------------------------------------------
// forward references

struct timespec;

//-----------------------------------------------------------------------------
// Sigval structure

union sigval
{
    int   sival_int;    // used when application-defined value is an int
    void  *sival_ptr;   // used when application-defined value is a pointer
};

//-----------------------------------------------------------------------------
// Siginfo structure passed to an SA_SIGINFO style handler

typedef struct
{
    int          si_signo;      // signal number
    int          si_code;       // cause of signal
    union sigval si_value;      // signal value
} siginfo_t;

// Values for si_code
# define SI_USER	1
# define SI_QUEUE	2
# define SI_TIMER	3
# define SI_ASYNCIO	4
# define SI_MESGQ	5
# define SI_EXCEPT      6       // signal is result of an exception delivery

//-----------------------------------------------------------------------------
// Basic types

// Integral type that can be accessed atomically - from ISO C 7.7
typedef cyg_atomic sig_atomic_t;

// Type of signal handler functions
typedef void (*sa_sighandler_t)(int);

// Type of signal handler used if SA_SIGINFO is set in sa_flags
typedef void (*sa_siginfoaction_t)(int signo, siginfo_t *info,
                                  void *context);

//-----------------------------------------------------------------------------
//Signal handlers for use with signal() and sigaction(). We avoid 0
//because in an embedded system this may be start of ROM and thus
//a possible function pointer for reset.

#define SIG_DFL ((sa_sighandler_t) 1)      // Default action
#define SIG_IGN ((sa_sighandler_t) 2)      // Ignore action
#define SIG_ERR ((sa_sighandler_t)-1)      // Error return

//-----------------------------------------------------------------------------
// Signal values

#define SIGNULL   0    // Reserved signal - do not use (POSIX 3.3.1.1)
#define SIGHUP    1    // Hangup on controlling terminal (POSIX)
#define SIGINT    2    // Interactive attention (ISO C)
#define SIGQUIT   3    // Interactive termination (POSIX)
#define SIGILL    4    // Illegal instruction (not reset when caught) (ISO C)
#define SIGTRAP   5    // Trace trap (not reset when caught)
#define SIGIOT    6    // IOT instruction
#define SIGABRT   6    // Abnormal termination - used by abort() (ISO C)
#define SIGEMT    7    // EMT instruction
#define SIGFPE    8    // Floating Point Exception e.g. div by 0 (ISO C)
#define SIGKILL   9    // Kill (cannot be caught or ignored) (POSIX)
#define SIGBUS    10   // Bus error (POSIX)
#define SIGSEGV   11   // Invalid memory reference (ISO C)
#define SIGSYS    12   // Bad argument to system call (used by anything?)
#define SIGPIPE   13   // Write on a pipe with no one to read it (POSIX)
#define SIGALRM   14   // Alarm timeout (POSIX)
#define SIGTERM   15   // Software termination request (ISO C)
#define SIGUSR1   16   // Application-defined signal 1 (POSIX)
#define SIGUSR2   17   // Application-defined signal 2 (POSIX)


//-----------------------------------------------------------------------------
// Signal sets.
// At present we define a single 32 bit integer mask. We may need, at
// some future point, to extend this to 64 bits, or a structure
// containing an array of masks.

typedef cyg_uint32 sigset_t;

//-----------------------------------------------------------------------------
// struct sigaction describes the action to be taken when we get a signal

struct sigaction
{
    sigset_t               sa_mask;             // Additional signals to be blocked
    int                    sa_flags;            // Special flags
    union
    {
        sa_sighandler_t    sa_handler;          // signal handler
        sa_siginfoaction_t sa_sigaction;        // Function to call instead of
                                                // sa_handler if SA_SIGINFO is
                                                // set in sa_flags
    } sa_sigactionhandler;
#define sa_handler   sa_sigactionhandler.sa_handler
#define sa_sigaction sa_sigactionhandler.sa_sigaction
};

// sa_flag bits
#define SA_NOCLDSTOP 1   // Don't generate SIGCHLD when children stop
#define SA_SIGINFO   2   // Use the sa_siginfoaction_t style signal
                         // handler, instead of the single argument handler

//-----------------------------------------------------------------------------
// Sigevent structure.

struct sigevent
{
    int                  sigev_notify;
    int                  sigev_signo;
    union sigval         sigev_value;
    void               (*sigev_notify_function) (union sigval);
    pthread_attr_t      *sigev_notify_attributes;
};

# define SIGEV_NONE	1
# define SIGEV_SIGNAL   2
# define SIGEV_THREAD	3

//-----------------------------------------------------------------------------
// Functions to generate signals

// Deliver sig to a process.
// eCos only supports the value 0 for pid.
externC int kill (pid_t pid, int sig);

externC int pthread_kill (pthread_t thread, int sig);

//-----------------------------------------------------------------------------
// Functions to catch signals

// Install signal handler for sig.
externC int sigaction  (int sig, const struct sigaction *act,
                        struct sigaction *oact);

// Queue signal to process with value.
externC int sigqueue  (pid_t pid, int sig, const union sigval value);

//-----------------------------------------------------------------------------
// Functions to deal with current blocked and pending masks

// Set process blocked signal mask
externC int sigprocmask  (int how, const sigset_t *set, sigset_t *oset);

// Set calling thread's blocked signal mask
externC int pthread_sigmask (int how, const sigset_t *set, sigset_t *oset);

// Get set of pending signals for this process
externC int sigpending  (sigset_t *set);

// Values for the how arguments:
#define SIG_BLOCK       1
#define SIG_UNBLOCK     2
#define SIG_SETMASK     3

//-----------------------------------------------------------------------------
// Wait for or accept signals

// Block signals in set and wait for a signal
externC int sigsuspend  (const sigset_t *set);

// Wait for a signal in set to arrive
externC int sigwait  (const sigset_t *set, int *sig);

// Do the same as sigwait() except return a siginfo_t object too.
externC int sigwaitinfo  (const sigset_t *set, siginfo_t *info);

// Do the same as sigwaitinfo() but return anyway after timeout.
externC int sigtimedwait  (const sigset_t *set, siginfo_t *info,
                           const struct timespec *timeout);

//-----------------------------------------------------------------------------
// Signal sets

// Clear all signals from set.
externC int sigemptyset  (sigset_t *set);

// Set all signals in set.
externC int sigfillset  (sigset_t *set);

// Add signo to set.
externC int sigaddset  (sigset_t *set, int signo);

// Remove signo from set.
externC int sigdelset  (sigset_t *set, int signo);

// Test whether signo is in set
externC int sigismember  (const sigset_t *set, int signo);

//-----------------------------------------------------------------------------
// alarm, pause and sleep

// Generate SIGALRM after some number of seconds
externC unsigned int alarm( unsigned int seconds );

// Wait for a signal to be delivered.
externC int pause( void );

// Wait for a signal, or the given number of seconds
externC unsigned int sleep( unsigned int seconds );

//-----------------------------------------------------------------------------
// signal() - ISO C 7.7.1   //
//
// Installs a new signal handler for the specified signal, and returns
// the old handler
//

externC sa_sighandler_t signal(int __sig, sa_sighandler_t __handler);

// raise() - ISO C 7.7.2 //
//
// Raises the signal, which will cause the current signal handler for
// that signal to be called

externC int raise(int __sig);

#endif // ifdef CYGPKG_POSIX_SIGNALS

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_SIGNAL_H
// End of signal.h
