#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      REF4955 Interrupt and clock support
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
// Contributors: jskov, nickg
// Date:         2000-05-09
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the REF4955 board.
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
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED


// The first 6 correspond to the interrupt lines in the status/cause regs
#define CYGNUM_HAL_INTERRUPT_V320USC_INT0       0
#define CYGNUM_HAL_INTERRUPT_V320USC_INT1       1
#define CYGNUM_HAL_INTERRUPT_EXT                2
#define CYGNUM_HAL_INTERRUPT_LAN                3
#define CYGNUM_HAL_INTERRUPT_IO                 4
#define CYGNUM_HAL_INTERRUPT_COMPARE            5


#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

// This overlaps with LB_MBI below but it doesn't matter. It's only used
// by the HAL to access the special chaining entry in the ISR tables.
// All other attempted access to the ISR table will be redirected to this
// entry (curtsey of HAL_TRANSLATE_VECTOR). The other vector definitions
// are still valid, but only for enable/disable/config etc. (i.e., in
// chaining mode they have associated entries in the ISR tables).
#define CYGNUM_HAL_INTERRUPT_CHAINING           6

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
    (_index_) = CYGNUM_HAL_INTERRUPT_CHAINING

#endif


// The next 32 correspond to the interrupt lines in the V320USC's interrupt
// controller. These are decoded from the controller when an interrupt
// enters via CYGNUM_HAL_INTERRUPT_V320USC_INT0.
#define CYGNUM_HAL_INTERRUPT_INTC_V320USC_base  6 
#define CYGNUM_HAL_INTERRUPT_LB_MBI             6 
#define CYGNUM_HAL_INTERRUPT_PCI_MBI            7 
#define CYGNUM_HAL_INTERRUPT_RESERVED_2         8 
#define CYGNUM_HAL_INTERRUPT_I2O_OP_NE          9 
#define CYGNUM_HAL_INTERRUPT_I2O_IF_NF          10
#define CYGNUM_HAL_INTERRUPT_I2O_IP_NE          11
#define CYGNUM_HAL_INTERRUPT_I2O_OP_NF          12
#define CYGNUM_HAL_INTERRUPT_I2O_OF_NE          13
#define CYGNUM_HAL_INTERRUPT_RESERVED_8         14
#define CYGNUM_HAL_INTERRUPT_RESERVED_9         15
#define CYGNUM_HAL_INTERRUPT_RESERVED_10        16
#define CYGNUM_HAL_INTERRUPT_RESERVED_11        17
#define CYGNUM_HAL_INTERRUPT_TIMER0             18
#define CYGNUM_HAL_INTERRUPT_TIMER1             19
#define CYGNUM_HAL_INTERRUPT_RESERVED_14        20     
#define CYGNUM_HAL_INTERRUPT_ENUM               21
#define CYGNUM_HAL_INTERRUPT_DMA0               22
#define CYGNUM_HAL_INTERRUPT_DMA1               23
#define CYGNUM_HAL_INTERRUPT_RESERVED_18        24
#define CYGNUM_HAL_INTERRUPT_RESERVED_19        25
#define CYGNUM_HAL_INTERRUPT_PWR_STATE          26
#define CYGNUM_HAL_INTERRUPT_HBI                27
#define CYGNUM_HAL_INTERRUPT_WDI                28
#define CYGNUM_HAL_INTERRUPT_BWI                29
#define CYGNUM_HAL_INTERRUPT_PSLAVE_PI          30
#define CYGNUM_HAL_INTERRUPT_PMASTER_PI         31
#define CYGNUM_HAL_INTERRUPT_PCI_T_ABORT        32
#define CYGNUM_HAL_INTERRUPT_PCI_M_ABORT        33
#define CYGNUM_HAL_INTERRUPT_DRAM_PI            34
#define CYGNUM_HAL_INTERRUPT_RESERVED_29        35
#define CYGNUM_HAL_INTERRUPT_DI0                36
#define CYGNUM_HAL_INTERRUPT_DI1                37

// The next 6 correspond to the interrupt lines specific to the PCI
// connector (decoded from CYGNUM_HAL_INTERRUPT_V320USC_INT1)
#define CYGNUM_HAL_INTERRUPT_INTC_PCI_base      38
#define CYGNUM_HAL_INTERRUPT_SERR               38
#define CYGNUM_HAL_INTERRUPT_PERR               39
#define CYGNUM_HAL_INTERRUPT_INTD               40
#define CYGNUM_HAL_INTERRUPT_INTC               41
#define CYGNUM_HAL_INTERRUPT_INTB               42
#define CYGNUM_HAL_INTERRUPT_INTA               43

// The next 5 correspond to the interrupt lines specific to the REF4955
// board (decoded from CYGNUM_HAL_INTERRUPT_IO)
#define CYGNUM_HAL_INTERRUPT_INTC_IO_base       44
#define CYGNUM_HAL_INTERRUPT_SOFTWARE           44
#define CYGNUM_HAL_INTERRUPT_INT_SWITCH         45
#define CYGNUM_HAL_INTERRUPT_PARALLEL           46
#define CYGNUM_HAL_INTERRUPT_DEBUG_UART         47
#define CYGNUM_HAL_INTERRUPT_USER_UART          48


// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     CYGNUM_HAL_INTERRUPT_V320USC_INT0
#define CYGNUM_HAL_ISR_MAX                     CYGNUM_HAL_INTERRUPT_USER_UART
#define CYGNUM_HAL_ISR_COUNT                   (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_COMPARE

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif

//--------------------------------------------------------------------------
// Interrupt controler information

// V320USC 
#define CYGARC_REG_INT_STAT   0xb80000ec

#define CYGARC_REG_INT_CFG0   0xb80000e0
#define CYGARC_REG_INT_CFG1   0xb80000e4
#define CYGARC_REG_INT_CFG2   0xb80000e8
#define CYGARC_REG_INT_CFG3   0xb8000158

#define CYGARC_REG_INT_CFG_INT0 0x00000100
#define CYGARC_REG_INT_CFG_INT1 0x00000200
#define CYGARC_REG_INT_CFG_INT2 0x00000400
#define CYGARC_REG_INT_CFG_INT3 0x00000800


// FPGA
#define CYGARC_REG_PCI_STAT   0xb5300000
#define CYGARC_REG_PCI_MASK   0xb5300030

#define CYGARC_REG_IO_STAT    0xb5300010
#define CYGARC_REG_IO_MASK    0xb5300040


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
    else if ((_vector_) >= CYGNUM_HAL_INTERRUPT_INTC_IO_base)               \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_IO_base;  \
        HAL_READ_UINT8(CYGARC_REG_IO_MASK, _mask_ );                        \
        _mask_ &= ~(1<<_shift_);                                            \
        HAL_WRITE_UINT8(CYGARC_REG_IO_MASK, _mask_ );                       \
    }                                                                       \
    else if (_vector_ >= CYGNUM_HAL_INTERRUPT_INTC_PCI_base)                \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_PCI_base; \
        HAL_READ_UINT8(CYGARC_REG_PCI_MASK, _mask_ );                       \
        _mask_ &= ~(1<<_shift_);                                            \
        HAL_WRITE_UINT8(CYGARC_REG_PCI_MASK, _mask_ );                      \
    } else { /* V320USC */                                                  \
        cyg_uint32 _mask_;                                                  \
        cyg_uint32 _shift_ =                                                \
            (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_V320USC_base;              \
        HAL_READ_UINT32(CYGARC_REG_INT_CFG0, _mask_ );                      \
        _mask_ &= !(1<<_shift_);                                            \
        HAL_WRITE_UINT32(CYGARC_REG_INT_CFG0, _mask_ );                     \
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
    else if ((_vector_) >= CYGNUM_HAL_INTERRUPT_INTC_IO_base)               \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_IO_base;  \
        HAL_READ_UINT8(CYGARC_REG_IO_MASK, _mask_ );                        \
        _mask_ |= (1<<_shift_);                                             \
        HAL_WRITE_UINT8(CYGARC_REG_IO_MASK, _mask_ );                       \
    }                                                                       \
    else if (_vector_ >= CYGNUM_HAL_INTERRUPT_INTC_PCI_base)                \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_PCI_base; \
        HAL_READ_UINT8(CYGARC_REG_PCI_MASK, _mask_ );                       \
        _mask_ |= (1<<_shift_);                                             \
        HAL_WRITE_UINT8(CYGARC_REG_PCI_MASK, _mask_ );                      \
    } else { /* V320USC */                                                  \
        cyg_uint32 _mask_;                                                  \
        cyg_uint32 _shift_ =                                                \
            (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_V320USC_base;              \
        HAL_READ_UINT32(CYGARC_REG_INT_CFG0, _mask_ );                      \
        _mask_ |= (1<<_shift_);                                             \
        HAL_WRITE_UINT32(CYGARC_REG_INT_CFG0, _mask_ );                     \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
    CYG_MACRO_START                                                     \
    cyg_uint32 _srvector_ = _vector_;                                   \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_INTC_IO_base) {              \
        _srvector_ = CYGNUM_HAL_INTERRUPT_IO;                           \
    } else if (_vector_ >= CYGNUM_HAL_INTERRUPT_INTC_PCI_base) {        \
        _srvector_ = CYGNUM_HAL_INTERRUPT_V320USC_INT1;                 \
    } else if (_vector_ >= CYGNUM_HAL_INTERRUPT_INTC_V320USC_base) {    \
        cyg_uint32 _mask_;                                              \
        cyg_uint32 _shift_ =                                            \
            (_vector_)-CYGNUM_HAL_INTERRUPT_INTC_V320USC_base;          \
        _mask_ = (1<<_shift_);                                          \
        HAL_WRITE_UINT32(CYGARC_REG_INT_STAT, _mask_ );                 \
        _srvector_ = CYGNUM_HAL_INTERRUPT_V320USC_INT0;                 \
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

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED


//----------------------------------------------------------------------------
// Reset.
#define CYGARC_REG_BOARD_RESET 0xb5400000

#define HAL_PLATFORM_RESET() HAL_WRITE_UINT8(CYGARC_REG_BOARD_RESET,0)

#define HAL_PLATFORM_RESET_ENTRY 0xbfc00000

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
