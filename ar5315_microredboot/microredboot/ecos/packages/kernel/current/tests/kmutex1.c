/*=================================================================
//
//        kmutex1.c
//
//        Kernel C API Mutex test 1
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
// Author(s):     dsm
// Contributors:    dsm
// Date:          1998-03-23
// Description:   Tests basic mutex functionality.
// Omissions:     Timed wait.
//####DESCRIPTIONEND####
*/

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

#ifdef CYGFUN_KERNEL_API_C

#include "testaux.h"

#define NTHREADS 3
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_handle_t thread[NTHREADS];

static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];


static cyg_mutex_t m0, m1;
static cyg_cond_t cvar0, cvar1, cvar2;

static cyg_ucount8 m0d=0, m1d=0;

static void finish( cyg_ucount8 t )
{
    cyg_mutex_lock( &m1 ); {
        m1d |= 1<<t;
        if( 0x7 == m1d )
            CYG_TEST_PASS_FINISH("Kernel C API Mutex 1 OK");
        cyg_cond_wait( &cvar2 );
    } /* cyg_mutex_unlock( &m1 ); */
    CYG_TEST_FAIL_FINISH("Not reached");    
}

static void entry0( cyg_addrword_t data )
{
    cyg_mutex_lock( &m0 ); {
        CHECK( ! cyg_mutex_trylock( &m0 ) );
        cyg_mutex_lock( &m1 ); {
            CHECK( ! cyg_mutex_trylock( &m0 ) );            
        } cyg_mutex_unlock( &m1 );
    } cyg_mutex_unlock( &m0 );

    cyg_mutex_lock( &m0 ); {
        while ( 0 == m0d )
            cyg_cond_wait( &cvar0 );
        CHECK( 1 == m0d++ );
        cyg_cond_signal( &cvar0 );
        while ( 4 != m0d )
            cyg_cond_wait( &cvar1 );
        CHECK( 4 == m0d );
    } cyg_mutex_unlock( &m0 );

    finish( (cyg_ucount8)data );
}

static void entry1( cyg_addrword_t data )
{
    cyg_mutex_lock( &m0 ); {
        CHECK( cyg_mutex_trylock( &m1 ) ); {
        } cyg_mutex_unlock( &m1 );
    } cyg_mutex_unlock( &m0 );

    cyg_mutex_lock( &m0 ); {
        CHECK( 0 == m0d++ );
        cyg_cond_broadcast( &cvar0 );
    } cyg_mutex_unlock( &m0 );
    
    cyg_mutex_lock( &m0 ); {
        while( 1 == m0d )
            cyg_cond_wait( &cvar0 );
        CHECK( 2 == m0d++ );
        cyg_cond_signal( &cvar0 );
        while( 3 == m0d )
            cyg_cond_wait( &cvar1 );
    } cyg_mutex_unlock( &m0 );

    finish( (cyg_ucount8)data );
}

static void entry2( cyg_addrword_t data )
{
    cyg_mutex_lock( &m0 ); {
        while( 3 != m0d ) {
            cyg_cond_wait( &cvar0 );
        }
        CHECK( 3 == m0d++ );
        cyg_cond_broadcast( &cvar1 );
    } cyg_mutex_unlock( &m0 );

    finish( (cyg_ucount8)data );
}

void kmutex1_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0 , (cyg_addrword_t)0, "kmutex1-0",
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_thread_create(4, entry1 , (cyg_addrword_t)1, "kmutex1-1",
        (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
    cyg_thread_resume(thread[1]);

    cyg_thread_create(4, entry2 , (cyg_addrword_t)2, "kmutex1-2",
        (void *)stack[2], STACKSIZE, &thread[2], &thread_obj[2]);
    cyg_thread_resume(thread[2]);

    cyg_mutex_init( &m0 );
    cyg_mutex_init( &m1 );

    cyg_cond_init( &cvar0, &m0 );
    cyg_cond_init( &cvar1, &m0 );
    cyg_cond_init( &cvar2, &m1 );

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{ 
    kmutex1_main();
}

#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

/* EOF kmutex1.c */
