//===========================================================================
//
//      test4.c
//
//      uITRON "C" test program four
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
    /* test configuration for enough cyclic handlers */            \
    defined( CYGPKG_UITRON_CYCLICS )                            && \
    (CYGNUM_UITRON_CYCLICS >= 3)                                && \
    (CYGNUM_UITRON_CYCLICS < 90)                                && \
                                                                   \
    /* test configuration for enough alarm handlers */             \
    defined( CYGPKG_UITRON_ALARMS )                             && \
    (CYGNUM_UITRON_ALARMS >= 3)                                 && \
    (CYGNUM_UITRON_ALARMS < 90)                                 && \
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

void hand1(void)
{
    CYG_TEST_INFO("Handler 1 called");
    intercount++;
}

void hand2(void)
{
    CYG_TEST_CHECK( 2 == intercount, "handler out of sync" );
    CYG_TEST_INFO("Handler 2 called");
    intercount++;
}

void task1( unsigned int arg )
{
    ER ercd;

    T_DCYC dcyc;
    T_DALM dalm;
    T_RCYC rcyc;
    T_RALM ralm;

    unsigned int tm;

    static char foo[] = "Test message";
    VP info = (VP)foo;

    // Increase times when running on HW since overhead of GDB packet
    // acknowledgements may cause tests of timing to fail.
    if (cyg_test_is_simulator)
        tm = 1;
    else
        tm = 4;

    CYG_TEST_INFO( "Task 1 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 1 == scratch, "tid not 1" );

    dcyc.exinf  = (VP)info;
    dcyc.cycatr = TA_HLNG;
    dcyc.cychdr = (FP)&hand1;
    dcyc.cycact = TCY_INI;              // bad
    dcyc.cyctim = 2;

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = def_cyc(3, &dcyc);
    CYG_TEST_CHECK( E_PAR == ercd, "def_cyc bad ercd !E_PAR" );
#endif // we can test bad param error returns

    dcyc.cycact = TCY_OFF;              // make good
    dcyc.cyctim = 0;                    // bad

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = def_cyc(3, &dcyc);
    CYG_TEST_CHECK( E_PAR == ercd, "def_cyc bad ercd !E_PAR" );
#endif // we can test bad param error returns

    dcyc.cyctim = 1;                    // make good

    ercd = def_cyc(3, &dcyc);
    CYG_TEST_CHECK( E_OK == ercd, "def_cyc bad ercd" );    

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = def_cyc(-6, &dcyc);
    CYG_TEST_CHECK( E_PAR == ercd, "def_cyc bad ercd !E_PAR" );
    ercd = def_cyc(99, &dcyc);
    CYG_TEST_CHECK( E_PAR == ercd, "def_cyc bad ercd !E_PAR" );

    ercd = act_cyc(-6, TCY_OFF);
    CYG_TEST_CHECK( E_PAR == ercd, "act_cyc bad ercd !E_PAR" );
    ercd = act_cyc(99, TCY_OFF);
    CYG_TEST_CHECK( E_PAR == ercd, "act_cyc bad ercd !E_PAR" );
    ercd = act_cyc( 3, ~0);
    CYG_TEST_CHECK( E_PAR == ercd, "act_cyc bad ercd !E_PAR" );
    
    ercd = ref_cyc(&rcyc, -6);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_cyc bad ercd !E_PAR" );
    ercd = ref_cyc(&rcyc, 99);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_cyc bad ercd !E_PAR" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_cyc(NULL,  3);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_cyc bad ercd !E_PAR" );
#endif
    ercd = ref_cyc(NADR,  3);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_cyc bad ercd !E_PAR" );
#endif // we can test bad param error returns

    ercd = def_cyc(3, (T_DCYC *)NADR);
    CYG_TEST_CHECK( E_OK == ercd, "def_cyc bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = ref_cyc(&rcyc,  3);
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_cyc bad ercd !E_NOEXS" );    
#endif // we can test bad param error returns

    CYG_TEST_PASS( "bad calls: def_cyc, act_cyc, ref_cyc" );

    dalm.exinf  = (VP)info;
    dalm.almatr = TA_HLNG;
    dalm.almhdr = (FP)&hand2;
    dalm.tmmode = ~0;                   // bad
    dalm.almtim = 20;

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = def_alm(3, &dalm);
    CYG_TEST_CHECK( E_PAR == ercd, "def_alm bad ercd !E_PAR" );
#endif // we can test bad param error returns

    dalm.tmmode = TTM_REL;              // make good
    dalm.almtim = 0;                    // bad

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = def_alm(3, &dalm);
    CYG_TEST_CHECK( E_PAR == ercd, "def_alm bad ercd !E_PAR" );
#endif // we can test bad param error returns

    dalm.almtim = 1000;                 // make good

    ercd = def_alm(3, &dalm);
    CYG_TEST_CHECK( E_OK == ercd, "def_alm bad ercd" );

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = ref_alm(&ralm, -6);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_alm bad ercd !E_PAR" );
    ercd = ref_alm(&ralm, 99);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_alm bad ercd !E_PAR" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_alm(NULL,  3);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_alm bad ercd !E_PAR" );
#endif
    ercd = ref_alm(NADR,  3);
    CYG_TEST_CHECK( E_PAR == ercd, "ref_alm bad ercd !E_PAR" );

#endif // we can test bad param error returns
    ercd = def_alm(3, (T_DALM *)NADR);
    CYG_TEST_CHECK( E_OK == ercd, "def_cyc bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = ref_alm(&ralm,  3);
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_cyc bad ercd !E_NOEXS" );    
#endif // we can test bad param error returns

    CYG_TEST_PASS( "bad calls: def_alm, act_alm, ref_alm" );
    
    dcyc.exinf  = (VP)info;
    dcyc.cycatr = TA_HLNG;
    dcyc.cychdr = (FP)&hand1;
    dcyc.cycact = TCY_ON;
    dcyc.cyctim = 50*tm;    
    
    ercd = def_cyc(3, &dcyc);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 45*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 50*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_ON == rcyc.cycact, "rcyc.cycact should be TCY_ON" );
    ercd = act_cyc(3, TCY_OFF);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 45*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 50*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_OFF == rcyc.cycact, "rcyc.cycact should be TCY_OFF" );
    ercd = act_cyc(3, TCY_ON);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc bad ercd" );
    
    CYG_TEST_PASS("good calls: def_cyc, act_cyc, ref_cyc");

    dalm.exinf  = (VP)info;
    dalm.almatr = TA_HLNG;
    dalm.almhdr = (FP)&hand2;
    dalm.tmmode = TTM_REL;
    dalm.almtim = 120*tm;

    ercd = def_alm(3, &dalm);
    CYG_TEST_CHECK( E_OK == ercd, "def_alm bad ercd" );
    ercd = ref_alm(&ralm, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_alm bad ercd" );
    CYG_TEST_CHECK( info == ralm.exinf, "ralm.exinf should be info" );
    CYG_TEST_CHECK( 115*tm < ralm.lfttim, "ralm.lfttim too small" );
    CYG_TEST_CHECK( ralm.lfttim <= 120*tm, "ralm.lfttim too big" );

    // Expect handlers to be called at approximate times
    // time           intercount
    // tm*50  hand1   0
    // tm*100 hand1   1
    // tm*120 hand2   2
    // tm*150 hand1   3

    ercd = dly_tsk(160*tm);
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 4 == intercount, "handlers not both called" );
    
    ercd = act_cyc(3, TCY_OFF);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc(off) bad ercd" );

    ercd = dly_tsk(60*tm);              // enough for at least one tick
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 4 == intercount, "cyclic not disabled" );

    // approx time now 220, so we expect a cycle in about 30 ticks
    ercd = act_cyc(3, TCY_ON);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc(on) bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 25*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 35*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_ON == rcyc.cycact, "rcyc.cycact should be TCY_ON" );

    // now resynchronize with right now:
    ercd = act_cyc(3, TCY_ON|TCY_INI);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc(on) bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 45*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 50*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_ON == rcyc.cycact, "rcyc.cycact should be TCY_ON" );

    // wait a bit and check that time marches on, or even down
    ercd = dly_tsk(10*tm);
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 35*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 45*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_ON == rcyc.cycact, "rcyc.cycact should be TCY_ON" );

    // now turn it off and re-synch with right now:
    ercd = act_cyc(3, TCY_OFF|TCY_INI);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc(on) bad ercd" );
    ercd = ref_cyc(&rcyc, 3);
    CYG_TEST_CHECK( E_OK == ercd, "ref_cyc bad ercd" );
    CYG_TEST_CHECK( info == rcyc.exinf, "rcyc.exinf should be info" );
    CYG_TEST_CHECK( 45*tm < rcyc.lfttim, "rcyc.lfttim too small" );
    CYG_TEST_CHECK( rcyc.lfttim <= 50*tm, "rcyc.lfttim too big" );
    CYG_TEST_CHECK( TCY_OFF == rcyc.cycact, "rcyc.cycact should be TCY_OFF" );

    ercd = act_cyc(3, TCY_OFF);
    CYG_TEST_CHECK( E_OK == ercd, "act_cyc(on) bad ercd" );

    CYG_TEST_PASS("good calls: def_cyc, act_cyc, ref_cyc, def_alm, ref_alm");

    // all done
    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}



void task2( unsigned int arg )
{
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
void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( N_A_MSG );
}
#endif // N_A_MSG defined ie. we are N/A.

// EOF test4.c
