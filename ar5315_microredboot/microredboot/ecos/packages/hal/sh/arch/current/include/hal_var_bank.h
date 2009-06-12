#ifndef CYGONCE_HAL_VAR_BANK_H
#define CYGONCE_HAL_VAR_BANK_H
//=============================================================================
//
//      hal_var_bank.h
//
//      Architecture abstractions for variants with banked registers
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett 
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov, nickg
// Date:        2002-01-11
// Purpose:     Architecture abstractions for variants with banked registers
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//-----------------------------------------------------------------------------
// Processor saved states:

typedef struct
{
    // These are common to all saved states
    cyg_uint32   r[16];                 // Data regs
    cyg_uint32   macl;                  // Multiply and accumulate - low
    cyg_uint32   mach;                  // Multiply and accumulate - high

#ifdef CYGHWR_HAL_SH_FPU
    cyg_uint32  fr[CYGHWR_HAL_SH_FPU_REGS]; // Floating point registers
    cyg_uint32  fpul;                   // Floating point comm reg
    cyg_uint32  fpscr;                  // Floating point status/control reg
#endif
    
    cyg_uint32   pr;                    // Procedure Reg
    cyg_uint32   sr;                    // Status Reg
    cyg_uint32   pc;                    // Program Counter

    // This marks the limit of state saved during a context switch and
    // is used to calculate necessary stack allocation for context switches.
    // It would probably be better to have a union instead...
    cyg_uint32   context_size[0];

    // These are only saved on interrupts
    cyg_uint32   vbr;                   // Vector Base Register
    cyg_uint32   gbr;                   // Global Base Register

    // These are only saved on interrupts
    cyg_uint32   event;                 // EXCEVT or INTEVT
} HAL_SavedRegisters;

//-----------------------------------------------------------------------------
// Context Initialization
// Initialize the context of a thread.
// Arguments:
// _sparg_ name of variable containing current sp, will be written with new sp
// _thread_ thread object address, passed as argument to entry point
// _entry_ entry point address.
// _id_ bit pattern used in initializing registers, for debugging.

#ifdef CYGHWR_HAL_SH_FPU
#include <cyg/hal/sh_regs.h>
#define HAL_THREAD_INIT_CONTEXT_FPU( _regs_ )           \
{                                                       \
    int _i_;                                            \
    (_regs_)->fpul = 0;                                 \
    (_regs_)->fpscr = CYG_FPSCR;                        \
    for( _i_ = 0; _i_ < CYGHWR_HAL_SH_FPU_REGS; _i_++ ) \
        (_regs_)->fr[_i_] = 0;                          \
}
#else
#define HAL_THREAD_INIT_CONTEXT_FPU( _regs_ )
#endif

#define HAL_THREAD_INIT_CONTEXT( _sparg_, _thread_, _entry_, _id_ )           \
    CYG_MACRO_START                                                           \
    register CYG_WORD _sp_ = (CYG_WORD)_sparg_;                               \
    register HAL_SavedRegisters *_regs_;                                      \
    int _i_;                                                                  \
    _sp_ = _sp_ & ~(CYGARC_ALIGNMENT-1);                                      \
    /* Note that _regs_ below should be aligned if HAL_SavedRegisters */      \
    /* stops being aligned to CYGARC_ALIGNMENT                        */      \
    _regs_ = (HAL_SavedRegisters *)((_sp_) - sizeof(HAL_SavedRegisters));     \
    for( _i_ = 0; _i_ < 16; _i_++ ) (_regs_)->r[_i_] = (_id_)|_i_;            \
    (_regs_)->r[15] = (CYG_WORD)(_regs_);      /* SP = top of stack      */   \
    (_regs_)->r[04] = (CYG_WORD)(_thread_);    /* R4 = arg1 = thread ptr */   \
    (_regs_)->mach = 0;                        /* MACH = 0               */   \
    (_regs_)->macl = 0;                        /* MACL = 0               */   \
    (_regs_)->pr = (CYG_WORD)(_entry_);        /* PR = entry point       */   \
    (_regs_)->sr = 0x70000000;                 /* SR = enable interrupts */   \
    (_regs_)->pc = (CYG_WORD)(_entry_);        /* set PC for thread dbg  */   \
    HAL_THREAD_INIT_CONTEXT_FPU( _regs_ );     /* Init FPU state         */   \
    _sparg_ = (CYG_ADDRESS)_regs_;                                            \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// Thread register state manipulation for GDB support.

