#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      Malta Interrupt and clock support
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
//               gthomas, jlarmour, dmoseley
// Date:         2001-03-20
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the Malta board.
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

// First an assembly safe part

//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

// These are decoded via the IP bits of the cause
// register when an external interrupt is delivered.

#define CYGNUM_HAL_INTERRUPT_SOUTH_BRIDGE_INTR   0
#define CYGNUM_HAL_INTERRUPT_SOUTH_BRIDGE_SMI    1
#define CYGNUM_HAL_INTERRUPT_CBUS_UART           2
#define CYGNUM_HAL_INTERRUPT_COREHI              3
#define CYGNUM_HAL_INTERRUPT_CORELO              4
#define CYGNUM_HAL_INTERRUPT_COMPARE             5

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_BASE       6

#define CYGNUM_HAL_INTERRUPT_CTRL1_BASE          6
#define CYGNUM_HAL_INTERRUPT_TIMER               6
#define CYGNUM_HAL_INTERRUPT_KEYBOARD            7
#define CYGNUM_HAL_INTERRUPT_CASCADE             8 // this is where int ctrl2 is cascaded
#define CYGNUM_HAL_INTERRUPT_TTY1                9
#define CYGNUM_HAL_INTERRUPT_TTY0               10
#define CYGNUM_HAL_INTERRUPT_11                 11
#define CYGNUM_HAL_INTERRUPT_FLOPPY             12
#define CYGNUM_HAL_INTERRUPT_PARALLEL           13

#define CYGNUM_HAL_INTERRUPT_CTRL2_BASE         14
#define CYGNUM_HAL_INTERRUPT_REAL_TIME_CLOCK    14
#define CYGNUM_HAL_INTERRUPT_I2C                15
#define CYGNUM_HAL_INTERRUPT_PCI_AB             16
#define CYGNUM_HAL_INTERRUPT_PCI_CD             17
#define CYGNUM_HAL_INTERRUPT_MOUSE              18
#define CYGNUM_HAL_INTERRUPT_19                 19
#define CYGNUM_HAL_INTERRUPT_IDE_PRIMARY        20
#define CYGNUM_HAL_INTERRUPT_IDE_SECONDARY      21

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

// This overlaps with CYGNUM_HAL_INTERRUPT_EXTERNAL_BASE above but it
// doesn't matter. It's only used by the HAL to access the special
// chaining entry in the ISR tables.  All other attempted access to
// the ISR table will be redirected to this entry (courtesy of
// HAL_TRANSLATE_VECTOR). The other vector definitions are still
// valid, but only for enable/disable/config etc. (i.e., in chaining
// mode they have associated entries in the ISR tables).
#define CYGNUM_HAL_INTERRUPT_CHAINING           6

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
    (_index_) = CYGNUM_HAL_INTERRUPT_CHAINING

// Min/Max ISR numbers
#define CYGNUM_HAL_ISR_MIN                 0
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_CHAINING

#else

// Min/Max ISR numbers
#define CYGNUM_HAL_ISR_MIN                 0
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_IDE_SECONDARY
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

#define CYGNUM_HAL_ISR_COUNT               (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC           CYGNUM_HAL_INTERRUPT_COMPARE

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif


//--------------------------------------------------------------------------
#ifndef __ASSEMBLER__

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/plf_io.h>

//--------------------------------------------------------------------------
// Interrupt controller access.

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

// Array which stores the configured priority levels for the configured
// interrupts.
externC volatile CYG_BYTE hal_interrupt_level[CYGNUM_HAL_ISR_COUNT];

