//===========================================================================
//
//      test8.c
//
//      uITRON "C" test program eight
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-10-12
// Purpose:     uITRON API testing
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al
#include <cyg/infra/testcase.h>         // testing infrastructure

#ifdef CYGPKG_UITRON                    // we DO want the uITRON package

#ifdef CYGSEM_KERNEL_SCHED_MLQUEUE      // we DO want prioritized threads

#ifdef CYGFUN_KERNEL_THREADS_TIMER      // we DO want timout-able calls

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK     // we DO want the realtime clock

// we're OK if it's C++ or neither of those two is defined:
#if defined( __cplusplus ) || \
    (!defined( CYGIMP_UITRON_INLINE_FUNCS ) && \
     !defined( CYGIMP_UITRON_CPP_OUTLINE_FUNCS) )

// =================== TEST CONFIGURATION ===================
#if \
    /* test configuration for enough tasks */                      \
    (CYGNUM_UITRON_TASKS >= 4)                                  && \
    (CYGNUM_UITRON_TASKS < 90)                                  && \
    (CYGNUM_UITRON_START_TASKS == 1)                            && \
    ( !defined(CYGPKG_UITRON_TASKS_CREATE_DELETE) ||               \
      CYGNUM_UITRON_TASKS_INITIALLY >= 4             )          && \
                                                                   \
    /* test configuration for enough semaphores */                 \
    defined( CYGPKG_UITRON_SEMAS )                              && \
    (CYGNUM_UITRON_SEMAS >= 3)                                  && \
    (CYGNUM_UITRON_SEMAS < 90)                                  && \
    ( !defined(CYGPKG_UITRON_SEMAS_CREATE_DELETE) ||               \
      CYGNUM_UITRON_SEMAS_INITIALLY >= 3             )          && \
                                                                   \
    /* the end of the large #if statement */                       \
    1 

// ============================ END ============================



#include <cyg/compat/uitron/uit_func.h> // uITRON

externC void
cyg_package_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO( "Calling cyg_uitron_start()" );
    cyg_uitron_start();
}

volatile int intercount = 0;
INT scratch;

void newtask( unsigned int arg );
void task2( unsigned int arg );
void task3( unsigned int arg );
void task4( unsigned int arg );

T_CTSK t_ctsk = { NULL, 0, (FP)&newtask, 1, CYGNUM_UITRON_STACK_SIZE };
T_RTSK t_rtsk;

