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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-02-14
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
    CYG_WORD32          er6;
    CYG_WORD32          er5;
    CYG_WORD32          er4;
    CYG_WORD32          er3;
    CYG_WORD32          er2;
    CYG_WORD32          er1;
    CYG_WORD32          er0;
    
    /* On interrupts the PC and PSW are pushed automatically by the     */
    /* CPU and SP is pushed for debugging reasons. On a thread switch   */
    /* the saved context is made to look the same.                      */

    CYG_WORD32          sp;  /* Saved copy of SP in some states      */
    CYG_WORD32          ccr;
    CYG_WORD32          exr;
    CYG_WORD32          pc;
} HAL_SavedRegisters;

#define HAL_THREAD_INIT_CONTEXT_EXTRA(_regs_, _id_) \
    {_regs_->exr   = 0;}

#define HAL_GET_GDB_EXTRA_REGISTERS(_regval_,_regs_) \
    {_regval_[10] = (_regs_)->exr;}

#define HAL_SET_GDB_EXTRA_REGISTERS( _regs_,_regval_) \
    {(_regs_)->exr = _regval_[10];}

// Internal peripheral registers
#include <cyg/hal/mod_regs_adc.h>
#include <cyg/hal/mod_regs_bsc.h>
#include <cyg/hal/mod_regs_dmac.h>
#include <cyg/hal/mod_regs_intc.h>
#include <cyg/hal/mod_regs_pio.h>
#include <cyg/hal/mod_regs_sci.h>
#include <cyg/hal/mod_regs_sys.h>
#include <cyg/hal/mod_regs_tmr.h>
#include <cyg/hal/mod_regs_ppg.h>
#include <cyg/hal/mod_regs_wdt.h>


//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
