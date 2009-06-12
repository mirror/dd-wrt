//===========================================================================
//
//      testcx6.cxx
//
//      uITRON "C++" test program six
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
T_MSG *t_msg = (T_MSG *)&scratch;
T_MSG *msg;
VP vp;

T_CSEM t_csem = { NULL, 0, 0 };
T_CMBX t_cmbx = { NULL, 0 };
T_CFLG t_cflg = { NULL, 0, 0 };
T_RSEM t_rsem;
T_RMBX t_rmbx;
T_RFLG t_rflg;


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

#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
    tests++;
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_sem( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_sem bad ercd !E_ID" );
    ercd = del_sem( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_sem bad ercd !E_ID" );
    ercd = cre_sem( -6, &t_csem );
    CYG_TEST_CHECK( E_ID == ercd, "cre_sem bad ercd !E_ID" );
    ercd = cre_sem( 99, &t_csem );
    CYG_TEST_CHECK( E_ID == ercd, "cre_sem bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    ercd = cre_sem( 3, &t_csem );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_sem bad ercd !E_OBJ" );
    // delete it so we can play
    ercd = del_sem( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
    // check it is deleted
    ercd = sig_sem( 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sig_sem bad ercd !E_NOEXS" );
    ercd = preq_sem( 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "preq_sem bad ercd !E_NOEXS" );
    ercd = twai_sem( 3, 10 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "twai_sem bad ercd !E_NOEXS" );
    ercd = wai_sem( 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "wai_sem bad ercd !E_NOEXS" );
    ercd = ref_sem( &t_rsem, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_sem bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_sem( 3, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_sem bad ercd !E_PAR" );
#endif
    t_csem.sematr = 0xfff;
    ercd = cre_sem( 3, &t_csem );
    CYG_TEST_CHECK( E_RSATR == ercd, "cre_sem bad ercd !E_RSATR" );
    t_csem.sematr = 0;
#endif // we can test bad param error returns
    ercd = cre_sem( 3, &t_csem );
    CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
    // and check we can use it
    ercd = sig_sem( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "sig_sem bad ercd" );
    ercd = wai_sem( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "wai_sem bad ercd" );
    ercd = preq_sem( 3 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "preq_sem bad ercd !E_TMOUT" );
    ercd = twai_sem( 3, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_sem bad ercd !E_TMOUT" );
    ercd = ref_sem( &t_rsem, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_sem bad ercd" );

    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_sem bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = twai_sem( 2, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_sem bad ercd !E_DLT" );

    // check they are deleted
    ercd = sig_sem( 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sig_sem bad ercd !E_NOEXS" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sig_sem bad ercd !E_NOEXS" );
    // re-create and do it again
    ercd = cre_sem( 1, &t_csem );
    CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );
    ercd = cre_sem( 2, &t_csem );
    CYG_TEST_CHECK( E_OK == ercd, "cre_sem bad ercd" );

    // now wait while task 2 deletes the wait objects again
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = wai_sem( 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_sem bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = twai_sem( 2, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_sem bad ercd !E_DLT" );

    // check they are deleted
    ercd = sig_sem( 1 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sig_sem bad ercd !E_NOEXS" );
    ercd = sig_sem( 2 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "sig_sem bad ercd !E_NOEXS" );

    CYG_TEST_PASS("create/delete semaphores");
#endif // CYGPKG_UITRON_SEMAS_CREATE_DELETE


#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
    tests++;
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_flg( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_flg bad ercd !E_ID" );
    ercd = del_flg( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_flg bad ercd !E_ID" );
    ercd = cre_flg( -6, &t_cflg );
    CYG_TEST_CHECK( E_ID == ercd, "cre_flg bad ercd !E_ID" );
    ercd = cre_flg( 99, &t_cflg );
    CYG_TEST_CHECK( E_ID == ercd, "cre_flg bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    ercd = cre_flg( 3, &t_cflg );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_flg bad ercd !E_OBJ" );
    // delete it so we can play
    ercd = del_flg( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    // check it is deleted
    ercd = set_flg( 3, 0x6789 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "set_flg bad ercd !E_NOEXS" );
    ercd = clr_flg( 3, 0x9876 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "clr_flg bad ercd !E_NOEXS" );
    ercd = pol_flg( &scratch, 3, 0xdddd, TWF_ANDW );
    CYG_TEST_CHECK( E_NOEXS == ercd, "pol_flg bad ercd !E_NOEXS" );
    ercd = twai_flg( &scratch, 3, 0x4444, TWF_ORW, 10 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "twai_flg bad ercd !E_NOEXS" );
    ercd = wai_flg( &scratch, 3, 0xbbbb, TWF_ANDW | TWF_CLR );
    CYG_TEST_CHECK( E_NOEXS == ercd, "wai_flg bad ercd !E_NOEXS" );
    ercd = ref_flg( &t_rflg, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_flg bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_flg( 3, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_flg bad ercd !E_PAR" );
#endif
    t_cflg.flgatr = 0xfff;
    ercd = cre_flg( 3, &t_cflg );
    CYG_TEST_CHECK( E_RSATR == ercd, "cre_flg bad ercd !E_RSATR" );
#endif // we can test bad param error returns
    // now create it well
    t_cflg.flgatr = 0;
    t_cflg.iflgptn = 0;
    ercd = cre_flg( 3, &t_cflg );
    CYG_TEST_CHECK( E_OK == ercd, "cre_flg bad ercd" );
    // and check we can use it
    ercd = clr_flg( 3, 0x7256 );
    CYG_TEST_CHECK( E_OK == ercd, "clr_flg bad ercd" );
    ercd = set_flg( 3, 0xff );
    CYG_TEST_CHECK( E_OK == ercd, "set_flg bad ercd" );
    ercd = wai_flg( &scratch, 3, 0xaa, TWF_ANDW | TWF_CLR );
    CYG_TEST_CHECK( E_OK == ercd, "wai_flg bad ercd" );
    ercd = pol_flg( &scratch, 3, 0xaa, TWF_ANDW | TWF_CLR );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pol_flg bad ercd !E_TMOUT" );
    ercd = twai_flg( &scratch, 3, 0xaa, TWF_ANDW | TWF_CLR, 2 );
    CYG_TEST_CHECK( E_TMOUT == ercd, "twai_flg bad ercd !E_TMOUT" );
    ercd = ref_flg( &t_rflg, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 == t_rflg.flgptn, "ref_flg bad ercd" );
    // now create it again with a preset pattern and check that we can
    // detect that pattern:
    ercd = del_flg( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    t_cflg.flgatr = 0;
    t_cflg.iflgptn = 0x1234;
    ercd = cre_flg( 3, &t_cflg );
    CYG_TEST_CHECK( E_OK == ercd, "cre_flg bad ercd" );
    // and check we can use it
    ercd = wai_flg( &scratch, 3, 0x1200, TWF_ANDW );
    CYG_TEST_CHECK( E_OK == ercd, "wai_flg bad ercd" );
    ercd = pol_flg( &scratch, 3, 0x0034, TWF_ANDW );
    CYG_TEST_CHECK( E_OK == ercd, "pol_flg bad ercd" );
    ercd = twai_flg( &scratch, 3, 0x1004, TWF_ANDW, 10 );
    CYG_TEST_CHECK( E_OK == ercd, "twai_flg bad ercd" );
    ercd = pol_flg( &scratch, 3, 0xffedcb, TWF_ORW );
    CYG_TEST_CHECK( E_TMOUT == ercd, "pol_flg bad ercd !E_TMOUT" );
    ercd = ref_flg( &t_rflg, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0x1234 == t_rflg.flgptn, "ref_flg bad ercd" );
    ercd = clr_flg( 3, 0 );
    ercd = ref_flg( &t_rflg, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_flg bad ercd" );
    CYG_TEST_CHECK( 0 == t_rflg.flgptn, "ref_flg bad ercd" );

    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = wai_flg( &scratch, 1, 0xaa, TWF_ANDW );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_flg bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = twai_flg( &scratch, 2, 0x55, TWF_ANDW, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_flg bad ercd !E_DLT" );

    // check they are deleted
    ercd = set_flg( 1, 0x22 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "set_flg bad ercd !E_NOEXS" );
    ercd = clr_flg( 2, 0xdd );
    CYG_TEST_CHECK( E_NOEXS == ercd, "clr_flg bad ercd !E_NOEXS" );
    // re-create and do it again
    t_cflg.iflgptn = 0x5555;
    ercd = cre_flg( 1, &t_cflg );
    CYG_TEST_CHECK( E_OK == ercd, "cre_flg bad ercd" );
    t_cflg.iflgptn = 0;
    ercd = cre_flg( 2, &t_cflg );
    CYG_TEST_CHECK( E_OK == ercd, "cre_flg bad ercd" );

    // now wait while task 2 deletes the wait objects again
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = wai_flg( &scratch, 1, 0xaaaa, TWF_ORW | TWF_CLR );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_flg bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = twai_flg( &scratch, 2, 0xffff, TWF_ORW, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_flg bad ercd !E_DLT" );

    // check they are deleted
    ercd = clr_flg( 1, 0xd00d );
    CYG_TEST_CHECK( E_NOEXS == ercd, "clr_flg bad ercd !E_NOEXS" );
    ercd = set_flg( 2, 0xfff00 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "set_flg bad ercd !E_NOEXS" );

    CYG_TEST_PASS("create/delete flags");
#endif // CYGPKG_UITRON_FLAGS_CREATE_DELETE

#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
    tests++;
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    ercd = del_mbx( -6 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mbx bad ercd !E_ID" );
    ercd = del_mbx( 99 );
    CYG_TEST_CHECK( E_ID == ercd, "del_mbx bad ercd !E_ID" );
    ercd = cre_mbx( -6, &t_cmbx );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mbx bad ercd !E_ID" );
    ercd = cre_mbx( 99, &t_cmbx );
    CYG_TEST_CHECK( E_ID == ercd, "cre_mbx bad ercd !E_ID" );
#endif // we can test bad param error returns
    // try a pre-existing object
    ercd = cre_mbx( 3, &t_cmbx );
    CYG_TEST_CHECK( E_OBJ == ercd, "cre_mbx bad ercd !E_OBJ" );
    // delete it so we can play
    ercd = del_mbx( 3 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
    // check it is deleted
    ercd = snd_msg( 3, t_msg );
    CYG_TEST_CHECK( E_NOEXS == ercd, "snd_msg bad ercd !E_NOEXS" );
    ercd = rcv_msg( &msg, 3 );       
    CYG_TEST_CHECK( E_NOEXS == ercd, "rcv_msg bad ercd !E_NOEXS" );
    ercd = trcv_msg( &msg, 3, 10 );   
    CYG_TEST_CHECK( E_NOEXS == ercd, "trcv_msg bad ercd !E_NOEXS" );
    ercd = prcv_msg( &msg, 3 );        
    CYG_TEST_CHECK( E_NOEXS == ercd, "prcv_msg bad ercd !E_NOEXS" );
    ercd = ref_mbx( &t_rmbx, 3 );
    CYG_TEST_CHECK( E_NOEXS == ercd, "ref_mbx bad ercd !E_NOEXS" );
#ifdef CYGSEM_UITRON_BAD_PARAMS_RETURN_ERRORS
    // now try creating it (badly)
#ifndef CYGSEM_UITRON_PARAMS_NULL_IS_GOOD_PTR
    ercd = cre_mbx( 3, NULL );
    CYG_TEST_CHECK( E_PAR == ercd, "cre_mbx bad ercd !E_PAR" );
#endif
    t_cmbx.mbxatr = 0xfff;
    ercd = cre_mbx( 3, &t_cmbx );
    CYG_TEST_CHECK( E_RSATR == ercd, "cre_mbx bad ercd !E_RSATR" );
    t_cmbx.mbxatr = 0;
#endif // we can test bad param error returns
    ercd = cre_mbx( 3, &t_cmbx );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mbx bad ercd" );
    // and check we can use it
    ercd = snd_msg( 3, t_msg );
    CYG_TEST_CHECK( E_OK == ercd, "snd_msg bad ercd" );
    ercd = rcv_msg( &msg, 3 );       
    CYG_TEST_CHECK( E_OK == ercd, "rcv_msg bad ercd" );
    ercd = trcv_msg( &msg, 3, 2 );   
    CYG_TEST_CHECK( E_TMOUT == ercd, "trcv_msg bad ercd !E_TMOUT" );
    ercd = prcv_msg( &msg, 3 );                            
    CYG_TEST_CHECK( E_TMOUT == ercd, "prcv_msg bad ercd !E_TMOUT" );
    ercd = ref_mbx( &t_rmbx, 3 );
    CYG_TEST_CHECK( E_OK == ercd, "ref_mbx bad ercd" );

    // now wait while task 2 deletes the wait objects
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rcv_msg( &msg, 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_mbx bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = trcv_msg( &msg, 2, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_mbx bad ercd !E_DLT" );

    // check they are deleted
    ercd = snd_msg( 1, t_msg );
    CYG_TEST_CHECK( E_NOEXS == ercd, "snd_msg bad ercd !E_NOEXS" );
    ercd = snd_msg( 2, t_msg );       
    CYG_TEST_CHECK( E_NOEXS == ercd, "snd_msg bad ercd !E_NOEXS" );
    // re-create and do it again
    ercd = cre_mbx( 1, &t_cmbx );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mbx bad ercd" );
    ercd = cre_mbx( 2, &t_cmbx );
    CYG_TEST_CHECK( E_OK == ercd, "cre_mbx bad ercd" );

    // now wait while task 2 deletes the wait objects again
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = rcv_msg( &msg, 1 );
    CYG_TEST_CHECK( E_DLT == ercd, "wai_mbx bad ercd !E_DLT" );
    ercd = wup_tsk( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "wup_tsk bad ercd" );
    ercd = trcv_msg( &msg, 2, 20 );
    CYG_TEST_CHECK( E_DLT == ercd, "twai_mbx bad ercd !E_DLT" );

    // check they are deleted
    ercd = snd_msg( 1, t_msg );
    CYG_TEST_CHECK( E_NOEXS == ercd, "snd_msg bad ercd !E_NOEXS" );
    ercd = snd_msg( 2, t_msg );       
    CYG_TEST_CHECK( E_NOEXS == ercd, "snd_msg bad ercd !E_NOEXS" );

    CYG_TEST_PASS("create/delete mboxes");
#endif // CYGPKG_UITRON_MBOXES_CREATE_DELETE

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

#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
    ercd = del_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_sem( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_sem( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_sem bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
#endif // CYGPKG_UITRON_SEMAS_CREATE_DELETE

#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
    ercd = del_flg( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_flg( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_flg( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_flg( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_flg bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
#endif // CYGPKG_UITRON_FLAGS_CREATE_DELETE

#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
    ercd = del_mbx( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mbx( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mbx( 1 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
    ercd = del_mbx( 2 );
    CYG_TEST_CHECK( E_OK == ercd, "del_mbx bad ercd" );
    ercd = slp_tsk();
    CYG_TEST_CHECK( E_OK == ercd, "slp_tsk bad ercd" );
#endif // CYGPKG_UITRON_MBOXES_CREATE_DELETE

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

// EOF testcx6.cxx
