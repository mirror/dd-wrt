#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H

//==========================================================================
//
//      var_intr.h
//
//      H8/300H Interrupt and clock support
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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-02-14
// Purpose:      H8/300H Interrupt Support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for H8/300H variants of the H8/300
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

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/plf_intr.h>
#include <cyg/hal/var_arch.h>

//--------------------------------------------------------------------------
// Interrupt vectors.

// The level-specific hardware vectors
// These correspond to VSRs and are the values to use for HAL_VSR_GET/SET
#define CYGNUM_HAL_VECTOR_RESET                0
#define CYGNUM_HAL_VECTOR_RSV1                 1
#define CYGNUM_HAL_VECTOR_RSV2                 2
#define CYGNUM_HAL_VECTOR_RSV3                 3
#define CYGNUM_HAL_VECTOR_RSV4                 4
#define CYGNUM_HAL_VECTOR_RSV5                 5
#define CYGNUM_HAL_VECTOR_RSV6                 6
#define CYGNUM_HAL_VECTOR_NMI                  7
#define CYGNUM_HAL_VECTOR_TRAP0                8
#define CYGNUM_HAL_VECTOR_TRAP1                9
#define CYGNUM_HAL_VECTOR_TRAP2                10
#define CYGNUM_HAL_VECTOR_TRAP3                11

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                     11
#define CYGNUM_HAL_VSR_COUNT                   12

// Exception numbers. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_NMI               CYGNUM_HAL_VECTOR_NMI

#if 0
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS       0
#endif

#define CYGNUM_HAL_EXCEPTION_MIN               CYGNUM_HAL_VSR_MIN
#define CYGNUM_HAL_EXCEPTION_MAX               CYGNUM_HAL_VSR_MAX
#define CYGNUM_HAL_EXCEPTION_COUNT             CYGNUM_HAL_VSR_COUNT

// The decoded interrupts
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_0        12
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_1        13
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_2        14
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_3        15
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_4        16
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_5        17
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_6        18
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_7        19

#define CYGNUM_HAL_INTERRUPT_WDT               20
#define CYGNUM_HAL_INTERRUPT_RFSHCMI           21
#define CYGNUM_HAL_INTERRUPT_RSV22             22
#define CYGNUM_HAL_INTERRUPT_ADI               23

#define CYGNUM_HAL_INTERRUPT_IMIA0             24
#define CYGNUM_HAL_INTERRUPT_IMIB0             25
#define CYGNUM_HAL_INTERRUPT_OVI0              26
#define CYGNUM_HAL_INTERRUPT_RSV27             27

#define CYGNUM_HAL_INTERRUPT_IMIA1             28
#define CYGNUM_HAL_INTERRUPT_IMIB1             29
#define CYGNUM_HAL_INTERRUPT_OVI1              30
#define CYGNUM_HAL_INTERRUPT_RSV31             31

#define CYGNUM_HAL_INTERRUPT_IMIA2             32
#define CYGNUM_HAL_INTERRUPT_IMIB2             33
#define CYGNUM_HAL_INTERRUPT_OVI2              34
#define CYGNUM_HAL_INTERRUPT_RSV35             35

#define CYGNUM_HAL_INTERRUPT_CMIA0             36
#define CYGNUM_HAL_INTERRUPT_CMIB0             37
#define CYGNUM_HAL_INTERRUPT_CMIAB1            38
#define CYGNUM_HAL_INTERRUPT_TOVI01            39

#define CYGNUM_HAL_INTERRUPT_CMIA2             40
#define CYGNUM_HAL_INTERRUPT_CMIB2             41
#define CYGNUM_HAL_INTERRUPT_CMIAB3            42
#define CYGNUM_HAL_INTERRUPT_TOVI23            43

#define CYGNUM_HAL_INTERRUPT_DEND0A            44
#define CYGNUM_HAL_INTERRUPT_DEND0B            45
#define CYGNUM_HAL_INTERRUPT_DEND1A            46
#define CYGNUM_HAL_INTERRUPT_DEND1B            47

#define CYGNUM_HAL_INTERRUPT_RSV48             48
#define CYGNUM_HAL_INTERRUPT_RSV49             49
#define CYGNUM_HAL_INTERRUPT_RSV50             50
#define CYGNUM_HAL_INTERRUPT_RSV51             51

