//===========================================================================
//
//      testcx9.cxx
//
//      uITRON "C++" test program nine
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
// Date:        1998-10-16
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

VP vp;

T_CMPL t_cmpl = { NULL, 0, 1000 };
T_RMPL t_rmpl;
T_CMPF t_cmpf = { NULL, 0, 10, 100 };
T_RMPF t_rmpf;


extern "C" {
    void task1( unsigned int arg );
    void task2( unsigned int arg );
    void task3( unsigned int arg );
    void task4( unsigned int arg );
}

void task1( unsigned int arg )
{
    ER ercd;
    int tests = 0;

    CYG_TEST_INFO( "Task 1 running" );

    ercd = dis_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "dis_dsp bad ercd" );
    ercd = sta_tsk( 2, 22222 );
    CYG_TEST_CHECK( E_OK == ercd, "sta_tsk bad ercd" );
    ercd = chg_pri( 2, 5 );
    CYG_TEST_CHECK( E_OK == ercd, "chg_pri bad ercd" );
    ercd = ena_dsp();
    CYG_TEST_CHECK( E_OK == ercd, "ena_dsp bad ercd" );
    ercd = dly_tsk( 10 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
    tests++;
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_mpf( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mpf bad ercd !E_ID" );
    ercd = del_mpf( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mpf bad ercd !E_ID" );
    ercd = cre_mpf( -6, &t_cmpf );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mpf bad ercd !E_ID" );
    ercd = cre_mpf( 99, &t_cmpf );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mpf bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    // [first get a valid block from it for the freeing test later]
    ercd = pget_blf( &vp, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blf bad ercd" );
    ercd = cre_mpf( 3, &t_cmpf );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_mpf bad ercd !E_OBJ" );
    // delete it so we can play
    ercd = del_mpf( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
    // check it is deleted
    ercd = rel_blf( 3, vp );            // vp did come from this pool
    CYG_TEST_CHECK( E_NOEXS == ercd, "rel_blf bad ercd !E_NOEXS" );
    ercd = pget_blf( &vp, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blf bad ercd !E_NOEXS" );
    ercd = tget_blf( &vp, 3, 10 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "tget_blf bad ercd !E_NOEXS" );
    ercd = get_blf( &vp, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blf bad ercd !E_NOEXS" );
    ercd = ref_mpf( &t_rmpf, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_mpf bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_mpf( 3, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_mpf bad ercd !E_PAR" );
#endif
    t_cmpf.mpfatr = 0xfff;
    ercd = cre_mpf( 3, &t_cmpf );
    CYG_TEST_CHECK( E_RSATR == ercd, "cre_mpf bad ercd !E_RSATR" );
#endif // we can test bad param error returns
    t_cmpf.mpfatr = 0;
    t_cmpf.mpfcnt = 10000;
    t_cmpf.blfsz = 100;
    ercd = cre_mpf( 3, &t_cmpf );
    CYG_TEST_CHECK( E_NOMEM == ercd, "cre_mpf bad ercd" );
    t_cmpf.mpfcnt = 100;
    t_cmpf.blfsz = 100000;
    ercd = cre_mpf( 3, &t_cmpf );
    CYG_TEST_CHECK( E_NOMEM == ercd, "cre_mpf bad ercd" );
    // now create it well
    t_cmpf.mpfatr = 0;
    t_cmpf.mpfcnt = 10;
    t_cmpf.blfsz = 100;
    ercd = cre_mpf( 3, &t_cmpf );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpf bad ercd" );
    // and check we can use it
    ercd = pget_blf( &vp, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blf bad ercd" );
    ercd = tget_blf( &vp, 3, 10 );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blf bad ercd" );
    ercd = get_blf( &vp, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "get_blf bad ercd" );
    ercd = rel_blf( 3, vp );            // vp did come from new pool
    CYG_TEST_CHECK( E_OK == ercd, "rel_blf bad ercd" );
    ercd = rel_blf( 3, vp );            // vp already freed
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blf bad ercd !E_PAR" );
    ercd = ref_mpf( &t_rmpf, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpf bad ercd" );

    // In order to wait on the pools, we must first consume all they have:
    while ( E_OK == (ercd = pget_blf( &vp, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    while ( E_OK == (ercd = tget_blf( &vp, 2, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf bad ercd !E_TMOUT" );
    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = get_blf( &vp, 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "get_blf bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = tget_blf( &vp, 2, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "tget_blf bad ercd !E_DLT" );
    // check they are deleted
    ercd = get_blf( &vp, 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blf bad ercd !E_NOEXS" );
    ercd = pget_blf( &vp, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blf bad ercd !E_NOEXS" );

    // re-create and do it again
    t_cmpf.mpfcnt = 90;
    t_cmpf.blfsz = 20;
    ercd = cre_mpf( 1, &t_cmpf );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpf bad ercd" );
    t_cmpf.mpfcnt = 5;
    t_cmpf.blfsz = 200;
    ercd = cre_mpf( 2, &t_cmpf );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpf bad ercd" );

    // In order to wait on the pools, we must first consume all they have:
    while ( E_OK == (ercd = pget_blf( &vp, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blf bad ercd !E_TMOUT" );
    while ( E_OK == (ercd = tget_blf( &vp, 2, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blf bad ercd !E_TMOUT" );
    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = get_blf( &vp, 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "get_blf bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = tget_blf( &vp, 2, 10 );
    CYG_TEST_CHECK( E_DLT == ercd, "tget_blf bad ercd !E_DLT" );
    // check they are deleted
    ercd = tget_blf( &vp, 1, 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blf bad ercd !E_NOEXS" );
    ercd = get_blf( &vp, 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blf bad ercd !E_NOEXS" );

    CYG_TEST_PASS("create/delete fixed mempools");
#endif // CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE

#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
    tests++;
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_mpl( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mpl bad ercd !E_ID" );
    ercd = del_mpl( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mpl bad ercd !E_ID" );
    ercd = cre_mpl( -6, &t_cmpl );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mpl bad ercd !E_ID" );
    ercd = cre_mpl( 99, &t_cmpl );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mpl bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    // [first get a valid block from it for the freeing test later]
    ercd = pget_blk( &vp, 3, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blk bad ercd" );
    ercd = cre_mpl( 3, &t_cmpl );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_mpl bad ercd !E_OBJ" );
    // delete it so we can play
    ercd = del_mpl( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
    // check it is deleted
    ercd = rel_blk( 3, vp );            // vp did come from this pool
    CYG_TEST_CHECK( E_NOEXS == ercd, "rel_blk bad ercd !E_NOEXS" );
    ercd = pget_blk( &vp, 3, 100 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blk bad ercd !E_NOEXS" );
    ercd = tget_blk( &vp, 3, 100, 10 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "tget_blk bad ercd !E_NOEXS" );
    ercd = get_blk( &vp, 3, 100 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blk bad ercd !E_NOEXS" );
    ercd = ref_mpl( &t_rmpl, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_mpl bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_mpl( 3, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_mpl bad ercd !E_PAR" );
#endif
    t_cmpl.mplatr = 0xfff;
    ercd = cre_mpl( 3, &t_cmpl );
    CYG_TEST_CHECK( E_RSATR == ercd, "cre_mpl bad ercd !E_RSATR" );
#endif // we can test bad param error returns
    t_cmpl.mplatr = 0;
    t_cmpl.mplsz = 100000000;
    ercd = cre_mpl( 3, &t_cmpl );
    CYG_TEST_CHECK( E_NOMEM == ercd, "cre_mpl bad ercd" );
    // now create it well
    t_cmpl.mplatr = 0;
    t_cmpl.mplsz = 1000;
    ercd = cre_mpl( 3, &t_cmpl );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpl bad ercd" );
    // and check we can use it
    ercd = pget_blk( &vp, 3, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "pget_blk bad ercd" );
    ercd = pget_blk( &vp, 3, 100000000 ); // way too large
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    ercd = tget_blk( &vp, 3, 100, 10 );
    CYG_TEST_CHECK( E_OK == ercd, "tget_blk bad ercd" );
    ercd = get_blk( &vp, 3, 100 );
    CYG_TEST_CHECK( E_OK == ercd, "get_blk bad ercd" );
    ercd = rel_blk( 3, vp );            // vp did come from new pool
    CYG_TEST_CHECK( E_OK == ercd, "rel_blk bad ercd" );
    ercd = rel_blk( 3, vp );            // vp already freed
    CYG_TEST_CHECK( E_PAR == ercd, "rel_blk bad ercd !E_PAR" );
    ercd = ref_mpl( &t_rmpl, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mpl bad ercd" );

    // In order to wait on the pools, we must first consume all they have:
    while ( E_OK == (ercd = pget_blk( &vp, 1, 100 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    while ( E_OK == (ercd = tget_blk( &vp, 2, 100, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk bad ercd !E_TMOUT" );
    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = get_blk( &vp, 1, 200 );
    CYG_TEST_CHECK( E_DLT == ercd, "get_blk bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = tget_blk( &vp, 2, 100, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "tget_blk bad ercd !E_DLT" );
    // check they are deleted
    ercd = get_blk( &vp, 1, 200 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blk bad ercd !E_NOEXS" );
    ercd = pget_blk( &vp, 2, 20 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blk bad ercd !E_NOEXS" );

    // re-create and do it again
    ercd = cre_mpl( 1, &t_cmpl );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpl bad ercd" );
    ercd = cre_mpl( 2, &t_cmpl );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mpl bad ercd" );

    // In order to wait on the pools, we must first consume all they have:
    while ( E_OK == (ercd = pget_blk( &vp, 1, 20 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "pget_blk bad ercd !E_TMOUT" );
    while ( E_OK == (ercd = tget_blk( &vp, 2, 400, 1 )) ) /* nothing */;
    CYG_TEST_CHECK( E_TMOUT == ercd, "tget_blk bad ercd !E_TMOUT" );
    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = get_blk( &vp, 1, 200 );
    CYG_TEST_CHECK( E_DLT == ercd, "get_blk bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = tget_blk( &vp, 2, 500, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "tget_blk bad ercd !E_DLT" );
    // check they are deleted
    ercd = tget_blk( &vp, 1, 200, 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "get_blk bad ercd !E_NOEXS" );
    ercd = get_blk( &vp, 2, 20 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pget_blk bad ercd !E_NOEXS" );

    CYG_TEST_PASS("create/delete variable mempools");
#endif // CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE

    ercd = ter_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "ter_tsk bad ercd" );
    ercd = dly_tsk( 5 );
    CYG_TEST_CHECK( E_OK == ercd, "dly_tsk bad ercd" );

    // all done
    if ( 0 == tests ) {
        CYG_TEST_NA( "No objects have create/delete enabled" );
    }
    else {
        CYG_TEST_EXIT( "All done" );
    }
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

    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );

#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
    ercd = del_mpf( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpf( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpf( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpf( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpf bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
#endif // CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE

#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
    ercd = del_mpl( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpl( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpl( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mpl( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mpl bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
#endif // CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE

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

// EOF testcx9.cxx
