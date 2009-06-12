//===========================================================================
//
//      testcx5.cxx
//
//      uITRON "C++" test program five
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
// Author(s):   dsm
// Contributors:        dsm
// Date:        1998-06-12
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
    /* test configuration for enough flag objects */               \
    defined( CYGPKG_UITRON_FLAGS )                              && \
    (CYGNUM_UITRON_FLAGS >= 3)                                  && \
    (CYGNUM_UITRON_FLAGS < 90)                                  && \
    ( !defined(CYGPKG_UITRON_FLAGS_CREATE_DELETE) ||               \
      CYGNUM_UITRON_FLAGS_INITIALLY >= 3             )          && \
                                                                   \
    /* test configuration for enough message boxes */              \
    defined( CYGPKG_UITRON_MBOXES )                             && \
    (CYGNUM_UITRON_MBOXES >= 3)                                 && \
    (CYGNUM_UITRON_MBOXES < 90)                                 && \
    ( !defined(CYGPKG_UITRON_MBOXES_CREATE_DELETE) ||              \
      CYGNUM_UITRON_MBOXES_INITIALLY >= 3            )          && \
                                                                   \
    /* test configuration for enough fixed memory pools */         \
    defined( CYGPKG_UITRON_MEMPOOLFIXED )                       && \
    (CYGNUM_UITRON_MEMPOOLFIXED >= 3)                           && \
    (CYGNUM_UITRON_MEMPOOLFIXED < 90)                           && \
    ( !defined(CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE) ||        \
      CYGNUM_UITRON_MEMPOOLFIXED_INITIALLY >= 3       )         && \
                                                                   \
    /* test configuration for enough variable mempools */          \
    defined( CYGPKG_UITRON_MEMPOOLVAR )                         && \
    (CYGNUM_UITRON_MEMPOOLVAR >= 3)                             && \
    (CYGNUM_UITRON_MEMPOOLVAR < 90)                             && \
    ( !defined(CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE) ||          \
      CYGNUM_UITRON_MEMPOOLVAR_INITIALLY >= 3       )           && \
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

UINT scratch;
T_MSG *t_msg;
VP vp;


extern "C" {
    void task1( unsigned int arg );
    void task2( unsigned int arg );
    void task3( unsigned int arg );
    void task4( unsigned int arg );
}


void task1( unsigned int arg )
{
    ER ercd;
    int i;
    T_RSYS rsys;

    CYG_TEST_INFO( "Task 1 running" );

    // check initial state
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_TSK == rsys.sysstat, "system state not TSS_TSK" );
    // disable intrs and check state
    ercd = loc_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "loc_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_LOC == rsys.sysstat, "system state not TSS_LOC" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // try an illegal op
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_CTX == ercd, "dly_tsk bad ercd !E_CTX" );
#endif // CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // enable intrs and check state and a legal sleep
    ercd = unl_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "unl_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_TSK == rsys.sysstat, "system state not TSS_TSK" );
    ercd = dly_tsk( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    // disable intrs and try scheduler illegal ops
    ercd = loc_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "loc_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_LOC == rsys.sysstat, "system state not TSS_LOC" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_CTX == ercd, "dis_dsp bad ercd !E_CTX" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_CTX == ercd, "ena_dsp bad ercd !E_CTX" );
#endif // CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // enable again and check state
    ercd = unl_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "unl_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_TSK == rsys.sysstat, "system state not TSS_TSK" );
    // disable the scheduler and check state
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_DDSP == rsys.sysstat, "system state not TSS_DDSP" );
    // disable intrs and check state
    ercd = loc_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "loc_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_LOC == rsys.sysstat, "system state not TSS_LOC" );
    // then unlock and check state
    ercd = unl_cpu();
    CYG_TEST_CHECK( E_OK == ercd, "unl_cpu bad ercd" );
    ercd = ref_sys( &rsys );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sys bad ercd" );
    CYG_TEST_CHECK( TSS_TSK == rsys.sysstat, "system state not TSS_TSK" );

    CYG_TEST_PASS( "Interrupt dis/enabling and interactions" );

    // and now we can do the rest of the test

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai bad ercd !E_OBJ" );
    ercd = rel_wai( 1 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai(me) bad ercd !E_OBJ" );
    ercd = rel_wai( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "rel_wai bad ercd !E_ID" );
    ercd = rel_wai( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "rel_wai bad ercd !E_ID" );
#endif // we can test bad param error returns

    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = sta_tsk( 2, 22222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = chg_pri( 2, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai bad ercd !E_OBJ" );
    ercd = rel_wai( 1 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai(me) bad ercd !E_OBJ" );
#endif // we can test bad param error returns

    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "wai_sem bad ercd !E_RLWAI" );

    ercd = twai_sem( 1, 20 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "twai_sem bad ercd !E_RLWAI" );

    ercd = wai_flg( &scratch, 1, 9999, 0 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "wai_flg bad ercd !E_RLWAI" );

    ercd = twai_flg( &scratch, 1, 9999, 0, 10 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "twai_flg bad ercd !E_RLWAI" );

    ercd = rcv_msg( &t_msg, 1 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "rcv_msg bad ercd !E_RLWAI" );

    ercd = trcv_msg( &t_msg, 1, 10 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "trcv_msg bad ercd !E_RLWAI" );

    // these are loops so as to consume the whole of the mempool
    // in order to wait at the end
    for ( i = 0; i < 10; i++ )
        if ( E_OK != (ercd = get_blf( &vp, 3 ) ) )
            break;
    CYG_TEST_CHECK( E_RLWAI == ercd, "get_blf bad ercd !E_RLWAI" );

    for ( i = 0; i < 10; i++ )
        if ( E_OK != (ercd = tget_blf( &vp, 3, 10 ) ) )
            break;
    CYG_TEST_CHECK( E_RLWAI == ercd, "tget_blf bad ercd !E_RLWAI" );

    for ( i = 0; i < 10; i++ )
        if ( E_OK != (ercd = get_blk( &vp, 1, 1000 ) ) )
            break;
    CYG_TEST_CHECK( E_RLWAI == ercd, "get_blk bad ercd !E_RLWAI" );

    for ( i = 0; i < 10; i++ )
        if ( E_OK != (ercd = tget_blk( &vp, 1, 1000, 10 ) ) )
            break;
    CYG_TEST_CHECK( E_RLWAI == ercd, "tget_blk bad ercd !E_RLWAI" );

    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "dly_tsk bad ercd !E_RLWAI" );

    ercd = tslp_tsk( 10 );
    CYG_TEST_CHECK( E_RLWAI == ercd, "tslp_tsk bad ercd !E_RLWAI" );

    ercd = slp_tsk();
    CYG_TEST_CHECK( E_RLWAI == ercd, "slp_tsk bad ercd !E_RLWAI" );

    ercd = ter_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

    CYG_TEST_PASS("release wait: various waiting calls");

    // all done
    CYG_TEST_EXIT( "All done" );
    ext_tsk();
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

    for ( i = 0 ; i < 100; i++ ) {
        ercd = rel_wai( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "rel_wai bad ercd" );
    }        
    // we expect task2 to be killed here
    CYG_TEST_FAIL( "Task 2 ran to completion!" );
}

void task3( unsigned int arg )
{
}

void task4( unsigned int arg )
{
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
externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG defined ie. we are N/A.

// EOF testcx5.cxx
