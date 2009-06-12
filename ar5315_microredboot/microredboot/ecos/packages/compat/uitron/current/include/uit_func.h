#ifndef CYGONCE_COMPAT_UITRON_UIT_FUNC_H
#define CYGONCE_COMPAT_UITRON_UIT_FUNC_H
//===========================================================================
//
//      uit_func.h
//
//      uITRON compatibility functions
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
// Purpose:     uITRON compatibility functions
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ------------------------------------------------------------------------
// Source Code Organization
//
// First, see pkgconf/uitron.h for details of applicable configuration
// options.
//
// This file uit_func.h provides prototypes for the uITRON API.  All the
// uITRON functions are listed here.  The prototypes are configurable
// either to have C or C++ linkage, and if being compiled in a C++
// environment, to be inline.
//
// The function prototypes are all in terms of uITRON type definitions from
// uit_type.h, which is included at the head of uit_func.h.
//
// The implementations of the uITRON functions are in uit_func.inl, which
// is either included at the end of uit_func.h (if functions are inline) or
// in uit_func.cxx (if outline).
// 
// uit_func.cxx provides some startup functions plus, if the uITRON
// functions are out of line, uit_func.inl is included to instantiate those
// functions.
// 
// uITRON system objects (tasks, semaphores...) are described in
// uit_obj.hxx.  This is a C++ file and is used by the implementation of
// the uITRON functions.
// 
// The uITRON system objects are instantiated in uit_obj.cxx, which uses
// uit_obj.hxx to define the objects, and the configuration file
// pkgconf/uitron.h to construct them as required.
//
// The include graph from an application, which should only include
// uit_func.h, is similar to the following:
//
// 
// [inline uITRON functions:]
//
//    <your_app.c>
//    .       uit_func.h                       ; prototypes for funcs
//    .       .       pkgconf/uitron.h         ; configuration info
//    .       .       uit_type.h               ; typedefs for func args
//    .       (function prototypes)
//    .       .       uit_func.inl             ; full function bodies
//    .       .       .       uit_objs.hxx     ; defs of uITRON data
//    .       .       (function implementations)
//    
//
// [out-of-line uITRON functions:]
//
//    <your_app.c>
//    .       uit_func.h                       ; prototypes for funcs
//    .       .       pkgconf/uitron.h         ; configuration info
//    .       .       uit_type.h               ; typedefs for func args
//    .       (function prototypes)
//    
//
// [other uITRON compilation units:]
//
//    uit_func.cxx                             ; out-of-line functions
//    .       pkgconf/uitron.h                 ; configuration info
//    .       uit_func.h                       ; prototypes for funcs
//    .       .       uit_type.h               ; typedefs for func args
//    .       (function prototypes)
//    .       .       uit_func.inl             ; full function bodies
//    .       .       .       uit_objs.hxx     ; defs of uITRON data
//    .       .       (function implementations)
// 
//
//    uit_objs.cxx                             ; static uITRON data objects
//    .       pkgconf/uitron.h                 ; configuration info
//    .       uit_objs.hxx                     ; defs of uITRON data
//    (static uITRON system objects)
//
// 
// The various include files are protected against multiple inclusion and
// so may be safely re-included as convenient.
// 
// ------------------------------------------------------------------------

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

#include <cyg/infra/cyg_type.h>         // types; cyg_int32, CYG_ADDRWORD

#include <cyg/compat/uitron/uit_type.h> // uITRON types; ER ID TMO T_MSG

// ------------------------------------------------------------------------
// Object operations:
//
// The functions can be inlined in C compiled by C++, or C++ of course,
// and also outlined in extern "C" functions, eg. for taking the address
// of, or for use by a pure C program, or of course outlined in C++ for
// Code size reasons.
//
//
// Summary:
//
// IF compiling in C
// THEN functions must be C linkage and out of line:
//      do NOT specify CYGIMP_UITRON_INLINE_FUNCS nor
//                     CYGIMP_UITRON_CPP_OUTLINE_FUNCS.
// IF compiling in C++
// THEN functions can be inline: specify CYGIMP_UITRON_INLINE_FUNCS
//   OR by default, functions are out of line:
//       outline functions can have C++ linkage:
//                 specify CYGIMP_UITRON_CPP_OUTLINE_FUNCS
//       OR by default, outline functions have C linkage.


#ifdef __cplusplus
// C++ environment; functions can be inline or not as we please.
// If not inline they might as well be "C" linkage for sharing with
// any pure "C" code present.

#ifdef CYGIMP_UITRON_INLINE_FUNCS

#define CYG_UIT_FUNC_EXTERN_BEGIN
#define CYG_UIT_FUNC_EXTERN_END
#define CYG_UIT_FUNC_INLINE             inline
#ifndef CYGPRI_UITRON_FUNCS_HERE_AND_NOW
#define CYGPRI_UITRON_FUNCS_HERE_AND_NOW
#endif

