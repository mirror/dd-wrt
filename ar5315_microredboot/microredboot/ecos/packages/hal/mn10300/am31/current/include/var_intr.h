#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H

//==========================================================================
//
//      var_intr.h
//
//      AM31 Interrupt and clock support
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
// Purpose:      AM31 Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for AM31 variants of the MN10300
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

//--------------------------------------------------------------------------

// The MN10300 has a somewhat complex interrupt structure. Besides the
// reset and NMI vectors there are seven maskable interrupt vectors
// which must point to code in the 64k starting at 0x40000000. There
// are also 25 Interrupt control groups, each of which can have 4
// interrupt lines attached, for a theoretical total of 100 interrupts
// (!). Some of these are dedicated to specific devices, other to
// external pins, and others are not connected to anything, resulting
// in only 45 that can actually be delivered. Each control group may
// be assigned one of seven interrupt levels, and is delivered to the
// corresponding vector. Software can then use a register to determine
// the delivering group and detect from there which interrupt has been
// delivered.
//
// The approach we will adopt at present is for the code attached to
// each vector to save state and jump via a table to a VSR. The
// default VSR will fully decode the delivered interrupt into a table
// of isr/data/object entries. VSR replacement will operate on the
// first level indirection table rather than the hardware
// vectors. This is the fastest mechanism, however it needs 100*3*4 +
// 7*4 = 1228 bytes for the tables.
// 

//--------------------------------------------------------------------------
// Interrupt vectors.

// The level-specific hardware vectors
// These correspond to VSRs and are the values to use for HAL_VSR_GET/SET
#define CYGNUM_HAL_VECTOR_0                    0
#define CYGNUM_HAL_VECTOR_1                    1
#define CYGNUM_HAL_VECTOR_2                    2
#define CYGNUM_HAL_VECTOR_3                    3
#define CYGNUM_HAL_VECTOR_4                    4
#define CYGNUM_HAL_VECTOR_5                    5
#define CYGNUM_HAL_VECTOR_6                    6
#define CYGNUM_HAL_VECTOR_NMI_ENTRY            7
#define CYGNUM_HAL_VECTOR_TRAP                 8
#define CYGNUM_HAL_VECTOR_NMI                  9
#define CYGNUM_HAL_VECTOR_WATCHDOG             10
#define CYGNUM_HAL_VECTOR_SYSTEM_ERROR         11

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                     11
#define CYGNUM_HAL_VSR_COUNT                   12

// Exception numbers. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_NMI               CYGNUM_HAL_VECTOR_NMI
#define CYGNUM_HAL_EXCEPTION_WATCHDOG          CYGNUM_HAL_VECTOR_WATCHDOG
#define CYGNUM_HAL_EXCEPTION_SYSTEM_ERROR      CYGNUM_HAL_VECTOR_SYSTEM_ERROR

#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS       CYGNUM_HAL_EXCEPTION_SYSTEM_ERROR

#define CYGNUM_HAL_EXCEPTION_MIN               CYGNUM_HAL_VSR_MIN
#define CYGNUM_HAL_EXCEPTION_MAX               CYGNUM_HAL_VSR_MAX
#define CYGNUM_HAL_EXCEPTION_COUNT             CYGNUM_HAL_VSR_COUNT

// The decoded interrupts

#define CYGNUM_HAL_INTERRUPT_NMIRQ                0
#define CYGNUM_HAL_INTERRUPT_WATCHDOG             1
#define CYGNUM_HAL_INTERRUPT_SYSTEM_ERROR         2
#define CYGNUM_HAL_INTERRUPT_RESERVED_3           3

#define CYGNUM_HAL_INTERRUPT_RESERVED_4           4
#define CYGNUM_HAL_INTERRUPT_RESERVED_5           5
#define CYGNUM_HAL_INTERRUPT_RESERVED_6           6
#define CYGNUM_HAL_INTERRUPT_RESERVED_7           7

#define CYGNUM_HAL_INTERRUPT_TIMER_0              8
#define CYGNUM_HAL_INTERRUPT_RESERVED_9           9
#define CYGNUM_HAL_INTERRUPT_RESERVED_10          10
#define CYGNUM_HAL_INTERRUPT_RESERVED_11          11

#define CYGNUM_HAL_INTERRUPT_TIMER_1              12
#define CYGNUM_HAL_INTERRUPT_RESERVED_13          13
#define CYGNUM_HAL_INTERRUPT_RESERVED_14          14
#define CYGNUM_HAL_INTERRUPT_RESERVED_15          15

