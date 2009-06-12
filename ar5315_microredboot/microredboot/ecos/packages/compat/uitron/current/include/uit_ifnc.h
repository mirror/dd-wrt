#ifndef CYGONCE_COMPAT_UITRON_UIT_IFNC_H
#define CYGONCE_COMPAT_UITRON_UIT_IFNC_H
//===========================================================================
//
//      uit_ifnc.h
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

// ------------------------------------------------------------------------
// Source Code Organization
//
// First, see pkgconf/uitron.h for details of applicable configuration
// options.
//
// This file uit_ifnc.h provides prototypes for the task-independent parts
// of the uITRON API, that is functions named ixxx_yyy() for calling in
// ISRs.  We also define the uitron helper DSR that is needed to despool
// stored up requests.
// ------------------------------------------------------------------------

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

#include <cyg/infra/cyg_type.h>         // types; cyg_int32, CYG_ADDRWORD

#include <cyg/compat/uitron/uit_type.h> // uITRON types; ER ID TMO T_MSG
#include <cyg/compat/uitron/uit_func.h> // uITRON funcs and control macros.

// ========================================================================
//         u I T R O N   F U N C T I O N S
// The function declarations themselves:

// ------------------- These functions can be inline if so configured
CYG_UIT_FUNC_EXTERN_BEGIN

// ******************************************************
// ***    6.5 C Language Interfaces                   ***
// ******************************************************

// - Task Management Functions

// (None)
        
// - Task-Dependent Synchronization Functions
        
//ER      irsm_tsk ( ID tskid );
//ER      ifrsm_tsk ( ID tskid );

ER      iwup_tsk ( ID tskid );
        
// - Synchronization and Communication Functions
        
ER      isig_sem ( ID semid );

ER      iset_flg ( ID flgid, UINT setptn );

ER      isnd_msg ( ID mbxid, T_MSG *pk_msg );
        
// - Extended Synchronization and Communication Functions
        
// - Interrupt Management Functions
        
// (None)
   
// ---------------------------------------------------------------

#define CYGPRI_UITRON_SET_RETCODE( _z_ ) do {                                   \
    extern volatile int cyg_uit_dsr_actions_head;                               \
    extern volatile int cyg_uit_dsr_actions_tail;                               \
    (_z_) = (cyg_uit_dsr_actions_head == cyg_uit_dsr_actions_tail) ? 1 : 3;     \
} while ( 0 )

//void    ret_wup ( ID tskid );
// Awaken the task (safely) and return Cyg_Interrupt::CALL_DSR
#define ret_wup( _id_ ) do {                    \
    register int retcode;                       \
    (void)iwup_tsk( (_id_) );                   \
    CYGPRI_UITRON_SET_RETCODE( retcode );       \
    return retcode;                             \
} while ( 0 )

// Subsitute a version of ret_int that returns Cyg_Interrupt::CALL_DSR
#undef ret_int
#define ret_int()  do {                         \
    register int retcode;                       \
    CYGPRI_UITRON_SET_RETCODE( retcode );       \
    return retcode;                             \
} while ( 0 )


// - Memorypool Management Functions

// (None)
        
// - Time Management Functions
        
// (None)
        
// - System Management Functions
        
// (None)
        
// - Network Support Functions
        
// (None)
        
CYG_UIT_FUNC_EXTERN_END
// ------------------- End of functions that can be inlined


// ========================================================================
// DSR: use this DSR with the uITRON-type ISR that uses the functions above
// to get delayed/safe execution of the wakeup-type functions above.

#ifdef __cplusplus
extern "C"
#endif
void cyg_uitron_dsr( unsigned int vector, unsigned int count, unsigned int data );


// ========================================================================

#ifdef CYGPRI_UITRON_FUNCS_HERE_AND_NOW
// functions are inline OR we are in the outline implementation, so define
// the functions as inlines or plain functions depending on the value of
// CYG_UIT_FUNC_INLINE from above.
#include <cyg/compat/uitron/uit_ifnc.inl>
#endif // CYGPRI_UITRON_FUNCS_HERE_AND_NOW

// ------------------------------------------------------------------------
#endif // CYGPKG_UITRON

#endif // CYGONCE_COMPAT_UITRON_UIT_IFNC_H
// EOF uit_ifnc.h
