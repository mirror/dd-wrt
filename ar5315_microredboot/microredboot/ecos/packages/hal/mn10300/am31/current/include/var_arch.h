#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H

//==========================================================================
//
//      var_arch.h
//
//      Architecture specific abstractions
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg
// Date:         1999-02-17
// Purpose:      Define architecture abstractions
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture.
// Usage:        #include <cyg/hal/var_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Processor saved states:

typedef struct HAL_SavedRegisters
{
    // These are common to all saved states and are in the order
    // stored and loaded by the movm instruction.
    CYG_ADDRWORD        vector;         /* Vector number/dummy          */
    CYG_ADDRWORD        lar;            /* Loop address register        */
    CYG_ADDRWORD        lir;            /* Loop instruction register    */
    CYG_ADDRWORD        mdr;            /* Multiply/Divide register     */
    CYG_ADDRWORD        a1;
    CYG_ADDRWORD        a0;
    CYG_ADDRWORD        d1;
    CYG_ADDRWORD        d0;
    CYG_ADDRWORD        a3;
    CYG_ADDRWORD        a2;
    CYG_ADDRWORD        d3;
    CYG_ADDRWORD        d2;
    
    /* On interrupts the PC and PSW are pushed automatically by the     */
    /* CPU and SP is pushed for debugging reasons. On a thread switch   */
    /* the saved context is made to look the same.                      */

    CYG_ADDRWORD  sp;             /* Saved copy of SP in some states      */
    CYG_ADDRWORD  psw;            /* Status word                          */
    CYG_ADDRWORD  pc;             /* Program Counter                      */
    
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
