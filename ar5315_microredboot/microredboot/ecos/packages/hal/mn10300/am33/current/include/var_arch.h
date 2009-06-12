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
// Contributors: nickg, dmoseley
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
// Processor Status word bitmasks

#define HAL_ARCH_AM33_PSW_nSL           (1L << 16)
#define HAL_ARCH_AM33_PSW_ML            (1L << 19)
#define HAL_ARCH_AM33_PSW_FE            (1L << 20)

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
    CYG_ADDRWORD        mcvf;           /* MAC overflow flag            */
    CYG_ADDRWORD        mcrl;           /* MAC register low             */
    CYG_ADDRWORD        mcrh;           /* MAC register high            */
    CYG_ADDRWORD        mdrq;           /* Fast multiply/divide register*/
    CYG_ADDRWORD        e1;             /* extended registers           */
    CYG_ADDRWORD        e0;
    CYG_ADDRWORD        e7;
    CYG_ADDRWORD        e6;
    CYG_ADDRWORD        e5;
    CYG_ADDRWORD        e4;
    CYG_ADDRWORD        e3;
    CYG_ADDRWORD        e2;
    
    /* On interrupts the PC and PSW are pushed automatically by the     */
    /* CPU and SP is pushed for debugging reasons. On a thread switch   */
    /* the saved context is made to look the same.                      */

    CYG_ADDRWORD  sp;             /* Saved copy of SP in some states      */
    CYG_ADDRWORD  psw;            /* Status word                          */
    CYG_ADDRWORD  pc;             /* Program Counter                      */
    
} HAL_SavedRegisters;

//--------------------------------------------------------------------------
// Extra initialization for AM33 extended register set.

#define HAL_THREAD_INIT_CONTEXT_EXTRA(_regs_, _id_)     \
CYG_MACRO_START                                         \
    (_regs_)->e0        = (_id_)|0xeee0;                \
    (_regs_)->e1        = (_id_)|0xeee1;                \
    (_regs_)->e2        = (_id_)|0xeee2;                \
    (_regs_)->e3        = (_id_)|0xeee3;                \
    (_regs_)->e4        = (_id_)|0xeee4;                \
    (_regs_)->e5        = (_id_)|0xeee5;                \
    (_regs_)->e6        = (_id_)|0xeee6;                \
    (_regs_)->e7        = (_id_)|0xeee7;                \
    (_regs_)->mcrl      = 0;                            \
    (_regs_)->mcrh      = 0;                            \
    (_regs_)->mdrq      = 0;                            \
    (_regs_)->mcvf      = 0;                            \
CYG_MACRO_END

//--------------------------------------------------------------------------
// The following macros copy the extra AM33 registers between a
// HAL_SavedRegisters structure and a GDB register dump.
// The CYGMON version should handle the SSP and USP and MDRQ registers.

#ifdef CYGPKG_CYGMON
#define HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ ) am33_get_gdb_extra_registers( _regval_, _regs_ )
extern void am33_get_gdb_extra_registers(CYG_ADDRWORD *registers, HAL_SavedRegisters *regs);
#else // CYGPKG_CYGMON
#define HAL_GET_GDB_EXTRA_REGISTERS( _regval_, _regs_ ) \
CYG_MACRO_START                                         \
    (_regval_)[15] = (_regs_)->e0;                      \
    (_regval_)[16] = (_regs_)->e1;                      \
    (_regval_)[17] = (_regs_)->e2;                      \
    (_regval_)[18] = (_regs_)->e3;                      \
    (_regval_)[19] = (_regs_)->e4;                      \
    (_regval_)[20] = (_regs_)->e5;                      \
    (_regval_)[21] = (_regs_)->e6;                      \
    (_regval_)[22] = (_regs_)->e7;                      \
                                                        \
    (_regval_)[23] = (_regs_)->sp;                      \
    (_regval_)[24] = (_regs_)->sp;                      \
    (_regval_)[25] = (_regs_)->sp;                      \
                                                        \
    (_regval_)[26] = (_regs_)->mcrh;                    \
    (_regval_)[27] = (_regs_)->mcrl;                    \
    (_regval_)[28] = (_regs_)->mcvf;                    \
CYG_MACRO_END
#endif // CYGPKG_CYGMON

#ifdef CYGPKG_CYGMON
#define HAL_SET_GDB_EXTRA_REGISTERS( _regval_, _regs_ ) am33_set_gdb_extra_registers( _regs_, _regval_ )
extern void am33_set_gdb_extra_registers(CYG_ADDRWORD *registers, HAL_SavedRegisters *regs);
#else // CYGPKG_CYGMON
#define HAL_SET_GDB_EXTRA_REGISTERS( _regs_, _regval_ ) \
CYG_MACRO_START                                         \
    (_regs_)->e0 = (_regval_)[15];                      \
    (_regs_)->e1 = (_regval_)[16];                      \
    (_regs_)->e2 = (_regval_)[17];                      \
    (_regs_)->e3 = (_regval_)[18];                      \
    (_regs_)->e4 = (_regval_)[19];                      \
    (_regs_)->e5 = (_regval_)[20];                      \
    (_regs_)->e6 = (_regval_)[21];                      \
    (_regs_)->e7 = (_regval_)[22];                      \
                                                        \
    (_regs_)->sp = (_regval_)[23];                      \
    (_regs_)->sp = (_regval_)[24];                      \
    (_regs_)->sp = (_regval_)[25];                      \
                                                        \
    (_regs_)->mcrh = (_regval_)[26];                    \
    (_regs_)->mcrl = (_regval_)[27];                    \
    (_regs_)->mcvf = (_regval_)[28];                    \
CYG_MACRO_END
#endif // CYGPKG_CYGMON

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
