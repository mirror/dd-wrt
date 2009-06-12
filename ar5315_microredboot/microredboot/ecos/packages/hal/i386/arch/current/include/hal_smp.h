#ifndef CYGONCE_HAL_SMP_H
#define CYGONCE_HAL_SMP_H

//=============================================================================
//
//      hal_smp.h
//
//      SMP support
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
// Author(s):   nickg
// Contributors:  nickg
// Date:        2001-08-03
// Purpose:     Define SMP support abstractions
// Usage:       #include <cyg/hal/hal_smp.h>

//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_HAL_SMP_SUPPORT

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>

//=============================================================================

/*------------------------------------------------------------------------*/
// APIC definitions

#define HAL_APIC_ID         0x0020
#define HAL_APIC_VER        0x0030
#define HAL_APIC_TPR        0x0080
#define HAL_APIC_EOI        0x00b0
#define HAL_APIC_LDR        0x00d0
#define HAL_APIC_DFR        0x00e0
#define HAL_APIC_SPIV       0x00f0

#define HAL_APIC_ISR        0x0100
#define HAL_APIC_TMR        0x0180
#define HAL_APIC_IRR        0x0200

#define HAL_APIC_ICR_LO     0x0300
#define HAL_APIC_ICR_HI     0x0310

#define HAL_APIC_LVT_TIMER  0x0320
#define HAL_APIC_LVT_PC     0x0340
#define HAL_APIC_LVT_INT0   0x0350
#define HAL_APIC_LVT_INT1   0x0360
#define HAL_APIC_LVT_ERROR  0x0370
#define      HAL_APIC_LVT_MASK      0x00010000

/*------------------------------------------------------------------------*/
// APIC access macros

#define HAL_APIC_READ( __addr, __val )                                  \
{                                                                       \
    HAL_READMEM_UINT32(cyg_hal_smp_local_apic+(__addr), __val );        \
}

#define HAL_APIC_WRITE( __addr, __val )                                 \
{                                                                       \
    HAL_WRITEMEM_UINT32(cyg_hal_smp_local_apic+(__addr), __val );       \
}

/*------------------------------------------------------------------------*/
// I/O APIC definitions

#define HAL_IOAPIC_REGSEL           0x0000
#define HAL_IOAPIC_REGWIN           0x0010

#define HAL_IOAPIC_REG_APICID       0x0000
#define HAL_IOAPIC_REG_APICVER      0x0001
#define HAL_IOAPIC_REG_APICARB      0x0002
#define HAL_IOAPIC_REG_REDTBL       0x0010
#define HAL_IOAPIC_REG_REDIR_LO(n)  (HAL_IOAPIC_REG_REDTBL+((n)*2))
#define HAL_IOAPIC_REG_REDIR_HI(n)  (HAL_IOAPIC_REG_REDTBL+((n)*2)+1)

/*------------------------------------------------------------------------*/
// I/O APIC access macros

#define HAL_IOAPIC_READ( __reg, __val )                                 \
{                                                                       \
    HAL_WRITEMEM_UINT32( cyg_hal_smp_io_apic+HAL_IOAPIC_REGSEL, __reg );    \
    HAL_READMEM_UINT32( cyg_hal_smp_io_apic+HAL_IOAPIC_REGWIN, __val );     \
}

#define HAL_IOAPIC_WRITE( __reg, __val )                                \
{                                                                       \
    HAL_WRITEMEM_UINT32( cyg_hal_smp_io_apic+HAL_IOAPIC_REGSEL, __reg );    \
    HAL_WRITEMEM_UINT32( cyg_hal_smp_io_apic+HAL_IOAPIC_REGWIN, __val );    \
}

//-----------------------------------------------------------------------------
// SMP configuration determined from platform during initialization

__externC CYG_ADDRESS cyg_hal_smp_local_apic;

__externC CYG_ADDRESS cyg_hal_smp_io_apic;

__externC CYG_WORD32 cyg_hal_smp_cpu_count;

__externC CYG_BYTE cyg_hal_smp_cpu_flags[CYGPKG_HAL_SMP_CPU_MAX];

__externC CYG_BYTE cyg_hal_isa_bus_id;
__externC CYG_BYTE cyg_hal_isa_bus_irq[16];

__externC CYG_BYTE cyg_hal_pci_bus_id;
__externC CYG_BYTE cyg_hal_pci_bus_irq[4];

//-----------------------------------------------------------------------------
// CPU numbering macros

#define HAL_SMP_CPU_TYPE        cyg_uint32

#define HAL_SMP_CPU_MAX         CYGPKG_HAL_SMP_CPU_MAX

#define HAL_SMP_CPU_COUNT()     cyg_hal_smp_cpu_count

#define HAL_SMP_CPU_THIS()                      \
({                                              \
    HAL_SMP_CPU_TYPE __id;                      \
    HAL_APIC_READ( HAL_APIC_ID, __id );         \
    (__id>>24)&0xF;                             \
})

#define HAL_SMP_CPU_NONE        (CYGPKG_HAL_SMP_CPU_MAX+1)

