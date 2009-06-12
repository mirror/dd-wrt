#ifndef CYGONCE_HAL_PCMB_INTR_H
#define CYGONCE_HAL_PCMB_INTR_H

//==========================================================================
//
//      pcmb_intr.h
//
//      i386/pc Interrupt and clock support
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
// Author(s):    proven
// Contributors: proven, jskov, pjo
// Date:         1999-10-15
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock on a standard PC Motherboard.
//               This file contains info about interrupts and
//               peripherals that are common on all PCs; for example,
//               the clock always activates irq 0 and would therefore
//               be listed here.
//              
// Usage:
//               #include <cyg/hal/pcmb_intr.h>
//               ...
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_i386.h>
#include <pkgconf/hal_i386_pcmb.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/plf_intr.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_smp.h>

//--------------------------------------------------------------------------
// Interrupt vectors.

#define CYGNUM_HAL_INTERRUPT_IRQ0                32
#define CYGNUM_HAL_INTERRUPT_IRQ1                33
#define CYGNUM_HAL_INTERRUPT_IRQ2                34
#define CYGNUM_HAL_INTERRUPT_IRQ3                35
#define CYGNUM_HAL_INTERRUPT_IRQ4                36
#define CYGNUM_HAL_INTERRUPT_IRQ5                37
#define CYGNUM_HAL_INTERRUPT_IRQ6                38
#define CYGNUM_HAL_INTERRUPT_IRQ7                39
#define CYGNUM_HAL_INTERRUPT_IRQ8                40
#define CYGNUM_HAL_INTERRUPT_IRQ9                41
#define CYGNUM_HAL_INTERRUPT_IRQ10               42
#define CYGNUM_HAL_INTERRUPT_IRQ11               43
#define CYGNUM_HAL_INTERRUPT_IRQ12               44
#define CYGNUM_HAL_INTERRUPT_IRQ13               45
#define CYGNUM_HAL_INTERRUPT_IRQ14               46
#define CYGNUM_HAL_INTERRUPT_IRQ15               47

#define CYGNUM_HAL_INTERRUPT_TIMER               32
#define CYGNUM_HAL_INTERRUPT_KEYBOARD            33
#define CYGNUM_HAL_INTERRUPT_SLAVE8259           34
#define CYGNUM_HAL_INTERRUPT_COM2                35
#define CYGNUM_HAL_INTERRUPT_COM1                36
#define CYGNUM_HAL_INTERRUPT_LPT2                37
#define CYGNUM_HAL_INTERRUPT_FDD                 38
#define CYGNUM_HAL_INTERRUPT_LPT1                39
#define CYGNUM_HAL_INTERRUPT_WALLCLOCK           40
#define CYGNUM_HAL_INTERRUPT_SLAVE8259REDIR      41
#define CYGNUM_HAL_INTERRUPT_COPRO               45
#define CYGNUM_HAL_INTERRUPT_HDD                 46

#define CYGNUM_HAL_ISR_MIN                       32
#define CYGNUM_HAL_ISR_MAX                       255
#define CYGNUM_HAL_ISR_COUNT    (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

#define CYGNUM_HAL_INTERRUPT_RTC                 CYGNUM_HAL_INTERRUPT_TIMER

#ifdef CYGPKG_HAL_SMP_SUPPORT

#define CYGNUM_HAL_SMP_CPU_INTERRUPT_VECTOR( _n_ ) (64+(_n_))

#endif

//--------------------------------------------------------------------------
// Interrupt vector translation

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
        ((_index_) = ((_vector_)-CYGNUM_HAL_ISR_MIN))


//--------------------------------------------------------------------------
// PIC interrupt acknowledge

#ifndef CYGPKG_HAL_SMP_SUPPORT

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )   \
CYG_MACRO_START                                 \
    int x;                                      \
    HAL_TRANSLATE_VECTOR( _vector_, x );        \
    if ((x >= 8) && (x < 16))                   \
        HAL_WRITE_UINT8( 0xa0, 0x20 );          \
    if ((x >= 0) && (x < 16))                   \
        HAL_WRITE_UINT8( 0x20, 0x20 );          \
CYG_MACRO_END

#else

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )   \
{                                               \
    HAL_APIC_WRITE( HAL_APIC_EOI, 0 );          \
}

#endif

//--------------------------------------------------------------------------
// PIC per-interrupt source masking

#ifndef CYGPKG_HAL_SMP_SUPPORT

#define HAL_INTERRUPT_MASK( _vector_ )                  \
CYG_MACRO_START                                         \
    int x;                                              \
    HAL_TRANSLATE_VECTOR( _vector_, x );                \
    if (x >= 8)                                         \
    {                                                   \
        x = 1 << (x - 8) ;                              \
        asm(                                            \
            "inb $0xA1, %%al;"                          \
            "orl %0, %%eax;"                            \
            "outb %%al, $0xA1;"                         \
            :	/* No outputs. */                       \
            :	"g" (x)                                 \
            :	"eax"                                   \
            );                                          \
    }                                                   \
    else                                                \
    {                                                   \
        x = 1 << x ;                                    \
        asm(                                            \
            "inb $0x21, %%al;"                          \
            "orl %0, %%eax;"                            \
            "outb %%al, $0x21;"                         \
            :	/* No outputs. */                       \
            :	"g" (x)                                 \
            :	"eax"                                   \
            );                                          \
    }                                                   \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )                \
