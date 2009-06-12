#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H

//==========================================================================
//
//      var_intr.h
//
//      H8S Interrupt and clock support
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
// Date:         2003-01-01
// Purpose:      H8S Interrupt Support
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
#define CYGNUM_HAL_VECTOR_RESET_P              0
#define CYGNUM_HAL_VECTOR_RESET_M              1
#define CYGNUM_HAL_VECTOR_RSV2                 2
#define CYGNUM_HAL_VECTOR_RSV3                 3
#define CYGNUM_HAL_VECTOR_RSV4                 4
#define CYGNUM_HAL_VECTOR_TRACE                5
#define CYGNUM_HAL_VECTOR_DIRECT               6
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
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_0        16
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_1        17
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_2        18
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_3        19
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_4        20
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_5        21
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_6        22
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_7        23
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_8        24
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_9        25
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_10       26
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_11       27
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_12       28
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_13       29
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_14       30
#define CYGNUM_HAL_INTERRUPT_EXTERNAL_15       31

#define CYGNUM_HAL_INTERRUPT_DTC               32
#define CYGNUM_HAL_INTERRUPT_WDT               33
#define CYGNUM_HAL_INTERRUPT_RSV34             34
#define CYGNUM_HAL_INTERRUPT_RFSHCMI           35
#define CYGNUM_HAL_INTERRUPT_RSV36             36
#define CYGNUM_HAL_INTERRUPT_RSV37             37
#define CYGNUM_HAL_INTERRUPT_ADI               38
#define CYGNUM_HAL_INTERRUPT_RSV39             39

#define CYGNUM_HAL_INTERRUPT_TGI0A             40
#define CYGNUM_HAL_INTERRUPT_TGI0B             41
#define CYGNUM_HAL_INTERRUPT_TGI0C             42
#define CYGNUM_HAL_INTERRUPT_TGI0D             43
#define CYGNUM_HAL_INTERRUPT_TGI0V             44
#define CYGNUM_HAL_INTERRUPT_RSV45             45
#define CYGNUM_HAL_INTERRUPT_RSV46             46
#define CYGNUM_HAL_INTERRUPT_RSV47             47

#define CYGNUM_HAL_INTERRUPT_TGI1A             48
#define CYGNUM_HAL_INTERRUPT_TGI1B             49
#define CYGNUM_HAL_INTERRUPT_TGI1V             50
#define CYGNUM_HAL_INTERRUPT_TGI1U             51

#define CYGNUM_HAL_INTERRUPT_TGI2A             52
#define CYGNUM_HAL_INTERRUPT_TGI2B             53
#define CYGNUM_HAL_INTERRUPT_TGI2V             54
#define CYGNUM_HAL_INTERRUPT_TGI2U             55

#define CYGNUM_HAL_INTERRUPT_TGI3A             56
#define CYGNUM_HAL_INTERRUPT_TGI3B             57
#define CYGNUM_HAL_INTERRUPT_TGI3C             58
#define CYGNUM_HAL_INTERRUPT_TGI3D             59
#define CYGNUM_HAL_INTERRUPT_TGI3V             60
#define CYGNUM_HAL_INTERRUPT_RSV61             61
#define CYGNUM_HAL_INTERRUPT_RSV62             62
#define CYGNUM_HAL_INTERRUPT_RSV63             63

#define CYGNUM_HAL_INTERRUPT_TGI4A             64
#define CYGNUM_HAL_INTERRUPT_TGI4B             65
#define CYGNUM_HAL_INTERRUPT_TGI4V             66
#define CYGNUM_HAL_INTERRUPT_TGI4U             67

#define CYGNUM_HAL_INTERRUPT_TGI5A             68
#define CYGNUM_HAL_INTERRUPT_TGI5B             69
#define CYGNUM_HAL_INTERRUPT_TGI5V             70
#define CYGNUM_HAL_INTERRUPT_TGI5U             71

#define CYGNUM_HAL_INTERRUPT_CMIA0             72
#define CYGNUM_HAL_INTERRUPT_CMIB0             73
#define CYGNUM_HAL_INTERRUPT_OVI0              74
#define CYGNUM_HAL_INTERRUPT_RSV75             75

#define CYGNUM_HAL_INTERRUPT_CMIA1             76
#define CYGNUM_HAL_INTERRUPT_CMIB1             77
#define CYGNUM_HAL_INTERRUPT_OVI1              78
#define CYGNUM_HAL_INTERRUPT_RSV79             79

#define CYGNUM_HAL_INTERRUPT_DEND0A            80
#define CYGNUM_HAL_INTERRUPT_DEND0B            81
#define CYGNUM_HAL_INTERRUPT_DEND1A            82
#define CYGNUM_HAL_INTERRUPT_DEND1B            83