#define CYGNUM_HAL_INTERRUPT_TIMER_2              16
#define CYGNUM_HAL_INTERRUPT_RESERVED_17          17
#define CYGNUM_HAL_INTERRUPT_RESERVED_18          18
#define CYGNUM_HAL_INTERRUPT_RESERVED_19          19

#define CYGNUM_HAL_INTERRUPT_TIMER_3              20
#define CYGNUM_HAL_INTERRUPT_RESERVED_21          21
#define CYGNUM_HAL_INTERRUPT_RESERVED_22          22
#define CYGNUM_HAL_INTERRUPT_RESERVED_23          23

#define CYGNUM_HAL_INTERRUPT_TIMER_4              24
#define CYGNUM_HAL_INTERRUPT_RESERVED_25          25
#define CYGNUM_HAL_INTERRUPT_RESERVED_26          26
#define CYGNUM_HAL_INTERRUPT_RESERVED_27          27

#define CYGNUM_HAL_INTERRUPT_TIMER_5              28
#define CYGNUM_HAL_INTERRUPT_RESERVED_29          29
#define CYGNUM_HAL_INTERRUPT_RESERVED_30          30
#define CYGNUM_HAL_INTERRUPT_RESERVED_31          31

#define CYGNUM_HAL_INTERRUPT_TIMER_6              32
#define CYGNUM_HAL_INTERRUPT_RESERVED_33          33
#define CYGNUM_HAL_INTERRUPT_RESERVED_34          34
#define CYGNUM_HAL_INTERRUPT_RESERVED_35          35

#define CYGNUM_HAL_INTERRUPT_TIMER_6_COMPARE_A    36
#define CYGNUM_HAL_INTERRUPT_RESERVED_37          37
#define CYGNUM_HAL_INTERRUPT_RESERVED_38          38
#define CYGNUM_HAL_INTERRUPT_RESERVED_39          39

#define CYGNUM_HAL_INTERRUPT_TIMER_6_COMPARE_B    40
#define CYGNUM_HAL_INTERRUPT_RESERVED_41          41
#define CYGNUM_HAL_INTERRUPT_RESERVED_42          42
#define CYGNUM_HAL_INTERRUPT_RESERVED_43          43

#define CYGNUM_HAL_INTERRUPT_RESERVED_44          44
#define CYGNUM_HAL_INTERRUPT_RESERVED_45          45
#define CYGNUM_HAL_INTERRUPT_RESERVED_46          46
#define CYGNUM_HAL_INTERRUPT_RESERVED_47          47

#define CYGNUM_HAL_INTERRUPT_DMA0                 48
#define CYGNUM_HAL_INTERRUPT_RESERVED_49          49
#define CYGNUM_HAL_INTERRUPT_RESERVED_50          50
#define CYGNUM_HAL_INTERRUPT_RESERVED_51          51

#define CYGNUM_HAL_INTERRUPT_DMA1                 52
#define CYGNUM_HAL_INTERRUPT_RESERVED_53          53
#define CYGNUM_HAL_INTERRUPT_RESERVED_54          54
#define CYGNUM_HAL_INTERRUPT_RESERVED_55          55

#define CYGNUM_HAL_INTERRUPT_DMA2                 56
#define CYGNUM_HAL_INTERRUPT_RESERVED_57          57
#define CYGNUM_HAL_INTERRUPT_RESERVED_58          58
#define CYGNUM_HAL_INTERRUPT_RESERVED_59          59

#define CYGNUM_HAL_INTERRUPT_DMA3                 60
#define CYGNUM_HAL_INTERRUPT_RESERVED_61          61
#define CYGNUM_HAL_INTERRUPT_RESERVED_62          62
#define CYGNUM_HAL_INTERRUPT_RESERVED_63          63

#define CYGNUM_HAL_INTERRUPT_SERIAL_0_RX          64
#define CYGNUM_HAL_INTERRUPT_RESERVED_65          65
#define CYGNUM_HAL_INTERRUPT_RESERVED_66          66
#define CYGNUM_HAL_INTERRUPT_RESERVED_67          67

#define CYGNUM_HAL_INTERRUPT_SERIAL_0_TX          68
#define CYGNUM_HAL_INTERRUPT_RESERVED_69          69
#define CYGNUM_HAL_INTERRUPT_RESERVED_70          70
#define CYGNUM_HAL_INTERRUPT_RESERVED_71          71

