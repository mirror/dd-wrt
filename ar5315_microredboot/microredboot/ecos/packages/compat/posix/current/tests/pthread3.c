//==========================================================================
//
//        pthread3.cxx
//
//        POSIX pthread test 2 - cancellation
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
// Contributors:  nickg, jlarmour
// Date:          2000-04-10
// Description:   Tests POSIX cancellation.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cyg/infra/testcase.h>
#include <pkgconf/posix.h>

#ifndef CYGPKG_POSIX_PTHREAD
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
#include <unistd.h> // sleep()

//--------------------------------------------------------------------------
// Thread info

#define NTHREADS 3

char thread_stack[NTHREADS][PTHREAD_STACK_MIN*2];

pthread_t thread[NTHREADS];

void *pthread_entry1( void *arg);
void *pthread_entry2( void *arg);
void *pthread_entry3( void *arg);

void *(*pthread_entry[NTHREADS])(void *) =
{
    pthread_entry1,
    pthread_entry2,
    pthread_entry3
};

//--------------------------------------------------------------------------

volatile cyg_bool cancel_handler1_called = false;
volatile cyg_bool cancel_handler2_called = false;
volatile cyg_bool cancel_handler3_called = false;
volatile cyg_bool thread_ready[NTHREADS];

//--------------------------------------------------------------------------

void cancel_handler1( void * arg )
{
    CYG_TEST_INFO( "cancel_handler1 called" );

    CYG_TEST_CHECK( (long)arg == 0x12340000, "cancel_handler1: bad arg value");
    
    cancel_handler1_called = true;
}

//--------------------------------------------------------------------------

void cancel_handler2( void * arg )
{
    CYG_TEST_INFO( "cancel_handler2 called" );

    CYG_TEST_CHECK( (long)arg == 0xFFFF1111, "cancel_handler2: bad arg value");
    
    cancel_handler2_called = true;    
}

//--------------------------------------------------------------------------

void cancel_handler3( void * arg )
{
    CYG_TEST_INFO( "cancel_handler3 called" );

    CYG_TEST_CHECK( (long)arg == 0x12340001, "cancel_handler3: bad arg value");    
    
    cancel_handler3_called = true;
}

//--------------------------------------------------------------------------

void function1(void)
{

    pthread_cleanup_push( cancel_handler2, (void *)0xFFFF1111 );

    for(;;)
    {
        sched_yield();
        pthread_testcancel();
    }

    pthread_cleanup_pop( 0 );
}

//--------------------------------------------------------------------------

void *pthread_entry1( void *arg)
{
    int retval = 1;

    CYG_TEST_INFO( "pthread_entry1 entered");

    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );
    
    pthread_cleanup_push( cancel_handler1, arg );

    thread_ready[0] = true;

    function1();    
    
    pthread_cleanup_pop( 0 );

    pthread_exit( (void *)retval );
}

//--------------------------------------------------------------------------

void *pthread_entry2( void *arg)
{
    int retval = 1;

    CYG_TEST_INFO( "pthread_entry2 entered");

    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    
    pthread_cleanup_push( cancel_handler3, arg );

    thread_ready[1] = true;

    for(;;) sched_yield();
    
    pthread_cleanup_pop( 0 );

    pthread_exit( (void *)retval );
}

//--------------------------------------------------------------------------

void *pthread_entry3( void *arg)
{
    int retval = 1;

    CYG_TEST_INFO( "pthread_entry3 entered");

    pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );
    
    thread_ready[2] = true;

    // stop in a cancellation point
    sleep( 99999 );
    
    pthread_exit( (void *)retval );
}

//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int i, j;
    int ret;
    void *retval[NTHREADS];

    CYG_TEST_INIT();

    // Create test threads
    for( i = 0; i < NTHREADS; i++ )
    {
        pthread_attr_t attr;
        pthread_attr_init( &attr );

        pthread_attr_setstackaddr( &attr, (void *)&thread_stack[i][sizeof(thread_stack[i])] );
        pthread_attr_setstacksize( &attr, sizeof(thread_stack[i]) );

        ret = pthread_create( &thread[i],
                              &attr,
                              pthread_entry[i],
                              (void *)(0x12340000+i));
        CYG_TEST_CHECK( ret == 0, "pthread_create() returned error");
    }

    // Let the threads get going    
    for ( i = 0; i < NTHREADS ; i++ ) {
        while ( thread_ready[i] == false )
            sched_yield();
    }

    // Now wait a bit to be sure that the other threads have reached
    // their cancellation points.
    for ( j = 0; j < 20 ; j++ )
        sched_yield();
    
    // Now cancel them
    for( i = 0; i < NTHREADS; i++ )    
        pthread_cancel( thread[i] );
        
    // Now join with threads
    for( i = 0; i < NTHREADS; i++ )
        pthread_join( thread[i], &retval[i] );


    // check retvals
    for( i = 0; i < NTHREADS; i++ )
        CYG_TEST_CHECK( retval[i] == PTHREAD_CANCELED,
                        "thread didn't exit with PTHREAD_CANCELED" );

    CYG_TEST_CHECK( cancel_handler1_called, "cancel_handler1 not called" );
    CYG_TEST_CHECK( cancel_handler2_called, "cancel_handler2 not called" );
    CYG_TEST_CHECK( cancel_handler3_called, "cancel_handler3 not called" );

    CYG_TEST_PASS_FINISH( "pthread3" );
        
}

#endif

//--------------------------------------------------------------------------
// end of pthread3.c
