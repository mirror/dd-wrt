//==========================================================================
//
//        pthread2.cxx
//
//        POSIX pthread test 2 - per-thread data
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
// Description:   Tests POSIX per-thread data.
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

//--------------------------------------------------------------------------
// Thread stack.

char thread_stack[2][PTHREAD_STACK_MIN*2];

pthread_t thread[2];

pthread_key_t key;

//--------------------------------------------------------------------------

void key_destructor( void *val )
{
    int ret;

    CYG_TEST_INFO( "key destructor called" );

    if( (long)val == 0xAAAAAAAA )
    {
        ret = pthread_setspecific( key, NULL );
        CYG_TEST_CHECK( ret == 0, "pthread_setspecific() returned error");
    }
    else
    {
        ret = pthread_setspecific( key, (void *)0xAAAAAAAA );
        CYG_TEST_CHECK( ret == 0, "pthread_setspecific() returned error");
    }
}

//--------------------------------------------------------------------------

void *pthread_entry( void *arg)
{
    int ret;
    int retval = 1;
    void *val;
    
    CYG_TEST_INFO( "Thread running" );

    ret = pthread_setspecific( key, arg );
    CYG_TEST_CHECK( ret == 0, "pthread_setspecific() returned error");
 
    val = pthread_getspecific( key );
    CYG_TEST_CHECK( val == arg, "pthread_getspecific() did not return expected value");
    if( val != arg ) retval = 0;
    
    sched_yield();

    val = pthread_getspecific( key );
    CYG_TEST_CHECK( val == arg, "pthread_getspecific() did not return expected value");
    if( val != arg ) retval = 0;
    
    pthread_exit( (void *)retval );
}

//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int i;
    int ret;
    void *retval[2];

    CYG_TEST_INIT();

    // allocate data key
    ret = pthread_key_create( &key, key_destructor );
    CYG_TEST_CHECK( ret == 0, "pthread_key_create() returned error");
    
    // Create test threads
    for( i = 0; i < 2; i++ )
    {
        pthread_attr_t attr;
        pthread_attr_init( &attr );

        pthread_attr_setstackaddr( &attr, (void *)&thread_stack[i][sizeof(thread_stack[i])] );
        pthread_attr_setstacksize( &attr, sizeof(thread_stack[i]) );

        ret = pthread_create( &thread[i],
                              &attr,
                              pthread_entry,
                              (void *)(0x12340000+i));
        CYG_TEST_CHECK( ret == 0, "pthread_create() returned error");
    }
    
    // Now join with threads
    for( i = 0; i < 2; i++ )
        pthread_join( thread[i], &retval[i] );


    ret = pthread_key_delete( key );
    CYG_TEST_CHECK( ret == 0, "pthread_key_delete() returned error");
            
    // check retvals

    for( i = 0; i < 2; i++ )
        if( !(long)retval[0] )
            CYG_TEST_FAIL_FINISH( "pthread2" );
                
    CYG_TEST_PASS_FINISH( "pthread2" );
        
}

#endif

//--------------------------------------------------------------------------
// end of pthread2.c
