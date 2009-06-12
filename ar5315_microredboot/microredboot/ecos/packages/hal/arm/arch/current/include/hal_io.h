#ifndef CYGONCE_HAL_IO_H
#define CYGONCE_HAL_IO_H

//=============================================================================
//
//      hal_io.h
//
//      HAL device IO register support.
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg, gthomas
// Contributors: Fabrice Gautier
// Date:         1998-09-11
// Purpose:      Define IO register support
// Description:  The macros defined here provide the HAL APIs for handling
//               device IO control registers.
//              
// Usage:
//               #include <cyg/hal/hal_io.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/basetype.h>

//-----------------------------------------------------------------------------
// Include plf_io.h for platforms. Either via var_io.h or directly.
#ifdef CYGBLD_HAL_ARM_VAR_IO_H
#include <cyg/hal/var_io.h>
#else
#include <cyg/hal/plf_io.h>
#endif


//-----------------------------------------------------------------------------
// IO Register address.
// This type is for recording the address of an IO register.

typedef volatile CYG_ADDRWORD HAL_IO_REGISTER;

//-----------------------------------------------------------------------------
// HAL IO macros.
#ifndef HAL_IO_MACROS_DEFINED

//-----------------------------------------------------------------------------
// BYTE Register access.
// Individual and vectorized access to 8 bit registers.

// Little-endian version or big-endian version that doesn't need address munging
#if (CYG_BYTEORDER == CYG_LSBFIRST) || defined(HAL_IO_MACROS_NO_ADDRESS_MUNGING)

#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_BYTE *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile CYG_BYTE *)(_register_)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_BYTE *)(_register_))[_j_];        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_BYTE *)(_register_))[_j_] = (_buf_)[_i_];        \
    CYG_MACRO_END

#define HAL_READ_UINT8_STRING( _register_, _buf_, _count_ )             \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile CYG_BYTE *)(_register_))[_i_];        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_STRING( _register_, _buf_, _count_ )            \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile CYG_BYTE *)(_register_)) = (_buf_)[_i_];             \
    CYG_MACRO_END

#else // Big-endian version

#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_BYTE *)((CYG_ADDRWORD)(_register_)^3)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile CYG_BYTE *)((CYG_ADDRWORD)(_register_)^3)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = _r_[_j_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        _r_[_j_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END

#define HAL_READ_UINT8_STRING( _register_, _buf_, _count_ )             \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0; _i_ < (_count_); _i_++;                               \
        (_buf_)[_i_] = _r_[_i_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_STRING( _register_, _buf_, _count_ )            \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        _r_[_i_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END

#endif // Big-endian

//-----------------------------------------------------------------------------
// 16 bit access.
// Individual and vectorized access to 16 bit registers.
    
// Little-endian version or big-endian version that doesn't need address munging
#if (CYG_BYTEORDER == CYG_LSBFIRST) || defined(HAL_IO_MACROS_NO_ADDRESS_MUNGING)

#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD16 *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile CYG_WORD16 *)(_register_)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_WORD16 *)(_register_))[_j_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_WORD16 *)(_register_))[_j_] = (_buf_)[_i_];      \
    CYG_MACRO_END

#define HAL_READ_UINT16_STRING( _register_, _buf_, _count_)             \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile CYG_WORD16 *)(_register_))[_i_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_STRING( _register_, _buf_, _count_)            \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile CYG_WORD16 *)(_register_))[_i_] = (_buf_)[_i_];      \
    CYG_MACRO_END


#else // Big-endian version

#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD16 *)((CYG_ADDRWORD)(_register_)^3)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile CYG_WORD16 *)((CYG_ADDRWORD)(_register_)^3)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = _r_[_j_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        _r_[_j_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END

#define HAL_READ_UINT16_STRING( _register_, _buf_, _count_)             \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0 = 0; _i_ < (_count_); _i_++)                           \
        (_buf_)[_i_] = _r_[_i_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_STRING( _register_, _buf_, _count_)            \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0 = 0; _i_ < (_count_); _i_++)                           \
        _r_[_i_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END


#endif // Big-endian

//-----------------------------------------------------------------------------
// 32 bit access.
// Individual and vectorized access to 32 bit registers.

// Note: same macros for little- and big-endian systems.
    
#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD32 *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile CYG_WORD32 *)(_register_)) = (_value_))

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_WORD32 *)(_register_))[_j_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_WORD32 *)(_register_))[_j_] = (_buf_)[_i_];      \
    CYG_MACRO_END

#define HAL_READ_UINT32_STRING( _register_, _buf_, _count_)             \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        (_buf_)[_i_] = ((volatile CYG_WORD32 *)(_register_))[_i_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_STRING( _register_, _buf_, _count_)            \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_;                                                    \
    for( _i_ = 0; _i_ < (_count_); _i_++)                               \
        ((volatile CYG_WORD32 *)(_register_))[_i_] = (_buf_)[_i_];      \
    CYG_MACRO_END


#define HAL_IO_MACROS_DEFINED

#endif // !HAL_IO_MACROS_DEFINED

// Enforce a flow "barrier" to prevent optimizing compiler from reordering 
// operations.
#define HAL_IO_BARRIER()

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_IO_H
// End of hal_io.h
