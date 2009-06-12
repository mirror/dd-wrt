//===========================================================================
//
//      testcx2.cxx
//
//      uITRON "C++" test program two
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
// Date:        1998-03-13
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

int intercom = 0;
int intercount = 0;
INT scratch = 0;

extern "C" {
    void task1( unsigned int arg );
    void task2( unsigned int arg );
    void task3( unsigned int arg );
    void task4( unsigned int arg );
}

void task1( unsigned int arg )
{
    ER ercd;

    T_RSEM sem_info;
    T_RFLG flg_info;
    T_RMBX mbx_info;
    T_RMPF mpf_info;
    T_RMPL mpl_info;
    UINT flagptn;
    static char foo[] = "Test message";
    T_MSG *msgptr = (T_MSG *)foo;
    T_MSG *rxptr = NULL;
    VP blfptr = (VP)foo;
    VP blkptr = (VP)foo;

    int delay = 10;
    if (cyg_test_is_simulator)
        delay = 3;

    CYG_TEST_INFO( "Task 1 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 1 == scratch, "tid not 1" );
    
    // start a lower prio task to interact with
    intercom = 1;
    ercd = sta_tsk( 2, 222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

    // Semaphores; all the illegal argument combinations first
    CYG_TEST_INFO( "Testing semaphore ops" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = sig_sem( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "sig_sem bad ercd !E_ID" );
    ercd = sig_sem( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "sig_sem bad ercd !E_ID" );
    ercd = wai_sem( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "wai_sem bad ercd !E_ID" );
    ercd = wai_sem( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "wai_sem bad ercd !E_ID" );
    ercd = preq_sem( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "preq_sem bad ercd !E_ID" );
    ercd = preq_sem( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "preq_sem bad ercd !E_ID" );
    ercd = twai_sem( -6, delay );
    CYG_TEST_CHECK( E_ID == ercd, "twai_sem bad ercd !E_ID" );
    ercd = twai_sem( 99, delay );
    CYG_TEST_CHECK( E_ID == ercd, "twai_sem bad ercd !E_ID" );
    ercd = twai_sem( 2, -999 );
    CYG_TEST_CHECK( E_PAR == ercd, "twai_sem bad ercd !E_PAR" );
    ercd = ref_sem( &sem_info, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_sem bad ercd !E_ID" );
    ercd = ref_sem( &sem_info, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_sem bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_sem( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_sem bad ercd !E_PAR" );
#endif
    CYG_TEST_PASS( "bad calls: sig_sem, [t]wai_sem, preq_sem, ref_sem" );
#endif // we can test bad param error returns

    // check the waitable functions versus dispatch disable
    ercd = preq_sem( 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "preq_sem bad ercd !E_TMOUT" );
    ercd = twai_sem( 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem bad ercd !E_TMOUT" );
    ercd = twai_sem( 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem(POL) bad ercd !E_TMOUT" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = wai_sem( 2 );
    CYG_TEST_CHECK( E_CTX == ercd, "wai_sem bad ercd !E_CTX" );
    ercd = twai_sem( 2, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "twai_sem bad ercd !E_CTX" );
    ercd = twai_sem( 2, TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "twai_sem(FEVR) bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = twai_sem( 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem(POL) bad ercd !E_TMOUT" );
    ercd = preq_sem( 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "preq_sem bad ercd !E_TMOUT" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = preq_sem( 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "preq_sem bad ercd !E_TMOUT" );
    ercd = twai_sem( 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem bad ercd !E_TMOUT" );
    ercd = twai_sem( 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem(POL) bad ercd !E_TMOUT" );
    CYG_TEST_PASS( "bad calls: wai_sem, twai_sem with dis_dsp" );

    // check ref_sem with various states
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 1 == sem_info.semcnt, "semcnt should be 1" );
    ercd = preq_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 2 == sem_info.semcnt, "semcnt should be 2" );
    ercd = wai_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 1 == sem_info.semcnt, "semcnt should be 1" );
    ercd = twai_sem( 2, delay );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    intercom = 0;
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    intercom = 1;
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 != sem_info.wtsk, "sem.wtsk should be non0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be non0" );
#if 1
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
#else // old, non-uITRON semantics
    CYG_TEST_CHECK( 1 == sem_info.semcnt, "semcnt should be 1" );
#endif
    ercd = dly_tsk( delay );               // let task 2 pick up the signal
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_sem( &sem_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );
    CYG_TEST_CHECK( 0 == sem_info.wtsk, "sem.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == sem_info.semcnt, "semcnt should be 0" );
    CYG_TEST_PASS( "good calls: sig_sem, [t]wai,preq_sem with ref_sem" );

    // Flags; all the illegal argument combinations first
    CYG_TEST_INFO( "Testing flag ops" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = set_flg( -6, 1 );
    CYG_TEST_CHECK( E_ID == ercd, "set_flg bad ercd !E_ID" );
    ercd = set_flg( 99, 1 );
    CYG_TEST_CHECK( E_ID == ercd, "set_flg bad ercd !E_ID" );
    ercd = clr_flg( -6, 1 );
    CYG_TEST_CHECK( E_ID == ercd, "clr_flg bad ercd !E_ID" );
    ercd = clr_flg( 99, 1 );
    CYG_TEST_CHECK( E_ID == ercd, "sig_flg bad ercd !E_ID" );
    ercd = wai_flg( &flagptn, -6, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_ID == ercd, "wai_flg bad ercd !E_ID" );
    ercd = wai_flg( &flagptn, 99, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_ID == ercd, "wai_flg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = wai_flg( NULL, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_PAR == ercd, "wai_flg bad ercd !E_PAR" );
#endif
    ercd = wai_flg( &flagptn, 2, 7, 34657 );
    CYG_TEST_CHECK( E_PAR == ercd, "wai_flg bad ercd !E_PAR" );
    ercd = wai_flg( &flagptn, 2, 0, TWF_ANDW );
    CYG_TEST_CHECK( E_PAR == ercd, "wai_flg bad ercd !E_PAR" );
    ercd = pol_flg( &flagptn, -6, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_ID == ercd, "pol_flg bad ercd !E_ID" );
    ercd = pol_flg( &flagptn, 99, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_ID == ercd, "pol_flg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = pol_flg( NULL, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_PAR == ercd, "pol_flg bad ercd !E_PAR" );
#endif
    ercd = pol_flg( &flagptn, 2, 7, 34657 );
    CYG_TEST_CHECK( E_PAR == ercd, "pol_flg bad ercd !E_PAR" );
    ercd = pol_flg( &flagptn, 2, 0, TWF_ANDW );
    CYG_TEST_CHECK( E_PAR == ercd, "pol_flg bad ercd !E_PAR" );
    ercd = twai_flg( &flagptn, -6, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_ID == ercd, "twai_flg bad ercd !E_ID" );
    ercd = twai_flg( &flagptn, 99, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_ID == ercd, "twai_flg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = twai_flg( NULL, 2, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "twai_flg bad ercd !E_PAR" );
#endif
    ercd = twai_flg( &flagptn, 2, 7, 34657, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "twai_flg bad ercd !E_PAR" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, -999 );
    CYG_TEST_CHECK( E_PAR == ercd, "twai_flg bad ercd !E_PAR" );
    ercd = twai_flg( &flagptn, 2, 0, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "twai_flg bad ercd !E_PAR" );
    ercd = ref_flg( &flg_info, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_flg bad ercd !E_ID" );
    ercd = ref_flg( &flg_info, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_flg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_flg( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_flg bad ercd !E_PAR" );
#endif
    CYG_TEST_PASS( "bad calls: set_flg, clr_flg, [t]wai,pol_flg, ref_flg" );
#endif // we can test bad param error returns

    // check the waitable functions versus dispatch disable
    ercd = pol_flg( &flagptn, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pol_flg bad ercd !E_TMOUT" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg bad ercd !E_TMOUT" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg(POL) bad ercd !E_TMOUT" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = wai_flg( &flagptn, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_CTX == ercd, "wai_flg bad ercd !E_CTX" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "twai_flg bad ercd !E_CTX" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "twai_flg(FEVR) bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg(POL) bad ercd !E_TMOUT" );
    ercd = pol_flg( &flagptn, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pol_flg bad ercd !E_TMOUT" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = pol_flg( &flagptn, 2, 7, TWF_ANDW );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pol_flg bad ercd !E_TMOUT" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg bad ercd !E_TMOUT" );
    ercd = twai_flg( &flagptn, 2, 7, TWF_ANDW, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg(POL) bad ercd !E_TMOUT" );
    CYG_TEST_PASS( "bad calls: wai_flg, twai_flg with dis_dsp" );

    // check ref_flg with various states
    ercd = ref_flg( &flg_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 == flg_info.wtsk, "flg.wtsk should be non0" );
    CYG_TEST_CHECK( 0 == flg_info.flgptn, "flgptn should be 0" );
    intercom = 0;
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    intercom = 1;
    ercd = ref_flg( &flg_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 != flg_info.wtsk, "flg.wtsk should be non0" );
    CYG_TEST_CHECK( 0 == flg_info.flgptn, "flgptn should be 0" );
    ercd = set_flg( 2, 0x5555 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_flg bad ercd" );
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_flg( &flg_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 != flg_info.wtsk, "flg.wtsk should be non0" );
    CYG_TEST_CHECK( 0x5555 == flg_info.flgptn, "flgptn should be 0x5555" );
    ercd = clr_flg( 2, 0xF0F0 );
    CYG_TEST_CHECK( E_OK == ercd, "clr_flg bad ercd" );
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_flg( &flg_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 != flg_info.wtsk, "flg.wtsk should be non0" );
    CYG_TEST_CHECK( 0x5050 == flg_info.flgptn, "flgptn should be 0x5050" );
    ercd = set_flg( 2, 0xFFFF );
    CYG_TEST_CHECK( E_OK == ercd, "sig_flg bad ercd" );
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_flg( &flg_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 == flg_info.wtsk, "flg.wtsk should be 0" );
    CYG_TEST_CHECK( 0xFFFF == flg_info.flgptn, "flgptn should be 0xFFFF" );
    CYG_TEST_PASS( "good calls: clr_flg, set_flg, wai_flg with ref_flg" );

    // Mailboxes; all the illegal argument combinations first
    CYG_TEST_INFO( "Testing mailbox ops" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = snd_msg( -6, msgptr );
    CYG_TEST_CHECK( E_ID == ercd, "snd_msg bad ercd !E_ID" );
    ercd = snd_msg( 99, msgptr );
    CYG_TEST_CHECK( E_ID == ercd, "snd_msg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = snd_msg( 2, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "snd_msg bad ercd !E_PAR" );
#endif
    ercd = rcv_msg( &rxptr, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "rcv_msg bad ercd !E_ID" );
    ercd = rcv_msg( &rxptr, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "rcv_msg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = rcv_msg( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "rcv_msg bad ercd !E_PAR" );
#endif
    ercd = prcv_msg( &rxptr, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "prcv_msg bad ercd !E_ID" );
    ercd = prcv_msg( &rxptr, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "prcv_msg bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = prcv_msg( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "prcv_msg bad ercd !E_PAR" );
#endif
    ercd = trcv_msg( &rxptr, -6, delay );
    CYG_TEST_CHECK( E_ID == ercd, "trcv_msg bad ercd !E_ID" );
    ercd = trcv_msg( &rxptr, 99, delay );
    CYG_TEST_CHECK( E_ID == ercd, "trcv_msg bad ercd !E_ID" );
    ercd = trcv_msg( &rxptr, 2, -999 );
    CYG_TEST_CHECK( E_PAR == ercd, "trcv_msg bad ercd !E_PAR" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = trcv_msg( NULL, 2, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "trcv_msg bad ercd !E_PAR" );
#endif
    ercd = ref_mbx( &mbx_info, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mbx bad ercd !E_ID" );
    ercd = ref_mbx( &mbx_info, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mbx bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_mbx( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_mbx bad ercd !E_PAR" );
#endif
    CYG_TEST_PASS( "bad calls: snd_msg, [pt]rcv_msg, ref_mbx" );
#endif // we can test bad param error returns

    // check the waitable functions versus dispatch disable
    ercd = prcv_msg( &rxptr, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "prcv_msg bad ercd !E_TMOUT" );
    ercd = trcv_msg( &rxptr, 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg bad ercd !E_TMOUT" );
    ercd = trcv_msg( &rxptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg(POL) bad ercd !E_TMOUT" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rcv_msg( &rxptr, 2 );
    CYG_TEST_CHECK( E_CTX == ercd, "rcv_msg bad ercd !E_CTX" );
    ercd = trcv_msg( &rxptr, 2, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "trcv_msg bad ercd !E_CTX" );
    ercd = trcv_msg( &rxptr, 2, TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "trcv_msg(FEVR) bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = trcv_msg( &rxptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg(POL) bad ercd !E_TMOUT" );
    ercd = prcv_msg( &rxptr, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "prcv_msg bad ercd !E_TMOUT" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = prcv_msg( &rxptr, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "prcv_msg bad ercd !E_TMOUT" );
    ercd = trcv_msg( &rxptr, 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg bad ercd !E_TMOUT" );
    ercd = trcv_msg( &rxptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg(POL) bad ercd !E_TMOUT" );
    CYG_TEST_PASS( "bad calls: rcv_msg, trcv_msg with dis_dsp" );

    // check ref_mbx with various states
    ercd = ref_mbx( &mbx_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mbx bad ercd" );
    CYG_TEST_CHECK( 0 == mbx_info.wtsk, "mbx.wtsk should be 0" );
    CYG_TEST_CHECK( NADR == mbx_info.pk_msg, "mbx peek should be NADR" );
    intercom = 0;
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    intercom = 1;
    ercd = ref_mbx( &mbx_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mbx bad ercd" );
    CYG_TEST_CHECK( 0 != mbx_info.wtsk, "mbx.wtsk should be non0" );
    CYG_TEST_CHECK( NADR == mbx_info.pk_msg, "mbx peek should be NADR" );
    ercd = snd_msg( 2, msgptr );
    CYG_TEST_CHECK( E_OK == ercd, "snd_msg bad ercd" );
    ercd = ref_mbx( &mbx_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mbx bad ercd" );
    CYG_TEST_CHECK( 0 == mbx_info.wtsk, "mbx.wtsk should be 0" );
#if 1
    CYG_TEST_CHECK( NADR == mbx_info.pk_msg, "mbx peek should be NADR" );
#else // old, non-uITRON semantics
    CYG_TEST_CHECK( msgptr == mbx_info.pk_msg, "mbx peek should be msgptr" );
#endif
    ercd = dly_tsk( delay );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_mbx( &mbx_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mbx bad ercd" );
    CYG_TEST_CHECK( 0 == mbx_info.wtsk, "mbx.wtsk should be 0" );
    CYG_TEST_CHECK( NADR == mbx_info.pk_msg, "mbx peek should be NADR" );
    // fill the message box, expect E_QOVR
    for ( scratch = 0 ; scratch < 100 ; scratch++ ) {
        if ( E_OK != ( ercd = snd_msg( 2, msgptr ) ) )
            break;
    }
    CYG_TEST_CHECK( (100 == scratch) || (E_QOVR == ercd),
                    "snd_msg bad ercd !E_QOVR/E_OK" );
    // empty the message box, expect the right number and E_TMOUT
    for (             ;     1         ; scratch-- ) {
        if ( E_OK != ( ercd = prcv_msg( &rxptr, 2 ) ) )
            break;
    }
    CYG_TEST_CHECK( 0 == scratch, "rcv_msg count bad scratch!=0" );
    CYG_TEST_CHECK( E_TMOUT == ercd, "rcv_msg bad ercd !E_TMOUT" );

    CYG_TEST_PASS( "good calls: rcv_msg, snd_msg with ref_msg" );

    // Fixed block memory pools: all the illegal argument combinations first
    CYG_TEST_INFO( "Testing fixed block memory ops" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rel_blf( -6, blfptr );
    CYG_TEST_CHECK( E_ID == ercd, "rel_blf bad ercd !E_ID" );
    ercd = rel_blf( 99, blfptr );
    CYG_TEST_CHECK( E_ID == ercd, "rel_blf bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = rel_blf( 2, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blf bad ercd !E_PAR" );
#endif
#endif // we can test bad param error returns
    ercd = rel_blf( 2, blfptr );        // it did not come from a mpf
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blf bad ercd !E_PAR" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blf( &blfptr, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "get_blf bad ercd !E_ID" );
    ercd = get_blf( &blfptr, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "get_blf bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = get_blf( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "get_blf bad ercd !E_PAR" );
#endif
    ercd = pget_blf( &blfptr, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "pget_blf bad ercd !E_ID" );
    ercd = pget_blf( &blfptr, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "pget_blf bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = pget_blf( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "pget_blf bad ercd !E_PAR" );
#endif
    ercd = tget_blf( &blfptr, -6, delay );
    CYG_TEST_CHECK( E_ID == ercd, "tget_blf bad ercd !E_ID" );
    ercd = tget_blf( &blfptr, 99, delay );
    CYG_TEST_CHECK( E_ID == ercd, "tget_blf bad ercd !E_ID" );
    ercd = tget_blf( &blfptr, 2, -999 );
    CYG_TEST_CHECK( E_PAR == ercd, "tget_blf bad ercd !E_PAR" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = tget_blf( NULL, 2, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "tget_blf bad ercd !E_PAR" );
#endif
    ercd = ref_mpf( &mpf_info, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mpf bad ercd !E_ID" );
    ercd = ref_mpf( &mpf_info, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mpf bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_mpf( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_mpf bad ercd !E_PAR" );
#endif
    CYG_TEST_PASS( "bad calls: rel_blf, [pt]get_blf, ref_mpf " );
#endif // we can test bad param error returns

    // check the waitable functions versus dispatch disable
    ercd = pget_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blf bad ercd" );
    ercd = rel_blf( 2, blfptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blf bad ercd" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = rel_blf( 2, blfptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_CTX == ercd, "get_blf bad ercd !E_CTX" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blf bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = pget_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blf bad ercd" );
    ercd = rel_blf( 2, blfptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blf bad ercd" );
    ercd = rel_blf( 2, blfptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
    // consume the whole thing then do it again, expecting E_TMOUT
    while ( E_OK == (ercd = pget_blf( &blfptr, 2 ) ) )
        continue;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    ercd = pget_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf bad ercd !E_TMOUT" );
    ercd = tget_blf( &blfptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf(POL) bad ercd !E_TMOUT" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_CTX == ercd, "get_blf bad ercd !E_CTX" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blf bad ercd !E_CTX" );
    ercd = tget_blf( &blfptr, 2, TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blf(FEVR) bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = tget_blf( &blfptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf(POL) bad ercd !E_TMOUT" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = pget_blf( &blfptr, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    ercd = tget_blf( &blfptr, 2, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    ercd = tget_blf( &blfptr, 2, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf(POL) bad ercd !E_TMOUT" );
    CYG_TEST_PASS( "bad calls: rel_blf, [pt]get_blf with ena_dsp" );

    // check ref_mpf with various states
    ercd = ref_mpf( &mpf_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpf bad ercd" );
    CYG_TEST_CHECK( 0 == mpf_info.wtsk, "mpf.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == mpf_info.frbcnt, "mpf.frbcnt should be 0" );
    intercom = 0;
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    intercom = 1;
    ercd = ref_mpf( &mpf_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpf bad ercd" );
    CYG_TEST_CHECK( 0 != mpf_info.wtsk, "mpf.wtsk should be non0" );
    CYG_TEST_CHECK( 0 == mpf_info.frbcnt, "mpf.frbcnt should be 0" );
    ercd = rel_blf( 2, blfptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
    ercd = ref_mpf( &mpf_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpf bad ercd" );
    CYG_TEST_CHECK( 0 == mpf_info.wtsk, "mpf.wtsk should be 0" );
#if 1
    CYG_TEST_CHECK( 0 == mpf_info.frbcnt, "mpf.frbcnt should be 0" );
#else // old, non-uITRON semantics
    CYG_TEST_CHECK( 0 != mpf_info.frbcnt, "mpf.frbcnt should be non0" );
#endif
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_mpf( &mpf_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpf bad ercd" );
    CYG_TEST_CHECK( 0 == mpf_info.wtsk, "mpf.wtsk should be 0" );
    CYG_TEST_CHECK( 0 == mpf_info.frbcnt, "mpf.frbcnt should be 0" );
    CYG_TEST_PASS( "good calls: rel_blf, get_blf with ref_mpf" );

    // Variable block memory pools; illegal arguments
    CYG_TEST_INFO( "Testing variable block memory ops" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rel_blk( -6, blkptr );
    CYG_TEST_CHECK( E_ID == ercd, "rel_blk bad ercd !E_ID" );
    ercd = rel_blk( 99, blkptr );
    CYG_TEST_CHECK( E_ID == ercd, "rel_blk bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = rel_blk( 2, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blk bad ercd !E_PAR" );
#endif
#endif // we can test bad param error returns
    ercd = rel_blk( 2, blkptr );        // it did not come from a mpl
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blk bad ercd !E_PAR" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blk( &blkptr, -6, 100 );
    CYG_TEST_CHECK( E_ID == ercd, "get_blk bad ercd !E_ID" );
    ercd = get_blk( &blkptr, 99, 100 );
    CYG_TEST_CHECK( E_ID == ercd, "get_blk bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = get_blk( NULL, 2, 100 );
    CYG_TEST_CHECK( E_PAR == ercd, "get_blk bad ercd !E_PAR" );
#endif
    ercd = pget_blk( &blkptr, -6, 100 );
    CYG_TEST_CHECK( E_ID == ercd, "pget_blk bad ercd !E_ID" );
    ercd = pget_blk( &blkptr, 99, 100 );
    CYG_TEST_CHECK( E_ID == ercd, "pget_blk bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = pget_blk( NULL, 2, 100 );
    CYG_TEST_CHECK( E_PAR == ercd, "pget_blk bad ercd !E_PAR" );
#endif
    ercd = tget_blk( &blkptr, -6, 100, delay );
    CYG_TEST_CHECK( E_ID == ercd, "tget_blk bad ercd !E_ID" );
    ercd = tget_blk( &blkptr, 99, 100, delay );
    CYG_TEST_CHECK( E_ID == ercd, "tget_blk bad ercd !E_ID" );
    ercd = tget_blk( &blkptr, 2, 100, -999 );
    CYG_TEST_CHECK( E_PAR == ercd, "tget_blk bad ercd !E_PAR" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = tget_blk( NULL, 2, 100, delay );
    CYG_TEST_CHECK( E_PAR == ercd, "tget_blk bad ercd !E_PAR" );
#endif
    ercd = ref_mpl( &mpl_info, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mpl bad ercd !E_ID" );
    ercd = ref_mpl( &mpl_info, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_mpl bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_mpl( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_mpl bad ercd !E_PAR" );
#endif
    CYG_TEST_PASS( "bad calls: rel_blk, [pt]get_blk, ref_mpl " );
#endif // we can test bad param error returns

    // check the waitable functions versus dispatch disable
    ercd = pget_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blk bad ercd" );
    ercd = rel_blk( 2, blkptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blk bad ercd" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = rel_blk( 2, blkptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_CTX == ercd, "get_blk bad ercd !E_CTX" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blk bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = pget_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blk bad ercd" );
    ercd = rel_blk( 2, blkptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blk bad ercd" );
    ercd = rel_blk( 2, blkptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
    // consume the whole thing then do it again, expecting E_TMOUT
    while ( E_OK == (ercd = pget_blk( &blkptr, 2, 100 ) ) )
        continue;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    ercd = pget_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk bad ercd !E_TMOUT" );
    ercd = tget_blk( &blkptr, 2, 100, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk(POL) bad ercd !E_TMOUT" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = get_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_CTX == ercd, "get_blk bad ercd !E_CTX" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blk bad ercd !E_CTX" );
    ercd = tget_blk( &blkptr, 2, 100, TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "tget_blk(FEVR) bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = tget_blk( &blkptr, 2, 100, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk(POL) bad ercd !E_TMOUT" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = pget_blk( &blkptr, 2, 100 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    ercd = tget_blk( &blkptr, 2, 100, delay );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    ercd = tget_blk( &blkptr, 2, 100, TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk(POL) bad ercd !E_TMOUT" );
    CYG_TEST_PASS( "bad calls: rel_blk, [pt]get_blk with ena_dsp" );

    // check ref_mpl with various states
    ercd = ref_mpl( &mpl_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpl bad ercd" );
    CYG_TEST_CHECK( 0 == mpl_info.wtsk, "mpl.wtsk should be 0" );
    CYG_TEST_CHECK( mpl_info.maxsz <= mpl_info.frsz,
                    "mpl.maxsz not < mpl.frsz" );
    intercom = 0;
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    intercom = 1;
    ercd = ref_mpl( &mpl_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpl bad ercd" );
    CYG_TEST_CHECK( 0 != mpl_info.wtsk, "mpl.wtsk should be non0" );
    ercd = rel_blk( 2, blkptr );
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
    ercd = ref_mpl( &mpl_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpl bad ercd" );
    CYG_TEST_CHECK( 0 == mpl_info.wtsk, "mpl.wtsk should be 0" );
    ercd = dly_tsk( delay );               // let task 2 start waiting
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_mpl( &mpl_info, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpl bad ercd" );
    CYG_TEST_CHECK( 0 == mpl_info.wtsk, "mpl.wtsk should be 0" );
    CYG_TEST_PASS( "good calls: rel_blk, get_blk with ref_mpl" );

    // all done
    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}



void task2( unsigned int arg )
{
    ER ercd;
    T_MSG *msgp = NULL;
    UINT flgval = 0;
    VP blfp = NULL;
    VP blkp = NULL;

    CYG_TEST_INFO( "Task 2 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 2 == scratch, "tid not 2" );
    if ( 222 != arg )
        CYG_TEST_FAIL( "Task 2 arg not 222" );

    while ( intercom ) {
        ercd = rot_rdq( 0 );
        CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    }
    ercd = wai_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    while ( intercom ) {
        ercd = rot_rdq( 0 );
        CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    }
    ercd = wai_flg( &flgval, 2, 99, TWF_ANDW );
    CYG_TEST_CHECK( E_OK == ercd, "wai_flg bad ercd" );
    CYG_TEST_CHECK( 99 == (99 & flgval), "flg value no good" );
    while ( intercom ) {
        ercd = rot_rdq( 0 );
        CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    }
    ercd = rcv_msg( &msgp, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "rcv_msg bad ercd" );
    CYG_TEST_CHECK( NULL != msgp, "no msg received" );
    CYG_TEST_CHECK( NADR != msgp, "no msg received" );
    while ( intercom ) {
        ercd = rot_rdq( 0 );
        CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    }
    ercd = get_blf( &blfp, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "get_blf bad ercd" );
    CYG_TEST_CHECK( NULL != blfp, "no blf allocated" );
    CYG_TEST_CHECK( NADR != blfp, "no blf allocated" );
    while ( intercom ) {
        ercd = rot_rdq( 0 );
        CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    }
    ercd = get_blk( &blkp, 2, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "get_blk bad ercd" );
    CYG_TEST_CHECK( NULL != blkp, "no blk allocated" );
    CYG_TEST_CHECK( NADR != blkp, "no blk allocated" );

    ext_tsk();
    CYG_TEST_FAIL( "Task 2 failed to exit" );
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
#define N_A_MSG "no CYGVAR_KERNEL_COUNTERS_CLOCK "
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

// EOF testcx2.cxx