#define CYGNUM_HAL_INTERRUPT_EXDEND0           84
#define CYGNUM_HAL_INTERRUPT_EXDEND1           85
#define CYGNUM_HAL_INTERRUPT_EXDEND2           86
#define CYGNUM_HAL_INTERRUPT_EXDEND3           87

#define CYGNUM_HAL_INTERRUPT_ERI0              88
#define CYGNUM_HAL_INTERRUPT_RXI0              89
#define CYGNUM_HAL_INTERRUPT_TXI0              90
#define CYGNUM_HAL_INTERRUPT_TEI0              91

#define CYGNUM_HAL_INTERRUPT_ERI1              92
#define CYGNUM_HAL_INTERRUPT_RXI1              93
#define CYGNUM_HAL_INTERRUPT_TXI1              94
#define CYGNUM_HAL_INTERRUPT_TEI1              95

#define CYGNUM_HAL_INTERRUPT_ERI2              96
#define CYGNUM_HAL_INTERRUPT_RXI2              97
#define CYGNUM_HAL_INTERRUPT_TXI2              98
#define CYGNUM_HAL_INTERRUPT_TEI2              99

#define CYGNUM_HAL_INTERRUPT_RSV100            100
#define CYGNUM_HAL_INTERRUPT_RSV101            101
#define CYGNUM_HAL_INTERRUPT_RSV102            102
#define CYGNUM_HAL_INTERRUPT_RSV103            103
#define CYGNUM_HAL_INTERRUPT_RSV104            104
#define CYGNUM_HAL_INTERRUPT_RSV105            105
#define CYGNUM_HAL_INTERRUPT_RSV106            106
#define CYGNUM_HAL_INTERRUPT_RSV107            107
#define CYGNUM_HAL_INTERRUPT_RSV108            108
#define CYGNUM_HAL_INTERRUPT_RSV109            109
#define CYGNUM_HAL_INTERRUPT_RSV110            110
#define CYGNUM_HAL_INTERRUPT_RSV111            111
#define CYGNUM_HAL_INTERRUPT_RSV112            112
#define CYGNUM_HAL_INTERRUPT_RSV113            113
#define CYGNUM_HAL_INTERRUPT_RSV114            114
#define CYGNUM_HAL_INTERRUPT_RSV115            115
#define CYGNUM_HAL_INTERRUPT_RSV116            116
#define CYGNUM_HAL_INTERRUPT_RSV117            117
#define CYGNUM_HAL_INTERRUPT_RSV118            118
#define CYGNUM_HAL_INTERRUPT_RSV119            119
#define CYGNUM_HAL_INTERRUPT_RSV120            120
#define CYGNUM_HAL_INTERRUPT_RSV121            121
#define CYGNUM_HAL_INTERRUPT_RSV122            122
#define CYGNUM_HAL_INTERRUPT_RSV123            123
#define CYGNUM_HAL_INTERRUPT_RSV124            124
#define CYGNUM_HAL_INTERRUPT_RSV125            125
#define CYGNUM_HAL_INTERRUPT_RSV126            126
#define CYGNUM_HAL_INTERRUPT_RSV127            127


#define CYGNUM_HAL_ISR_MIN                     16
#define CYGNUM_HAL_ISR_MAX                     127

#define CYGNUM_HAL_ISR_COUNT                   (3+((CYGNUM_HAL_ISR_MAX+1)/4))

// The vector used by the Real time clock

#define CYGNUM_HAL_INTERRUPT_RTC                CYGNUM_HAL_INTERRUPT_CMIA1

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
            "stc exr,r0l\n\t"           \
            "orc #0x7,exr\n\t"          \
            "and.b #0x07,r0l\n\t"       \
            "mov.l er0,%0"              \
            : "=r"(_old_)               \
            :                           \
            : "er0"                     \
            );

#define HAL_ENABLE_INTERRUPTS()         \
        asm volatile (                  \
            "andc #0xf8,exr"            \
            );

#define HAL_RESTORE_INTERRUPTS(_old_)   \
        asm volatile (                  \
            "mov.l %0,er0\n\t"          \
            "and.b #0x07,r0l\n\t"       \
            "stc exr,r0h\n\t"           \
            "and.b #0xf8,r0h\n\t"       \
            "or.b r0h,r0l\n\t"          \
            "ldc r0l,exr"               \
            :                           \
            : "r"(_old_)                \
            : "er0"                     \
            );

#define HAL_QUERY_INTERRUPTS(_old_)     \
        asm volatile (                  \
            "sub.l er0,er0\n\t"         \
            "stc exr,r0l\n\t"           \
            "and.b #0x07,r0l\n\t"       \
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
