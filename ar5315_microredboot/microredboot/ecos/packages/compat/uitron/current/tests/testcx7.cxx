//===========================================================================
//
//      testcx7.cxx
//      
//      uITRON "C++" test program seven
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
// Date:        1998-10-14
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

#include <cyg/kernel/test/stackmon.h>   // stack analysis tools

#include <cyg/compat/uitron/uit_func.h> // uITRON

externC void
cyg_package_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO( "Calling cyg_uitron_start()" );
    cyg_uitron_start();
}


extern "C" {
    void task1( unsigned int arg );
    void task2( unsigned int arg );
    void task3( unsigned int arg );
    void task4( unsigned int arg );
}

// ========================================================================

enum {
    START_WAITOP = 0,
    SLEEP = 0,
    DELAY,
    SEMGET,
    FLAGWAIT,
    MSGGET,
    MEMFIXEDGET,
    MEMVARGET,
    DONE_WAITOP
};
typedef int WAITOP;

enum {
    START_TYPE = 0,
    PLAIN = 0,
    TIMED = 1,
    DONE_TYPE
};
typedef int WAITTYPE;

enum {
    START_KILLOP = 0,

    // These are the 5 ways out of a wait that we perm
    // with other circumstances:
    SIGNAL = 0,                         // do the appropriate producer op
    TIMEOUT,                            // wait for the timeout to fire
    RELEASE,                            // do a rel_wai()
    DELETE,                             // delete the object; del_xxx()
    KILL,                               // do a ter_tsk() on the waiter

    SUSPEND_SIGNAL_RESUME,
    SUSPEND_TIMEOUT_RESUME,
    SUSPEND_RELEASE_RESUME,
    SUSPEND_DELETE_RESUME,
    SUSPEND_KILL,                       // resume not applicable

    SUSPEND_SIGNAL_KILL,
    SUSPEND_TIMEOUT_KILL,
    SUSPEND_RELEASE_KILL,
    SUSPEND_DELETE_KILL,
    // SUSPEND_KILL_KILL not applicable

#if 0
    // support these later if _really_ keen.
    SUSPEND_SIGNAL_DELETE_RESUME,
    SUSPEND_TIMEOUT_DELETE_RESUME,
    SUSPEND_RELEASE_DELETE_RESUME,
    // SUSPEND_DELETE_DELETE_RESUME not applicable
    // SUSPEND_KILL_DELETE_RESUME not applicable

    SUSPEND_SIGNAL_DELETE_KILL,
    SUSPEND_TIMEOUT_DELETE_KILL,
    SUSPEND_RELEASE_DELETE_KILL,
    // SUSPEND_DELETE_DELETE_KILL,
    SUSPEND_KILL_DELETE                 // 2nd kill not applicable
#endif
          
    DONE_KILLOP
};
typedef int KILLOP;
        
// ========================================================================

char * waitstrings[] =
{ "Sleep ", "Delay ", "Sema  ", "Flag  ", "Mbox  ", "MemFix", "MemVar" };

char * typestrings[] =
{ " (Plain) : ", " (Timed) : " };

char * killstrings[] =
{ "Signal",
  "Wait-for-timeout",
  "Release-wait",
  "Delete-object",
  "Kill-task",

  "Suspend/Signal/Resume",
  "Suspend/Wait-for-timeout/Resume",
  "Suspend/Release-wait/Resume",
  "Suspend/Delete-object/Resume",
  "Suspend/Kill-task",

  "Suspend/Signal/Kill-task",
  "Suspend/Wait-for-timeout/Kill-task",
  "Suspend/Release-wait/Kill-task",
  "Suspend/Delete-object/Kill-task",
  

};

// ========================================================================

inline int task2arg( WAITOP wait, WAITTYPE waittype, KILLOP kill )
{
    return waittype + (wait << 1) + (kill << 8);
}