#define CYGNUM_HAL_INTERRUPT_SERIAL_1_RX          72
#define CYGNUM_HAL_INTERRUPT_RESERVED_73          73
#define CYGNUM_HAL_INTERRUPT_RESERVED_74          74
#define CYGNUM_HAL_INTERRUPT_RESERVED_75          75

#define CYGNUM_HAL_INTERRUPT_SERIAL_1_TX          76
#define CYGNUM_HAL_INTERRUPT_RESERVED_77          77
#define CYGNUM_HAL_INTERRUPT_RESERVED_78          78
#define CYGNUM_HAL_INTERRUPT_RESERVED_79          79

#define CYGNUM_HAL_INTERRUPT_SERIAL_2_RX          80
#define CYGNUM_HAL_INTERRUPT_RESERVED_81          81
#define CYGNUM_HAL_INTERRUPT_RESERVED_82          82
#define CYGNUM_HAL_INTERRUPT_RESERVED_83          83

#define CYGNUM_HAL_INTERRUPT_SERIAL_2_TX          84
#define CYGNUM_HAL_INTERRUPT_RESERVED_85          85
#define CYGNUM_HAL_INTERRUPT_RESERVED_86          86
#define CYGNUM_HAL_INTERRUPT_RESERVED_87          87

#define CYGNUM_HAL_INTERRUPT_RESERVED_88          88
#define CYGNUM_HAL_INTERRUPT_RESERVED_89          89
#define CYGNUM_HAL_INTERRUPT_RESERVED_90          90
#define CYGNUM_HAL_INTERRUPT_RESERVED_91          91

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_0           92
#define CYGNUM_HAL_INTERRUPT_RESERVED_93          93
#define CYGNUM_HAL_INTERRUPT_RESERVED_94          94
#define CYGNUM_HAL_INTERRUPT_RESERVED_95          95

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_1           96
#define CYGNUM_HAL_INTERRUPT_RESERVED_97          97
#define CYGNUM_HAL_INTERRUPT_RESERVED_98          98
#define CYGNUM_HAL_INTERRUPT_RESERVED_99          99

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_2           100
#define CYGNUM_HAL_INTERRUPT_RESERVED_101         101
#define CYGNUM_HAL_INTERRUPT_RESERVED_102         102
#define CYGNUM_HAL_INTERRUPT_RESERVED_103         103

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_3           104
#define CYGNUM_HAL_INTERRUPT_RESERVED_105         105
#define CYGNUM_HAL_INTERRUPT_RESERVED_106         106
#define CYGNUM_HAL_INTERRUPT_RESERVED_107         107

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_4           108
#define CYGNUM_HAL_INTERRUPT_RESERVED_109         109
#define CYGNUM_HAL_INTERRUPT_RESERVED_110         110
#define CYGNUM_HAL_INTERRUPT_RESERVED_111         111

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_5           112
#define CYGNUM_HAL_INTERRUPT_RESERVED_113         113
#define CYGNUM_HAL_INTERRUPT_RESERVED_114         114
#define CYGNUM_HAL_INTERRUPT_RESERVED_115         115

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_6           116
#define CYGNUM_HAL_INTERRUPT_RESERVED_117         117
#define CYGNUM_HAL_INTERRUPT_RESERVED_118         118
#define CYGNUM_HAL_INTERRUPT_RESERVED_119         119

#define CYGNUM_HAL_INTERRUPT_EXTERNAL_7           120
#define CYGNUM_HAL_INTERRUPT_RESERVED_121         121
#define CYGNUM_HAL_INTERRUPT_RESERVED_122         122
#define CYGNUM_HAL_INTERRUPT_RESERVED_123         123

#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     123

#define CYGNUM_HAL_ISR_COUNT                   (3+((CYGNUM_HAL_ISR_MAX+1)/4))

// The vector used by the Real time clock

#define CYGNUM_HAL_INTERRUPT_RTC                CYGNUM_HAL_INTERRUPT_TIMER_5

//--------------------------------------------------------------------------
// Interrupt vector translation.

#if !defined(HAL_TRANSLATE_VECTOR) && !defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

