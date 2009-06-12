#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      VRC437X Interrupt and clock support
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
// Contributors: nickg, jskov,
//               gthomas, jlarmour
// Date:         1999-02-16
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the VRC437X board.
//              
// Usage:
//              #include <cyg/hal/plf_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Interrupt controller stuff.

// The first 6 correspond to the interrupt lines in the status/cause regs
#define CYGNUM_HAL_INTERRUPT_VRC437X            0
#define CYGNUM_HAL_INTERRUPT_IPL0               1
#define CYGNUM_HAL_INTERRUPT_IPL1               2
#define CYGNUM_HAL_INTERRUPT_IPL2               3
#define CYGNUM_HAL_INTERRUPT_POWER              4
#define CYGNUM_HAL_INTERRUPT_COMPARE            5
// The next 32 correspond to the interrupt lines in the 4372 interrupt
// controller. These are decoded from the controller when an interrupt
// on any of the IPL[0:2] lines in signalled.
#define CYGNUM_HAL_INTERRUPT_REALTIME_A         6
#define CYGNUM_HAL_INTERRUPT_REALTIME_B         7
#define CYGNUM_HAL_INTERRUPT_DUART              8
#define CYGNUM_HAL_INTERRUPT_TIMER              9
#define CYGNUM_HAL_INTERRUPT_PARALLEL           10
#define CYGNUM_HAL_INTERRUPT_PCI_INTA           11
#define CYGNUM_HAL_INTERRUPT_PCI_INTB           12
#define CYGNUM_HAL_INTERRUPT_PCI_INTC           13
#define CYGNUM_HAL_INTERRUPT_PCI_INTD           14
#define CYGNUM_HAL_INTERRUPT_INT_9              15
#define CYGNUM_HAL_INTERRUPT_INT_10             16
#define CYGNUM_HAL_INTERRUPT_INT_11             17
#define CYGNUM_HAL_INTERRUPT_INT_12             18
#define CYGNUM_HAL_INTERRUPT_INT_13             19
#define CYGNUM_HAL_INTERRUPT_DMA_0              20
#define CYGNUM_HAL_INTERRUPT_DMA_1              21
#define CYGNUM_HAL_INTERRUPT_DMA_2              22
#define CYGNUM_HAL_INTERRUPT_DMA_3              23
#define CYGNUM_HAL_INTERRUPT_TICK_0             24
#define CYGNUM_HAL_INTERRUPT_TICK_1             25
#define CYGNUM_HAL_INTERRUPT_UNUSED_20          26
#define CYGNUM_HAL_INTERRUPT_UNUSED_21          27
#define CYGNUM_HAL_INTERRUPT_IO_TIMEOUT         28
#define CYGNUM_HAL_INTERRUPT_PCI_PERR           29
#define CYGNUM_HAL_INTERRUPT_PCI_SERR           30
#define CYGNUM_HAL_INTERRUPT_PCI_SIG_TA         31
#define CYGNUM_HAL_INTERRUPT_PCI_REC_TA         32
#define CYGNUM_HAL_INTERRUPT_PCI_SIG_MA         33
#define CYGNUM_HAL_INTERRUPT_PCI_ADD            34
#define CYGNUM_HAL_INTERRUPT_PCI_RET_ERR        35
#define CYGNUM_HAL_INTERRUPT_UNUSED_30          36
#define CYGNUM_HAL_INTERRUPT_UNUSED_31          37

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     37
#define CYGNUM_HAL_ISR_COUNT                   38

// The vector used by the Real time clock. The default here is to use
// interrupt 5, which is connected to the counter/comparator registers
// in many MIPS variants.

#ifndef CYGNUM_HAL_INTERRUPT_RTC
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_COMPARE
#endif

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

