#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H

//==========================================================================
//
//      imp_intr.h
//
//      RM7000 Interrupt and clock support
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2000-05-09
// Purpose:      RM7000 Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for variants of the RM7000 MIPS
//               architecture.
//              
// Usage:
//              #include <cyg/hal/var_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/plf_intr.h>

//--------------------------------------------------------------------------
// Interrupt controller information

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED
#define HAL_INTERRUPT_MASK( _vector_ )                                      \
    CYG_MACRO_START                                                         \
    if( (_vector_) <= CYGNUM_HAL_INTERRUPT_COMPARE )                        \
    {                                                                       \
        asm volatile (                                                      \
            "mfc0   $3,$12\n"                                               \
            "la     $2,0x00000400\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "nor    $2,$2,$0\n"                                             \
            "and    $3,$3,$2\n"                                             \
            "mtc0   $3,$12\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"(_vector_)                                                 \
            : "$2", "$3"                                                    \
            );                                                              \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* int 6:9 are masked in the Interrupt Control register */          \
        asm volatile (                                                      \
            "cfc0   $3,$20\n"                                               \
            "la     $2,0x00000004\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "nor    $2,$2,$0\n"                                             \
            "and    $3,$3,$2\n"                                             \
            "ctc0   $3,$20\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"((_vector_) )				                    \
            : "$2", "$3"                                                    \
            );                                                              \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                                    \
    CYG_MACRO_START                                                         \
    if( (_vector_) <= CYGNUM_HAL_INTERRUPT_COMPARE )                        \
    {                                                                       \
        asm volatile (                                                      \
            "mfc0   $3,$12\n"                                               \
            "la     $2,0x00000400\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "or     $3,$3,$2\n"                                             \
            "mtc0   $3,$12\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"(_vector_)                                                 \
            : "$2", "$3"                                                    \
            );                                                              \
    }                                                                       \
    else								    \
    {                                                                       \
        /* int 6:9 are masked in the Interrupt Control register */          \
        asm volatile (                                                      \
            "cfc0   $3,$20\n"                                               \
            "la     $2,0x00000004\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "or     $3,$3,$2\n"                                             \
            "ctc0   $3,$20\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"((_vector_) )                                              \
            : "$2", "$3"                                                    \
            );                                                              \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
    CYG_MACRO_START                                                     \
    /* All 10 interrupts have pending bits in the cause register */     \
    cyg_uint32 _srvector_ = _vector_;                                   \
    asm volatile (                                                      \
        "mfc0   $3,$13\n"                                               \
        "la     $2,0x00000400\n"                                        \
        "sllv   $2,$2,%0\n"                                             \
        "nor    $2,$2,$0\n"                                             \
        "and    $3,$3,$2\n"                                             \
        "mtc0   $3,$13\n"                                               \
        "nop; nop; nop\n"                                               \
        :                                                               \
        : "r"(_srvector_)                                               \
        : "$2", "$3"                                                    \
        );                                                              \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif

//--------------------------------------------------------------------------
// Interrupt vectors.

// Vectors and handling of these are defined in platform HALs since the
// CPU itself does not have a builtin interrupt controller.

//--------------------------------------------------------------------------
// Clock control

// This is handled by the default code

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VAR_INTR_H
// End of var_intr.h
