//===========================================================================
//
//      testintr.c
//
//      uITRON "C" test program for ixxx_yyy interrupt safe operators
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
// Contributors:hmt
// Date:        1998-08-20
// Purpose:     uITRON API testing
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/system.h>
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

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#include <cyg/infra/diag.h>

#include <cyg/compat/uitron/uit_func.h> // uITRON
#include <cyg/compat/uitron/uit_ifnc.h> // uITRON interrupt funcs

void set_interrupt_number( void );

unsigned int clock_interrupt = 0;

externC void
cyg_package_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO( "Calling cyg_uitron_start()" );
    set_interrupt_number();
    cyg_uitron_start();
}

extern "C" {
    void task1( unsigned int arg );
    void task2( unsigned int arg );
    void task3( unsigned int arg );
    void task4( unsigned int arg );
}

volatile int intercom = 0;
INT scratch = 0;

// Plan: replace (by direct intervention) the ISR and DSR of the regular
// timer interrupt; be sure to ack the clock intr using the appropriate hal
// macros.
// 
// The new ISR(s) will simply use the interrupt-safe signalling functions
// to control a 2nd task.  Main task will check on the state thereof.
//
// We must test the ixxx_yyy() funcs with the scheduler already locked
// also, by direct sched calls on the KAPI.  This must verify that the
// signal only happens when the scheduler unlocks.
// 
// The 4 producer ops are:
//     iwup_tsk ( ID tskid );
//     isig_sem ( ID semid );
//     iset_flg ( ID flgid, UINT setptn );
//     isnd_msg ( ID mbxid, T_MSG *pk_msg );
//
// and return macros are:
//     ret_wup( ID tskid );
//     ret_int();
//
// These ISRs perform the producer ops on all available objects in turn.
// Tasks 2-4
// Semas 1-4
// Flags 1-4 with marching bit data; they'll all be set to 0x1ff eventually
// Mboxes 1-4 with an arbitrary pointer

enum {
    NOTHING = 0,
    SLP,
    SEM,
    FLG,
    MBX,
    EXIT
};

#define ACK_CLOCK() CYG_MACRO_START                             \
    HAL_CLOCK_RESET( CYGNUM_HAL_INTERRUPT_RTC,                  \
                     CYGNUM_KERNEL_COUNTERS_RTC_PERIOD );       \
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_RTC );      \
CYG_MACRO_END

#define CHECK_TID() CYG_MACRO_START                     \
    int my_tid;                                         \
    ER ercd;                                            \
    ercd = get_tid( &my_tid );                          \
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" ); \
    CYG_TEST_CHECK( 0 == my_tid, "tid not 0 in ISR" );  \
CYG_MACRO_END


unsigned int
isr_wup_tsk( unsigned int vector, unsigned int data )
{
    // Hit TASKS in range 2..4
    static int wtid = 2;
    ACK_CLOCK();
    CHECK_TID();
    iwup_tsk( wtid );
    wtid++;
    if ( 5 == wtid ) wtid = 2;
    ret_int();
}

unsigned int
isr_ret_wup( unsigned int vector, unsigned int data )
{
    // Hit TASKS in range 2..4
    static int rwid = 2;
    ACK_CLOCK();
    CHECK_TID();
    rwid++;
    if ( 6 == rwid ) rwid = 3;
    ret_wup( rwid - 1 );
}

unsigned int
isr_sig_sem( unsigned int vector, unsigned int data )
{
    // Hit SEMAS in range 1..3
    static int ssid = 1;
    ACK_CLOCK();
    CHECK_TID();
    isig_sem( ssid );
    ssid++;
    if ( ssid == 4 ) ssid = 1;
    ret_int();
}

unsigned int
isr_set_flg( unsigned int vector, unsigned int data )
{
    // Hit FLAGS in range 1..4
    static int sfid = 1;
    static int sfdata = 0xff;
    ACK_CLOCK();
    CHECK_TID();
    iset_flg( sfid, sfdata );
    sfid++;
    if ( sfid == 5 ) sfid = 1;
//    sfdata <<= 1;
//    if ( sfdata == 0x20 ) sfdata = 1; // so that eventually all 0x1f set
    ret_int();
}

