/*========================================================================
//
//      mqueue2.c
//
//      POSIX Message queues tests - mq_notify
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-05-18
// Purpose:       This file provides tests for POSIX mqueue mq_notify
// Description:   
// Usage:         
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/posix.h>

#ifndef CYGPKG_POSIX_MQUEUES
# define NA_MSG "Message queues not configured"
#elif !defined(CYGPKG_POSIX_SIGNALS)
# define NA_MSG "No POSIX signals configured"
#endif

#ifdef NA_MSG
#include <cyg/infra/testcase.h>      // test API
void
cyg_user_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( NA_MSG );
}

#else

/* INCLUDES */

#include <fcntl.h>                   // O_*
#include <errno.h>                   // errno
#include <sys/stat.h>                // file modes
#include <mqueue.h>                  // Mqueue Header
#include <cyg/infra/testcase.h>      // test API
#include <signal.h>                  // signals

/* GLOBALS */
sig_atomic_t signals=0;
char buf[20];
unsigned int prio;


/* FUNCTIONS */

static int
my_memcmp(const void *m1, const void *m2, size_t n)
{
    char *s1 = (char *)m1;
    char *s2 = (char *)m2;

    while (n--) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++;
        s2++;
    }
    return 0;
} // my_memcmp()

static char *
my_strcpy(char *s1, const char *s2)
{
    char *s = s1;
    while (*s2) {
        *s1++ = *s2++;
    }
    return s;
} // my_strcpy()

static size_t
my_strlen(const char *s)
{
    const char *start = s;
    while (*s)
        s++;
    return (s - start);
} // my_strcpy()



//************************************************************************

static void
sigusr1_handler( int signo, siginfo_t *info, void *context )
{
    ssize_t recvlen;
    char mybuf[20];
    unsigned int myprio;
    mqd_t *q = (mqd_t *)info->si_value.sival_ptr;

    CYG_TEST_PASS_FAIL( SIGUSR1 == signo, "correct signal number #1" );
    CYG_TEST_PASS_FAIL( SIGUSR1 == info->si_signo, "correct signal number #2" );
    CYG_TEST_PASS_FAIL( SI_MESGQ == info->si_code, "correct signal code" );

    signals++;

    // retrieve message and compare with buf
    recvlen = mq_receive( *q, mybuf, sizeof(mybuf), &myprio );
    CYG_TEST_PASS_FAIL( recvlen == my_strlen(buf),
                        "receive message length" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, mybuf, my_strlen(buf)),
                        "received message data intact" );
    CYG_TEST_PASS_FAIL( prio == myprio,
                        "received at correct priority" );
}

//************************************************************************

int
main(void)
{
    mqd_t q1;
    struct mq_attr attr;
    mode_t mode;
    int err;
    ssize_t recvlen;
    char mybuf[20];
    unsigned int myprio;
    struct sigevent ev;
    struct sigaction act;

    CYG_TEST_INIT();
    CYG_TEST_INFO( "Starting POSIX message test 2" );

#if 0
    if ( 0 != pthread_create( &thr, NULL, &thread, NULL ) ) {
        CYG_TEST_FAIL_FINISH( "Couldn't create a helper thread" );
    }
#endif

    attr.mq_flags = 0;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 20;
    mode = S_IRWXU|S_IRWXG|S_IRWXO; // rwx for all

    q1 = mq_open( "/mq1", O_CREAT|O_NONBLOCK|O_RDWR, mode, &attr );
    CYG_TEST_PASS_FAIL( q1 != (mqd_t)-1, "simple mq_open (write only)" );
    
    err = mq_getattr( q1, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDWR == (attr.mq_flags & O_RDWR)) &&
                        (0 == attr.mq_curmsgs ), "getattr attributes correct" );


    act.sa_sigaction = &sigusr1_handler;
    sigfillset( &act.sa_mask ); // enable all signals
    act.sa_flags = SA_SIGINFO;
    
    if ( 0 != sigaction( SIGUSR1, &act, NULL ) ) {
        CYG_TEST_FAIL_FINISH( "Couldn't register signal handler" );
    }

    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = SIGUSR1;
    ev.sigev_value.sival_ptr = (void *)&q1;

    err = mq_notify( q1, &ev );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_notify" );

    my_strcpy( buf, "Vik is the best" );
    prio = 7;
    err = mq_send( q1, buf, my_strlen(buf), prio );

    CYG_TEST_PASS_FAIL( 0 == err, "mq_send #1" );

    CYG_TEST_PASS_FAIL( 1 == signals, "got notification" );

    my_strcpy( buf, "Scrummy Vik" );
    prio = 6;
    err = mq_send( q1, buf, my_strlen(buf), prio );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send #2" );

    CYG_TEST_PASS_FAIL( 1 == signals, "correctly didn't get notification" );

    recvlen = mq_receive( q1, mybuf, sizeof(mybuf), &myprio );
    CYG_TEST_PASS_FAIL( recvlen == my_strlen(buf),
                        "receive message length" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, mybuf, my_strlen(buf)),
                        "received message data intact" );
    CYG_TEST_PASS_FAIL( prio == myprio,
                        "received at correct priority" );

    err = mq_notify( q1, &ev );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_notify #2" );

    err = mq_notify( q1, &ev );
    CYG_TEST_PASS_FAIL( -1 == err, "second mq_notify returns error" );
    CYG_TEST_PASS_FAIL( EBUSY == errno,
                        "errno correct for second mq_notify error" );

    err = mq_notify( q1, NULL );
    CYG_TEST_PASS_FAIL( 0 == err, "clear notification" );

    my_strcpy( buf, "Vik is k3wl" );
    prio = 8;
    err = mq_send( q1, buf, my_strlen(buf), prio );

    CYG_TEST_PASS_FAIL( 0 == err, "mq_send #2" );

    CYG_TEST_PASS_FAIL( 1 == signals, "correctly didn't get notification #2" );

    recvlen = mq_receive( q1, mybuf, sizeof(mybuf), &myprio );
    CYG_TEST_PASS_FAIL( recvlen == my_strlen(buf),
                        "receive message length" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, mybuf, my_strlen(buf)),
                        "received message data intact" );
    CYG_TEST_PASS_FAIL( prio == myprio,
                        "received at correct priority" );
    
    err = mq_close( q1 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_close" );

    err = mq_unlink( "/mq1" );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_unlink" );

    CYG_TEST_EXIT("POSIX message test 2");

    return 0;
} // main()

//------------------------------------------------------------------------

#endif

/* EOF mqueue2.c */
