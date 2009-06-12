/*==========================================================================
//
//        kflag1.cxx
//
//        Kernel C API Flag test 1
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
// Author:        dsm
// Contributors:  hmt
// Date:          1998-10-19
// Description:   Tests basic flag functionality.
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

static cyg_flag_t f0, f1;
#ifdef CYGFUN_KERNEL_THREADS_TIMER
static cyg_flag_t f2;
#endif

static volatile cyg_ucount8 q = 0;
#define FIRST_THREAD_WAIT_TIME   5
#define SECOND_THREAD_WAIT_TIME 10
#define THIRD_THREAD_WAIT_TIME  20

static void entry0( cyg_addrword_t data )
{
    CYG_TEST_INFO("Testing cyg_flag_setbits() and cyg_flag_maskbits()");
    CYG_TEST_CHECK(0==cyg_flag_peek( &f0 ), "flag not initialized properly");
    cyg_flag_setbits( &f0, 0x1);
    CYG_TEST_CHECK(1==cyg_flag_peek( &f0 ), "setbits");
    cyg_flag_setbits( &f0, 0x3);
    CYG_TEST_CHECK(3==cyg_flag_peek( &f0 ), "setbits");
    cyg_flag_maskbits( &f0, ~0x5);
    CYG_TEST_CHECK(2==cyg_flag_peek( &f0 ), "maskbits");
    cyg_flag_setbits( &f0, ~0 );
    CYG_TEST_CHECK(~0u==cyg_flag_peek( &f0 ), "setbits all set");
    cyg_flag_maskbits( &f0, 0 );
    CYG_TEST_CHECK(0==cyg_flag_peek( &f0 ), "maskbits all clear");
    CYG_TEST_CHECK(0==q++, "bad synchronization");

    CYG_TEST_INFO("Testing cyg_flag_wait()");
    cyg_flag_setbits( &f1, 0x4);
    CYG_TEST_CHECK(0x4==cyg_flag_peek( &f1 ), "setbits");
    CYG_TEST_CHECK(1==q++, "bad synchronization");
    cyg_flag_setbits( &f1, 0x18);                   // wake t1
    cyg_flag_wait( &f1, 0x11, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR);
    CYG_TEST_CHECK(0==cyg_flag_peek( &f1 ), "flag value wrong");
    CYG_TEST_CHECK(3==q++, "bad synchronization");
    cyg_flag_setbits( &f0, 0x2);                    // wake t1
    cyg_flag_wait( &f1, 0x10, CYG_FLAG_WAITMODE_AND );
    cyg_flag_setbits( &f0, 0x1);                    // wake t1

    cyg_flag_wait( &f1, 0x11, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR);

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_flag_wait( &f2, 0x2, CYG_FLAG_WAITMODE_OR);    
    CYG_TEST_CHECK(20==q,"bad synchronization");
    cyg_flag_timed_wait( &f2, 0x10, CYG_FLAG_WAITMODE_AND,
                         cyg_current_time()+THIRD_THREAD_WAIT_TIME);
    CYG_TEST_CHECK(21==q++,"bad synchronization");
#endif
    cyg_flag_wait( &f0, 1, CYG_FLAG_WAITMODE_OR);

    CYG_TEST_FAIL_FINISH("Not reached");
}

static void entry1( cyg_addrword_t data )
{
    cyg_flag_wait( &f1, 0xc, CYG_FLAG_WAITMODE_AND);
    CYG_TEST_CHECK(2==q++, "bad synchronization");
    CYG_TEST_CHECK(0x1c==cyg_flag_peek( &f1 ), "flag value wrong");
    cyg_flag_setbits( &f1, 0x1);                    // wake t0
    cyg_flag_wait( &f0, 0x3, CYG_FLAG_WAITMODE_OR);
    CYG_TEST_CHECK(4==q++, "bad synchronization");
    CYG_TEST_CHECK(2==cyg_flag_peek( &f0 ), "flag value wrong");
    
    cyg_flag_setbits( &f1, 0xf0);                   // wake t0,t2
    cyg_flag_wait( &f0, 0x5, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR);
    CYG_TEST_CHECK(0==cyg_flag_peek( &f0 ), "flag value wrong");
    CYG_TEST_CHECK(0xf0==cyg_flag_peek( &f1 ), "flag value wrong");
    CYG_TEST_CHECK(5==q++, "bad synchronization");
    cyg_flag_maskbits( &f1, 0 );
    CYG_TEST_CHECK(0==cyg_flag_peek( &f1 ), "flag value wrong");
    
    CYG_TEST_INFO("Testing cyg_flag_poll()");
    cyg_flag_setbits( &f0, 0x55);
    CYG_TEST_CHECK(0x55==cyg_flag_peek( &f0 ), "flag value wrong");
    CYG_TEST_CHECK(0x55==cyg_flag_poll( &f0, 0x3, CYG_FLAG_WAITMODE_OR),"bad poll() return");
    CYG_TEST_CHECK(0==cyg_flag_poll( &f0, 0xf, CYG_FLAG_WAITMODE_AND),"poll()");
    CYG_TEST_CHECK(0==cyg_flag_poll( &f0, 0xa, CYG_FLAG_WAITMODE_OR),"poll()");
    CYG_TEST_CHECK(0x55==cyg_flag_peek( &f0 ), "flag value wrong");
    CYG_TEST_CHECK(0x55==cyg_flag_poll( &f0, 0xf, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR),"poll");
    CYG_TEST_CHECK(0x0==cyg_flag_peek( &f0 ), "flag value wrong");
    cyg_flag_setbits( &f0, 0x50);
    CYG_TEST_CHECK(0x50==cyg_flag_poll( &f0, 0x10, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR),"poll");
    CYG_TEST_CHECK(0x0==cyg_flag_peek( &f0 ), "flag value wrong");

    CYG_TEST_INFO("Testing cyg_flag_waiting()");
    cyg_flag_maskbits( &f0, 0 );
    CYG_TEST_CHECK(!cyg_flag_waiting( &f0 ), "waiting()");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_thread_delay( 10 ); // allow other threads to reach wait on f1
    CYG_TEST_CHECK(cyg_flag_waiting( &f1 ), "waiting() not true");
    cyg_flag_setbits( &f1, ~0 );                   // wake one of t0,t2
    CYG_TEST_CHECK(cyg_flag_waiting( &f1 ),"waiting() not true");
#else
    cyg_flag_setbits( &f1, 0x11);                   // wake one of t0,t2
#endif
    cyg_flag_setbits( &f1, 0x11);                   // wake other of t0,t2    
    CYG_TEST_CHECK(!cyg_flag_waiting( &f1 ),"waiting not false");
    CYG_TEST_CHECK(0x0==cyg_flag_peek( &f1 ), "flag value wrong");

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    CYG_TEST_INFO("Testing cyg_flag_timed_wait()");
    q=20;
    cyg_flag_setbits( &f2, 0x2);                    // synchronize with t0,t2
    CYG_TEST_CHECK(20==q,"bad synchronization");
    cyg_flag_timed_wait( &f2, 0x20, CYG_FLAG_WAITMODE_AND,
                         cyg_current_time()+SECOND_THREAD_WAIT_TIME);
    CYG_TEST_CHECK(22==q++,"bad synchronization");
#endif

    CYG_TEST_PASS_FINISH("Kernel C API Flag 1 OK");
}

static void entry2( cyg_addrword_t data )
{
    cyg_flag_wait( &f1, 0x60, CYG_FLAG_WAITMODE_OR);
    cyg_flag_setbits( &f0, 0x4);

    cyg_flag_wait( &f1, 0x11, CYG_FLAG_WAITMODE_AND | CYG_FLAG_WAITMODE_CLR);
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_flag_wait( &f2, 0x2, CYG_FLAG_WAITMODE_OR);
    CYG_TEST_CHECK(20==q,"bad synchronization");
    CYG_TEST_CHECK(0==cyg_flag_timed_wait( &f2, 0x40, CYG_FLAG_WAITMODE_AND,
                                           cyg_current_time()+FIRST_THREAD_WAIT_TIME),
                   "timed wait() wrong");
    CYG_TEST_CHECK(20==q++,"bad synchronization");
    // Now wake t0 before it times out
    cyg_flag_setbits( &f2, 0x10);
#endif
    cyg_flag_wait( &f0, 1, CYG_FLAG_WAITMODE_OR);    
    
    CYG_TEST_FAIL_FINISH("Not reached");
}

void kflag1_main( void )
{
    CYG_TEST_INIT();

    cyg_flag_init( &f0 );
    cyg_flag_init( &f1 );
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_flag_init( &f2 );
#endif

    cyg_thread_create( 1, entry0 , (cyg_addrword_t)0, "kflag1-0", 
        (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);

    cyg_thread_create( 1, entry1 , (cyg_addrword_t)1, "kflag1-1", 
        (void *)stack[1], STACKSIZE, &thread[1], &thread_obj[1]);
    cyg_thread_resume(thread[1]);

    cyg_thread_create( 1, entry2 , (cyg_addrword_t)2, "kflag1-2", 
        (void *)stack[2], STACKSIZE, &thread[2], &thread_obj[2]);
    cyg_thread_resume(thread[2]);

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

externC void
cyg_start( void )
{
    kflag1_main();
}

#else /* def CYGFUN_KERNEL_API_C */
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA("Kernel C API layer disabled");
}
#endif /* def CYGFUN_KERNEL_API_C */

// EOF flag1.cxx
