//===========================================================================
//
//      test1.c
//
//      uITRON "C" test program one
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

volatile int intercom = 0;
volatile int intercount = 0;
INT scratch = 0;

#ifndef CYGSEM_KERNEL_SCHED_TIMESLICE
#define TIMESLICEMSG "Assuming no kernel timeslicing"
#define TSGO()          (1)
#define TSRELEASE()     CYG_EMPTY_STATEMENT
#define TSSTOP()        CYG_EMPTY_STATEMENT
#define TSLOCK()        CYG_EMPTY_STATEMENT
#define TSUNLOCK()      CYG_EMPTY_STATEMENT
#define ICWAIT( _i_ )   CYG_EMPTY_STATEMENT

#else
// Now follow some nasty bodges to control the scheduling when basically it
// isn't controlled ie. timeslicing is on.  It's bodgy because we're
// testing normal synchronization methods, so we shouldn't rely on them for
// comms between threads here.  Instead there's a mixture of communicating
// via a flag (ts_interlock) which stops the "controlled" thread running
// away, and waiting for the controlled thread to run enough for us.
//
// Tasks 3 and 4 are waited for by the control task: task 3 locks the
// scheduler so is immediately descheduled when it unlocks it, task 4 does
// waiting-type operations, so we must give it chance to run by yielding a
// few times ourselves.  Note the plain constant in ICWAIT() below.

#define TIMESLICEMSG "Assuming kernel timeslicing ENABLED"
volatile int ts_interlock = 0;
#define TSGO()          (ts_interlock)
#define TSRELEASE()     ts_interlock = 1
#define TSSTOP()        ts_interlock = 0

#define TSLOCK()        CYG_MACRO_START                                 \
    ER ercd2 = dis_dsp();                                               \
    CYG_TEST_CHECK( E_OK == ercd2, "dis_dsp (TSLOCK) bad ercd2" );      \
CYG_MACRO_END

#define TSUNLOCK()      CYG_MACRO_START                                 \
    ER ercd3 = ena_dsp();                                               \
    CYG_TEST_CHECK( E_OK == ercd3, "ena_dsp (TSUNLOCK) bad ercd3" );    \
CYG_MACRO_END

#define ICWAIT( _i_ )   CYG_MACRO_START                                 \
    int loops;                                                          \
    for ( loops = 3; (0 < loops) || ((_i_) > intercount); loops-- ) {   \
        ER ercd4 = rot_rdq( 0 ); /* yield */                            \
        CYG_TEST_CHECK( E_OK == ercd4, "rot_rdq (ICWAIT) bad ercd4" );  \
    }                                                                   \
CYG_MACRO_END
#endif // CYGSEM_KERNEL_SCHED_TIMESLICE

/*
#define IC() \
CYG_MACRO_START \
  static char *msgs[] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "LOTS" }; \
  CYG_TEST_INFO( msgs[ intercount > 5 ? 5 : intercount ] ); \
CYG_MACRO_END
*/

// #define CYG_TEST_UITRON_TEST1_LOOPING 1

