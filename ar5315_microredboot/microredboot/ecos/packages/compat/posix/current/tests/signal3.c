//==========================================================================
//
//        signal3.cxx
//
//        POSIX signal test 3
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
// Date:          2003-01-30
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

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <cyg/infra/diag.h>

volatile int sigusr1_called = 0;

//--------------------------------------------------------------------------
// Signal handler functions

static void sigusr1( int signo )
{
    CYG_TEST_INFO( "sigusr1() handler called" );
    CYG_TEST_CHECK( signo == SIGUSR1, "Signal not SIGUSR1");

    sigusr1_called++;
}

//--------------------------------------------------------------------------

int main (int argc, char **argv)
{
    int ret_val;
    sigset_t set;
    int sig;
    struct itimerspec timerValue;	// Timeout value on eCos
    timer_t timer1;			// Timer
    struct sigevent sev;

    CYG_TEST_INIT();

    {
        struct sigaction sa;

        sa.sa_handler = sigusr1;
        sigfillset( &sa.sa_mask );
        sa.sa_flags = 0;

        ret_val = sigaction( SIGUSR1, &sa, NULL );

        CYG_TEST_CHECK( ret_val == 0 , "sigaction returned error");
    }
    
    // unblock all the signals
    sigfillset (&set);
    pthread_sigmask (SIG_UNBLOCK, &set, (sigset_t*)NULL);
    
    //--------------------------------------------------------------------
    // <start of timer initialization section>
    //--------------------------------------------------------------------
	
    // Notification type --- Deliver the signal
    sev.sigev_notify                = SIGEV_SIGNAL;
    sev.sigev_signo                 = SIGUSR1;
    sev.sigev_value.sival_int       = 0xABCDEF01;
	
    // Timer values	--- 1 Second
    timerValue.it_value.tv_sec           = 1;
    timerValue.it_value.tv_nsec          = 0;
    timerValue.it_interval.tv_sec        = 1;
    timerValue.it_interval.tv_nsec       = 0;
	
    ret_val = timer_create (CLOCK_REALTIME, &sev, &timer1);

    CYG_TEST_CHECK( ret_val==0, "Error in creating the timer");

    ret_val = timer_settime (timer1, 0, &timerValue, NULL );
    CYG_TEST_CHECK( ret_val==0,"Error in setting the time");

    //--------------------------------------------------------------------
    // <end of timer initialization section>
    //--------------------------------------------------------------------

    CYG_TEST_INFO ("Timer initialisation is completed..");

    CYG_TEST_INFO ("Calling pause()");
    ret_val = pause();
    CYG_TEST_CHECK( ret_val==-1, "pause() did not return -1");
    CYG_TEST_CHECK( EINTR==errno, "errno set to EINTR");
    CYG_TEST_CHECK( sigusr1_called==1, "Siguser1 handler not called");
    
    // Block all the signals
    sigfillset (&set);
    pthread_sigmask (SIG_BLOCK, &set, (sigset_t*)NULL);

    CYG_TEST_INFO ("Calling sigwait()");    
    // Wait for any signal to arrive
    sigfillset (&set);
    ret_val = sigwait (&set, &sig);

    CYG_TEST_CHECK( ret_val==0, "sigwait returned error");
    CYG_TEST_CHECK( sig==SIGUSR1, "sigwait returned wrong signo!");
    CYG_TEST_CHECK( sigusr1_called==1, "Siguser1 handler called!");
    
    CYG_TEST_INFO ("Program terminating");

    CYG_TEST_PASS_FINISH( "signal3" );
    return 0;
}


#endif

//--------------------------------------------------------------------------
// end of signal3.c