inline void decodearg( int arg, WAITOP *pwait, WAITTYPE *pwaittype, KILLOP *pkill )
{
    *pwaittype = (arg & 1) ? TIMED : PLAIN;
    *pwait  = (arg >> 1) & 0x7f;
    *pkill  = (arg >> 8);
}

static char *strdog( char *p, char *q )
{
    while ( 0 != (*p++ = *q++) );
    return p - 1;
}

static char *
makemsg( char *z, WAITOP wait, WAITTYPE waittype, KILLOP kill )
{
    static char buf[ 1000 ];
    char *p = buf;
    p = strdog( p, z );
    p = strdog( p, waitstrings[ wait ] );
    p = strdog( p, typestrings[ waittype ] );
    p = strdog( p, killstrings[ kill ] );
    *p = 0;
    return buf;
}

// ========================================================================

volatile int intercom = 0;

// ========================================================================

T_RTSK rtsk;

void
do_suspend( void )
{
    ER ercd;
    ercd = ref_tsk( &rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_WAI == rtsk.tskstat, "bad tskstat !TTS_WAI" );
    ercd = sus_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "sus_tsk bad ercd" );
    ercd = ref_tsk( &rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_WAS == rtsk.tskstat, "bad tskstat !TTS_WAS" );
}

void
do_resume( void )
{
    ER ercd;
    ercd = ref_tsk( &rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_SUS == rtsk.tskstat, "bad tskstat !TTS_SUS" );
    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = rsm_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "rsm_tsk bad ercd" );
    ercd = ref_tsk( &rtsk, 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
    CYG_TEST_CHECK( TTS_RDY == rtsk.tskstat, "bad tskstat !TTS_RDY" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
}

// ========================================================================

#define T1_WAIT (7)
#define T2_WAIT (5)

#define T1_MALLOC (110)
#ifdef CYGSEM_KERNEL_MEMORY_COALESCE
#define T2_MALLOC (100)
#else
#define T2_MALLOC T1_MALLOC
#endif

VP vptmp;
VP vp = NULL;
VP vp1 = NULL;
VP t2vp = NULL;
VP t2vp_backup = NULL;

UINT scratch;

T_MSG *msg = (T_MSG *)&scratch;
T_MSG *msg1;

void
do_prep( WAITOP wait )
{
    ER ercd;
    switch ( wait ) {
    case SLEEP:
    case DELAY:
    case SEMGET:
    case FLAGWAIT:
    case MSGGET:
        // do nothing for all of those
        break;
    case MEMFIXEDGET:
        // allocate all the memory in the pool; remember a couple
        // for freeing as the signalling operation:
        t2vp = NULL;
        vp = vptmp = NULL;
        do {
            vp1 = vptmp;
            vptmp = vp;
            ercd = pget_blf( &vp, 1 );
        } while ( E_OK == ercd );
        CYG_TEST_CHECK( E_TMOUT == ercd, "get_blf bad ercd" );
        CYG_TEST_CHECK( NULL != vp,  "no allocated block to free" );
        CYG_TEST_CHECK( NULL != vp1, "no allocated block to free1" );
        break;
    case MEMVARGET:
        // allocate all the memory in the pool; remember a couple
        // for freeing as the signalling operation:
        t2vp = NULL;
        vp = vptmp = NULL;
        do {
            vp1 = vptmp;
            vptmp = vp;
            ercd = pget_blk( &vp, 1, T1_MALLOC );
        } while ( E_OK == ercd );
        CYG_TEST_CHECK( E_TMOUT == ercd, "get_blk bad ercd" );
        CYG_TEST_CHECK( NULL != vp,  "no allocated block to free" );
        CYG_TEST_CHECK( NULL != vp1, "no allocated block to free1" );
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
}

void
do_tidyup( WAITOP wait )
{
    ER ercd;
    switch ( wait ) {
    case SLEEP:
    case DELAY:
    case SEMGET:
    case MSGGET:
        // do nothing for all of those
        break;
    case FLAGWAIT:
        // clear the flag variable
        ercd = clr_flg( 1, 0 );
        CYG_TEST_CHECK( E_OK == ercd, "clr_flg bad ercd, tidy vp" );
        break;
    case MEMFIXEDGET:
        if ( NULL != vp ) {
            ercd = rel_blf( 1, vp );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd, tidy vp" );
        }
        if ( NULL != vp1 ) {
            ercd = rel_blf( 1, vp1 );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd, tidy vp1" );
        }
        if ( NULL != t2vp ) {
            ercd = rel_blf( 1, t2vp );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd, tidy t2vp" );
        }
        break;
    case MEMVARGET:
        if ( NULL != vp ) {
            ercd = rel_blk( 1, vp );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd, tidy vp" );
        }
        if ( NULL != vp1 ) {
            ercd = rel_blk( 1, vp1 );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd, tidy vp1" );
        }
        if ( NULL != t2vp ) {
            ercd = rel_blk( 1, t2vp );
            CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd, tidy t2vp" );
        }
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
}

void
do_recreate( WAITOP wait )
{ 
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
    static T_CSEM t_csem = { NULL, 0, 0 };
#endif
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
    static T_CMBX t_cmbx = { NULL, 0 };
#endif
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
    static T_CFLG t_cflg = { NULL, 0, 0 };
#endif
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
    static T_CMPF t_cmpf = { NULL, 0, 20, 95 };
#endif
#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
    static T_CMPL t_cmpl = { NULL, 0, 2000 };
#endif
    ER ercd = E_OK;
    switch ( wait ) {
    case SLEEP:
    case DELAY:
        // do nothing for all of those
        break;
    case SEMGET:
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
        // create the semaphore
        ercd = cre_sem( 1, &t_csem );
        CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_recreate SEMGET" );
#endif
        break;
    case FLAGWAIT:
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
        // create the flag
        ercd = cre_flg( 1, &t_cflg );
        CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_recreate FLAGWAIT" );
#endif
        break;
    case MSGGET:
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
        // create the mbox
        ercd = cre_mbx( 1, &t_cmbx );
        CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_recreate MSGGET" );
#endif
        break;
    case MEMFIXEDGET:
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
        // create the mempool
        ercd = cre_mpf( 1, &t_cmpf );
        CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_recreate MEMFIXEDGET" );
#endif
        break;
    case MEMVARGET:
#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
        // create the mempool
        ercd = cre_mpl( 1, &t_cmpl );
        CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_recreate MEMVARGET" );
#endif
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
    // this is just to use ercd to prevent warnings
    CYG_TEST_CHECK( E_OK == ercd, "<blank> bad ercd" );
}



void
do_signal( WAITOP wait )
{
    ER ercd;
    switch ( wait ) {
    case SLEEP:
        // send a wakeup
        ercd = wup_tsk( 2 );
        CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
        break;
    case DELAY:
        // simply wait for task 2's delay to complete
        ercd = dly_tsk( T1_WAIT );
        CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
        break;
    case SEMGET:
        // signal the semaphore
        ercd = sig_sem( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
        break;
    case FLAGWAIT:
        // set the flag bits
        ercd = set_flg( 1, 0xff );
        CYG_TEST_CHECK( E_OK == ercd, "set_flg bad ercd" );
        break;
    case MSGGET:
        // send a message
        ercd = snd_msg( 1, msg );
        CYG_TEST_CHECK( E_OK == ercd, "snd_msg bad ercd" );
        break;
    case MEMFIXEDGET:
        // release a couple of blocks we allocated earlier.  I hope.
        CYG_TEST_CHECK( NULL != vp,  "no allocated block to free" );
        CYG_TEST_CHECK( NULL != vp1, "no allocated block to free1" );
        ercd = rel_blf( 1, vp );
        CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
        vp = NULL;
        ercd = rel_blf( 1, vp1 );
        CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd1" );
        vp1 = NULL;
        break;
    case MEMVARGET:
        // release a couple of blocks we allocated earlier.  I hope.
        CYG_TEST_CHECK( NULL != vp,  "no allocated block to free" );
        CYG_TEST_CHECK( NULL != vp1, "no allocated block to free1" );
        ercd = rel_blk( 1, vp );
        CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
        vp = NULL;
        ercd = rel_blk( 1, vp1 );
        CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd1" );
        vp1 = NULL;
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
}

void
do_delete( WAITOP wait )
{
    ER ercd = E_OK;
    switch ( wait ) {
    case SLEEP:
    case DELAY:
        CYG_TEST_FAIL( "bad call to do_delete( SLEEP or DELAY )" );
        break;
    case SEMGET:
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
        // delete the semaphore
        ercd = del_sem( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_delete( SEMGET )" );
#endif
        break;
    case FLAGWAIT:
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
        // delete the flag
        ercd = del_flg( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_delete( FLAGWAIT )" );
#endif
        break;
    case MSGGET:
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
        // delete the mbox
        ercd = del_mbx( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_delete( MSGGET )" );
#endif
        break;
    case MEMFIXEDGET:
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
        // delete the mempool
        ercd = del_mpf( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_delete( MEMFIXEDGET )" );
#endif
        break;
    case MEMVARGET:
#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
        // delete the mempool
        ercd = del_mpl( 1 );
        CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
#else
        CYG_TEST_FAIL( "bad call to do_delete( MEMVARGET )" );
#endif
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
    // this is just to use ercd to prevent warnings
    CYG_TEST_CHECK( E_OK == ercd, "<blank> bad ercd" );
}
 


ER
do_wait( WAITOP wait, WAITTYPE type )
{
    switch ( wait ) {
    case SLEEP:
        return ( PLAIN == type ) ? slp_tsk() : tslp_tsk( T2_WAIT );
    case DELAY:
        return dly_tsk( T2_WAIT );      // forget the type
    case SEMGET:
        return ( PLAIN == type ) ? wai_sem( 1 ) : twai_sem( 1, T2_WAIT );
    case FLAGWAIT:
        return ( PLAIN == type ) ?
            wai_flg( &scratch, 1, 0x55, TWF_ANDW ) :
           twai_flg( &scratch, 1, 0xaa, TWF_ANDW, T2_WAIT );
    case MSGGET:
        return ( PLAIN == type ) ?
            rcv_msg( &msg1, 1 ) :
           trcv_msg( &msg1, 1, T2_WAIT );
    case MEMFIXEDGET:
        return ( PLAIN == type ) ?
            get_blf( &t2vp, 1 ) :
           tget_blf( &t2vp, 1, T2_WAIT );
    case MEMVARGET:
        return ( PLAIN == type ) ?
            get_blk( &t2vp, 1, T2_MALLOC ) :
           tget_blk( &t2vp, 1, T2_MALLOC, T2_WAIT );
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
    CYG_TEST_FAIL( "Bad wait in do_wait" );
    return E_SYS;
}

void
check_waitstate( WAITOP wait, int waiting )
{
    ER ercd;
    int waity = 0;
    switch ( wait ) {
    case SLEEP:
    case DELAY:
        return;                         // do nothing for these
    case SEMGET: {
        T_RSEM rsem;
        ercd = ref_sem( &rsem, 1 );
        waity = rsem.wtsk;
        break;
    }
    case FLAGWAIT: {
        T_RFLG rflg;
        ercd = ref_flg( &rflg, 1 );
        waity = rflg.wtsk;
        break;
    }
    case MSGGET: {
        T_RMBX rmbx;
        ercd = ref_mbx( &rmbx, 1 );
        waity = rmbx.wtsk;
        break;
    }
    case MEMFIXEDGET: {
        T_RMPF rmpf;
        ercd = ref_mpf( &rmpf, 1 );
        waity = rmpf.wtsk;
        break;
    }
    case MEMVARGET: {
        T_RMPL rmpl;
        ercd = ref_mpl( &rmpl, 1 );
        waity = rmpl.wtsk;
        break;
    }
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
    if ( waiting )
        CYG_TEST_CHECK( waity, "Object has no task waiting!" );
    else
        CYG_TEST_CHECK( !waity, "Object had a task waiting!" );
}

// ========================================================================
void task1( unsigned int arg )
{
    ER ercd;
    WAITOP wait;
    WAITTYPE type;
    KILLOP kill;

    CYG_TEST_INFO( "Task 1 running" );

    {
        extern Cyg_Thread cyg_uitron_TASKS[];
        cyg_test_dump_thread_stack_stats(
            "Startup, task1", &cyg_uitron_TASKS[ 0 ] );
        cyg_test_dump_thread_stack_stats(
            "Startup, task2", &cyg_uitron_TASKS[ 1 ] );
        cyg_test_dump_interrupt_stack_stats( "Startup" );
        cyg_test_dump_idlethread_stack_stats( "Startup" );
        cyg_test_clear_interrupt_stack();
    }

    ercd = chg_pri( 1, 8 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );

    for ( wait = START_WAITOP; wait < DONE_WAITOP ; wait++) {
        for ( type = START_TYPE; type < DONE_TYPE   ; type++ ) {
            for ( kill = START_KILLOP; kill < DONE_KILLOP ; kill++ ) {
                
                // These clauses deal with a couple of special cases:
                // [doing it this way helps keep the rest of the code
                //  nicely general and orthogonal]
                // 
                // 1) DELAY: dly_tsk(): when this times out, the retcode is
                // E_OK rather than E_TMOUT, and it always times out.  The
                // "signalling" method here is just to wait yourself.  So we
                // do not test DELAY with TIMED type.
                //
                // 2) PLAIN tests with TIMEOUT kill operations: a PLAIN test
                // will not time out, it'll wait forever, so waiting for it
                // so to do is pointless; further, we would check for the
                // wrong error code.  So we do not test PLAIN tests with
                // TIMOUT kill operations.
                //
                // 3) SLEEP or DELAY tests with DELETE operations: there is
                // no synchronization to delete in those cases.
                // 3a) Individual object types are tested for delete support,
                // and if there is none, the test is skipped.

                if ( DELAY == wait && TIMED == type )
                    continue;

                if ( PLAIN == type &&
                     ( (        TIMEOUT        == kill) ||
                       (SUSPEND_TIMEOUT_RESUME == kill) ||
                       (SUSPEND_TIMEOUT_KILL   == kill) ) )
                    continue;

                if ( ( 
#ifndef CYGPKG_UITRON_SEMAS_CREATE_DELETE
                    (SEMGET      == wait) ||
#endif
#ifndef CYGPKG_UITRON_FLAGS_CREATE_DELETE
                    (FLAGWAIT    == wait) ||
#endif
#ifndef CYGPKG_UITRON_MBOXES_CREATE_DELETE
                    (MSGGET      == wait) ||
#endif
#ifndef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
                    (MEMFIXEDGET == wait) ||
#endif
#ifndef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
                    (MEMVARGET   == wait) ||
#endif
                    (SLEEP       == wait) ||
                    (DELAY       == wait)
                    ) &&
                     ((DELETE                == kill) ||
                      (SUSPEND_DELETE_RESUME == kill) ||
                      (SUSPEND_DELETE_KILL   == kill)) )
                    continue;


                CYG_TEST_INFO( makemsg( "T1: ", wait, type, kill ) );

                intercom = 0;

                // prepare the synchronization objects
                // (actually, just empty the mempools)
                do_prep( wait );

                // start task 2 at a higher priority than myself
                ercd = dis_dsp();
                CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
                ercd = sta_tsk( 2, task2arg( wait, type, kill ) );
                CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
                ercd = chg_pri( 2, 5 );
                CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
                ercd = ena_dsp();
                CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
                // task 2 should run now, until it waits.

                ercd = ref_tsk( &rtsk, 2 );
                CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
                CYG_TEST_CHECK( TTS_WAI == rtsk.tskstat, "bad tskstat" );
                CYG_TEST_CHECK( 5 == rtsk.tskpri, "bad tskpri" );

                switch ( kill ) {
                case SIGNAL:
                    // signal the task appropriately
                    do_signal( wait );
                    // it should now have run to completion
                    break;
                case TIMEOUT:
                    check_waitstate( wait, 1 );
                    // wait for the timeout to occur
                    ercd = dly_tsk( T1_WAIT );
                    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
                    check_waitstate( wait, 0 );
                    // it should now have run to completion
                    break;
                case RELEASE:
                    // hit the task with a release-wait
                    ercd = rel_wai( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "rel_wai bad ercd" );
                    // it should now have run to completion
                    break;
                case DELETE:
                    // delete the object appropriately
                    do_delete( wait );
                    // it should now have run to completion
                    break;
                case KILL:
                    // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                case SUSPEND_SIGNAL_RESUME:
                    // suspend the task
                    do_suspend();
                    // signal the task appropriately
                    do_signal( wait );
                    // resume the task
                    do_resume();
                    // it should now have run to completion
                    break;
                case SUSPEND_TIMEOUT_RESUME:
                    check_waitstate( wait, 1 );
                    // suspend the task
                    do_suspend();
                    check_waitstate( wait, 1 );
                    // wait for the timeout to occur
                    ercd = dly_tsk( T1_WAIT );
                    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
                    check_waitstate( wait, 0 );
                    // resume the task
                    do_resume();
                    // it should now have run to completion
                    break;
                case SUSPEND_RELEASE_RESUME:
                    // suspend the task
                    do_suspend();
                    // hit the task with a release-wait
                    ercd = rel_wai( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "rel_wai bad ercd" );
                    // resume the task
                    do_resume();
                    // it should now have run to completion
                    break;
                case SUSPEND_DELETE_RESUME:
                    // suspend the task
                    do_suspend();
                    // delete the object appropriately
                    do_delete( wait );
                    // resume the task
                    do_resume();
                    // it should now have run to completion
                    break;
                case SUSPEND_KILL:
                    // suspend the task
                    do_suspend();
                    // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                case SUSPEND_SIGNAL_KILL:
                    // suspend the task
                    do_suspend();
                    // signal the task appropriately
                    do_signal( wait );
                    // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                case SUSPEND_TIMEOUT_KILL:
                    check_waitstate( wait, 1 );
                    // suspend the task
                    do_suspend();
                    check_waitstate( wait, 1 );
                    // wait for the timeout to occur
                    ercd = dly_tsk( T1_WAIT );
                    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );
                    check_waitstate( wait, 0 );
                    // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                case SUSPEND_RELEASE_KILL:
                    // suspend the task
                    do_suspend();
                    // hit the task with a release-wait
                    ercd = rel_wai( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "rel_wai bad ercd" );
                     // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                case SUSPEND_DELETE_KILL:
                    // suspend the task
                    do_suspend();
                    // delete the object appropriately
                    do_delete( wait );
                    // kill the task
                    ercd = ter_tsk( 2 );
                    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
                    // it should now have terminated without running
                    break;
                default:
                    CYG_TEST_FAIL( "bad switch" );
                    break;
                }
                
                // task 2 should be dormant now, however it got there
                ercd = ref_tsk( &rtsk, 2 );
                CYG_TEST_CHECK( E_OK == ercd, "ref_tsk bad ercd" );
                CYG_TEST_CHECK( TTS_DMT == rtsk.tskstat, "bad tskstat" );

                if ( (SUSPEND_SIGNAL_KILL == kill) &&
                     ((MEMFIXEDGET == wait) || (MEMVARGET == wait)) ) {
                    // it was a killed successful memory alloc, so we have
                    // lost the pointer to memory allocated; there is an
                    // implicit storeleak problem when the task trying
                    // to allocate is signalled then killed.
                    // Recreate the pointer from an old version:
                    CYG_TEST_CHECK( NULL == t2vp, "t2vp WAS allocated!" );
                    t2vp = t2vp_backup;
                }

                switch ( kill ) {
                case KILL:
                case SUSPEND_KILL:
                case SUSPEND_SIGNAL_KILL:
                case SUSPEND_TIMEOUT_KILL:
                case SUSPEND_RELEASE_KILL:
                case SUSPEND_DELETE_KILL:
                    // if task 2 was killed, expect only one increment
                    CYG_TEST_CHECK( 1 == intercom, "intercom bad value !1" );
                    break;
                default:
                    // otherwise expect two increments
                    CYG_TEST_CHECK( 2 == intercom, "intercom bad value !2" );
                    break;
                }
                
                // tidy up or recreate the synchronization objects
                if ( (DELETE                == kill) ||
                     (SUSPEND_DELETE_RESUME == kill) ||
                     (SUSPEND_DELETE_KILL   == kill) )
                    do_recreate( wait );
                else
                    do_tidyup( wait );
            }
        }
    }
    CYG_TEST_PASS("synchronization interaction tests");

    {
        extern Cyg_Thread cyg_uitron_TASKS[];
        cyg_test_dump_thread_stack_stats(
            "All done, task1", &cyg_uitron_TASKS[ 0 ] );
        cyg_test_dump_thread_stack_stats(
            "All done, task2", &cyg_uitron_TASKS[ 1 ] );
        cyg_test_dump_interrupt_stack_stats( "All done" );
        cyg_test_dump_idlethread_stack_stats( "All done" );
    }
    // all done
    CYG_TEST_EXIT( "All done" );
    ext_tsk();
}



void task2( unsigned int arg )
{
    ER ercd;
    WAITOP wait;
    WAITTYPE waittype;
    KILLOP kill;

    decodearg( arg, &wait, &waittype, &kill );

//    CYG_TEST_INFO( makemsg( " 2: ", wait, waittype, kill ) );

    intercom++;
    ercd = do_wait( wait, waittype );
    intercom++;

    switch ( kill ) {
    case SIGNAL:
    case SUSPEND_SIGNAL_RESUME:
        // we expect to have been signalled correctly
        CYG_TEST_CHECK( E_OK == ercd, "T2 wait bad ercd" );
        // here we know that the op completed OK
        if ( (MEMFIXEDGET == wait) || (MEMVARGET == wait) ) {
            // it was a successful memory alloc of whichever type,
            // so we can save away a copy of t2vp for working round an
            // implicit storeleak problem when the task trying to allocate
            // is signalled then killed:
            CYG_TEST_CHECK( NULL != t2vp, "No t2vp allocated!" );
            t2vp_backup = t2vp;
        }
        break;
    case TIMEOUT:
    case SUSPEND_TIMEOUT_RESUME:
        // we expect to have timed out - if it's a timeout op.
        CYG_TEST_CHECK( E_TMOUT == ercd, "T2 timeout bad ercd, !E_TMOUT" );
        break;
    case RELEASE:
    case SUSPEND_RELEASE_RESUME:
        // we expect to have suffered a release wait.
        CYG_TEST_CHECK( E_RLWAI == ercd, "T2 release bad ercd, !E_RLWAI" );
        break;
    case DELETE:
    case SUSPEND_DELETE_RESUME:
        // we expect to be told the object is gone
        CYG_TEST_CHECK( E_DLT == ercd, "T2 release bad ercd, !E_DLT" );
        break;
    case KILL:
    case SUSPEND_KILL:
    case SUSPEND_SIGNAL_KILL:
    case SUSPEND_TIMEOUT_KILL:
    case SUSPEND_RELEASE_KILL:
    case SUSPEND_DELETE_KILL:
        // we expect to have been killed here, ie. this won't execute!
        CYG_TEST_FAIL( "Task 2 ran to completion!" );
        break;
    default:
        CYG_TEST_FAIL( "bad switch" );
        break;
    }
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

// EOF testcx7.cxx