unsigned int
isr_snd_msg( unsigned int vector, unsigned int data )
{
    // Hit MBOXES in range 1..4
    static int smid = 1;
    ACK_CLOCK();
    CHECK_TID();
    isnd_msg( smid, (T_MSG *)&smid );
    smid++;
    if ( smid == 5 ) smid = 1;
    ret_int();
}


void attach_isr( unsigned int (*isr)(unsigned int, unsigned int) );
void detach_isr( unsigned int (*isr)(unsigned int, unsigned int) );

void lock_sched( void );
void unlock_sched( void );

volatile int count = -1;

/*
#define BIGDELAY  50000000
#define SMALLDELAY (BIGDELAY/SMALLLOOPS)
#define SMALLLOOPS 3

#define xxxLONGDELAY()                                     \
do {                                                    \
    int i;                                              \
    for ( i = 0; i < BIGDELAY; i++ )                    \
        if ( wakeups[ 4 ] > prewups[ 4 ] + 99 ) break;  \
} while ( 0 )
   
#define xxxDELAYLOCKSCHED()                                        \
do {                                                            \
    int i,j;                                                    \
    for ( j = 0; j < SMALLLOOPS; j++ ) {                        \
        lock_sched();                                           \
        for ( i = 0; i < SMALLDELAY; i++ )                      \
            if ( wakeups[ 4 ] > prewups[ 4 ] + 99 ) break;      \
        unlock_sched();                                         \
        if ( wakeups[ 4 ] > prewups[ 4 ] + 99 ) break;          \
    }                                                           \
} while ( 0 )
*/

#define SMALLDELAYHW  (5000000)
#define EVENTSHW      (     20)
#define SMALLDELAYSIM ( 100000)
#define EVENTSSIM     (      4)

#define SMALLDELAY (smalldelay)
#define EVENTS     (events)

static int smalldelay = SMALLDELAYHW;
static int events     = EVENTSHW;

#define LONGDELAY() do {                                        \
    count = 0;                                                  \
    do count++; while ( wakeups[ 4 ] < prewups[ 4 ] + EVENTS ); \
} while ( 0 )


#define DELAYLOCKSCHED()                                              \
do {                                                                  \
    count = 0;                                                        \
    int i;                                                            \
    do {                                                              \
        lock_sched();                                                 \
        for ( i = 0; i < SMALLDELAY; i++ ) {                          \
            count++;                                                  \
            if ( wakeups[ 4 ] >= prewups[ 4 ] + EVENTS )              \
                break;                                                \
        }                                                             \
        unlock_sched();                                               \
        CYG_TEST_INFO("  [Still iterating, please wait....]  ");      \
    } while ( wakeups[ 4 ] < prewups[ 4 ] + EVENTS );                 \
} while ( 0 )

#define DELAY()                                 \
if ( 1 & loops )                                \
    DELAYLOCKSCHED();                           \
else                                            \
    LONGDELAY();
    

volatile int wakeups[ 5 ] = { 0,0,0,0,0 };
volatile int prewups[ 5 ] = { 0,0,0,0,0 };


