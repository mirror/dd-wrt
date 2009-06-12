//===========================================================================
//
//      uit_ifnc.cxx
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
// Purpose:     uITRON compatibility functions for use in ISRs
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

// invoke the inline function definition to create static C linkage
// functions here:
#define CYGIMP_UITRON_INLINE_FUNCS 1
#include <cyg/compat/uitron/uit_func.h>

// Now ensure that we create *outline* funcs for the ixxx_yyy() functions
// here, with C names or whatever, as required.

#undef CYGPRI_UITRON_FUNCS_HERE_AND_NOW
#undef CYGIMP_UITRON_INLINE_FUNCS
#undef CYG_UIT_FUNC_EXTERN_BEGIN
#undef CYG_UIT_FUNC_EXTERN_END

#ifdef CYGIMP_UITRON_CPP_OUTLINE_FUNCS
#define CYG_UIT_FUNC_EXTERN_BEGIN       extern "C++" {
#define CYG_UIT_FUNC_EXTERN_END         }
#else
#define CYG_UIT_FUNC_EXTERN_BEGIN       extern "C" {
#define CYG_UIT_FUNC_EXTERN_END         }
#endif

// Get extern C prototypes (or whatever uit_func.h above did)
#include <cyg/compat/uitron/uit_ifnc.h>

#undef  CYG_UIT_FUNC_INLINE
#define CYG_UIT_FUNC_INLINE /* blank */
#define CYGPRI_UITRON_FUNCS_HERE_AND_NOW
#include <cyg/compat/uitron/uit_ifnc.inl>

volatile int cyg_uit_dsr_actions_head = 0;
volatile int cyg_uit_dsr_actions_tail = 0;

Cyg_Uit_Action::action
cyg_uit_dsr_actions[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

ID
cyg_uit_dsr_act_ids[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

CYG_ADDRWORD
cyg_uit_dsr_act_a1s[ CYGNUM_UITRON_ISR_ACTION_QUEUESIZE ];

void
cyg_uitron_dsr( unsigned int vector, unsigned int count, unsigned int data )
{
    while ( cyg_uit_dsr_actions_tail != cyg_uit_dsr_actions_head ) {
        switch ( cyg_uit_dsr_actions[ cyg_uit_dsr_actions_tail ] ) {
        case Cyg_Uit_Action::WUP_TSK:
            (void)wup_tsk( cyg_uit_dsr_act_ids[ cyg_uit_dsr_actions_tail ] );
            break;
#ifdef CYGPKG_UITRON_SEMAS
#if 0 < CYG_UITRON_NUM( SEMAS )
        case Cyg_Uit_Action::SIG_SEM:
            (void)sig_sem( cyg_uit_dsr_act_ids[ cyg_uit_dsr_actions_tail ] );
            break;
#endif // 0 < CYG_UITRON_NUM( SEMAS )
#endif // CYGPKG_UITRON_SEMAS
#ifdef CYGPKG_UITRON_FLAGS
#if 0 < CYG_UITRON_NUM( FLAGS )
        case Cyg_Uit_Action::SET_FLG:
            (void)set_flg( cyg_uit_dsr_act_ids[ cyg_uit_dsr_actions_tail ],
                     (UINT)cyg_uit_dsr_act_a1s[ cyg_uit_dsr_actions_tail ] );
            break;
#endif // 0 < CYG_UITRON_NUM( FLAGS )
#endif // CYGPKG_UITRON_FLAGS
#ifdef CYGPKG_UITRON_MBOXES
#if 0 < CYG_UITRON_NUM( MBOXES )
        case Cyg_Uit_Action::SND_MSG:
            (void)snd_msg( cyg_uit_dsr_act_ids[ cyg_uit_dsr_actions_tail ],
                  (T_MSG *)cyg_uit_dsr_act_a1s[ cyg_uit_dsr_actions_tail ] );
            break;
#endif // 0 < CYG_UITRON_NUM( MBOXES )
#endif // CYGPKG_UITRON_MBOXES
        default:
            CYG_FAIL( "enum Cyg_Uit_Action out of range!" );
        }
        cyg_uit_dsr_actions_tail =
            CYGNUM_UITRON_ISR_ACTION_QUEUEMASK & (1+cyg_uit_dsr_actions_tail);
    }
}

#endif // CYGPKG_UITRON

// EOF uit_ifnc.cxx
