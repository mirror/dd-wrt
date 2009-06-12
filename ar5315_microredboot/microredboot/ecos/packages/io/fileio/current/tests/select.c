//==========================================================================
//
//      select.c
//
//      Test select implementation
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
// Purpose:             Test select implementation
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
#endif

#include <cyg/infra/testcase.h>

#ifndef NA_MSG

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#ifdef CYGPKG_IO_SERIAL
#include <pkgconf/io_serial.h>
#endif

#define __ECOS 1                        // dont like this at all

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#ifdef CYGPKG_NET
#include <network.h>
#include <arpa/inet.h>
#define TEST_NET
#endif

#ifdef CYGPKG_IO_SERIAL_LOOP
#define TEST_DEV
#endif

#include <pthread.h>
#include <signal.h>


#include <cyg/infra/diag.h>            // HAL polled output

//--------------------------------------------------------------------------

#define SHOW_RESULT( _fn, _res ) \
diag_printf("INFO: " #_fn "() returned %d %s\n", _res, _res<0?strerror(errno):"");

//--------------------------------------------------------------------------
// Thread stack.

char thread1_stack[PTHREAD_STACK_MIN*2];
char thread2_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------
// Local variables

// Thread IDs
pthread_t thread1;
pthread_t thread2;

#ifdef TEST_NET        
struct sockaddr_in sa;
#endif

//--------------------------------------------------------------------------
// Test buffers
// The buffer size here must be less that the size of the serial device
// buffers since the serial devices do not currently implement flow
// control.

#define TEST_BUFSIZE 100

#ifdef TEST_NET        
static char buf1[TEST_BUFSIZE];
static char buf2[TEST_BUFSIZE];
static char buf3[TEST_BUFSIZE];
#endif
#ifdef TEST_DEV
static char sbuf1[TEST_BUFSIZE];
static char sbuf2[TEST_BUFSIZE];
static char sbuf3[TEST_BUFSIZE];
#endif


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

void *pthread_entry1( void *arg)
{
#ifdef TEST_NET        
    int fd = 0, fd2 = -1;
    struct sockaddr_in accsa;
    socklen_t accsa_len = sizeof(accsa);
    int netstate = 0;
#endif    
#ifdef TEST_DEV    
    int ser0;
    int serstate = 0;
#endif
#if defined(TEST_DEV) || defined(TEST_NET)
    int i;
    ssize_t done;
#endif    
    int netdone = 0;
    int serdone = 0;
    int err;
    fd_set rd, wr;
    
    CYG_TEST_INFO( "Thread 1 running" );

    FD_ZERO( &rd );
    FD_ZERO( &wr );
    
#ifdef TEST_DEV

    CYG_TEST_INFO( "Thread1: calling open()");
    ser0 = open("/dev/ser0", O_RDWR );
    if( ser0 < 0 ) SHOW_RESULT( open, ser0 );
    CYG_TEST_CHECK( ser0 >= 0, "open(/dev/ser0) returned error");

    FD_SET( ser0, &rd );
#else
    serdone = 1;
#endif

#ifdef TEST_NET

    for( i = 0; i < TEST_BUFSIZE; i++ ) buf1[i] = i;
    
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
    
#else
    netdone = 1;
#endif

    while(!(netdone && serdone))
    {
        fd_set rd_res = rd;
        fd_set wr_res = wr;
        
        CYG_TEST_INFO( "Thread1: calling select()");
        show_fdsets( "Thread1 request: ", 8, &rd_res, &wr_res, NULL );
        err = select( 8, &rd_res, &wr_res, NULL, NULL );
        if( err < 0 ) SHOW_RESULT( select, err );    
        CYG_TEST_CHECK( err >= 0, "select() returned error");
        show_fdsets( "Thread1 result: ", 8, &rd_res, &wr_res, NULL );        
        
#ifdef TEST_NET
        switch( netstate )
        {
        case 0:
            CYG_TEST_INFO( "Thread1: netstate 0");
            if( FD_ISSET( fd, &rd_res ) )
            {
                CYG_TEST_INFO( "Thread1: calling accept(fd)");
                fd2 = accept( fd, (struct sockaddr *)&accsa, &accsa_len );
                if( fd2 < 0 ) SHOW_RESULT( accept, fd2 );    
                CYG_TEST_CHECK( fd2 >= 0, "accept() returned error");

                FD_CLR( fd, &rd );
                FD_SET( fd2, &wr );

                netstate++;                
            }
            break;


        case 1:
            CYG_TEST_INFO( "Thread1: netstate 1");
            if( FD_ISSET( fd2, &wr_res ) )
            {
                
                CYG_TEST_INFO( "Thread1: calling write(fd2)");
                done = write( fd2, buf1, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");

                FD_CLR( fd2, &wr );
                FD_SET( fd2, &rd );

                netstate++;
            }
            break;

        case 2:
            CYG_TEST_INFO( "Thread1: netstate 2");
            if( FD_ISSET( fd2, &rd_res ) )
            {
                CYG_TEST_INFO( "Thread1: calling read(fd2)");
                done = read( fd2, buf3, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

                for( i = 0; i < TEST_BUFSIZE; i++ )
                    if( buf1[i] != buf3[i] )
                        diag_printf("buf1[%d](%02x) != buf3[%d](%02x)\n",i,buf1[i],i,buf3[i]);

                FD_CLR( fd2, &rd );

                netstate++;
                netdone = 1;
                CYG_TEST_INFO( "Thread1: netdone");
            }
            break;
            
        }
#endif        

#ifdef TEST_DEV
        switch( serstate )
        {
        case 0:
            CYG_TEST_INFO( "Thread1: serstate 0");
            if( FD_ISSET( ser0, &rd_res ) )
            {
                CYG_TEST_INFO( "Thread1: calling read(ser0)");
                done = read( ser0, sbuf2, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

                for( i = 0; i < TEST_BUFSIZE; i++ )
                    if( sbuf1[i] != sbuf2[i] )
                        diag_printf("buf1[%d](%02x) != buf2[%d](%02x)\n",i,sbuf1[i],i,sbuf2[i]);

                FD_CLR( ser0, &rd );
                FD_SET( ser0, &wr );
                serstate++;
                
            }
            break;
            
        case 1:
            CYG_TEST_INFO( "Thread1: serstate 1");
            if( FD_ISSET( ser0, &wr_res ) )
            {
                CYG_TEST_INFO( "Thread1: calling write(ser0)");
                done = write( ser0, sbuf2, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");

                FD_CLR( ser0, &wr );
                
                serstate++;
                serdone = 1;
                CYG_TEST_INFO( "Thread1: serdone");
            }
            else FD_SET( ser0, &wr );
            break;
        }
#endif        
    }

#ifdef TEST_NET    
    CYG_TEST_INFO( "Thread1: calling close(fd)");
    err = close(fd);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");    

    if( fd2 >= 0 )
    {
        CYG_TEST_INFO( "Thread1: calling close(fd2)");
        err = close(fd2);
        if( err < 0 ) SHOW_RESULT( close, err );    
        CYG_TEST_CHECK( err == 0, "close() returned error");
    }
#endif
#ifdef TEST_DEV
    CYG_TEST_INFO( "Thread1: calling close(ser0)");
    err = close(ser0);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");    
#endif
    
    CYG_TEST_INFO( "Thread1: calling pthread_exit()");    
    pthread_exit( arg );
}

//--------------------------------------------------------------------------

void *pthread_entry2( void *arg)
{
#ifdef TEST_NET        
    int fd;
    int netstate = 0;
#endif    
#ifdef TEST_DEV    
    int ser1;
    int serstate = 0;
#endif    
#if defined(TEST_DEV) || defined(TEST_NET)
    int i;
    ssize_t done;
#endif    
    int netdone = 0;
    int serdone = 0;
    int err;
    fd_set rd, wr;
    
    CYG_TEST_INFO( "Thread 2 running" );

    FD_ZERO( &rd );
    FD_ZERO( &wr );

#ifdef TEST_NET            
    CYG_TEST_INFO( "Thread2: calling socket()");
    fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( fd < 0 ) SHOW_RESULT( socket, fd );
    CYG_TEST_CHECK( fd >= 0, "socket() returned error");

    CYG_TEST_INFO( "Thread2: calling connect()");
    err = connect( fd, (struct sockaddr *)&sa, sizeof(sa));
    if( err < 0 ) SHOW_RESULT( connect, err );    
    CYG_TEST_CHECK( err == 0, "connect() returned error");

    FD_SET( fd, &rd );
#else
    netdone = 1;
#endif

#ifdef TEST_DEV
    for( i = 0; i < TEST_BUFSIZE; i++ ) sbuf1[i] = i;
    
    CYG_TEST_INFO( "Thread2: calling open(/dev/ser1)");
    ser1 = open("/dev/ser1", O_RDWR );
    if( ser1 < 0 ) SHOW_RESULT( open, ser1 );
    CYG_TEST_CHECK( ser1 >= 0, "open(/dev/ser1) returned error");

    CYG_TEST_INFO( "Thread2: calling write(ser1)");
    done = write( ser1, sbuf1, TEST_BUFSIZE);
    if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
    CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");
    
    FD_SET( ser1, &wr );
    
#else
    serdone = 1;
#endif    
    
    while(!(netdone && serdone))
    {
        fd_set rd_res = rd;
        fd_set wr_res = wr;
        
        CYG_TEST_INFO( "Thread2: calling select()");
        show_fdsets( "Thread2 request: ", 8, &rd_res, &wr_res, NULL );
        err = select( 8, &rd_res, &wr_res, NULL, NULL );
        if( err < 0 ) SHOW_RESULT( select, err );    
        CYG_TEST_CHECK( err >= 0, "select() returned error");    
        show_fdsets( "Thread2 result: ", 8, &rd_res, &wr_res, NULL );
                
#ifdef TEST_NET
        switch( netstate )
        {
        case 0:
            CYG_TEST_INFO( "Thread2: netstate 0");
            if( FD_ISSET( fd, &rd_res ) )
            {
                CYG_TEST_INFO( "Thread2: calling read()");
                done = read( fd, buf2, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

                for( i = 0; i < TEST_BUFSIZE; i++ )
                    if( buf1[i] != buf2[i] )
                        diag_printf("buf1[%d](%02x) != buf2[%d](%02x)\n",i,buf1[i],i,buf2[i]);

                netstate++;

                FD_CLR( fd, &rd );
                FD_SET( fd, &wr );
            }
            break;

        case 1:
            CYG_TEST_INFO( "Thread2: netstate 1");
            if( FD_ISSET( fd, &wr_res ) )
            {

                CYG_TEST_INFO( "Thread2: calling write()");
                done = write( fd, buf2, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( write, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "write() returned bad size");

                FD_CLR( fd, &wr );
                
                netstate++;
                netdone = 1;
                CYG_TEST_INFO( "Thread2: netdone");
            }
            break;
            
        }
#endif

#ifdef TEST_DEV
        switch( serstate )
        {
        case 0:
            CYG_TEST_INFO( "Thread2: serstate 0");
            if( FD_ISSET( ser1, &wr_res ) )
            {
                FD_CLR( ser1, &wr );
                FD_SET( ser1, &rd );
                serstate++;
            }
            break;

        case 1:
            CYG_TEST_INFO( "Thread2: serstate 1");            
            if( FD_ISSET( ser1, &rd_res ) )
            {
                CYG_TEST_INFO( "Thread2: calling read(ser1)");
                done = read( ser1, sbuf3, TEST_BUFSIZE);
                if( done != TEST_BUFSIZE ) SHOW_RESULT( read, done );
                CYG_TEST_CHECK( done == TEST_BUFSIZE, "read() returned bad size");    

                for( i = 0; i < TEST_BUFSIZE; i++ )
                    if( sbuf1[i] != sbuf3[i] )
                        diag_printf("sbuf1[%d](%02x) != sbuf3[%d](%02x)\n",i,sbuf1[i],i,sbuf3[i]);

                FD_CLR( ser1, &rd );

                serstate++;
                serdone = 1;
                CYG_TEST_INFO( "Thread2: serdone");
            }
            break;
        }
#endif        
    }

#ifdef TEST_NET    
    CYG_TEST_INFO( "Thread2: calling close(fd)");
    err = close(fd);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close() returned error");    
#endif
#ifdef TEST_DEV
    CYG_TEST_INFO( "Thread2: calling close(ser1)");
    err = close(ser1);
    if( err < 0 ) SHOW_RESULT( close, err );    
    CYG_TEST_CHECK( err == 0, "close(ser1) returned error");    
#endif
    
    
    CYG_TEST_INFO( "Thread2: calling pthread_exit()");    
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

#ifdef TEST_NET
    sa.sin_family = AF_INET;
    sa.sin_len = sizeof(sa);
    inet_aton("127.0.0.1", &sa.sin_addr);
    sa.sin_port = htons(1234);
    init_all_network_interfaces();
#endif
    
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

    CYG_TEST_PASS_FINISH("select");
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


// -------------------------------------------------------------------------
// EOF socket.c