//-----------------------------------------------------------------------------
// CPU startup

__externC void cyg_hal_cpu_release(HAL_SMP_CPU_TYPE cpu);

#define HAL_SMP_CPU_START( __cpu ) cyg_hal_cpu_release( __cpu );

#define HAL_SMP_CPU_RESCHEDULE_INTERRUPT( __cpu, __wait ) \
        cyg_hal_cpu_message( __cpu, HAL_SMP_MESSAGE_RESCHEDULE, 0, __wait);

#define HAL_SMP_CPU_TIMESLICE_INTERRUPT( __cpu, __wait ) \
        cyg_hal_cpu_message( __cpu, HAL_SMP_MESSAGE_TIMESLICE, 0, __wait);

//-----------------------------------------------------------------------------
// CPU message exchange

__externC void cyg_hal_cpu_message( HAL_SMP_CPU_TYPE cpu,
                                    CYG_WORD32 msg,
                                    CYG_WORD32 arg,
                                    CYG_WORD32 wait);

#define HAL_SMP_MESSAGE_TYPE            0xF0000000
#define HAL_SMP_MESSAGE_ARG             (~HAL_SMP_MESSAGE_TYPE)

#define HAL_SMP_MESSAGE_RESCHEDULE      0x10000000
#define HAL_SMP_MESSAGE_MASK            0x20000000
#define HAL_SMP_MESSAGE_UNMASK          0x30000000
#define HAL_SMP_MESSAGE_REVECTOR        0x40000000
#define HAL_SMP_MESSAGE_TIMESLICE       0x50000000

//-----------------------------------------------------------------------------
// Test-and-set support
// These macros provide test-and-set support for the least significant bit
// in a word. 

#define HAL_TAS_TYPE    volatile CYG_WORD32

#define HAL_TAS_SET( _tas_, _oldb_ )                    \
CYG_MACRO_START                                         \
{                                                       \
    register CYG_WORD32 __old;                          \
    __asm__ volatile (                                  \
                       "lock btsl   $0,%1\n"            \
                       "sbbl   %0,%0\n"                 \
                       : "=r" (__old), "=m" (_tas_)     \
                       :                                \
                       : "memory"                       \
                     );                                 \
    _oldb_ = ( __old & 1 ) != 0;                        \
}                                                       \
CYG_MACRO_END

#define HAL_TAS_CLEAR( _tas_, _oldb_ )                  \
CYG_MACRO_START                                         \
{                                                       \
    register CYG_WORD32 __old;                          \
    __asm__ volatile (                                  \
                       "lock btrl   $0,%1\n"            \
                       "sbbl   %0,%0\n"                 \
                       : "=r" (__old), "=m" (_tas_)     \
                       :                                \
                       : "memory"                       \
                     );                                 \
    _oldb_ = ( __old & 1 ) != 0;                        \
}                                                       \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// Spinlock support.
// Built on top of test-and-set code.

#define HAL_SPINLOCK_TYPE       volatile CYG_WORD32

#define HAL_SPINLOCK_INIT_CLEAR 0

#define HAL_SPINLOCK_INIT_SET   1

#define HAL_SPINLOCK_SPIN( _lock_ )             \
CYG_MACRO_START                                 \
{                                               \
    cyg_bool _val_;                             \
    do                                          \
    {                                           \
        HAL_TAS_SET( _lock_, _val_ );           \
    } while( _val_ );                           \
}                                               \
CYG_MACRO_END

#define HAL_SPINLOCK_CLEAR( _lock_ )            \
CYG_MACRO_START                                 \
{                                               \
    cyg_bool _val_;                             \
    HAL_TAS_CLEAR( _lock_ , _val_ );            \
}                                               \
CYG_MACRO_END

#define HAL_SPINLOCK_TRY( _lock_, _val_ )       \
    HAL_TAS_SET( _lock_, _val_ );               \
    (_val_) = (((_val_) & 1) == 0)

#define HAL_SPINLOCK_TEST( _lock_, _val_ )      \
    (_val_) = (((_lock_) & 1) != 0)

//-----------------------------------------------------------------------------
// Diagnostic output serialization

__externC HAL_SPINLOCK_TYPE cyg_hal_smp_diag_lock;

#define CYG_HAL_DIAG_LOCK_DATA_DEFN \
        HAL_SPINLOCK_TYPE cyg_hal_smp_diag_lock = HAL_SPINLOCK_INIT_CLEAR

#define CYG_HAL_DIAG_LOCK() HAL_SPINLOCK_SPIN( cyg_hal_smp_diag_lock )

#define CYG_HAL_DIAG_UNLOCK() HAL_SPINLOCK_CLEAR( cyg_hal_smp_diag_lock )

//-----------------------------------------------------------------------------
// Some extra definitions

__externC HAL_SPINLOCK_TYPE cyg_hal_ioapic_lock;

//-----------------------------------------------------------------------------

#endif // CYGPKG_HAL_SMP_SUPPORT

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_SMP_H
// End of hal_smp.h