#define CYGNUM_HAL_INTERRUPT_ERI0              52
#define CYGNUM_HAL_INTERRUPT_RXI0              53
#define CYGNUM_HAL_INTERRUPT_TXI0              54
#define CYGNUM_HAL_INTERRUPT_TEI0              55

#define CYGNUM_HAL_INTERRUPT_ERI1              56
#define CYGNUM_HAL_INTERRUPT_RXI1              57
#define CYGNUM_HAL_INTERRUPT_TXI1              58
#define CYGNUM_HAL_INTERRUPT_TEI1              59

#define CYGNUM_HAL_INTERRUPT_ERI2              60
#define CYGNUM_HAL_INTERRUPT_RXI2              61
#define CYGNUM_HAL_INTERRUPT_TXI2              62
#define CYGNUM_HAL_INTERRUPT_TEI2              63

#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     63

#define CYGNUM_HAL_ISR_COUNT                   (3+((CYGNUM_HAL_ISR_MAX+1)/4))

// The vector used by the Real time clock

#define CYGNUM_HAL_INTERRUPT_RTC                CYGNUM_HAL_INTERRUPT_CMIAB3

//--------------------------------------------------------------------------
// Interrupt vector translation.

#if !defined(HAL_TRANSLATE_VECTOR) && !defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

#define HAL_TRANSLATE_VECTOR(_vector_,_index_)                             \
              _index_ = (_vector_)

#endif

//--------------------------------------------------------------------------
// H8/300H specific version of HAL_INTERRUPT_CONFIGURE

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )                \
        hal_interrupt_configure( _vector_, _level_, _up_ )

externC void hal_interrupt_configure(int vector,int level,int up);

#define HAL_INTERRUPT_CONFIGURE_DEFINED

//--------------------------------------------------------------------------
// Interrupt control macros

#define HAL_DISABLE_INTERRUPTS(_old_)   \
        asm volatile (                  \
            "sub.l er0,er0\n\t"         \
            "stc ccr,r0l\n\t"           \
            "orc #0x80,ccr\n\t"        \
            "and.b #0xc0,r0l\n\t"       \
            "mov.l er0,%0"          \
            : "=r"(_old_)               \
            :                           \
            : "er0"                      \
            );

#define HAL_ENABLE_INTERRUPTS()         \
        asm volatile (                  \
            "andc #0x3f,ccr"             \
            );

#define HAL_RESTORE_INTERRUPTS(_old_)   \
        asm volatile (                  \
            "mov.l %0,er0\n\t"          \
            "and.b #0xc0,r0l\n\t"       \
            "stc ccr,r0h\n\t"           \
            "and.b #0x3f,r0h\n\t"       \
            "or.b r0h,r0l\n\t"          \
            "ldc r0l,ccr"               \
            :                           \
            : "r"(_old_)                \
            : "er0"                     \
            );

#define HAL_QUERY_INTERRUPTS(_old_)     \
        asm volatile (                  \
            "sub.l er0,er0\n\t"         \
            "stc ccr,r0l\n\t"           \
            "and.b #0xc0,r0l\n\t"       \
            "mov.l er0,%0"              \
            : "=r"(_old_)               \
            );

//--------------------------------------------------------------------------
// Clock control.

externC void hal_clock_initialize(cyg_uint32 period);
externC void hal_clock_reset(cyg_uint32 vector,cyg_uint32 period);
externC void hal_clock_read(cyg_uint32 *pvalue);

#define HAL_CLOCK_INITIALIZE( _period_ ) \
        hal_clock_initialize( _period_ )

#define HAL_CLOCK_RESET( _vector_, _period_ ) \
        hal_clock_reset( _vector_, _period_ )

#define HAL_CLOCK_READ( _pvalue_ ) \
        hal_clock_read( _pvalue_ )

// FIXME: above line should not use CYGNUM_KERNEL_COUNTERS_RTC_PERIOD since
// this means the HAL gets configured by kernel options even when the
// kernel is disabled!


//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VAR_INTR_H
// End of var_intr.h
