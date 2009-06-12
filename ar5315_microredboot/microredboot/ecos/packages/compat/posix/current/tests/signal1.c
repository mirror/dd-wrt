//==========================================================================
//
//        signal1.cxx
//
//        POSIX signal test 1
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-04-10
// Description:   Tests POSIX signal functionality.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/testcase.h>
#include <pkgconf/posix.h>

#if !defined(CYGPKG_POSIX_SIGNALS)
#define NA_MSG "POSIX signals not enabled"
#elif !defined(CYGPKG_POSIX_PTHREAD)
#define NA_MSG "POSIX threads not enabled"
#endif

#ifdef NA_MSG
void
cyg_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA(NA_MSG);
}
#else

#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>

//--------------------------------------------------------------------------
// Thread stack.

char thread_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------
// Local variables

// Sync semaphore
sem_t sem;

// Thread ID
pthread_t thread1;

volatile int sigusr2_called = 0;
volatile int sigalrm_called = 0;

//--------------------------------------------------------------------------
// Signal handler functions

static void sigusr2( int signo )
{
    CYG_TEST_INFO( "sigusr2() handler called" );
    CYG_TEST_CHECK( signo == SIGUSR2, "Signal not SIGUSR2");
    CYG_TEST_CHECK( pthread_equal(pthread_self(), thread1), "Not called in thread1");

    sigusr2_called++;
}

static void sigalrm( int signo )
{
    CYG_TEST_INFO( "sigalrm() handler called" );
    CYG_TEST_CHECK( signo == SIGALRM, "Signal not SIGALRM");
    CYG_TEST_CHECK( pthread_equal(pthread_self(), thread1), "Not called in thread1");

    sigalrm_called++;
}

//--------------------------------------------------------------------------

void *pthread_entry1( void *arg)
{
    sigset_t mask;
    siginfo_t info;
    struct timespec timeout;
    int sig, sig2, err;
    
    CYG_TEST_INFO( "Thread 1 running" );

    // Should have inherited parent's signal mask
    pthread_sigmask( 0, NULL, &mask );
    CYG_TEST_CHECK( sigismember( &mask, SIGALRM),
                                 "SIGALRM mask inherited");
    CYG_TEST_CHECK( sigismember( &mask, SIGUSR1),
                                 "SIGUSR1 mask inherited");
    CYG_TEST_CHECK( sigismember( &mask, SIGUSR2),
                                 "SIGUSR2 mask inherited");
    CYG_TEST_CHECK( sigismember( &mask, SIGSEGV),
                                 "SIGSEGV mask inherited");

    // Make a full set
    sigfillset( &mask );

    // remove USR2 and ALRM signals
    sigdelset( &mask, SIGUSR2 );
    sigdelset( &mask, SIGALRM );

    // Set signal mask
    pthread_sigmask( SIG_SETMASK, &mask, NULL );
    
    // Get main thread going again
    sem_post( &sem );

    // set up timeout
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;

    CYG_TEST_INFO( "Thread1: calling sigtimedwait()");
    
    // Wait for a signal to be delivered
    sig = sigtimedwait( &mask, &info, &timeout );

    sig2 = info.si_signo;
    
    CYG_TEST_CHECK( sig == sig2, "sigtimedwait return value not equal");
    CYG_TEST_CHECK( sig == SIGUSR1, "Signal not delivered");

    while( sigusr2_called != 2 )
    {
        CYG_TEST_INFO( "Thread1: calling pause()");        
        pause();
    }

    errno = 0; // strictly correct to reset errno first

    // now wait for SIGALRM to be delivered
    CYG_TEST_INFO( "Thread1: calling pause()");            
    err = pause();
    CYG_TEST_CHECK( -1==err, "pause returned -1");
    CYG_TEST_CHECK( EINTR==errno, "errno set to EINTR");

    // generate another SIGALRM and wait for it to be delivered too
    // we need to mask it first though

    // Make a full set
    sigfillset( &mask );

    // Set signal mask
    pthread_sigmask( SIG_SETMASK, &mask, NULL );
    
    alarm(1);
    CYG_TEST_INFO( "Thread1: calling sigwait()");            
    err = sigwait( &mask, &sig);
    CYG_TEST_CHECK( 0==err, "sigwait returned -1");
    CYG_TEST_CHECK( sig==SIGALRM, "sigwait caught alarm");

    CYG_TEST_INFO( "Thread1: calling pthread_exit()");    
    pthread_exit( (void *)((int)arg+sig2) );
}

//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int ret;
    sigset_t mask;
    pthread_attr_t attr;
    void *retval;
    union sigval value;
    
    CYG_TEST_INIT();

    // Make a full signal set
    sigfillset( &mask );

    
    // Install signal handlers
    {
        struct sigaction sa;

        sa.sa_handler = sigusr2;
        sa.sa_mask = mask;
        sa.sa_flags = 0;

        ret = sigaction( SIGUSR2, &sa, NULL );

        CYG_TEST_CHECK( ret == 0 , "sigaction returned error");
    }

    {
        struct sigaction sa;

        sa.sa_handler = sigalrm;
        sa.sa_mask = mask;
        sa.sa_flags = 0;

        ret = sigaction( SIGALRM, &sa, NULL );

        CYG_TEST_CHECK( ret == 0 , "sigaction returned error");
    }
    
    
    // Mask all signals
    pthread_sigmask( SIG_SETMASK, &mask, NULL );
    
    sem_init( &sem, 0, 0 );
    
    // Create test thread
    pthread_attr_init( &attr );

    pthread_attr_setstackaddr( &attr, (void *)&thread_stack[sizeof(thread_stack)] );
    pthread_attr_setstacksize( &attr, sizeof(thread_stack) );

    pthread_create( &thread1,
                    &attr,
                    pthread_entry1,
                    (void *)0x12345678);

    // Wait for other thread to get started
    CYG_TEST_INFO( "Main: calling sem_wait()");
    sem_wait( &sem );

    value.sival_int = 0;

    // send a signal to the other thread
    CYG_TEST_INFO( "Main: calling sigqueue(SIGUSR1)");
    sigqueue( 0, SIGUSR1, value );

    // Send the signal via kill
    CYG_TEST_INFO( "Main: calling kill(0, SIGUSR2)");
    kill( 0, SIGUSR2 );

    // Wait for thread1 to call pause()
    CYG_TEST_INFO( "Main: calling sleep(1)");
    sleep(1);

    // And again
    CYG_TEST_INFO( "Main: calling kill(0, SIGUSR2)");
    kill( 0, SIGUSR2 );

    // Set up an alarm for 1 second hence
    CYG_TEST_INFO( "Main: calling alarm(1)");
    alarm(1);
    
    // Wait for alarm signal to be delivered to thread1
    CYG_TEST_INFO( "Main: calling sleep(2)");
    sleep(2);
    
    // Now join with thread1
    CYG_TEST_INFO( "Main: calling pthread_join()");
    pthread_join( thread1, &retval );

    CYG_TEST_CHECK( sigusr2_called == 2, "SIGUSR2 signal handler not called twice" );

    CYG_TEST_CHECK( sigalrm_called == 1, "SIGALRM signal handler not called" );    
    
    // check retval
    
    if( (long)retval == 0x12345678+SIGUSR1 )
        CYG_TEST_PASS_FINISH( "signal1" );
    else
        CYG_TEST_FAIL_FINISH( "signal1" );
}

#endif

//--------------------------------------------------------------------------
// end of signal1.c