//--------------------------------------------------------------------------
// Vector translation.
// For chained interrupts we only have a single vector though which all
// are passed. For unchained interrupts we have a vector per interrupt.
// Vector 0 has a special catcher ISR for spurious interrupts from the VRC437X
// and vectors 1-3 are springboards, so we chain through vector 4.

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

#define HAL_TRANSLATE_VECTOR(_vector_,_index_)  \
{                                               \
    if ((_vector_)==0)                          \
        (_index_) = 0;                          \
    else                                        \
        (_index_) = 4;                          \
}   
#endif

//--------------------------------------------------------------------------
// controller access code

#define CYGHWR_HAL_MIPS_VRC4372_BASE            0xbc000000
#define CYGHWR_HAL_MIPS_VRC4372_INTC_POL        (CYGHWR_HAL_MIPS_VRC4372_BASE+0x200)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_TRIG       (CYGHWR_HAL_MIPS_VRC4372_BASE+0x204)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_PINS       (CYGHWR_HAL_MIPS_VRC4372_BASE+0x208)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_MASK0      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x20c)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_STAT0      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x210)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_MASK1      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x214)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_STAT1      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x218)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_MASK2      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x21c)
#define CYGHWR_HAL_MIPS_VRC4372_INTC_STAT2      (CYGHWR_HAL_MIPS_VRC4372_BASE+0x220)

#define CYGHWR_HAL_MIPS_VRC4372_INTC_MASK_OFF   8

// Array which stores the configured priority levels for the configured
// interrupts.
externC volatile CYG_BYTE cyg_hal_interrupt_level[CYGNUM_HAL_ISR_COUNT];

#define HAL_INTERRUPT_MASK( _vector_ )                                          \
CYG_MACRO_START                                                                 \
    if( _vector_ <= CYGNUM_HAL_INTERRUPT_COMPARE )                              \
    {                                                                           \
        asm volatile (                                                          \
            "mfc0   $3,$12\n"                                                   \
            "la     $2,0x00000400\n"                                            \
            "sllv   $2,$2,%0\n"                                                 \
            "nor    $2,$2,$0\n"                                                 \
            "and    $3,$3,$2\n"                                                 \
            "mtc0   $3,$12\n"                                                   \
            "nop; nop; nop\n"                                                   \
            :                                                                   \
            : "r"(_vector_)                                                     \
            : "$2", "$3"                                                        \
            );                                                                  \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        CYG_WORD32 _mask_;                                                      \
        CYG_BYTE _shift_;                                                       \
        HAL_IO_REGISTER _maskreg_ = CYGHWR_HAL_MIPS_VRC4372_INTC_MASK0 +        \
                                    cyg_hal_interrupt_level[_vector_] *         \
                                    CYGHWR_HAL_MIPS_VRC4372_INTC_MASK_OFF;      \
        HAL_READ_UINT32( _maskreg_, _mask_ );                                   \
        _shift_ = _vector_-CYGNUM_HAL_INTERRUPT_REALTIME_A;                     \
        _mask_ &= ~(1<<_shift_);                                                \
        HAL_WRITE_UINT32( _maskreg_, _mask_ );                                  \
    }                                                                           \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                                        \
CYG_MACRO_START                                                                 \
    if( _vector_ <= CYGNUM_HAL_INTERRUPT_COMPARE )                              \
    {                                                                           \
        asm volatile (                                                          \
            "mfc0   $3,$12\n"                                                   \
            "la     $2,0x00000400\n"                                            \
            "sllv   $2,$2,%0\n"                                                 \
            "or     $3,$3,$2\n"                                                 \
            "mtc0   $3,$12\n"                                                   \
            "nop; nop; nop\n"                                                   \
            :                                                                   \
            : "r"(_vector_)                                                     \
            : "$2", "$3"                                                        \
            );                                                                  \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        CYG_WORD32 _mask_;                                                      \
        CYG_BYTE _shift_;                                                       \
        HAL_IO_REGISTER _maskreg_ = CYGHWR_HAL_MIPS_VRC4372_INTC_MASK0 +        \
                                    cyg_hal_interrupt_level[_vector_] *         \
                                    CYGHWR_HAL_MIPS_VRC4372_INTC_MASK_OFF;      \
        HAL_READ_UINT32( _maskreg_, _mask_ );                                   \
        _shift_ = _vector_-CYGNUM_HAL_INTERRUPT_REALTIME_A;                     \
        _mask_ |= 1<<_shift_;                                                   \
        HAL_WRITE_UINT32( _maskreg_, _mask_ );                                  \
    }                                                                           \
CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
CYG_MACRO_START                                                         \
    CYG_WORD32 _srvector_ = _vector_;                                   \
    if( _vector_ > CYGNUM_HAL_INTERRUPT_COMPARE )                       \
    {                                                                   \
        CYG_WORD32 _stat_;                                              \
        CYG_BYTE _shift_;                                               \
        HAL_IO_REGISTER _statreg_ = CYGHWR_HAL_MIPS_VRC4372_INTC_STAT0 +\
            cyg_hal_interrupt_level[_vector_] *                         \
            CYGHWR_HAL_MIPS_VRC4372_INTC_MASK_OFF;                      \
        HAL_READ_UINT32( _statreg_, _stat_ );                           \
        _shift_ = _vector_-CYGNUM_HAL_INTERRUPT_REALTIME_A;             \
        _stat_ &= ~(1<<_shift_);                                        \
        HAL_WRITE_UINT32( _statreg_, _stat_ );                          \
        _srvector_ = cyg_hal_interrupt_level[_vector_] +                \
            CYGNUM_HAL_INTERRUPT_IPL0;                                  \
    }                                                                   \
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

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )                      \
CYG_MACRO_START                                                                 \
    if( (_vector_ >= CYGNUM_HAL_INTERRUPT_REALTIME_A) &&                        \
        (_vector_ <= CYGNUM_HAL_INTERRUPT_INT_13))                              \
    {                                                                           \
        CYG_WORD32 mask = 1<<(_vector_-CYGNUM_HAL_INTERRUPT_REALTIME_A);        \
        CYG_WORD32 pol;                                                         \
        CYG_WORD32 trig;                                                        \
        HAL_READ_UINT32( CYGHWR_HAL_MIPS_VRC4372_INTC_POL, pol );               \
        HAL_READ_UINT32( CYGHWR_HAL_MIPS_VRC4372_INTC_TRIG, trig );             \
        if( _level_ )                                                           \
        {                                                                       \
            pol &= ~mask;                                                       \
            if( _up_ ) trig |= mask;                                            \
            else trig &= ~mask;                                                 \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            pol |= mask;                                                        \
            if( !(_up_) ) trig |= mask;                                         \
            else trig &= ~mask;                                                 \
        }                                                                       \
        HAL_WRITE_UINT32( CYGHWR_HAL_MIPS_VRC4372_INTC_POL, pol );              \
        HAL_WRITE_UINT32( CYGHWR_HAL_MIPS_VRC4372_INTC_TRIG, trig );            \
    }                                                                           \
CYG_MACRO_END

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )    \
CYG_MACRO_START                                         \
    if( _vector_ > CYGNUM_HAL_INTERRUPT_COMPARE )       \
    {                                                   \
        cyg_uint32 _ilevel_ = _level_;                  \
        while( _ilevel_ > 2 ) _ilevel_ -= 2;            \
        cyg_hal_interrupt_level[_vector_] = _ilevel_;   \
    }                                                   \
CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

//--------------------------------------------------------------------------
// Control-C support.

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_DUART

externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_CTRLC_ISR hal_ctrlc_isr

#endif

//----------------------------------------------------------------------------
// Reset.

#define HAL_PLATFORM_RESET()            CYG_EMPTY_STATEMENT

#define HAL_PLATFORM_RESET_ENTRY        0xbfc00000

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