#else

#ifdef CYGIMP_UITRON_CPP_OUTLINE_FUNCS
#define CYG_UIT_FUNC_EXTERN_BEGIN       extern "C++" {
#define CYG_UIT_FUNC_EXTERN_END         }
#else
#define CYG_UIT_FUNC_EXTERN_BEGIN       extern "C" {
#define CYG_UIT_FUNC_EXTERN_END         }
#endif

#define CYG_UIT_FUNC_INLINE
#endif

#else // !__cplusplus
// Vanilla "C" environment; external "C" linkage, no inline functions

#ifdef CYGIMP_UITRON_INLINE_FUNCS
#error "Cannot inline uITRON functions in pure C environment"
#endif
#ifdef CYGIMP_UITRON_CPP_OUTLINE_FUNCS
#error "Cannot use C++ linkage of outline fns in pure C environment"
#endif

#define CYG_UIT_FUNC_EXTERN_BEGIN
#define CYG_UIT_FUNC_EXTERN_END
#define CYG_UIT_FUNC_INLINE             

#endif // !__cplusplus

// ========================================================================
//         u I T R O N   F U N C T I O N S
// The function declarations themselves:

CYG_UIT_FUNC_EXTERN_BEGIN

// this routine is outside the uITRON specification; call it from main() to
// start the uITRON tasks and scheduler.  It does not return.

#ifdef CYGNUM_UITRON_START_TASKS
void cyg_uitron_start( void );
#endif

// ******************************************************
// ***    6.5 C Language Interfaces                   ***
// ******************************************************

// - Task Management Functions

ER      cre_tsk ( ID tskid, T_CTSK *pk_ctsk );
ER      del_tsk ( ID tskid );
ER      sta_tsk ( ID tskid, INT stacd );
void    ext_tsk ( void );
void    exd_tsk ( void );
ER      ter_tsk ( ID tskid );

ER      dis_dsp ( void );
ER      ena_dsp ( void );
ER      chg_pri ( ID tskid, PRI tskpri );
ER      rot_rdq ( PRI tskpri );
ER      rel_wai ( ID tskid );
ER      get_tid ( ID *p_tskid );
ER      ref_tsk ( T_RTSK *pk_rtsk, ID tskid );
        
// - Task-Dependent Synchronization Functions
        
ER      sus_tsk ( ID tskid );
ER      rsm_tsk ( ID tskid );
ER      frsm_tsk ( ID tskid );
ER      slp_tsk ( void );
ER      tslp_tsk ( TMO tmout );
ER      wup_tsk ( ID tskid );
ER      can_wup ( INT *p_wupcnt, ID tskid );
        
// - Synchronization and Communication Functions
        
ER      cre_sem ( ID semid, T_CSEM *pk_csem );
ER      del_sem ( ID semid );
ER      sig_sem ( ID semid );
ER      wai_sem ( ID semid );
ER      preq_sem ( ID semid );
ER      twai_sem ( ID semid, TMO tmout );
ER      ref_sem ( T_RSEM *pk_rsem, ID semid );

ER      cre_flg ( ID flgid, T_CFLG *pk_cflg );
ER      del_flg ( ID flgid );
ER      set_flg ( ID flgid, UINT setptn );
ER      clr_flg ( ID flgid, UINT clrptn );
ER      wai_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode );
ER      pol_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode );
ER      twai_flg ( UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode,
              TMO tmout );
ER      ref_flg ( T_RFLG *pk_rflg, ID flgid );

ER      cre_mbx ( ID mbxid, T_CMBX* pk_cmbx );
ER      del_mbx ( ID mbxid );
ER      snd_msg ( ID mbxid, T_MSG *pk_msg );
ER      rcv_msg ( T_MSG **ppk_msg, ID mbxid );
ER      prcv_msg ( T_MSG **ppk_msg, ID mbxid );
ER      trcv_msg ( T_MSG **ppk_msg, ID mbxid, TMO tmout );
ER      ref_mbx ( T_RMBX *pk_rmbx, ID mbxid );
        
// - Extended Synchronization and Communication Functions
        
#if 0 // NOT SUPPORTED
ER      cre_mbf ( ID mbfid, T_CMBF *pk_cmbf );
ER      del_mbf ( ID mbfid );
ER      snd_mbf ( ID mbfid, VP msg, INT msgsz );
ER      psnd_mbf ( ID mbfid, VP msg, INT msgsz );
ER      tsnd_mbf ( ID mbfid, VP msg, INT msgsz, TMO tmout );
ER      rcv_mbf ( VP msg, INT *p_msgsz, ID mbfid );
ER      prcv_mbf ( VP msg, INT *p_msgsz, ID mbfid );
ER      trcv_mbf ( VP msg, INT *p_msgsz, ID mbfid, TMO tmout );
ER      ref_mbf ( T_RMBF *pk_rmbf, ID mbfid );
ER      cre_por ( ID porid, T_CPOR *pk_cpor );
ER      del_por ( ID porid );
ER      cal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz );
ER      pcal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz );
ER      tcal_por ( VP msg, INT *p_rmsgsz, ID porid, UINT calptn, INT
              cmsgsz, TMO tmout );
