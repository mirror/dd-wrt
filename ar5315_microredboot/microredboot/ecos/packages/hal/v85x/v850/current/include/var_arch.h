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
// Contributors: nickg, gthomas
// Date:         1999-02-17
// Purpose:      Define architecture abstractions
// Description:  This file contains any extra or modified definitions for
//               this variant of the architecture.
// Usage:        #include <cyg/hal/var_arch.h>
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/infra/cyg_type.h>

#define CYG_HAL_GDB_REG CYG_WORD32

// Copy a set of registers from a HAL_SavedRegisters structure into a
// GDB ordered array.    
#define HAL_GET_GDB_REGISTERS( _aregval_ , _regs_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        _regval_[_i_] = (_regs_)->d[_i_];                       \
                                                                \
    _regval_[37] = (_regs_)->psw;                               \
    _regval_[36] = (_regs_)->cause;                             \
    _regval_[64] = (_regs_)->pc;                                \
}

// Copy a GDB ordered array into a HAL_SavedRegisters structure.
#define HAL_SET_GDB_REGISTERS( _regs_ , _aregval_ )             \
{                                                               \
    CYG_HAL_GDB_REG *_regval_ = (CYG_HAL_GDB_REG *)(_aregval_); \
    int _i_;                                                    \
                                                                \
    for( _i_ = 0; _i_ < 32; _i_++ )                             \
        (_regs_)->d[_i_] = _regval_[_i_];                       \
                                                                \
    (_regs_)->psw = _regval_[37];                               \
    (_regs_)->cause = _regval_[36];                             \
    (_regs_)->pc = _regval_[64];                                \
}

//--------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_ARCH_H
// End of var_arch.h
