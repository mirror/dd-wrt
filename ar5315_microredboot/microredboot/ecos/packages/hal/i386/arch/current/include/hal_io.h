#ifndef CYGONCE_HAL_HAL_IO_H
#define CYGONCE_HAL_HAL_IO_H

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
// Author(s):    nickg
// Contributors: Fabrice Gautier
// Date:         1998-02-17
// Purpose:      Define IO register support
// Description:  The macros defined here provide the HAL APIs for handling
//               device IO control registers.
//              
// Usage:
//               #include <cyg/hal/hal_io.h>
//               ...
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/plf_io.h>


//-----------------------------------------------------------------------------
// IO Register address.
// This type is for recording the address of an IO register.

typedef volatile CYG_ADDRWORD HAL_IO_REGISTER;

//-----------------------------------------------------------------------------
// BYTE Register access.
// Individual and vectorized access to 8 bit registers.

#define HAL_READ_UINT8( _register_, _value_ )   \
CYG_MACRO_START                                 \
{                                               \
    asm volatile ( "xor %%eax,%%eax ;"          \
                   "inb %%dx, %%al"             \
                   : "=a" (_value_)             \
                   :  "d"(_register_)           \
        );                                      \
}                                               \
CYG_MACRO_END

#define HAL_WRITE_UINT8( _register_, _value_ )          \
CYG_MACRO_START                                         \
{                                                       \
    asm volatile ( "outb %%al,%%dx"                     \
                   :                                    \
                   : "a" (_value_), "d"(_register_) \
        );                                              \
}                                                       \
CYG_MACRO_END

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
CYG_MACRO_START                                                         \
    ! Not supported MACRO !                                             \
CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
CYG_MACRO_START                                                         \
    ! Not supported MACRO !                                             \
CYG_MACRO_END

#define HAL_READ_UINT8_STRING( _register_, _buf_, _count_)      \
CYG_MACRO_START                                                 \
    asm volatile ( "insb"                                       \
                   :                                            \
                   : "c" (_count_), "d"(_register_), "D"(_buf_) \
        );                                                      \
CYG_MACRO_END

#define HAL_WRITE_UINT8_STRING( _register_, _buf_, _count_)     \
CYG_MACRO_START                                                 \
    asm volatile ( "outsb"                                      \
                   :                                            \
                   : "c" (_count_), "d"(_register_), "S"(_buf_) \
        );                                                      \
CYG_MACRO_END


//-----------------------------------------------------------------------------
// 16 bit access.
// Individual and vectorized access to 16 bit registers.
    
#define HAL_READ_UINT16( _register_, _value_ )  \
CYG_MACRO_START                                 \
{                                               \
    asm volatile ( "xor %%eax,%%eax ;"          \
                   "inw %%dx, %%ax"             \
                   : "=a" (_value_)             \
                   :  "d"(_register_)           \
        );                                      \
}                                               \
CYG_MACRO_END

#define HAL_WRITE_UINT16( _register_, _value_ )         \
CYG_MACRO_START                                         \
{                                                       \
    asm volatile ( "outw %%ax,%%dx"                     \
                   :                                    \
                   : "a" (_value_), "d"(_register_) \
        );                                              \
}                                                       \
CYG_MACRO_END

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    ! Not supported MACRO !                                             \
    CYG_MACRO_END


#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    ! Not supported MACRO !                                             \
    CYG_MACRO_END

#define HAL_READ_UINT16_STRING( _register_, _buf_, _count_)             \
    CYG_MACRO_START                                                     \
    asm volatile ( "insw"                                               \
                   :                                                    \
                   : "c" (_count_), "d"(_register_), "D"(_buf_)         \
        );                                                              \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_STRING( _register_, _buf_, _count_)            \
    CYG_MACRO_START                                                     \
    asm volatile ( "outsw"                                              \
                   :                                                    \
                   : "c" (_count_), "d"(_register_), "S"(_buf_)         \
        );                                                              \
    CYG_MACRO_END



//-----------------------------------------------------------------------------
// 32 bit access.
// Individual and vectorized access to 32 bit registers.
    
#define HAL_READ_UINT32( _register_, _value_ )  \
CYG_MACRO_START                                 \
{                                               \
    asm volatile ( "inl %%dx, %%eax"            \
                   : "=a" (_value_)             \
                   :  "d"(_register_)           \
        );                                      \
}                                               \
CYG_MACRO_END

#define HAL_WRITE_UINT32( _register_, _value_ )         \
CYG_MACRO_START                                         \
{                                                       \
    asm volatile ( "outl %%eax,%%dx"                    \
                   :                                    \
                   : "a" (_value_), "d"(_register_)     \
        );                                              \
}                                                       \
CYG_MACRO_END

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    ! Not supported MACRO !                                             \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    ! Not supported MACRO !                                             \
    CYG_MACRO_END

#define HAL_READ_UINT32_STRING( _register_, _buf_, _count_)             \
    CYG_MACRO_START                                                     \
    asm volatile ( "insl"                                               \
                   :                                                    \
                   : "c" (_count_), "d"(_register_), "D"(_buf_)         \
        );                                                              \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_STRING( _register_, _buf_, _count_)            \
    CYG_MACRO_START                                                     \
    asm volatile ( "outsl"                                              \
                   :                                                    \
                   : "c" (_count_), "d"(_register_), "S"(_buf_)         \
        );                                                              \
    CYG_MACRO_END


//-----------------------------------------------------------------------------

// Macros for acessing shared memory structures

#define HAL_READMEM_UINT8(   _reg_, _val_ ) ((_val_) = *((volatile CYG_BYTE *)(_reg_)))
#define HAL_WRITEMEM_UINT8(  _reg_, _val_ ) (*((volatile CYG_BYTE *)(_reg_)) = (_val_))

#define HAL_READMEM_UINT16(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define HAL_WRITEMEM_UINT16( _reg_, _val_ ) (*((volatile CYG_WORD16 *)(_reg_)) = (_val_))

#define HAL_READMEM_UINT32(  _reg_, _val_ ) ((_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define HAL_WRITEMEM_UINT32( _reg_, _val_ ) (*((volatile CYG_WORD32 *)(_reg_)) = (_val_))

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_IO_H
// End of hal_io.h
