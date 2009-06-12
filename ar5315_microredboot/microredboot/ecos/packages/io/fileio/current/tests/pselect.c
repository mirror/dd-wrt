//==========================================================================
//
//      pselect.c
//
//      Test pselect implementation
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
// Date:                2002-11-08
// Purpose:             Test pselect implementation
// Description:         
//                      
//                      
//                      
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>

#ifndef CYGINT_ISO_PTHREAD_IMPL
# define NA_MSG "POSIX threads needed to run test"
#elif !defined CYGPKG_NET
# define NA_MSG "NET package needed to run test"
#elif !defined CYGPKG_POSIX_SIGNALS
# define NA_MSG "POSIX signals package needed to run test"
#endif

#include <cyg/infra/testcase.h>

#ifndef NA_MSG

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#define __ECOS 1                        // dont like this at all

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <network.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>

#include <sys/select.h>


#include <cyg/infra/diag.h>            // HAL polled output

//--------------------------------------------------------------------------

#define SHOW_RESULT( _fn, _res ) \
diag_printf("INFO: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");

//--------------------------------------------------------------------------
// Thread stacks

char thread1_stack[PTHREAD_STACK_MIN*2];
char thread2_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------
// Local variables

// Thread IDs
pthread_t thread1;
pthread_t thread2;

struct sockaddr_in sa;

volatile int sigusr1_calls = 0;
volatile int sigusr1_sent = 0;
volatile int pselect_wakeups = 0;
volatile int pselect_eintr = 0;

volatile cyg_bool running = true;

//--------------------------------------------------------------------------

void show_fdsets( char *s, int nfd, fd_set *rd, fd_set *wr, fd_set *ex )
{
    int i;
    diag_printf("INFO:<%s nfd %d ",s,nfd);

    if( rd )
    {
        diag_printf("rd: [");
        for( i = 0; i < nfd ; i++ )
            if( FD_ISSET( i, rd ) ) diag_printf("%d ",i);
        diag_printf("] ");        
    }    
    if( wr )
    {
        diag_printf("wr: [");                
        for( i = 0; i < nfd ; i++ )
            if( FD_ISSET( i, wr ) ) diag_printf("%d ",i);
        diag_printf("] ");        
    }
    if( ex )
    {
        diag_printf("ex: [");                
        for( i = 0; i < nfd ; i++ )
            if( FD_ISSET( i, ex ) ) diag_printf("%d ",i);
        diag_printf("] ");                
    }

    diag_printf(">\n");
}

//--------------------------------------------------------------------------

void sigusr1( int sig, siginfo_t *info, void *context )
{
    CYG_TEST_CHECK( pthread_self() == thread1, "Sigusr1: not called by thread 1\n");
    
    sigusr1_calls++;
}

//--------------------------------------------------------------------------
// Selecting thread

// This thread just opens up a socket ready to accept a connection and
// then calls pselect() to wait for it. The timeout is set to 0 so we
// actually just poll.

void *pthread_entry1( void *arg)
{
    int fd = 0;
    int err;
    fd_set rd, wr;
    sigset_t mask, oldmask;
    struct sigaction sigact;
    struct timespec ts;
    
    CYG_TEST_INFO( "Thread 1 running" );

    FD_ZERO( &rd );
    FD_ZERO( &wr );

    sigfillset( &mask );
    pthread_sigmask( SIG_SETMASK, &mask, &oldmask );
    
    sigdelset( &mask, SIGUSR1 );

    sigact.sa_mask = mask;
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = sigusr1;

    err = sigaction( SIGUSR1, &sigact, NULL );
    if( err < 0 ) SHOW_RESULT( sigact, err );

    CYG_TEST_INFO( "Thread1: calling socket()");        
    fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( fd < 0 ) SHOW_RESULT( socket, fd );
    CYG_TEST_CHECK( fd >= 0, "socket() returned error");

    CYG_TEST_INFO( "Thread1: calling bind()");
    err = bind( fd, (struct sockaddr *)&sa, sizeof(sa));
    if( err < 0 ) SHOW_RESULT( bind, err );    
    CYG_TEST_CHECK( err == 0, "bind() returned error");

    CYG_TEST_INFO( "Thread1: calling listen()");
    err = listen( fd, 3);
    if( err < 0 ) SHOW_RESULT( listen, err );    
    CYG_TEST_CHECK( err == 0, "listen() returned error");

    FD_SET( fd, &rd );

    ts.tv_sec = 0;
    ts.tv_nsec = 0;
        
    
    while( running )
    {
        fd_set rd_res = rd;
        fd_set wr_res = wr;

//        ts.tv_nsec = 1000000 * (pselect_wakeups % 10);
        
        err = pselect( 8, &rd_res, &wr_res, NULL, &ts, &mask );
        if( err < 0 )
        {
            if( errno == EINTR ) pselect_eintr++;
            else SHOW_RESULT( pselect, err );
        }
        if( err > 0 ) show_fdsets( "Thread1 result: ", 8, &rd_res, &wr_res, NULL );
        pselect_wakeups++;
        
    }

    // If we were interrupted at just the wrong point above we may still
    // have a SIGUSR1 signal pending that we didn't handle, and so won't
    // have accounted for. So let's look...
    CYG_TEST_CHECK( 0 == sigpending( &mask ), "sigpending() returned error");
    if (1 == sigismember(&mask, SIGUSR1) )
        pselect_eintr++;

    pthread_sigmask( SIG_SETMASK, &oldmask, NULL );

    pthread_exit(arg);
}