void task1( unsigned int arg )
{
    ER ercd;
    T_RTSK ref_tskd;

#ifdef CYG_TEST_UITRON_TEST1_LOOPING
    while ( 1 ) {
#endif // CYG_TEST_UITRON_TEST1_LOOPING

    CYG_TEST_INFO( "Task 1 running" );
    CYG_TEST_INFO( TIMESLICEMSG );

    intercom = 0;
    intercount = 0;

    CYG_TEST_INFO( "Testing get_tid and ref_tsk" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 1 == scratch, "tid not 1" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = get_tid( NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "get_tid bad ercd !E_PAR" );
#endif
    ercd = get_tid( NADR );
    CYG_TEST_CHECK( E_PAR == ercd, "get_tid bad ercd !E_PAR" );
    ercd = ref_tsk( &ref_tskd, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_tsk bad ercd !E_ID" );
    ercd = ref_tsk( &ref_tskd, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "ref_tsk bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = ref_tsk( NULL, 1 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_tsk bad ercd !E_PAR" );
#endif
    ercd = ref_tsk( NADR, 1 );
    CYG_TEST_CHECK( E_PAR == ercd, "ref_tsk bad ercd !E_PAR" );
#endif // we can test bad param error returns
    ercd = ref_tsk( &ref_tskd, 1 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_RUN == ref_tskd.tskstat, "Bad task status 1" );
    ercd = ref_tsk( &ref_tskd, 0 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_RUN == ref_tskd.tskstat, "Bad task status 0" );
    ercd = ref_tsk( &ref_tskd, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_DMT == ref_tskd.tskstat, "Bad task status 2" );
    CYG_TEST_CHECK( 2 == ref_tskd.tskpri, "Bad task prio 2" );

    ercd = rsm_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rsm_tsk DMT bad ercd !E_OBJ" );
    ercd = frsm_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "frsm_tsk DMT bad ercd !E_OBJ" );
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai DMT bad ercd !E_OBJ" );
    ercd = sus_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "sus_tsk DMT bad ercd !E_OBJ" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "wup_tsk DMT bad ercd !E_OBJ" );
    ercd = can_wup( &scratch, 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "can_wup DMT bad ercd !E_OBJ" );

    CYG_TEST_PASS( "get_tid, ref_tsk" );

    CYG_TEST_INFO( "Testing prio change and start task" );
    ercd = sta_tsk( 2, 99 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );

    // drop pri of task 2
    ercd = chg_pri( 2, 4 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ref_tsk( &ref_tskd, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_RDY == ref_tskd.tskstat, "Bad task status 2" );
    CYG_TEST_CHECK( 4 == ref_tskd.tskpri, "Bad task prio 2" );

    // drop our pri below task 2
    ercd = chg_pri( 0, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );

    ercd = ref_tsk( &ref_tskd, 1 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( 5 == ref_tskd.tskpri, "Bad task prio 1" );
    ercd = ref_tsk( &ref_tskd, 0 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( 5 == ref_tskd.tskpri, "Bad task prio 0" );
    ercd = ref_tsk( &ref_tskd, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    // it will have run to completion and regained its original prio
    CYG_TEST_CHECK( 2 == ref_tskd.tskpri, "Bad task prio 2" );
    CYG_TEST_CHECK( TTS_DMT == ref_tskd.tskstat, "Bad task status 2" );

    // retest these now that the task has executed once
    ercd = rsm_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rsm_tsk DMT bad ercd !E_OBJ" );
    ercd = frsm_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "frsm_tsk DMT bad ercd !E_OBJ" );
    ercd = rel_wai( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "rel_wai DMT bad ercd !E_OBJ" );
    ercd = sus_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "sus_tsk DMT bad ercd !E_OBJ" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "wup_tsk DMT bad ercd !E_OBJ" );
    ercd = can_wup( &scratch, 2 );
    CYG_TEST_CHECK( E_OBJ == ercd, "can_wup DMT bad ercd !E_OBJ" );

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = chg_pri( -6, 9 );
    CYG_TEST_CHECK( E_ID == ercd, "chg_pri bad ercd !E_ID" );
    ercd = chg_pri( 99, 9 );
    CYG_TEST_CHECK( E_ID == ercd, "chg_pri bad ercd !E_ID" );
    ercd = sta_tsk( -6, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "sta_tsk bad ercd !E_ID" );
    ercd = sta_tsk( 99, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "sta_tsk bad ercd !E_ID" );
#endif // we can test bad param error returns

    CYG_TEST_PASS( "sta_tsk, chg_pri" );

    CYG_TEST_INFO( "Testing delay and dispatch disabling" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_CTX == ercd, "dly_tsk bad ercd !E_CTX" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_CTX == ercd, "dly_tsk bad ercd !E_CTX" );
#endif // we can test bad param error returns
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

    CYG_TEST_PASS( "dly_tsk, ena_dsp, dis_dsp" );
    
    CYG_TEST_INFO( "Testing ready queue manipulation" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ercd = rot_rdq( 4 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ercd = rot_rdq( 5 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = rot_rdq( -6 );
    CYG_TEST_CHECK( E_PAR == ercd, "rot_rdq bad ercd !E_PAR" );
    ercd = rot_rdq( 99 );
    CYG_TEST_CHECK( E_PAR == ercd, "rot_rdq bad ercd !E_PAR" );
#endif // we can test bad param error returns
    
    CYG_TEST_PASS( "rot_rdq" );

    CYG_TEST_INFO( "Testing suspend/resume" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = sus_tsk( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "sus_tsk bad ercd !E_ID" );
    ercd = sus_tsk( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "sus_tsk bad ercd !E_ID" );
    ercd = rsm_tsk( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "rsm_tsk bad ercd !E_ID" );
    ercd = rsm_tsk( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "rsm_tsk bad ercd !E_ID" );
    ercd = frsm_tsk( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "frsm_tsk bad ercd !E_ID" );
    ercd = frsm_tsk( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "frsm_tsk bad ercd !E_ID" );
#endif // we can test bad param error returns
    // drop task 3 pri to same as us
    CYG_TEST_CHECK( 0 == intercount, "intercount != 0" );

    intercom = 3;                       // tell T3 to loop
    TSRELEASE();
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = sta_tsk( 3, 66 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = chg_pri( 3, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );

    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 1 );
    CYG_TEST_CHECK( 1 == intercount, "intercount != 1" );
    ercd = sus_tsk( 3 );
    TSRELEASE();
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    intercom = 0;                       // bad data to T3
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 1 == intercount, "intercount != 1" );
    intercom = 3;                       // tell T3 to loop
    TSRELEASE();
    ercd = rsm_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "rsm_tsk bad ercd" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 2 );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );

    CYG_TEST_INFO( "Command task 3 inner loop stop" );
    intercom = 2 + 4;
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );
    
    ercd = sus_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    intercom = 0;                       // bad data to T3
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ercd = sus_tsk( 3 );                // suspend AGAIN
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    ercd = sus_tsk( 3 );                //     AND AGAIN
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );
    ercd = rsm_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "rsm_tsk bad ercd" );
    ercd = rsm_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "rsm_tsk bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );
    intercom = 3;                       // tell T3 to loop
    TSRELEASE();
    ercd = rsm_tsk( 3 );                // expect restart this time
    CYG_TEST_CHECK( E_OK == ercd, "rsm_tsk bad ercd" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 3 );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );

    CYG_TEST_INFO( "Command task 3 inner loop stop 2" );
    intercom = 2 + 4;
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    
    ercd = sus_tsk( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    intercom = 0;                       // bad data to T3
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    ercd = sus_tsk( 3 );                // suspend AGAIN
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    ercd = sus_tsk( 3 );                //     AND AGAIN
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    intercom = 3;                       // tell T3 to loop
    TSRELEASE();
    ercd = frsm_tsk( 3 );               // expect restart this time
    CYG_TEST_CHECK( E_OK == ercd, "frsm_tsk bad ercd" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 4 );
    CYG_TEST_CHECK( 4 == intercount, "intercount != 4" );

    TSRELEASE();
    ercd = rsm_tsk( 3 );               // try it again
    CYG_TEST_CHECK( E_OBJ == ercd, "rsm_tsk bad ercd !E_OBJ" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 5 );
    CYG_TEST_CHECK( 5 == intercount, "intercount != 5" );

    TSRELEASE();
    ercd = frsm_tsk( 3 );               // try it again
    CYG_TEST_CHECK( E_OBJ == ercd, "frsm_tsk bad ercd !E_OBJ" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 6 );
    CYG_TEST_CHECK( 6 == intercount, "intercount != 6" );

    CYG_TEST_INFO( "Command task 3 all loops stop" );
    intercom = 4 + 8;
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 6 == intercount, "intercount != 6" );
    
    intercom = intercount = 0;

    CYG_TEST_PASS( "sus_tsk, rsm_tsk, frsm_tsk" );

    CYG_TEST_INFO( "Testing sleep/wakeup stuff" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = wup_tsk( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "wup_tsk bad ercd !E_ID" );
    ercd = wup_tsk( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "wup_tsk bad ercd !E_ID" );
    ercd = can_wup( &scratch, -6 );
    CYG_TEST_CHECK( E_ID == ercd, "can_wup bad ercd !E_ID" );
    ercd = can_wup( &scratch, 99 );
    CYG_TEST_CHECK( E_ID == ercd, "can_wup bad ercd !E_ID" );
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = can_wup( NULL, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "can_wup bad ercd !E_PAR" );
#endif
    ercd = can_wup( NADR, 2 );
    CYG_TEST_CHECK( E_PAR == ercd, "can_wup bad ercd !E_PAR" );
    
    ercd = wup_tsk( 0 );                // not ourself
    CYG_TEST_CHECK( E_ID == ercd, "wup_tsk bad ercd !E_ID" );
    ercd = wup_tsk( 1 );                // ourself
    CYG_TEST_CHECK( E_OBJ == ercd, "wup_tsk bad ercd !E_OBJ" );
#endif // we can test bad param error returns

#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = tslp_tsk( -6 );
    CYG_TEST_CHECK( E_PAR == ercd, "tslp_tsk bad ercd !E_PAR" );
#endif // we can test bad param error returns
    ercd = tslp_tsk( TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tslp_tsk bad ercd !E_TMOUT" );
    ercd = tslp_tsk( 5 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tslp_tsk bad ercd !E_TMOUT" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = tslp_tsk( TMO_FEVR );
    CYG_TEST_CHECK( E_CTX == ercd, "tslp_tsk bad ercd !E_CTX" );
     ercd = tslp_tsk( TMO_POL );
    CYG_TEST_CHECK( E_CTX == ercd, "tslp_tsk bad ercd !E_CTX" );
    ercd = tslp_tsk( 5 );
    CYG_TEST_CHECK( E_CTX == ercd, "tslp_tsk bad ercd !E_CTX" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = tslp_tsk( -6 );
    CYG_TEST_CHECK( E_PAR == ercd, "tslp_tsk bad ercd !E_PAR" );
#endif // we can test bad param error returns
    ercd = tslp_tsk( TMO_POL );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tslp_tsk bad ercd !E_TMOUT" );
    ercd = tslp_tsk( 5 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "tslp_tsk bad ercd !E_TMOUT" );

    // drop task 4 pri to same as us
    intercount = 0;
    intercom = 1;                       // test plain slp_tsk
    TSRELEASE();
    ercd = chg_pri( 4, 5 );
    CYG_TEST_CHECK( E_OBJ == ercd, "chg_pri bad ercd" );

    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = sta_tsk( 4, 77 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = chg_pri( 4, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );

    ercd = wup_tsk( 4 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 1 );
    CYG_TEST_CHECK( 1 == intercount, "intercount != 1" );
    intercom = 2;                       // test tslp_tsk
    TSRELEASE();
    ercd = wup_tsk( 4 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 2 );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );
    intercom = 3;                       // test tslp_tsk
    TSRELEASE();
    ercd = rot_rdq( 0 );
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 2 == intercount, "intercount != 2" );
    intercom = 1;                       // test slp_tsk next...
    ercd = dly_tsk( 20 );               // without a wup
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    ICWAIT( 3 );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );

    intercom = 1;                       // ...test slp_tsk
    TSRELEASE();
    ercd = dly_tsk( 20 );               // without a wup (yet)
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    TSRELEASE();
    ercd = tslp_tsk( 20 );              // yield again
    CYG_TEST_CHECK( E_TMOUT == ercd, "tslp_tsk bad ercd !E_TMOUT" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    TSRELEASE();
    ercd = rot_rdq( 0 );                // and again
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 3 == intercount, "intercount != 3" );
    TSRELEASE();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rot_rdq( 0 );                // and yield
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 4 );
    CYG_TEST_CHECK( 4 == intercount, "intercount != 4" );

    intercom = 1;                       // test slp_tsk
    TSRELEASE();
    ercd = dly_tsk( 20 );               // without a wup (yet)
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 4 == intercount, "intercount != 4" );

    // this wup will restart it when we yield:
    TSLOCK();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    // these will count up:
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    scratch = -1;
    ercd = can_wup( &scratch, 4 );
    CYG_TEST_CHECK( E_OK == ercd, "can_wup bad ercd" );
    CYG_TEST_CHECK( 2 == scratch, "Cancelled wups not 2" );
    CYG_TEST_CHECK( 4 == intercount, "intercount != 4" );
    TSUNLOCK();

    intercom = 4;                       // do nothing
    TSRELEASE();
    ercd = rot_rdq( 0 );                // and yield
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 5 );
    CYG_TEST_CHECK( 5 == intercount, "intercount != 5" );
    TSRELEASE();
    ercd = dly_tsk( 20 );               // let it do nothing
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

    TSRELEASE();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    TSRELEASE();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    TSRELEASE();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    TSRELEASE();
    ercd = dly_tsk( 20 );               // lots of wups but no sleep
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
    CYG_TEST_CHECK( 5 == intercount, "intercount != 5" );
    scratch = -1;
    ercd = can_wup( &scratch, 4 );
    CYG_TEST_CHECK( E_OK == ercd, "can_wup bad ercd" );
    CYG_TEST_CHECK( 3 == scratch, "Cancelled wups not 3" );
    // now check that they are cancelled by doing a wait again
    intercom = 1;                       // test slp_tsk
    TSRELEASE();
    ercd = rot_rdq( 0 );                // still without a wup
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );                // still without a wup
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    intercom = 4;                       // do nothing next
    TSRELEASE();
    ICWAIT( 6 );
    CYG_TEST_CHECK( 6 == intercount, "intercount != 6" );
    ercd = rot_rdq( 0 );                // still without a wup
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 6 == intercount, "intercount != 6" );
    TSRELEASE();
    ercd = wup_tsk( 4 );                // now issue a wup
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rot_rdq( 0 );                // it will run now
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    TSRELEASE();
    ercd = rot_rdq( 0 );                // it will run now
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 7 );
    CYG_TEST_CHECK( 7 == intercount, "intercount != 7" );

    TSRELEASE();
    intercom = 99;                      // exit, all done
    ercd = rot_rdq( 0 );                // let it run
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    ICWAIT( 8 );
    CYG_TEST_CHECK( 8 == intercount, "intercount != 8" );

    TSRELEASE();
    ercd = rot_rdq( 0 );                // let it run
    CYG_TEST_CHECK( E_OK == ercd, "rot_rdq bad ercd" );
    CYG_TEST_CHECK( 8 == intercount, "intercount != 8" );
   
    CYG_TEST_PASS( "wup_tsk, can_wup, slp_tsk, tslp_tsk" );