CYG_MACRO_START                                         \
    int x;                                              \
    HAL_TRANSLATE_VECTOR( _vector_, x );                \
    if (x >= 8)                                         \
    {                                                   \
        x = ~(1 << (x - 8)) ;                           \
        asm(                                            \
            "inb $0xA1, %%al;"                          \
            "andl %0, %%eax;"                           \
            "outb %%al, $0xA1;"                         \
            :	/* No outputs. */                       \
            :	"g" (x)                                 \
            :	"eax"                                   \
            );                                          \
    }                                                   \
    else                                                \
    {                                                   \
        x = ~(1 << x) ;                                 \
        asm(                                            \
            "inb $0x21, %%al;"                          \
            "andl %0, %%eax;"                           \
            "outb %%al, $0x21;"                         \
            :	/* No outputs. */                       \
            :	"g" (x)                                 \
            :	"eax"                                   \
            );                                          \
    }                                                   \
CYG_MACRO_END

#else

#define HAL_INTERRUPT_MASK( _vector_ )                          \
{                                                               \
    cyg_uint32 __vec, __val;                                    \
    HAL_TRANSLATE_VECTOR( _vector_, __vec );                    \
    HAL_SPINLOCK_SPIN( cyg_hal_ioapic_lock );                   \
    __vec = cyg_hal_isa_bus_irq[__vec];                         \
    HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_LO(__vec), __val );   \
    __val |= 0x00010000;                                        \
    HAL_IOAPIC_WRITE( HAL_IOAPIC_REG_REDIR_LO(__vec), __val );  \
    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );                  \
}

#define HAL_INTERRUPT_UNMASK( _vector_ )                        \
{                                                               \
    cyg_uint32 __vec, __val;                                    \
    HAL_TRANSLATE_VECTOR( _vector_, __vec );                    \
    HAL_SPINLOCK_SPIN( cyg_hal_ioapic_lock );                   \
    __vec = cyg_hal_isa_bus_irq[__vec];                         \
    HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_LO(__vec), __val );   \
    __val &= ~0x00010000;                                       \
    HAL_IOAPIC_WRITE( HAL_IOAPIC_REG_REDIR_LO(__vec), __val );  \
    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );                  \
}


#endif

//--------------------------------------------------------------------------
// PIC interrupt configuration
// Nothing supported here at present

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )      

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )            

#ifdef CYGPKG_HAL_SMP_SUPPORT

// Additional SMP interrupt configuration support.

__externC void hal_interrupt_set_cpu( CYG_WORD32 vector, HAL_SMP_CPU_TYPE cpu );
__externC void hal_interrupt_get_cpu( CYG_WORD32 vector, HAL_SMP_CPU_TYPE *cpu );

#define HAL_INTERRUPT_SET_CPU( _vector_, _cpu_ )                \
{                                                               \
    cyg_uint32 __vec, __val;                                    \
    HAL_TRANSLATE_VECTOR( _vector_, __vec );                    \
    HAL_SPINLOCK_SPIN( cyg_hal_ioapic_lock );                   \
    __vec = cyg_hal_isa_bus_irq[__vec];                         \
    HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_HI(__vec), __val );   \
    __val &= 0x00FFFFFF;                                        \
    __val |= (_cpu_)<<24;                                       \
    HAL_IOAPIC_WRITE( HAL_IOAPIC_REG_REDIR_HI(__vec), __val );  \
    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );                  \
}


#define HAL_INTERRUPT_GET_CPU( _vector_, _cpu_ )                \
{                                                               \
    cyg_uint32 __vec, __val;                                    \
    HAL_TRANSLATE_VECTOR( _vector_, __vec );                    \
    HAL_SPINLOCK_SPIN( cyg_hal_ioapic_lock );                   \
    __vec = cyg_hal_isa_bus_irq[__vec];                         \
    HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_HI(__vec), __val );   \
    (_cpu_) = (__val>>24) & 0xFF;                               \
    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );                  \
}


#endif

//---------------------------------------------------------------------------
// Clock support.

externC void hal_pc_clock_initialize(cyg_uint32) ;
externC void hal_pc_clock_read(cyg_uint32 *) ;

#define HAL_CLOCK_INITIALIZE(_period_)        hal_pc_clock_initialize(_period_)
#define HAL_CLOCK_RESET(_vec_, _period_)      /* Clock automatically reloads. */
#define HAL_CLOCK_READ(_pvalue_)              hal_pc_clock_read(_pvalue_)

// Timer IO ports
#define PC_PIT_CONTROL  (0x43)
#define PC_PIT_CLOCK_0	(0x40)
#define PC_PIT_CLOCK_1	(0x41)
#define PC_PIT_CLOCK_2	(0x42)

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PCMB_INTR_H
// End of pcmb_intr.h