#define HAL_INTERRUPT_MASK( _vector_ )                       \
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
    else if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL2_BASE)                 \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL2_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_SLAVE_OCW1, _mask_ );                      \
        _mask_ |= (1<<_shift_);                                             \
        HAL_WRITE_UINT8(HAL_PIIX4_SLAVE_OCW1, _mask_ );                     \
    }                                                                       \
    else /* CTRL1 */                                                        \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL1_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_MASTER_OCW1, _mask_ );                     \
        _mask_ |= (1<<_shift_);                                             \
        HAL_WRITE_UINT8(HAL_PIIX4_MASTER_OCW1, _mask_ );                    \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                     \
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
    else if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL2_BASE)                 \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL2_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_SLAVE_OCW1, _mask_ );                      \
        _mask_ &= ~(1<<_shift_);                                            \
        HAL_WRITE_UINT8(HAL_PIIX4_SLAVE_OCW1, _mask_ );                     \
    }                                                                       \
    else /* CTRL1 */                                                        \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL1_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_MASTER_OCW1, _mask_ );                     \
        _mask_ &= ~(1<<_shift_);                                            \
        HAL_WRITE_UINT8(HAL_PIIX4_MASTER_OCW1, _mask_ );                    \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                   \
    CYG_MACRO_START                                             \
    cyg_uint32 _srvector_ = _vector_;                           \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL2_BASE)          \
    {                                                           \
        HAL_WRITE_UINT8(HAL_PIIX4_SLAVE_OCW3, 0x20 );           \
    }                                                           \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL1_BASE)          \
    {                                                           \
        HAL_WRITE_UINT8(HAL_PIIX4_MASTER_OCW3, 0x20 );          \
    }                                                           \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_EXTERNAL_BASE) {     \
        _srvector_ = CYGNUM_HAL_INTERRUPT_SOUTH_BRIDGE_INTR;    \
    }                                                           \
    asm volatile (                                              \
        "mfc0   $3,$13\n"                                       \
        "la     $2,0x00000400\n"                                \
        "sllv   $2,$2,%0\n"                                     \
        "nor    $2,$2,$0\n"                                     \
        "and    $3,$3,$2\n"                                     \
        "mtc0   $3,$13\n"                                       \
        "nop; nop; nop\n"                                       \
        :                                                       \
        : "r"(_srvector_)                                       \
        : "$2", "$3"                                            \
        );                                                      \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )                  \
    CYG_MACRO_START                                                         \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL2_BASE)                      \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL2_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_ELCR2, _mask_ );                           \
        _mask_ &= ~(1<<_shift_);                                            \
        if (_level_) _mask_ |= (1<<_shift_);                                \
        _mask_ &= HAL_PIIX4_ELCR2_MASK;                                     \
        HAL_WRITE_UINT8(HAL_PIIX4_ELCR2, _mask_ );                          \
    }                                                                       \
    else if ((_vector_) >= CYGNUM_HAL_INTERRUPT_CTRL1_BASE)                 \
    {                                                                       \
        cyg_uint8 _mask_;                                                   \
        cyg_uint32 _shift_ = (_vector_)-CYGNUM_HAL_INTERRUPT_CTRL1_BASE;    \
        HAL_READ_UINT8(HAL_PIIX4_ELCR1, _mask_ );                           \
        _mask_ &= ~(1<<_shift_);                                            \
        if (_level_) _mask_ |= (1<<_shift_);                                \
        _mask_ &= HAL_PIIX4_ELCR1_MASK;                                     \
        HAL_WRITE_UINT8(HAL_PIIX4_ELCR1, _mask_ );                          \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#endif


//--------------------------------------------------------------------------
// Control-C support.

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_SER

externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

# define HAL_CTRLC_ISR hal_ctrlc_isr

#endif


//----------------------------------------------------------------------------
// Reset.
#ifndef CYGHWR_HAL_RESET_DEFINED
extern void hal_malta_reset( void );
#define CYGHWR_HAL_RESET_DEFINED
#define HAL_PLATFORM_RESET()             hal_malta_reset()

#define HAL_PLATFORM_RESET_ENTRY 0xbfc00000

#endif // CYGHWR_HAL_RESET_DEFINED

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