void task1( unsigned int arg )
{
    ER ercd;

    CYG_TEST_INFO( "Task 1 running" );

    // change us to prio 3 for flexibility
    ercd = chg_pri( 0, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
    // first, check that we can delete a task:
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_tsk( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_tsk bad ercd !E_ID" );
    ercd = del_tsk( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_tsk bad ercd !E_ID" );
    ercd = cre_tsk( -6, &t_ctsk );
    CYG_TEST_CHECK( E_ID == ercd, "cre_tsk bad ercd !E_ID" );
    ercd = cre_tsk( 99, &t_ctsk );
    CYG_TEST_CHECK( E_ID == ercd, "cre_tsk bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    ercd = cre_tsk( 2, &t_ctsk );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_tsk bad ercd !E_OBJ" );
    // try a pre-existing object - ourselves!
    ercd = cre_tsk( 1, &t_ctsk );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_tsk bad ercd !E_OBJ" );
    // try deleting an active task
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = sta_tsk( 2, 22222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = chg_pri( 2, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    // Task 2 is now ready-to-run, lower prio than us
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "del_tsk bad ercd !E_OBJ" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    // Task 2 is now sleeping
    CYG_TEST_CHECK( 1 == intercount, "bad intercount !1" );
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "del_tsk bad ercd !E_OBJ" );
    // try deleting a running task - ourselves!
    ercd = del_tsk( 1 );
    CYG_TEST_CHECK( E_OBJ == ercd, "del_tsk bad ercd !E_OBJ" );
    // terminate task 2; should then be OK to delete it
    ercd = ter_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
    CYG_TEST_CHECK( 1 == intercount, "bad intercount !1" );
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_tsk bad ercd" );
    CYG_TEST_CHECK( 1 == intercount, "bad intercount !1" );
    // and check it is deleted
    ercd = sta_tsk( 2, 99 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sta_tsk bad ercd !E_NOEXS" );
    ercd = ter_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ter_tsk bad ercd !E_NOEXS" );
    ercd = chg_pri( 2, 6 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "chg_pri bad ercd !E_NOEXS" );
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "rel_wai bad ercd !E_NOEXS" );
    ercd = sus_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sus_tsk bad ercd !E_NOEXS" );
    ercd = rsm_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "rsm_tsk bad ercd !E_NOEXS" );
    ercd = frsm_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "frsm_tsk bad ercd !E_NOEXS" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "wup_tsk bad ercd !E_NOEXS" );
    ercd = can_wup( &scratch, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "can_wup bad ercd !E_NOEXS" );
    ercd = ref_tsk( &t_rtsk, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_tsk bad ercd !E_NOEXS" );
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "del_tsk bad ercd !E_NOEXS" );
    // recreate task2, with the same function
    t_ctsk.task = (FP)&task2;
    t_ctsk.itskpri = 7;
    ercd = cre_tsk( 2, &t_ctsk );
    CYG_TEST_CHECK( E_OK == ercd, "cre_tsk bad ercd" );
    ercd = ref_tsk( &t_rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( 7 == t_rtsk.tskpri, "Bad tskpri in new task2 !7" );
    CYG_TEST_CHECK( TTS_DMT == t_rtsk.tskstat,
                    "Bad tskstat in new task2 !TTS_DMT" );
    CYG_TEST_CHECK( 1 == intercount, "bad intercount !1" );
    // now start the task and do the same lot again...
    ercd = cre_tsk( 2, &t_ctsk );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_tsk bad ercd !E_OBJ" );
    // try deleting an active task
    ercd = sta_tsk( 2, 22222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    // Task 2 is now ready-to-run, lower prio than us
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "del_tsk bad ercd !E_OBJ" );
    CYG_TEST_CHECK( 1 == intercount, "bad intercount !1" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 2 == intercount, "bad intercount !2" );
    // Task 2 is now sleeping
    ercd = ref_tsk( &t_rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( 7 == t_rtsk.tskpri, "Bad tskpri in new task2 !7" );
    CYG_TEST_CHECK( TTS_WAI == t_rtsk.tskstat,
                    "Bad tskstat in new task2 !TTS_WAI" );
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "del_tsk bad ercd !E_OBJ" );
    // up its priority
    ercd = chg_pri( 2, 1 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    // awaken task 2; it will then exit-and-delete itself:
    CYG_TEST_CHECK( 2 == intercount, "bad intercount !2" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "bad intercount !3" );
    // and check it is deleted
    ercd = sta_tsk( 2, 99 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sta_tsk bad ercd !E_NOEXS" );
    CYG_TEST_CHECK( 3 == intercount, "bad intercount !3" );
    ercd = ter_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ter_tsk bad ercd !E_NOEXS" );
    ercd = chg_pri( 2, 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "chg_pri bad ercd !E_NOEXS" );
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "rel_wai bad ercd !E_NOEXS" );
    ercd = sus_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sus_tsk bad ercd !E_NOEXS" );
    ercd = rsm_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "rsm_tsk bad ercd !E_NOEXS" );
    ercd = frsm_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "frsm_tsk bad ercd !E_NOEXS" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "wup_tsk bad ercd !E_NOEXS" );
    ercd = can_wup( &scratch, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "can_wup bad ercd !E_NOEXS" );
    ercd = ref_tsk( &t_rtsk, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_tsk bad ercd !E_NOEXS" );
    CYG_TEST_CHECK( 3 == intercount, "bad intercount !3" );
    ercd = del_tsk( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "del_tsk bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_tsk( 2, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_tsk bad ercd !E_PAR" );
#endif
    ercd = cre_tsk( 2, NADR );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_tsk bad ercd !E_PAR" );
    t_ctsk.stksz = 0x40000000;
    ercd = cre_tsk( 2, &t_ctsk );
    CYG_TEST_CHECK( E_NOMEM == ercd, "cre_tsk bad ercd !E_NOMEM" );
    t_ctsk.stksz = CYGNUM_UITRON_STACK_SIZE;
    CYG_TEST_CHECK( 3 == intercount, "bad intercount !3" );
#endif // we can test bad param error returns

    ercd = del_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_tsk bad ercd" );
    t_ctsk.task = (FP)&task4;
    t_ctsk.itskpri = 9;
    ercd = cre_tsk( 3, &t_ctsk );
    CYG_TEST_CHECK( E_OK == ercd, "cre_tsk bad ercd" );
    // check we can delete it again immediately
    ercd = del_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_tsk bad ercd" );
    ercd = ref_tsk( &t_rtsk, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_tsk bad ercd !E_NOEXS" );
    t_ctsk.task = (FP)&newtask;
    t_ctsk.itskpri = 1;
    ercd = cre_tsk( 3, &t_ctsk );
    CYG_TEST_CHECK( E_OK == ercd, "cre_tsk bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "bad intercount !3" );
    ercd = sta_tsk( 3, 999 );
    CYG_TEST_CHECK( E_OK == ercd, "cre_tsk bad ercd" );
    // it should have run now, and exited
    CYG_TEST_CHECK( 5 == intercount, "bad intercount !5" );
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    // and check that it will just run again...
    ercd = sta_tsk( 3, 999 );
    CYG_TEST_CHECK( E_OK == ercd, "cre_tsk bad ercd" );
    // it should have run now, and exited
    CYG_TEST_CHECK( 7 == intercount, "bad intercount !7" );
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    // all done.

    CYG_TEST_PASS("create/delete tasks");

    // all done
    CYG_TEST_EXIT( "All done" );
#else // ! CYGPKG_UITRON_TASKS_CREATE_DELETE
    CYG_TEST_NA( "Tasks do not have create/delete enabled" );
#endif // ! CYGPKG_UITRON_TASKS_CREATE_DELETE
    ext_tsk();
}



void newtask( unsigned int arg )
{
    ER ercd;
    int i;
    CYG_TEST_INFO( "Newtask running" );
    CYG_TEST_CHECK( 999 == arg, "Bad arg to newtask() !999" );
    ercd = get_tid( &i );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 3 == i, "tid not 3" );
    intercount++;
    ercd = sig_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    intercount++;
    // and just return
}

void task2( unsigned int arg )
{
    ER ercd;
    int i;
    CYG_TEST_INFO( "Task 2 running" );
    ercd = get_tid( &i );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 2 == i, "tid not 2" );
    if ( 22222 != arg )
        CYG_TEST_FAIL( "Task 2 arg not 22222" );

    intercount++;

    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );

    intercount++;

    exd_tsk(); // if we are not killed first

    intercount++; // shouldn't happen
}

void task3( unsigned int arg )
{
    CYG_TEST_FAIL( "How come I'm being run?" );
}

void task4( unsigned int arg )
{
    CYG_TEST_FAIL( "How come I'm being run?" );
}

#else // not enough (or too many) uITRON objects configured in
#define N_A_MSG "not enough uITRON objects to run test"
#endif // not enough (or too many) uITRON objects configured in
#else  // not C++ and some C++ specific options enabled
#define N_A_MSG "C++ specific options selected but this is C"
#endif  // not C++ and some C++ specific options enabled
#else // ! CYGVAR_KERNEL_COUNTERS_CLOCK   - can't test without it
#define N_A_MSG "no CYGVAR_KERNEL_COUNTERS_CLOCK"
#endif // ! CYGVAR_KERNEL_COUNTERS_CLOCK  - can't test without it
#else  // ! CYGFUN_KERNEL_THREADS_TIMER   - can't test without it
#define N_A_MSG "no CYGFUN_KERNEL_THREADS_TIMER"
#endif // ! CYGFUN_KERNEL_THREADS_TIMER   - can't test without it
#else  // ! CYGIMP_THREAD_PRIORITY        - can't test without it
#define N_A_MSG "no CYGSEM_KERNEL_SCHED_MLQUEUE"
#endif // ! CYGSEM_KERNEL_SCHED_MLQUEUE   - can't test without it
#else  // ! CYGPKG_UITRON
#define N_A_MSG "uITRON Compatibility layer disabled"
#endif // CYGPKG_UITRON

#ifdef N_A_MSG
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG defined ie. we are N/A.

// EOF test8.c
