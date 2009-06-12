#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H
//=============================================================================
//
//      var_arch.h
//
//      Architecture variant specific abstractions
//
//=============================================================================
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
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/proc_arch.h>

// The ColdFire family of processors has a simplified exception stack
// frame that looks like the following:
//
//             3322222222221111 111111
//             1098765432109876 5432109876543210
//          8 +----------------+----------------+
//            |         Program Counter         |
//          4 +----------------+----------------+
//            |Fmt/FS/Vector/FS|      SR        |
//  SP -->  0 +----------------+----------------+
//
// The stack self-aligns to a 4-byte boundary at an exception, with
// the Fmt/FS/Vector/FS field indicating the size of the adjustment
// (SP += 0,1,2,3 bytes).

// Define the Fmt/FS/Vector/FS word.
// The first four bits are the format word which tells the
// RTI instruction how to align the stack.
#define HAL_MCF52XX_RD_SF_FORMAT_MSK ((CYG_WORD16)0xF000)
// These bits are the vector number of the exception.
#define HAL_MCF52XX_RD_SF_VECTOR_MSK ((CYG_WORD16)0x03FC)
// These are bits 3-2, and 1-0 of the fault status used
// for bus and address errors.
#define HAL_MCF52XX_RD_SF_FS32_MSK   ((CYG_WORD16)0x0C00)
#define HAL_MCF52XX_RD_SF_FS10_MSK   ((CYG_WORD16)0x0003)

// Macros to access fields in the format vector word.

#define HAL_MCF52XX_RD_SF_FORMAT(_fmt_vec_word_)                            \
    ((((CYG_WORD16)(_fmt_vec_word_)) & HAL_MCF52XX_RD_SF_FORMAT_MSK) >> 12)

#define HAL_MCF52XX_RD_SF_VECTOR(_fmt_vec_word_)                            \
    ((((CYG_WORD16)(_fmt_vec_word_)) & HAL_MCF52XX_RD_SF_VECTOR_MSK) >> 2)

#define HAL_MCF52XX_RD_SF_FS(_fmt_vec_word_)                                \
     (((((CYG_WORD16)(_fmt_vec_word_)) & HAL_MCF52XX_RD_SF_FS32_MSK) >> 8)  \
     | (((CYG_WORD16)(_fmt_vec_word_)) & HAL_MCF52XX_RD_SF_FS10_MSK))

/*****************************************************************************
     Exception  handler  saved  context.   Some  exceptions  contain  extra
information following this common exception handler context.
*****************************************************************************/
typedef struct
{

    //   Data regs D0-D7

    #define HAL_EXC_NUM_D_REGS 8
    CYG_WORD32 d[HAL_EXC_NUM_D_REGS];

    //   Address regs A0-A6

    #define HAL_EXC_NUM_A_REGS 7
    CYG_ADDRESS a[HAL_EXC_NUM_A_REGS];

    //   Stack Pointer

    CYG_ADDRESS sp;

    //   16-bit format/vector word

    CYG_WORD16 fmt_vec_word;

    //   Status Reg

    CYG_WORD16 sr;

    //   Program Counter

    CYG_ADDRESS pc;

} __attribute__ ((aligned, packed)) HAL_SavedRegisters_exception;

#ifndef HAL_GENERIC_SAVED_CONTEXT
/*****************************************************************************
HAL_GENERIC_SAVED_CONTEXT -- Generic saved context structure

     This structure could contain  a normal saved  context or an  exception
context.

*****************************************************************************/
#define HAL_GENERIC_SAVED_CONTEXT \
typedef union \
{ \
    HAL_SavedRegisters_normal normal; \
    HAL_SavedRegisters_exception exception; \
} __attribute__ ((aligned, packed)) HAL_SavedRegisters;
#endif // HAL_GENERIC_SAVED_CONTEXT

//-----------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.

/* there are 180 bytes of registers on a 68020 w/68881      */
/* many of the fpa registers are 12 byte (96 bit) registers */
/*
#define NUMREGBYTES 180
enum regnames {D0,D1,D2,D3,D4,D5,D6,D7,
               A0,A1,A2,A3,A4,A5,A6,A7,
               PS,PC,
               FP0,FP1,FP2,FP3,FP4,FP5,FP6,FP7,
               FPCONTROL,FPSTATUS,FPIADDR
              };
*/

#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )              \
    CYG_MACRO_START                                             \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < HAL_NUM_D_REGS; _i_++ )                 \
        *_regval_++ = (_regs_)->nml_ctxt.d[_i_];                \
                                                                \
    for( _i_ = 0; _i_ < HAL_NUM_A_REGS; _i_++ )                 \
        *_regval_++ = (_regs_)->nml_ctxt.a[_i_];                \
                                                                \
    *_regval_++ = (_regs_)->nml_ctxt.sp;                        \
    *_regval_++ = (CYG_ADDRWORD) ((_regs_)->nml_ctxt.sr);       \
    *_regval_++ = (_regs_)->nml_ctxt.pc;                        \
    /* Undefined registers */                                   \
    for ( _i_ = 0; _i_ < 8; _i_++ )                             \
    {                                                           \
        *((CYG_WORD16*)_regval_)++ = _i_;                       \
        *((CYG_WORD16*)_regval_)++ = _i_;                       \
        *((CYG_WORD16*)_regval_)++ = _i_;                       \
    }                                                           \
    *_regval_++ = 0xBADC0DE0;                                   \
    *_regval_++ = 0xBADC0DE1;                                   \
    *_regval_++ = 0xBADC0DE2;                                   \
    CYG_MACRO_END

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
    CYG_MACRO_START                                             \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < HAL_NUM_D_REGS; _i_++ )                 \
        (_regs_)->nml_ctxt.d[_i_] = *_regval_++;                \
                                                                \
    for( _i_ = 0; _i_ < HAL_NUM_A_REGS; _i_++ )                 \
        (_regs_)->nml_ctxt.a[_i_] = *_regval_++;                \
                                                                \
    (_regs_)->nml_ctxt.sp = *_regval_++;                        \
    (_regs_)->nml_ctxt.sr = (CYG_WORD16) (*_regval_++);         \
    (_regs_)->nml_ctxt.pc = *_regval_++;                        \
    CYG_MACRO_END

/* ************************************************************************ */
/* These routines write to  the special purpose  registers in the  ColdFire */
/* core.  Since these registers are write-only in the supervisor model,  no */
/* corresponding read routines exist.                                       */

externC void mcf52xx_wr_vbr(CYG_WORD32);
externC void mcf52xx_wr_cacr(CYG_WORD32);
externC void mcf52xx_wr_acr0(CYG_WORD32);
externC void mcf52xx_wr_acr1(CYG_WORD32);
externC void mcf52xx_wr_rambar(CYG_WORD32);

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H