#define HAL_TRANSLATE_VECTOR(_vector_,_index_)                             \
              _index_ = (((_vector_)<=CYGNUM_HAL_INTERRUPT_SYSTEM_ERROR) ? \
                         (_vector_) :                                      \
                         (((_vector_)>>2)+CYGNUM_HAL_INTERRUPT_RESERVED_3))

#endif

//--------------------------------------------------------------------------
// MN10300 specific version of HAL_INTERRUPT_CONFIGURE

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )      \
CYG_MACRO_START                                                 \
    if( _vector_ >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0 )           \
    {                                                           \
        cyg_uint32 _v_ = _vector_;                              \
        cyg_uint16 _val_ = 0;                                   \
        cyg_uint16 _reg_;                                       \
                                                                \
        /* adjust vector to bit offset in EXTMD */              \
        _v_ -= CYGNUM_HAL_INTERRUPT_EXTERNAL_0;                 \
        _v_ >>= 1;                                              \
                                                                \
        /* set bits according to requirements */                \
        if( _up_ ) _val_ |= 1;                                  \
        if( !(_level_) ) _val_ |= 2;                            \
                                                                \
        /* get EXTMD */                                         \
        _reg_ = mn10300_interrupt_control[0x180>>1];            \
                                                                \
        /* clear old value and set new */                       \
        _reg_ &= ~(3<<_v_);                                     \
        _reg_ |= _val_<<_v_;                                    \
                                                                \
        /* restore EXTMD */                                     \
        mn10300_interrupt_control[0x180>>1] = _reg_;            \
    }                                                           \
CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE_DEFINED

//--------------------------------------------------------------------------
// Timer control registers.
// On the mn103002 we use timers 4 and 5

#define TIMER4_CR        0x340010a0
#define TIMER4_BR        0x34001090
#define TIMER4_MD        0x34001080

#define TIMER5_CR        0x340010a2
#define TIMER5_BR        0x34001092
#define TIMER5_MD        0x34001082

#define TIMER_CR        TIMER5_CR
#define TIMER_BR        TIMER5_BR
#define TIMER_MD        TIMER5_MD

#define TIMER0_MD       0x34001000
#define TIMER0_BR       0x34001010
#define TIMER0_CR       0x34001020

//--------------------------------------------------------------------------
// Clock control.

#define HAL_CLOCK_INITIALIZE( _period_ )                                \
{                                                                       \
    volatile cyg_uint16 *timer4_br      = (cyg_uint16 *)TIMER4_BR;      \
    volatile cyg_uint8 *timer4_md       = (cyg_uint8 *)TIMER4_MD;       \
    volatile cyg_uint16 *timer5_br      = (cyg_uint16 *)TIMER5_BR;      \
    volatile cyg_uint8 *timer5_md       = (cyg_uint8 *)TIMER5_MD;       \
                                                                        \
    /* Set timers 4 and 5 into cascade mode */                          \
                                                                        \
    *timer5_br = (_period_)>>16;                                        \
                                                                        \
    *timer5_md = 0x40;                                                  \
    *timer5_md = 0x83;                                                  \
                                                                        \
    *timer4_br = (_period_)&0x0000FFFF;                                 \
                                                                        \
    *timer4_md = 0x40;                                                  \
    *timer4_md = 0x80;                                                  \
}

#define HAL_CLOCK_RESET( _vector_, _period_ )

#define HAL_CLOCK_READ( _pvalue_ )                                      \
{                                                                       \
    volatile cyg_uint16 *timer4_cr = (cyg_uint16 *)TIMER4_CR;           \
    volatile cyg_uint16 *timer5_cr = (cyg_uint16 *)TIMER5_CR;           \
                                                                        \
    cyg_uint16 t5;                                                      \
    cyg_uint16 t4;                                                      \
                                                                        \
    /* Loop reading the two timers until we can read t5 twice   */      \
    /* with the same value. This avoids getting silly times if  */      \
    /* the timers carry between reading the two regs.           */      \
    do {                                                                \
        t5 = *timer5_cr;                                                \
        t4 = *timer4_cr;                                                \
    } while( t5 != *timer5_cr );                                        \
                                                                        \
    *(_pvalue_) = CYGNUM_KERNEL_COUNTERS_RTC_PERIOD - ((t5<<16) + t4);  \
}

// FIXME: above line should not use CYGNUM_KERNEL_COUNTERS_RTC_PERIOD since
// this means the HAL gets configured by kernel options even when the
// kernel is disabled!


//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_VAR_INTR_H
// End of var_intr.h