//--------------------------------------------------------------------------

void *pthread_entry2( void *arg)
{
    struct timespec zzz;
    int err;
    
    zzz.tv_sec = 0;
    zzz.tv_nsec = 10*1000000;
    
    CYG_TEST_INFO( "Thread 2: running" );

    CYG_TEST_INFO( "Thread 2: sleeping" );
    nanosleep( &zzz, NULL );
    nanosleep( &zzz, NULL );
    nanosleep( &zzz, NULL );
    
    while( sigusr1_sent < 20000 )
    {
        nanosleep( &zzz, NULL );

        err = pthread_kill( thread1, SIGUSR1 );
        if( err < 0 ) SHOW_RESULT( pthread_kill, err );

        sigusr1_sent++;

        if( (sigusr1_sent % 500) == 0 )
            diag_printf("INFO: <Thread 2: %d signals sent>\n",sigusr1_sent);
    }

    running = false;
        
    CYG_TEST_INFO( "Thread 2: exit" );
    pthread_exit( arg );
}

//==========================================================================
// main

int main( int argc, char **argv )
{
    void *retval;
    pthread_attr_t attr;
    struct sched_param schedparam;

    CYG_TEST_INIT();

    sa.sin_family = AF_INET;
    sa.sin_len = sizeof(sa);
    inet_aton("127.0.0.1", &sa.sin_addr);
    sa.sin_port = htons(1234);
    init_all_network_interfaces();
    
    // Create test threads

    {
        pthread_attr_init( &attr );

        schedparam.sched_priority = 5;
        pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
        pthread_attr_setschedpolicy( &attr, SCHED_RR );
        pthread_attr_setschedparam( &attr, &schedparam );
        pthread_attr_setstackaddr( &attr, (void *)&thread1_stack[sizeof(thread1_stack)] );
        pthread_attr_setstacksize( &attr, sizeof(thread1_stack) );

        pthread_create( &thread1,
                        &attr,
                        pthread_entry1,
                        (void *)0x12345671);
    }

    {
        pthread_attr_init( &attr );

        schedparam.sched_priority = 10;
        pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
        pthread_attr_setschedpolicy( &attr, SCHED_RR );
        pthread_attr_setschedparam( &attr, &schedparam );
        pthread_attr_setstackaddr( &attr, (void *)&thread2_stack[sizeof(thread2_stack)] );
        pthread_attr_setstacksize( &attr, sizeof(thread2_stack) );

        pthread_create( &thread2,
                        &attr,
                        pthread_entry2,
                        (void *)0x12345672);
    }
    
    // Now join with thread1
    CYG_TEST_INFO( "Main: calling pthread_join(thread1)");
    pthread_join( thread1, &retval );

    // And thread 2
    CYG_TEST_INFO( "Main: calling pthread_join(thread2)");
    pthread_join( thread2, &retval );

    diag_printf("INFO: pselect returns: %d\n", pselect_wakeups );
    diag_printf("INFO: pselect EINTR returns: %d\n", pselect_eintr );
    diag_printf("INFO: SIGUSR1 sent: %d\n", sigusr1_sent );
    diag_printf("INFO: SIGUSR1 delivered: %d\n", sigusr1_calls );
    
    CYG_TEST_CHECK( sigusr1_sent == sigusr1_calls, "SIGUSR1 calls != delivered");
    CYG_TEST_CHECK( sigusr1_sent == pselect_eintr, "SIGUSR1 calls != pselect EINTR wakeups");
    
    CYG_TEST_PASS_FINISH("pselect");
}

#else

//==========================================================================
// main

void cyg_start(void)
{
    CYG_TEST_INIT();

    CYG_TEST_NA(NA_MSG);
}

#endif