ER      acp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn );
ER      pacp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn );
ER      tacp_por ( RNO *p_rdvno, VP msg, INT *p_cmsgsz, ID porid, UINT
              acpptn, TMO tmout );
ER      fwd_por ( ID porid, UINT calptn, RNO rdvno, VP msg, INT cmsgsz
              );
ER      rpl_rdv ( RNO rdvno, VP msg, INT rmsgsz );
ER      ref_por ( T_RPOR *pk_rpor, ID porid );
#endif
        
// - Interrupt Management Functions
        
#if 0 // NOT SUPPORTED
ER      def_int ( UINT dintno, T_DINT *pk_dint );
void    ret_wup ( ID tskid );
#endif
#if 0
void    ret_int ( void );
#endif
#define ret_int() return
ER      loc_cpu ( void );
ER      unl_cpu ( void );

ER      dis_int ( UINT eintno );
ER      ena_int ( UINT eintno );

#if 0 // NOT SUPPORTED
ER      chg_iXX ( UINT iXXXX );
ER      ref_iXX ( UINT *p_iXXXX );
#endif
        
// - Memorypool Management Functions
        
ER      cre_mpl ( ID mplid, T_CMPL *pk_cmpl );
ER      del_mpl ( ID mplid );
ER      get_blk ( VP *p_blk, ID mplid, INT blksz );
ER      pget_blk ( VP *p_blk, ID mplid, INT blksz );
ER      tget_blk ( VP *p_blk, ID mplid, INT blksz, TMO tmout );
ER      rel_blk ( ID mplid, VP blk );
ER      ref_mpl ( T_RMPL *pk_rmpl, ID mplid );

ER      cre_mpf ( ID mpfid, T_CMPF *pk_cmpf );
ER      del_mpf ( ID mpfid );
ER      get_blf ( VP *p_blf, ID mpfid );
ER      pget_blf ( VP *p_blf, ID mpfid );
ER      tget_blf ( VP *p_blf, ID mpfid, TMO tmout );
ER      rel_blf ( ID mpfid, VP blf );
ER      ref_mpf ( T_RMPF *pk_rmpf, ID mpfid );
        
// - Time Management Functions
        
ER      set_tim ( SYSTIME *pk_tim );
ER      get_tim ( SYSTIME *pk_tim );
ER      dly_tsk ( DLYTIME dlytim );
ER      def_cyc ( HNO cycno, T_DCYC *pk_dcyc );
ER      act_cyc ( HNO cycno, UINT cycact );
ER      ref_cyc ( T_RCYC *pk_rcyc, HNO cycno );
ER      def_alm ( HNO almno, T_DALM *pk_dalm );
ER      ref_alm ( T_RALM *pk_ralm, HNO almno );
#if 0
void    ret_tmr ( void );
#endif
#define ret_tmr() return
        
// - System Management Functions
        
ER      get_ver ( T_VER *pk_ver );
ER      ref_sys ( T_RSYS *pk_rsys );
ER      ref_cfg ( T_RCFG *pk_rcfg );
#if 0 // NOT SUPPORTED
ER      def_svc ( FN s_fncd, T_DSVC *pk_dsvc );
ER      def_exc ( UINT exckind, T_DEXC *pk_dexc );
#endif
        
// - Network Support Functions
        
#if 0 // NOT SUPPORTED
ER      nrea_dat ( INT *p_reasz, VP dstadr, NODE srcnode, VP srcadr,
               INT datsz );
ER      nwri_dat ( INT *p_wrisz, NODE dstnode, VP dstadr, VP srcadr,
               INT datsz );
ER      nget_nod ( NODE *p_node );
ER      nget_ver ( T_VER *pk_ver, NODE node );
#endif

CYG_UIT_FUNC_EXTERN_END

// ========================================================================

#ifdef CYGPRI_UITRON_FUNCS_HERE_AND_NOW
// functions are inline OR we are in the outline implementation, so define
// the functions as inlines or plain functions depending on the value of
// CYG_UIT_FUNC_INLINE from above.
#include <cyg/compat/uitron/uit_func.inl>
#endif // CYGPRI_UITRON_FUNCS_HERE_AND_NOW

// ------------------------------------------------------------------------
#endif // CYGPKG_UITRON

#endif // CYGONCE_COMPAT_UITRON_UIT_FUNC_H
// EOF uit_func.h
