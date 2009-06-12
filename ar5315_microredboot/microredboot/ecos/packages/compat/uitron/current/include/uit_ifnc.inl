#ifndef CYGONCE_COMPAT_UITRON_UIT_IFNC_INL
#define CYGONCE_COMPAT_UITRON_UIT_IFNC_INL
//===========================================================================
//
//      uit_ifnc.inl
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
// Date:        1999-08-16
// Purpose:     uITRON compatibility functions
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifdef CYGPKG_UITRON

#ifdef CYGPRI_UITRON_FUNCS_HERE_AND_NOW

class Cyg_Uit_Action {
public:
    typedef enum {
        WUP_TSK,
        SIG_SEM,
        SET_FLG,
        SND_MSG
    } action;
};

extern volatile int cyg_uit_dsr_actions_head;
extern volatile int cyg_uit_dsr_actions_tail;

#define CYGNUM_UITRON_ISR_ACTION_QUEUEMASK (CYGNUM_UITRON_ISR_ACTION_QUEUESIZE-1)

#if ((~CYGNUM_UITRON_ISR_ACTION_QUEUEMASK) &            \
     ~((~CYGNUM_UITRON_ISR_ACTION_QUEUEMASK)-1))        \
    != CYGNUM_UITRON_ISR_ACTION_QUEUESIZE
#error CYGNUM_UITRON_ISR_ACTION_QUEUESIZE not a power of 2
#endif

extern Cyg_Uit_Action::action
cyg_uit_dsr_actions[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

extern ID
cyg_uit_dsr_act_ids[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

extern CYG_ADDRWORD
cyg_uit_dsr_act_a1s[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

CYG_UIT_FUNC_INLINE
ER
iwup_tsk ( ID tskid )
{
#ifdef CYGSEM_UITRON_ISRFUNCS_TRY_IMMEDIATE_EXECUTION
    if ( 1 >= Cyg_Scheduler::get_sched_lock() ) {
        // then this ISR is the first one, and the sched was locked by the
        // interrupt code.  So this is safe.
        return wup_tsk( tskid );
    }
#endif
    
    register int i, head;
    i = cyg_uit_dsr_actions_head;
    head = CYGNUM_UITRON_ISR_ACTION_QUEUEMASK & ( 1 + i );
    // If interrupts can be recursive, then there is a race here where a
    // slot may be overwritten by a recursive interrupt, or actions from
    // such lost; better though than having a slot contain *mixed* data
    // from two intermingled interrupts.
    if ( head != cyg_uit_dsr_actions_tail ) {
        cyg_uit_dsr_actions_head = head;
        cyg_uit_dsr_actions[ i ] = Cyg_Uit_Action::WUP_TSK;
        cyg_uit_dsr_act_ids[ i ] = tskid;
    }
    return E_OK;
}
        
#ifdef CYGPKG_UITRON_SEMAS
#if 0 < CYG_UITRON_NUM( SEMAS )
CYG_UIT_FUNC_INLINE
ER
isig_sem ( ID semid )
{
#ifdef CYGSEM_UITRON_ISRFUNCS_TRY_IMMEDIATE_EXECUTION
    if ( 1 >= Cyg_Scheduler::get_sched_lock() ) {
        // then this ISR is the first one, and the sched was locked by the
        // interrupt code.  So this is safe.
        return sig_sem( semid );
    }
#endif
    
    register int i, head;
    i = cyg_uit_dsr_actions_head;
    head = CYGNUM_UITRON_ISR_ACTION_QUEUEMASK & ( 1 + i );
    // If interrupts can be recursive, then there is a race here where a
    // slot may be overwritten by a recursive interrupt, or actions from
    // such lost; better though than having a slot contain *mixed* data
    // from two intermingled interrupts.
    if ( head != cyg_uit_dsr_actions_tail ) {
        cyg_uit_dsr_actions_head = head;
        cyg_uit_dsr_actions[ i ] = Cyg_Uit_Action::SIG_SEM;
        cyg_uit_dsr_act_ids[ i ] = semid;
    }
    return E_OK;
}
#endif // 0 < CYG_UITRON_NUM( SEMAS )
#endif // CYGPKG_UITRON_SEMAS

#ifdef CYGPKG_UITRON_FLAGS
#if 0 < CYG_UITRON_NUM( FLAGS )
CYG_UIT_FUNC_INLINE
ER
iset_flg ( ID flgid, UINT setptn )
{
#ifdef CYGSEM_UITRON_ISRFUNCS_TRY_IMMEDIATE_EXECUTION
    if ( 1 >= Cyg_Scheduler::get_sched_lock() ) {
        // then this ISR is the first one, and the sched was locked by the
        // interrupt code.  So this is safe.
        return set_flg( flgid, setptn );
    }
#endif
    
    register int i, head;
    i = cyg_uit_dsr_actions_head;
    head = CYGNUM_UITRON_ISR_ACTION_QUEUEMASK & ( 1 + i );
    // If interrupts can be recursive, then there is a race here where a
    // slot may be overwritten by a recursive interrupt, or actions from
    // such lost; better though than having a slot contain *mixed* data
    // from two intermingled interrupts.
    if ( head != cyg_uit_dsr_actions_tail ) {
        cyg_uit_dsr_actions_head = head;
        cyg_uit_dsr_actions[ i ] = Cyg_Uit_Action::SET_FLG;
        cyg_uit_dsr_act_ids[ i ] = flgid;
        cyg_uit_dsr_act_a1s[ i ] = (CYG_ADDRWORD)setptn;
    }
    return E_OK;
}
#endif // 0 < CYG_UITRON_NUM( FLAGS )
#endif // CYGPKG_UITRON_FLAGS

#ifdef CYGPKG_UITRON_MBOXES
#if 0 < CYG_UITRON_NUM( MBOXES )
CYG_UIT_FUNC_INLINE
ER
isnd_msg ( ID mbxid, T_MSG *pk_msg )
{
#ifdef CYGSEM_UITRON_ISRFUNCS_TRY_IMMEDIATE_EXECUTION
    if ( 1 >= Cyg_Scheduler::get_sched_lock() ) {
        // then this ISR is the first one, and the sched was locked by the
        // interrupt code.  So this is safe.
        return snd_msg( mbxid, pk_msg );
    }
#endif
    
    register int i, head;
    i = cyg_uit_dsr_actions_head;
    head = CYGNUM_UITRON_ISR_ACTION_QUEUEMASK & ( 1 + i );
    // If interrupts can be recursive, then there is a race here where a
    // slot may be overwritten by a recursive interrupt, or actions from
    // such lost; better though than having a slot contain *mixed* data
    // from two intermingled interrupts.
    if ( head != cyg_uit_dsr_actions_tail ) {
        cyg_uit_dsr_actions_head = head;
        cyg_uit_dsr_actions[ i ] = Cyg_Uit_Action::SND_MSG;
        cyg_uit_dsr_act_ids[ i ] = mbxid;
        cyg_uit_dsr_act_a1s[ i ] = (CYG_ADDRWORD)pk_msg;
    }
    return E_OK;
}
#endif // 0 < CYG_UITRON_NUM( MBOXES )
#endif // CYGPKG_UITRON_MBOXES
        
// ========================================================================

#endif // CYGPKG_UITRON

#endif // CYGPRI_UITRON_FUNCS_HERE_AND_NOW

#endif // CYGONCE_COMPAT_UITRON_UIT_IFNC_INL
//EOF uit_ifnc.inl