// Translate a stack pointer as saved by the thread context macros above into
// a pointer to a HAL_SavedRegisters structure.
#define HAL_THREAD_GET_SAVED_REGISTERS( _sp_, _regs_ )  \
        (_regs_) = (HAL_SavedRegisters *)(_sp_)

#ifdef CYGHWR_HAL_SH_FPU
#define HAL_GET_GDB_REGISTERS_FPU( _regval_, _regs_ )   \
{                                                       \
    _regval_[23] = (_regs_)->fpul;                      \
    _regval_[24] = (_regs_)->fpscr;                     \
                                                        \
    for( _i_ = 0; _i_ < 16; _i_++ )                     \
        _regval_[25+_i_] = (_regs_)->fr[_i_];           \
}

#define HAL_SET_GDB_REGISTERS_FPU( _regs_, _regval_ )   \
{                                                       \
    (_regs_)->fpul = _regval_[23];                      \
    (_regs_)->fpscr = _regval_[24];                     \
                                                        \
    for( _i_ = 0; _i_ < 16; _i_++ )                     \
        (_regs_)->fr[_i_] = _regval_[25+_i_];           \
}
#else
#define HAL_GET_GDB_REGISTERS_FPU( _regval_, _regs_ )
#define HAL_SET_GDB_REGISTERS_FPU( _regs_, _regval_ )
#endif

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_, _regs_ )              \
    CYG_MACRO_START                                             \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 8; _i_++ )                              \
    {                                                           \
        _regval_[_i_] = (_regs_)->r[_i_];                       \
        _regval_[43+_i_] = (_regs_)->r[_i_];                    \
        _regval_[51+_i_] = 0;                                   \
    }                                                           \
                                                                \
    for( /* _i_ = 8 */ ; _i_ < 16; _i_++ )                      \
        _regval_[_i_] = (_regs_)->r[_i_];                       \
                                                                \
    _regval_[16] = (_regs_)->pc;                                \
    _regval_[17] = (_regs_)->pr;                                \
    _regval_[18] = (_regs_)->gbr;                               \
    _regval_[19] = (_regs_)->vbr;                               \
    _regval_[20] = (_regs_)->mach;                              \
    _regval_[21] = (_regs_)->macl;                              \
    _regval_[22] = (_regs_)->sr;                                \
                                                                \
    HAL_GET_GDB_REGISTERS_FPU( _regval_, _regs_ );              \
                                                                \
    CYG_MACRO_END

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
    CYG_MACRO_START                                             \
    CYG_ADDRWORD *_regval_ = (CYG_ADDRWORD *)(_aregval_);       \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 16; _i_++ )                             \
        (_regs_)->r[_i_] = _regval_[_i_];                       \
                                                                \
    (_regs_)->pc = _regval_[16];                                \
    (_regs_)->pr = _regval_[17];                                \
    (_regs_)->gbr  = _regval_[18];                              \
    (_regs_)->vbr  = _regval_[19];                              \
    (_regs_)->mach = _regval_[20];                              \
    (_regs_)->macl = _regval_[21];                              \
    (_regs_)->sr = _regval_[22];                                \
                                                                \
    HAL_SET_GDB_REGISTERS_FPU( _regs_, _regval_ );              \
                                                                \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// CPU address space translation macros
#define CYGARC_BUS_ADDRESS(x)       ((CYG_ADDRWORD)(x) & 0x1fffffff)
#define CYGARC_CACHED_ADDRESS(x)    (CYGARC_BUS_ADDRESS(x)|0x80000000)
#define CYGARC_UNCACHED_ADDRESS(x)  (CYGARC_BUS_ADDRESS(x)|0xa0000000)

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_BANK_H
// End of hal_var_bank.h