#ifdef CYG_TEST_UITRON_TEST1_LOOPING
    chg_pri( 1, 1 );
    rot_rdq( 0 );
    ter_tsk( 2 );
    rot_rdq( 0 );
    ter_tsk( 3 );
    rot_rdq( 0 );
    ter_tsk( 4 );
    rot_rdq( 0 );
    }
#endif // CYG_TEST_UITRON_TEST1_LOOPING

    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}



void task2( unsigned int arg )
{
    ER ercd;
    CYG_TEST_PASS( "Task 2 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 2 == scratch, "tid not 2" );
    if ( 99 != arg )
        CYG_TEST_FAIL( "Task 2 arg not 99" );
    ext_tsk();
    CYG_TEST_FAIL( "Task 2 failed to exit" );
}

void task3( unsigned int arg )
{
    ER ercd;
    TSLOCK();
    CYG_TEST_PASS("Task3 running");
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 3 == scratch, "tid not 3" );
    if ( 66 != arg )
        CYG_TEST_FAIL( "Task 3 arg not 66" );

    while ( 2 & intercom ) {
        while ( 1 & intercom ) {
            intercount++;
            TSSTOP();
            do {
                TSUNLOCK();
                ercd = rot_rdq( 0 );        // yield()
                TSLOCK();
                CYG_TEST_CHECK( E_OK == ercd, "rot_rdq 1 (task3) bad ercd" );
            } while ( !TSGO() );
        }
        CYG_TEST_CHECK( 4 & intercom, "should not have got here yet 1" );
        TSSTOP();
        do {
            TSUNLOCK();
            ercd = rot_rdq( 0 );            // yield()
            TSLOCK();
            CYG_TEST_CHECK( E_OK == ercd, "rot_rdq 2 (task3) bad ercd" );
        } while ( !TSGO() );
    }
    CYG_TEST_CHECK( 8 & intercom, "should not have got here yet 2" );
    
    TSUNLOCK();
    ext_tsk();
    CYG_TEST_FAIL( "Task 3 failed to exit" );
}

