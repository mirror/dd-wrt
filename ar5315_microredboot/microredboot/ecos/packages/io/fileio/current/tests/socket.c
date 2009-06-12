//==========================================================================
//
//      socket.c
//
//      Test socket API
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
// Date:                2000-05-25
// Purpose:             Test socket API
// Description:         This program tests the socket API. Note that it is only
//                      intended to test the API and not the functionality of
//                      the underlying network stack. That is assumed to have
//                      been established by other tests elsewhere.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>


#define __ECOS 1

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/infra/testcase.h>

#if defined(CYGPKG_NET) && defined(CYGPKG_POSIX)
             
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <network.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>


#include <cyg/infra/diag.h>            // HAL polled output

//--------------------------------------------------------------------------

#define SHOW_RESULT( _fn, _res ) \
diag_printf(#_fn " returned %d %s\n", _res, _res<0?strerror(errno):"");

//--------------------------------------------------------------------------
// Thread stack.

char thread1_stack[PTHREAD_STACK_MIN*2];
char thread2_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------
// Local variables

// Thread IDs
pthread_t thread1;
pthread_t thread2;

struct sockaddr_in sa;

//--------------------------------------------------------------------------
// test buffers

#define TEST_BUFSIZE 512

static char buf1[TEST_BUFSIZE];
static char buf2[TEST_BUFSIZE];
static char buf3[TEST_BUFSIZE];

//--------------------------------------------------------------------------

void *pthread_entry1( void *arg)
{
    int fd, fd2;
    struct sockaddr_in accsa;
    socklen_t accsa_len = sizeof(accsa);
    int err;
//    int one = 1;
//    int so1 = sizeof(one);
    fd_set rd;
    int i;
    ssize_t done;
    
    CYG_TEST_INFO( "Thread 1 running" );

    CYG_TEST_INFO( "Thread1: calling socket()");        
    fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( fd < 0 ) SHOW_RESULT( socket, fd );
    CYG_TEST_CHECK( fd >= 0, "socket() returned error");

//    err = setsockopt( fd, SOL_SOCKET, SO_DONTROUTE, (void *)&one, so1);
//    if( err < 0 ) SHOW_RESULT( setsockopt, err );
//    CYG_TEST_CHECK( err == 0, "setsockopt() returned error");

    CYG_TEST_INFO( "Thread1: calling bind()");
    err = bind( fd, (struct sockaddr *)&sa, sizeof(sa));
    if( err < 0 ) SHOW_RESULT( bind, err );    
    CYG_TEST_CHECK( err == 0, "bind() returned error");

    CYG_TEST_INFO( "Thread1: calling listen()");
    err = listen( fd, 1);
    if( err < 0 ) SHOW_RESULT( listen, err );    
    CYG_TEST_CHECK( err == 0, "listen() returned error");

    FD_ZERO( &rd );
    FD_SET( fd, &rd );

    CYG_TEST_INFO( "Thread1: calling select()");
    err = select( fd+1, &rd, NULL, NULL, NULL );
    if( err < 0 ) SHOW_RESULT( select, err );    
    CYG_TEST_CHECK( err >= 0, "select() returned error");    
    CYG_TEST_CHECK( FD_ISSET( fd, &rd ), "Fd not set in select() result");

    CYG_TEST_INFO( "Thread1: calling accept()");
    fd2 = accept( fd, (struct sockaddr *)&accsa, &accsa_len );
    if( fd2 < 0 ) SHOW_RESULT( accept, fd2 );    
    CYG_TEST_CHECK( fd2 >= 0, "accept() returned error");


    for( i = 0; i < TEST_BUFSIZE; i++ ) buf1[i] = i;

    CYG_TEST_INFO( "Thread1: calling write()");
    done = write( fd2, buf1, TEST_BUFSIZE);
    if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
    CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");    

    FD_ZERO( &rd );
    FD_SET( fd2, &rd );

    CYG_TEST_INFO( "Thread1: calling select()");
    err = select( fd2+1, &rd, NULL, NULL, NULL );
    if( err < 0 ) SHOW_RESULT( select, err );    
    CYG_TEST_CHECK( err >= 0, "select() returned error");    
    CYG_TEST_CHECK( FD_ISSET( fd2, &rd ), "Fd2 not set in select() result");
    
    CYG_TEST_INFO( "Thread1: calling read()");
    done = read( fd2, buf3, TEST_BUFSIZE);
    if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
    CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

    for( i = 0; i < TEST_BUFSIZE; i++ )
        if( buf1[i] != buf3[i] )
            diag_printf("buf1[%d](%02x) != buf3[%d](%02x)\n",i,buf1[i],i,buf3[i]);

    CYG_TEST_INFO( "Thread1: calling close(fd)");
    err = close(fd);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");    

    CYG_TEST_INFO( "Thread1: calling close(fd2)");
    err = close(fd2);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");        
    
    CYG_TEST_INFO( "Thread1: calling pthread_exit()");    
    pthread_exit( arg );
}

//--------------------------------------------------------------------------

void *pthread_entry2( void *arg)
{
    int fd;
    int err;
//    int one = 1;
//    int so1 = sizeof(one);
    int i;
    ssize_t done;
    fd_set rd;
    
    CYG_TEST_INFO( "Thread 2 running" );

    CYG_TEST_INFO( "Thread2: calling socket()");
    fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( fd < 0 ) SHOW_RESULT( socket, fd );
    CYG_TEST_CHECK( fd >= 0, "socket() returned error");

//    err = setsockopt( fd, SOL_SOCKET, SO_DONTROUTE, (void *)&one, so1);
//    if( err < 0 ) SHOW_RESULT( setsockopt, err );    
//    CYG_TEST_CHECK( err == 0, "setsockopt() returned error");

    CYG_TEST_INFO( "Thread2: calling connect()");
    err = connect( fd, (struct sockaddr *)&sa, sizeof(sa));
    if( err < 0 ) SHOW_RESULT( connect, err );    
    CYG_TEST_CHECK( err == 0, "connect() returned error");

    FD_ZERO( &rd );
    FD_SET( fd, &rd );

    CYG_TEST_INFO( "Thread2: calling select()");
    err = select( fd+1, &rd, NULL, NULL, NULL );
    if( err < 0 ) SHOW_RESULT( select, err );    
    CYG_TEST_CHECK( err >= 0, "select() returned error");    
    CYG_TEST_CHECK( FD_ISSET( fd, &rd ), "Fd not set in select() result");

    CYG_TEST_INFO( "Thread2: calling read()");
    done = read( fd, buf2, TEST_BUFSIZE);
    if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
    CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

    for( i = 0; i < TEST_BUFSIZE; i++ )
        if( buf1[i] != buf2[i] )
            diag_printf("buf1[%d](%02x) != buf2[%d](%02x)\n",i,buf1[i],i,buf2[i]);

    CYG_TEST_INFO( "Thread2: calling write()");
    done = write( fd, buf2, TEST_BUFSIZE);
    if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
    CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");    

    CYG_TEST_INFO( "Thread2: calling close(fd)");
    err = close(fd);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");    
    
    CYG_TEST_INFO( "Thread2: calling pthread_exit()");    
    pthread_exit( arg );
}

//==========================================================================
// main

int main( int argc, char **argv )
{
    struct in_addr ina;
    char *addr1 = "127.0.255.106";
    char *addr2;
    void *retval;
    pthread_attr_t attr;
    struct sched_param schedparam;
    
    sa.sin_family = AF_INET;
    sa.sin_len = sizeof(sa);
    inet_aton("127.0.0.1", &sa.sin_addr);
    sa.sin_port = htons(1234);
    
    CYG_TEST_INIT();

    init_all_network_interfaces();
    
    // Test inet_ntoa() and inet_aton()

    inet_aton(addr1, &ina);
    addr2 = inet_ntoa(ina);
    CYG_TEST_CHECK( strcmp(addr1, addr2) == 0, "Bad inet adderess conversion");


    // Create test threads

    {
        pthread_attr_init( &attr );

        schedparam.sched_priority = 10;
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

        schedparam.sched_priority = 5;
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

    CYG_TEST_PASS_FINISH("socket");
}

#else

//==========================================================================
// main

int main( int argc, char **argv )
{
    CYG_TEST_INIT();

    CYG_TEST_NA("socket");
}

#endif


// -------------------------------------------------------------------------
// EOF socket.c