void task1( unsigned int arg )
{
    ER ercd;
    int loops;

    CYG_TEST_INFO( "Task 1 running" );

    if ( cyg_test_is_simulator ) {
        // take less time
        events     = EVENTSSIM;
    }


    // First test that dis_int() and ena_int() work for the clock interrupt
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = ena_int( 123456789 ); // Hope this is large enough to error
    CYG_TEST_CHECK( E_PAR == ercd, "ena_int bad ercd !E_PAR" );
    ercd = dis_int( 123456789 );
    CYG_TEST_CHECK( E_PAR == ercd, "dis_int bad ercd !E_PAR" );
#endif

    // This may take too long on a sim...
    // On the synthetic target this test cannot run reliably - the
    // loop counting assumes exclusive access to the processor.
#ifndef CYGPKG_HAL_SYNTH    
    if ( ! cyg_test_is_simulator ) {
        SYSTIME t1, t2;

        CYG_TEST_INFO( "Testing masking of clock interrupt" );

        ercd = get_tim( &t1 );
        CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );

        // Wait for a tick. This loop acts as a synchronizer for the loop
        // below, ensuring that it starts just after a tick.
        for ( loops = 0; loops < 10000000; loops++ ) {
            ercd = get_tim( &t2 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        // Wait for next tick. Reset loops counter so we get the
        // approximate loop count of one clock tick.
        for ( loops = 0; loops < 10000000; loops++ ) {
            ercd = get_tim( &t1 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }

        // save how many loops could be executed in one tick. Multiply
        // with 3 : we run loops in pairs below and add the time of
        // one extra to avoid small variations to trigger failures.
        intercom = loops * 3;

        ercd = ena_int( clock_interrupt ); // was initialized already
        CYG_TEST_CHECK( E_OK == ercd, "ena_int bad ercd" );

        ercd = get_tim( &t1 );
        CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );

        // Wait for a tick
        for ( loops = intercom; loops > 0; loops-- ) {
            ercd = get_tim( &t2 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        CYG_TEST_CHECK( 0 < loops, "No first tick" );
        // and a second one
        for (                 ; loops > 0; loops-- ) {
            ercd = get_tim( &t1 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        CYG_TEST_CHECK( 0 < loops, "No second tick" );
        
        // The PowerPC cannot disable the timer interrupt (separately).
#ifndef CYGPKG_HAL_POWERPC
        ercd = dis_int( clock_interrupt ); // was initialized already
        CYG_TEST_CHECK( E_OK == ercd, "dis_int bad ercd" );

        ercd = get_tim( &t1 );
        CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );

        // Wait for a tick (should not happen)
        for ( loops = intercom; loops > 0; loops-- ) {
            ercd = get_tim( &t2 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        CYG_TEST_CHECK( 0 == loops, "A tick occured - should be masked" );
        CYG_TEST_CHECK( t1 == t2, "Times are different" );

        // Now enable it again and ensure all is well:
        ercd = ena_int( clock_interrupt );
        CYG_TEST_CHECK( E_OK == ercd, "ena_int bad ercd" );
#endif
        
        ercd = get_tim( &t1 );
        CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );

        // Wait for a tick
        for ( loops = intercom; loops > 0; loops-- ) {
            ercd = get_tim( &t2 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        CYG_TEST_CHECK( 0 < loops, "No first tick" );
        // and a second one
        for (                 ; loops > 0; loops-- ) {
            ercd = get_tim( &t1 );
            CYG_TEST_CHECK( E_OK == ercd, "get_tim bad ercd" );
            if ( t2 != t1 )
                break;
        }
        CYG_TEST_CHECK( 0 < loops, "No second tick" );

        CYG_TEST_PASS( "dis_int(), ena_int() OK" );
    }
#endif
    
    intercom = 0;

    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 1 == scratch, "tid not 1" );

    // start all other tasks (our prio is 1 by default)
    ercd = sta_tsk( 2, 222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk 2 bad ercd" );
    ercd = sta_tsk( 3, 333 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk 3 bad ercd" );
    ercd = sta_tsk( 4, 444 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk 4 bad ercd" );
    // drop pri of other tasks all to 5
    ercd = chg_pri( 2, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri 2 bad ercd" );
    ercd = chg_pri( 3, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri 3 bad ercd" );
    ercd = chg_pri( 4, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri 4 bad ercd" );

    // Test sleep/wakeup
    intercom = SLP;
    // Drop our prio to lower; they will run in turn until asleep
    ercd = chg_pri( 1, 6 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri 1 (self) bad ercd" );
    
    loops = 4;
    do {

        if ( 1 & loops )
            CYG_TEST_INFO( " (toggling scheduler lock) " );
        else
            CYG_TEST_INFO( " (unlocked scheduler) " );


        CYG_TEST_CHECK(          0 == wakeups[0], "init: Wakeups[0] hit" );
        CYG_TEST_CHECK(          0 == wakeups[1], "init: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] == wakeups[2], "init: Wakeups[2] hit" );
        CYG_TEST_CHECK( prewups[3] == wakeups[3], "init: Wakeups[3] hit" );
        CYG_TEST_CHECK( prewups[4] == wakeups[4], "init: Wakeups[4] hit" );

        // -------- TIMERS AND TIMESLICING DISABLED ---------
        // install an isr that will wake them all up in turn
        attach_isr( isr_wup_tsk );
        DELAY();
        detach_isr( isr_wup_tsk );
        // -------- timers and timeslicing ENABLED ---------
        
        CYG_TEST_CHECK(         0 == wakeups[0], "iwup_tsk: Wakeups[0] hit" );
        CYG_TEST_CHECK(         0 == wakeups[1], "iwup_tsk: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] < wakeups[2], "iwup_tsk: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] < wakeups[3], "iwup_tsk: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] < wakeups[4], "iwup_tsk: Wakeups[4] not hit" );
        diag_printf( "INFO:<(fg loops %10d) thread wakeups : %2d %2d %2d >\n", count,
                     wakeups[2] - prewups[2],
                     wakeups[3] - prewups[3],
                     wakeups[4] - prewups[4] );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];
        
        // -------- TIMERS AND TIMESLICING DISABLED ---------
        // install an isr that will wake them all up in turn
        attach_isr( isr_ret_wup );
        DELAY();
        detach_isr( isr_ret_wup );
        // -------- timers and timeslicing ENABLED ---------
        
        CYG_TEST_CHECK(         0 == wakeups[0], "ret_wup: Wakeups[0] hit" );
        CYG_TEST_CHECK(         0 == wakeups[1], "ret_wup: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] < wakeups[2], "ret_wup: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] < wakeups[3], "ret_wup: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] < wakeups[4], "ret_wup: Wakeups[4] not hit" );
        diag_printf( "INFO:<(fg loops %10d) thread ret_wups: %2d %2d %2d >\n", count,
                     wakeups[2] - prewups[2],
                     wakeups[3] - prewups[3],
                     wakeups[4] - prewups[4] );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];
        
        // move them on to waiting for a semaphore
        intercom = SEM;
        ercd = wup_tsk( 2 );
        CYG_TEST_CHECK( E_OK == ercd, "wup_tsk(2) bad ercd" );
        ercd = wup_tsk( 3 );
        CYG_TEST_CHECK( E_OK == ercd, "wup_tsk(3) bad ercd" );
        ercd = wup_tsk( 4 );
        CYG_TEST_CHECK( E_OK == ercd, "wup_tsk(4) bad ercd" );
        
        CYG_TEST_CHECK(              0 == wakeups[0], "wup_tsk: Wakeups[0] hit" );
        CYG_TEST_CHECK(              0 == wakeups[1], "wup_tsk: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] + 1 == wakeups[2], "wup_tsk: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] + 1 == wakeups[3], "wup_tsk: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] + 1 == wakeups[4], "wup_tsk: Wakeups[4] not hit" );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];
        
        // -------- TIMERS AND TIMESLICING DISABLED ---------
        // install an isr that will wake them all up in turn
        attach_isr( isr_sig_sem );
        DELAY();
        detach_isr( isr_sig_sem );
        // -------- timers and timeslicing ENABLED ---------
        
        CYG_TEST_CHECK(         0 == wakeups[0], "isig_sem: Wakeups[0] hit" );
        CYG_TEST_CHECK(         0 == wakeups[1], "isig_sem: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] < wakeups[2], "isig_sem: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] < wakeups[3], "isig_sem: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] < wakeups[4], "isig_sem: Wakeups[4] not hit" );
        diag_printf( "INFO:<(fg loops %10d) semaphore waits: %2d %2d %2d >\n", count,
                     wakeups[2] - prewups[2],
                     wakeups[3] - prewups[3],
                     wakeups[4] - prewups[4] );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];

        // move them on to waiting for a flag
        intercom = FLG;
        ercd = sig_sem( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "sig_sem(1) bad ercd" );
        ercd = sig_sem( 2 );
        CYG_TEST_CHECK( E_OK == ercd, "sig_sem(2) bad ercd" );
        ercd = sig_sem( 3 );
        CYG_TEST_CHECK( E_OK == ercd, "sig_sem(3) bad ercd" );

        CYG_TEST_CHECK(              0 == wakeups[0], "sig_sem: Wakeups[0] hit" );
        CYG_TEST_CHECK(              0 == wakeups[1], "sig_sem: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] + 1 == wakeups[2], "sig_sem: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] + 1 == wakeups[3], "sig_sem: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] + 1 == wakeups[4], "sig_sem: Wakeups[4] not hit" );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];
        
        // -------- TIMERS AND TIMESLICING DISABLED ---------
        // install an isr that will wake them all up in turn
        attach_isr( isr_set_flg );
        DELAY();
        detach_isr( isr_set_flg );
        // -------- timers and timeslicing ENABLED ---------
        
        CYG_TEST_CHECK(         0 == wakeups[0], "iset_flg: Wakeups[0] hit" );
        CYG_TEST_CHECK(         0 == wakeups[1], "iset_flg: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] < wakeups[2], "iset_flg: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] < wakeups[3], "iset_flg: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] < wakeups[4], "iset_flg: Wakeups[4] not hit" );
        diag_printf( "INFO:<(fg loops %10d) flag waits/sets: %2d %2d %2d >\n", count,
                     wakeups[2] - prewups[2],
                     wakeups[3] - prewups[3],
                     wakeups[4] - prewups[4] );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];

        // move them on to waiting for a message box
        intercom = MBX;
        ercd = set_flg( 2, 0xfff );
        CYG_TEST_CHECK( E_OK == ercd, "set_flg(2) bad ercd" );
        ercd = set_flg( 3, 0xfff );
        CYG_TEST_CHECK( E_OK == ercd, "set_flg(3) bad ercd" );
        ercd = set_flg( 4, 0xfff );
        CYG_TEST_CHECK( E_OK == ercd, "set_flg(4) bad ercd" );
        
        CYG_TEST_CHECK(              0 == wakeups[0], "set_flg: Wakeups[0] hit" );
        CYG_TEST_CHECK(              0 == wakeups[1], "set_flg: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] + 1 == wakeups[2], "set_flg: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] + 1 == wakeups[3], "set_flg: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] + 1 == wakeups[4], "set_flg: Wakeups[4] not hit" );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];
        
        // -------- TIMERS AND TIMESLICING DISABLED ---------
        // install an isr that will wake them all up in turn
        attach_isr( isr_snd_msg );
        DELAY();
        detach_isr( isr_snd_msg );
        // -------- timers and timeslicing ENABLED ---------

        CYG_TEST_CHECK(         0 == wakeups[0], "isnd_msg: Wakeups[0] hit" );
        CYG_TEST_CHECK(         0 == wakeups[1], "isnd_msg: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] < wakeups[2], "isnd_msg: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] < wakeups[3], "isnd_msg: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] < wakeups[4], "isnd_msg: Wakeups[4] not hit" );
        diag_printf( "INFO:<(fg loops %10d) message rec'pts: %2d %2d %2d >\n", count,
                     wakeups[2] - prewups[2],
                     wakeups[3] - prewups[3],
                     wakeups[4] - prewups[4] );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];

        // move them on to exiting, all done
        if ( 1 == loops )
            // then we are about to exit
            intercom = EXIT;
        else
            intercom = SLP;
        ercd = snd_msg( 2, (T_MSG *)&intercom );
        CYG_TEST_CHECK( E_OK == ercd, "snd_msg(2) bad ercd" );
        ercd = snd_msg( 3, (T_MSG *)&intercom );
        CYG_TEST_CHECK( E_OK == ercd, "snd_msg(3) bad ercd" );
        ercd = snd_msg( 4, (T_MSG *)&intercom );
        CYG_TEST_CHECK( E_OK == ercd, "snd_msg(4) bad ercd" );

        CYG_TEST_CHECK(              0 == wakeups[0], "snd_msg: Wakeups[0] hit" );
        CYG_TEST_CHECK(              0 == wakeups[1], "snd_msg: Wakeups[1] hit" );
        CYG_TEST_CHECK( prewups[2] + 1 == wakeups[2], "snd_msg: Wakeups[2] not hit" );
        CYG_TEST_CHECK( prewups[3] + 1 == wakeups[3], "snd_msg: Wakeups[3] not hit" );
        CYG_TEST_CHECK( prewups[4] + 1 == wakeups[4], "snd_msg: Wakeups[4] not hit" );
        prewups[2] = wakeups[2];
        prewups[3] = wakeups[3];
        prewups[4] = wakeups[4];

        CYG_TEST_PASS( "Tested ISR invoked uITRON functions" );

    } while ( 0 < --loops );

    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}


void body( int n )
{
    unsigned int z;
    ER ercd;
    T_MSG *pk_msg;

    do {
        switch ( intercom ) {
        case NOTHING:
            ercd = slp_tsk();
            CYG_TEST_CHECK( E_OK == ercd, "slp_tsk (doing nothing)" );
            continue;
        case SLP:
            ercd = slp_tsk();
            CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
            wakeups[ n ]++;
            break;
        case SEM:
            ercd = wai_sem( n-1 ); // 1..3 for semas
            CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
            wakeups[ n ]++;
            break;
        case FLG:
            ercd = wai_flg( &z, n, (1<<n), TWF_CLR | TWF_ANDW );
            CYG_TEST_CHECK( E_OK == ercd, "wai_flg bad ercd" );
            CYG_TEST_CHECK( z & (1<<n), "Flag bit not set" );
            wakeups[ n ]++;
            break;
        case MBX:
            ercd = rcv_msg( &pk_msg, n );
            CYG_TEST_CHECK( E_OK == ercd, "rcv_msg bad ercd" );
            CYG_TEST_CHECK( pk_msg, "rcv_msg NULL msg" );
            wakeups[ n ]++;
            break;
        case EXIT:
            return;
        }
    } while ( 1 );
}

void task2( unsigned int arg )
{
    ER ercd;
    CYG_TEST_INFO( "Task 2 running" );
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 2 == scratch, "tid not 2" );
    if ( 222 != arg )
        CYG_TEST_FAIL( "Task 2 arg not 222" );
    body(2);
    CYG_TEST_INFO( "Task 2 exiting" );
    ext_tsk();
    CYG_TEST_FAIL( "Task 2 failed to exit" );
}

void task3( unsigned int arg )
{
    ER ercd;
    CYG_TEST_INFO("Task 3 running");
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 3 == scratch, "tid not 3" );
    if ( 333 != arg )
        CYG_TEST_FAIL( "Task 3 arg not 333" );
    body(3);
    CYG_TEST_INFO( "Task 3 exiting" );
    ext_tsk();
    CYG_TEST_FAIL( "Task 3 failed to exit" );
}

void task4( unsigned int arg )
{
    ER ercd;
    CYG_TEST_INFO("Task 4 running");
    ercd = get_tid( &scratch );
    CYG_TEST_CHECK( E_OK == ercd, "get_tid bad ercd" );
    CYG_TEST_CHECK( 4 == scratch, "tid not 4" );
    if ( 444 != arg )
        CYG_TEST_FAIL( "Task 4 arg not 444" );
    body(4);
    CYG_TEST_INFO( "Task 4 exiting" );
    ext_tsk();
    CYG_TEST_FAIL( "Task 4 failed to exit" );
}

// ------------------------------------------------------------------------
// Start of C++ aware portion, so to speak.
//

#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/intr.hxx>
#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/sched.inl>

void set_interrupt_number( void )
{
    clock_interrupt = CYGNUM_HAL_INTERRUPT_RTC;
}

// This snippet stolen from kernel/.../clock.cxx to be able to detach
// the RTC from its interrupt source.
class Cyg_RealTimeClock
    : public Cyg_Clock
{
public:
    Cyg_Interrupt       interrupt;

    static cyg_uint32 isr(cyg_vector vector, CYG_ADDRWORD data);

    static void dsr(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data);

    Cyg_RealTimeClock();
};


static Cyg_Interrupt uit_intr(
        (unsigned)CYGNUM_HAL_INTERRUPT_RTC, // Vector to attach to
        0,                              // Queue priority
        (unsigned)0,                    // Data pointer
        &isr_wup_tsk,                   // Interrupt Service Routine
        &cyg_uitron_dsr                 // Deferred Service Routine
);

void
attach_isr( unsigned int (*isr)(unsigned int, unsigned int) )
{
    int inuse;
    int old_ints;
    Cyg_RealTimeClock *prtc = (Cyg_RealTimeClock *)Cyg_Clock::real_time_clock;
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_MASK( CYGNUM_HAL_INTERRUPT_RTC );
    prtc->interrupt.detach();
#ifndef CYGIMP_KERNEL_INTERRUPTS_CHAIN
    // Only check that the vector was cleared when there's a specific
    // vector for the RTC.  In chain mode, other interrupt handlers
    // may prevent the shared vector from being cleared when detaching
    // the RTC ISR, and this assertion fails.
    HAL_INTERRUPT_IN_USE( CYGNUM_HAL_INTERRUPT_RTC, inuse );
    CYG_TEST_CHECK( !inuse, "Failed to detach clock ISR" );
#endif
    uit_intr = Cyg_Interrupt( 
        CYGNUM_HAL_INTERRUPT_RTC,       // Vector to attach to
        1,                              // Queue priority
        0,                              // Data pointer
        isr,                            // Interrupt Service Routine
        cyg_uitron_dsr                  // Deferred Service Routine
        );    
    uit_intr.attach();
    HAL_INTERRUPT_IN_USE( CYGNUM_HAL_INTERRUPT_RTC, inuse );
    CYG_TEST_CHECK( inuse, "Failed to attach new ISR" );
    ACK_CLOCK();
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_RTC );
    HAL_RESTORE_INTERRUPTS(old_ints);
}

void
detach_isr( unsigned int (*isr)(unsigned int, unsigned int) )
{
    int inuse;
    int old_ints;
    Cyg_RealTimeClock *prtc = (Cyg_RealTimeClock *)Cyg_Clock::real_time_clock;
    HAL_DISABLE_INTERRUPTS(old_ints);
    HAL_INTERRUPT_MASK( CYGNUM_HAL_INTERRUPT_RTC );
    uit_intr.detach();
#ifndef CYGIMP_KERNEL_INTERRUPTS_CHAIN
    // See comment above in attach_isr.
    HAL_INTERRUPT_IN_USE( CYGNUM_HAL_INTERRUPT_RTC, inuse );
    CYG_TEST_CHECK( !inuse, "Failed to detach my ISR" );
#endif
    prtc->interrupt.attach();
    HAL_INTERRUPT_IN_USE( CYGNUM_HAL_INTERRUPT_RTC, inuse );
    CYG_TEST_CHECK( inuse, "Failed to attach clock ISR" );
    ACK_CLOCK();
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_RTC );
    HAL_RESTORE_INTERRUPTS(old_ints);
}


void
lock_sched( void )
{
    cyg_uint32 l;
    Cyg_Scheduler::lock();
    l = Cyg_Scheduler::get_sched_lock();
    CYG_TEST_CHECK( 0 < l, "lock: Sched not locked" );
    CYG_TEST_CHECK( 2 > l, "lock: Sched already locked" );
}

void
unlock_sched( void )
{
    cyg_uint32 l;
    l = Cyg_Scheduler::get_sched_lock();
    CYG_TEST_CHECK( 0 < l, "unlock: Sched not locked" );
    CYG_TEST_CHECK( 2 > l, "unlock: Sched already locked" );
    Cyg_Scheduler::unlock();
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

// EOF testintr.c