void task4( unsigned int arg )
{
    ER ercd;
    CYG_TEST_PASS("Task4 running");
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 4 == scratch, "tid not 4" );
    if ( 77 != arg )
        CYG_TEST_FAIL( "Task 4 arg not 77" );
    while ( 1 ) {
        switch ( intercom ) {
        case 1:
            ercd = slp_tsk();
            CYG_TEST_CHECK( E_OK == ercd, "slp_tsk (task4) bad ercd" );
            break;
        case 2:
            ercd = tslp_tsk( 10 );
            CYG_TEST_CHECK( E_OK == ercd, "slp_tsk (task4) bad ercd" );
            break;
        case 3:
            ercd = tslp_tsk( 10 );
            CYG_TEST_CHECK( E_TMOUT == ercd,
                            "slp_tsk (task4) bad ercd !E_TMOUT" );
            break;
        case 4:
            // busily do nothing
            while ( 4 == intercom ) {
                ercd = rot_rdq( 0 );
                CYG_TEST_CHECK( E_OK == ercd,
                                "rot_rdq (task4 idle) bad ercd" );
            }
            break;
        case 99:
            goto out;
        default:
            CYG_TEST_FAIL( "Task 4 bad intercom" );
            goto out;
        }
        intercount++;
        TSSTOP();
        do {
            ercd = rot_rdq( 0 );            // yield()
            CYG_TEST_CHECK( E_OK == ercd, "rot_rdq (task4) bad ercd" );
        } while ( !TSGO() );
    }
out:
    ext_tsk();
    CYG_TEST_FAIL( "Task 4 failed to exit" );
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

// EOF test1.c
