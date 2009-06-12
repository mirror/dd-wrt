/*=================================================================
//
//        klock.c
//
//        Kernel lock test
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
// Date:          1998-03-18
// Description:   Tests some basic thread functions.
//####DESCRIPTIONEND####
*/
//==========================================================================

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>

//==========================================================================

#ifdef CYGFUN_KERNEL_API_C

#if (CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES == 0)

//==========================================================================

#include "testaux.h"

#include <cyg/hal/hal_arch.h>           // for CYGNUM_HAL_STACK_SIZE_TYPICAL

//==========================================================================

#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
#define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
#else
#define STACKSIZE 2000
#endif

//==========================================================================

static char stack[2][STACKSIZE];

static cyg_thread thread[2];

static cyg_handle_t pt0,pt1;

static cyg_mutex_t mx;
static cyg_cond_t cv;
static cyg_sem_t sem;
static cyg_flag_t fl;
static cyg_mbox mbox;
static cyg_handle_t mbh;

volatile static int thread0_state = 0;
volatile static int thread1_state = 0;

//==========================================================================

static void entry0( cyg_addrword_t data )
{
    CHECK( 222 == (int)data );

    // Do everything with the scheduler locked.
    cyg_scheduler_lock();

    // --------------------------------------------------
    // Mutex test
    
    cyg_mutex_lock( &mx );
    thread0_state = 1;
    
    // Get thread 2 running.
    cyg_thread_resume(pt1);
    thread0_state = 2;

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT
    cyg_cond_wait( &cv );
    thread0_state = 3;

    while( thread1_state < 2 ) cyg_thread_yield();
    
    cyg_cond_broadcast( &cv );
    thread0_state = 4;
#endif

    cyg_mutex_unlock( &mx );
    thread0_state = 5;


    // --------------------------------------------------
    // Semaphore test
    
    cyg_semaphore_wait( &sem );
    thread0_state = 6;

    cyg_semaphore_post( &sem );
    thread0_state = 7;

    while( thread1_state < 7 ) cyg_thread_yield();
    
    // --------------------------------------------------
    // Flags test

    cyg_flag_wait( &fl, 1, CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR);
    thread0_state = 8;    

    cyg_flag_setbits( &fl, 2 );
    thread0_state = 9;    
    
    // --------------------------------------------------
    // Message box test

#ifdef  CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    {
      void *mbret;

      mbret = cyg_mbox_get( mbh );
      CYG_TEST_CHECK( mbret == (void *)0xAAAAAAAA , "bad result from cyg_mbox_timed_get()");
      thread0_state = 10;
      
      cyg_mbox_put( mbh, (void *)0xBBBBBBBB );
      thread0_state = 11;
    }
#endif
    // --------------------------------------------------    
    
    thread0_state = 999;

    cyg_thread_yield();
    cyg_thread_yield();
    cyg_thread_yield();
    
    CYG_TEST_CHECK( thread0_state == 999, "thread 0 not in exit state");
    CYG_TEST_CHECK( thread1_state == 999, "thread 1 not in exit state");
    CYG_TEST_PASS_FINISH("Kernel lock test OK");    
}

//==========================================================================

static void entry1( cyg_addrword_t data )
{
    cyg_bool res;
    
    CHECK( 333 == (int)data );

    // Do everything with the scheduler locked.
    cyg_scheduler_lock();

    // --------------------------------------------------
    // Mutex test
#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT
    cyg_mutex_lock( &mx );
    thread1_state = 1;

    while( thread0_state < 2 ) cyg_thread_yield();
    
    cyg_cond_signal( &cv );
    thread1_state = 2;

    res = cyg_cond_timed_wait( &cv, cyg_current_time()+10 );
    CYG_TEST_CHECK( res , "FALSE result from cyg_cond_timed_wait()" );
    thread1_state = 3;

    cyg_mutex_unlock( &mx );
    thread1_state = 4;
#endif

    // --------------------------------------------------
    // Semaphore test
    
    while( thread0_state < 5 ) cyg_thread_yield();
    
    cyg_semaphore_post( &sem );
    thread1_state = 5;

    while( thread0_state < 6 ) cyg_thread_yield();
    thread1_state = 6;
    
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    res = cyg_semaphore_timed_wait( &sem, cyg_current_time()+10 );
#else
    res = cyg_semaphore_wait( &sem );
#endif
    CYG_TEST_CHECK( res , "FALSE result from cyg_semaphore[_timed]_wait()" );
    thread1_state = 7;

    // --------------------------------------------------
    // Flags test

    cyg_flag_setbits( &fl, 1 );
    thread1_state = 8;

#ifdef CYGFUN_KERNEL_THREADS_TIMER
    cyg_flag_timed_wait( &fl, 2, CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR,
                         cyg_current_time()+10 );
#else
    cyg_flag_wait( &fl, 2, CYG_FLAG_WAITMODE_OR|CYG_FLAG_WAITMODE_CLR );
#endif
    thread1_state = 9;
    
    // --------------------------------------------------
    // Message box test
#ifdef  CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    {
      void *mbret;
#ifdef CYGFUN_KERNEL_THREADS_TIMER
      cyg_mbox_timed_put( mbh, (void *)0xAAAAAAAA, cyg_current_time()+10 );
#else
      cyg_mbox_put( mbh, (void *)0xAAAAAAAA );
#endif
      thread1_state = 10;

#ifdef CYGFUN_KERNEL_THREADS_TIMER
      mbret = cyg_mbox_timed_get( mbh, cyg_current_time()+10);
#else
      mbret = cyg_mbox_get( mbh );
#endif
      CYG_TEST_CHECK( mbret == (void *)0xBBBBBBBB , "bad result from cyg_mbox[_timed]_get()");
      thread1_state = 9;
    }
#endif    
    // --------------------------------------------------
    
    thread1_state = 999;
    cyg_thread_exit();
}

//==========================================================================

void kthread1_main( void )
{
    CYG_TEST_INIT();

    cyg_thread_create(4, entry0, (cyg_addrword_t)222, "kthread1-0",
        (void *)stack[0], STACKSIZE, &pt0, &thread[0] );
    cyg_thread_create(4, entry1, (cyg_addrword_t)333, "kthread1-1",
        (void *)stack[1], STACKSIZE, &pt1, &thread[1] );

    // Init all the objects

    cyg_mutex_init( &mx );
    cyg_cond_init( &cv, &mx );
    cyg_semaphore_init( &sem, 0 );
    cyg_flag_init( &fl );
    cyg_mbox_create( &mbh, &mbox );
    
    cyg_thread_resume(pt0);

    cyg_scheduler_start();

    CYG_TEST_FAIL_FINISH("Not reached");
}

//==========================================================================

externC void
cyg_start( void )
{
    kthread1_main();
}

//==========================================================================

#else /* CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES == 0 */
# define NA_MSG "Schedule has unique priorities"
#endif

#else /* def CYGFUN_KERNEL_API_C */
# define NA_MSG "Kernel C API layer disabled"
#endif

#ifdef NA_MSG
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA(NA_MSG);
}
#endif

//==========================================================================
/* EOF klock.c */
