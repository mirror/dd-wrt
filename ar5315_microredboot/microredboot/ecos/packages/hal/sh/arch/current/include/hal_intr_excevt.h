#ifndef CYGONCE_HAL_INTR_EXCEVT_H
#define CYGONCE_HAL_INTR_EXCEVT_H

//==========================================================================
//
//      hal_intr_excevt.h
//
//      HAL Interrupt and clock support for variants with EXCEVT style
//      exception/interrupt mapping (SH3, SH4)
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
// Contributors: jskov,
// Date:         1999-04-24
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:        Is included from <cyg/hal/hal_intr.h>
//
//####DESCRIPTIONEND####
//
//==========================================================================

//--------------------------------------------------------------------------
// SH exception vectors. These correspond to VSRs and are the values
// to use for HAL_VSR_GET/SET
//
// Note that exceptions are decoded - there is a VSR slot for each exception
// source, while interrupts are handled via the same VSR.

#define CYGNUM_HAL_VECTOR_POWERON                0 // power-on
#define CYGNUM_HAL_VECTOR_RESET                  1 // reset
#define CYGNUM_HAL_VECTOR_TLBMISS_ACCESS         2 // TLB-miss/invalid load
#define CYGNUM_HAL_VECTOR_TLBMISS_WRITE          3 // TLB-miss/invalid store
#define CYGNUM_HAL_VECTOR_INITIAL_WRITE          4 // initial page write
#define CYGNUM_HAL_VECTOR_TLBERROR_ACCESS        5 // TLB prot violation l
#define CYGNUM_HAL_VECTOR_TLBERROR_WRITE         6 // TLB prot violation s
#define CYGNUM_HAL_VECTOR_DATA_ACCESS            7 // address error (load)
#define CYGNUM_HAL_VECTOR_DATA_WRITE             8 // address error (store)
// RESERVED 9-10
#define CYGNUM_HAL_VECTOR_TRAP                  11 // unconditional trap
#define CYGNUM_HAL_VECTOR_ILLEGAL_INSTRUCTION   12 // reserved instruction
#define CYGNUM_HAL_VECTOR_ILLEGAL_SLOT_INSTRUCTION  13 
                                           // illegal instruction in delay slot
#define CYGNUM_HAL_VECTOR_NMI                   14 // NMI
#define CYGNUM_HAL_VECTOR_INSTRUCTION_BP        15 // user breakpoint

#define CYGNUM_HAL_VECTOR_INTERRUPT             16 // all interrupts

#ifndef CYG_VECTOR_IS_INTERRUPT
# define CYG_VECTOR_IS_INTERRUPT(v) (CYGNUM_HAL_VECTOR_INSTRUCTION_BP < (v))
#endif

#define CYGNUM_HAL_VSR_MIN                   CYGNUM_HAL_VECTOR_POWERON
#ifndef CYGNUM_HAL_VSR_MAX
# define CYGNUM_HAL_VSR_MAX                  CYGNUM_HAL_VECTOR_INTERRUPT
#endif
#define CYGNUM_HAL_VSR_COUNT                 ( CYGNUM_HAL_VSR_MAX + 1 )

#ifndef CYGNUM_HAL_VSR_EXCEPTION_COUNT
# define CYGNUM_HAL_VSR_EXCEPTION_COUNT       (CYGNUM_HAL_VECTOR_INSTRUCTION_BP-CYGNUM_HAL_VECTOR_POWERON+1)
#endif

// For the stub exception handling
#define _CYGNUM_HAL_VECTOR_FIRST_MEM_FAULT       CYGNUM_HAL_EXCEPTION_TLBMISS_ACCESS
#define _CYGNUM_HAL_VECTOR_LAST_MEM_FAULT        CYGNUM_HAL_EXCEPTION_DATA_WRITE

// The decoded interrupts.
#define CYGNUM_HAL_INTERRUPT_NMI             0
#define CYGNUM_HAL_INTERRUPT_RESERVED_1E0    1
#define CYGNUM_HAL_INTERRUPT_LVL0            2
#define CYGNUM_HAL_INTERRUPT_LVL1            3
#define CYGNUM_HAL_INTERRUPT_LVL2            4
#define CYGNUM_HAL_INTERRUPT_LVL3            5
#define CYGNUM_HAL_INTERRUPT_LVL4            6
#define CYGNUM_HAL_INTERRUPT_LVL5            7
#define CYGNUM_HAL_INTERRUPT_LVL6            8
#define CYGNUM_HAL_INTERRUPT_LVL7            9
#define CYGNUM_HAL_INTERRUPT_LVL8            10
#define CYGNUM_HAL_INTERRUPT_LVL9            11
#define CYGNUM_HAL_INTERRUPT_LVL10           12
#define CYGNUM_HAL_INTERRUPT_LVL11           13
#define CYGNUM_HAL_INTERRUPT_LVL12           14
#define CYGNUM_HAL_INTERRUPT_LVL13           15
#define CYGNUM_HAL_INTERRUPT_LVL14           16
#define CYGNUM_HAL_INTERRUPT_RESERVED_3E0    17
#define CYGNUM_HAL_INTERRUPT_TMU0_TUNI0      18
#define CYGNUM_HAL_INTERRUPT_TMU1_TUNI1      19
#define CYGNUM_HAL_INTERRUPT_TMU2_TUNI2      20
#define CYGNUM_HAL_INTERRUPT_TMU2_TICPI2     21
#define CYGNUM_HAL_INTERRUPT_RTC_ATI         22
#define CYGNUM_HAL_INTERRUPT_RTC_PRI         23
#define CYGNUM_HAL_INTERRUPT_RTC_CUI         24
#define CYGNUM_HAL_INTERRUPT_SCI_ERI         25
#define CYGNUM_HAL_INTERRUPT_SCI_RXI         26
#define CYGNUM_HAL_INTERRUPT_SCI_TXI         27
#define CYGNUM_HAL_INTERRUPT_SCI_TEI         28
#define CYGNUM_HAL_INTERRUPT_WDT_ITI         29
#define CYGNUM_HAL_INTERRUPT_REF_RCMI        30
#define CYGNUM_HAL_INTERRUPT_REF_ROVI        31

#ifndef CYGNUM_HAL_ISR_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_REF_ROVI
#endif

#define CYGNUM_HAL_ISR_MIN                   CYGNUM_HAL_INTERRUPT_NMI
#define CYGNUM_HAL_ISR_COUNT                 ( CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1 )

// The vector used by the Real time clock
#ifndef CYGNUM_HAL_INTERRUPT_RTC
# define CYGNUM_HAL_INTERRUPT_RTC             CYGNUM_HAL_INTERRUPT_TMU0_TUNI0
#endif

//--------------------------------------------------------------------------
// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

// The exception indexes are EXPEVT/0x20. Variants may define additional
// exception vectors.

#define CYGNUM_HAL_EXCEPTION_POWERON                0 // power-on
#define CYGNUM_HAL_EXCEPTION_RESET                  1 // reset
#define CYGNUM_HAL_EXCEPTION_TLBMISS_ACCESS         2 // TLB-miss/invalid load
#define CYGNUM_HAL_EXCEPTION_TLBMISS_WRITE          3 // TLB-miss/invalid store
#define CYGNUM_HAL_EXCEPTION_INITIAL_WRITE          4 // initial page write
#define CYGNUM_HAL_EXCEPTION_TLBERROR_ACCESS        5 // TLB prot violation l
#define CYGNUM_HAL_EXCEPTION_TLBERROR_WRITE         6 // TLB prot violation s
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS            7 // address error (load)
#define CYGNUM_HAL_EXCEPTION_DATA_WRITE             8 // address error (store)
#define CYGNUM_HAL_EXCEPTION_TRAP                  11 // unconditional trap
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION   12 // reserved instruction
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_SLOT_INSTRUCTION  13 
                                           // illegal instruction in delay slot
#define CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP        15 // user breakpoint

#define CYGNUM_HAL_EXCEPTION_MIN          CYGNUM_HAL_EXCEPTION_POWERON

#ifndef CYGNUM_HAL_EXCEPTION_MAX
# define CYGNUM_HAL_EXCEPTION_MAX         CYGNUM_HAL_EXCEPTION_INSTRUCTION_BP
#endif

#define CYGNUM_HAL_EXCEPTION_COUNT           \
                 ( CYGNUM_HAL_EXCEPTION_MAX - CYGNUM_HAL_EXCEPTION_MIN + 1 )

#ifndef __ASSEMBLER__

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/sh_regs.h>            // register definitions
#include <cyg/hal/hal_io.h>             // io macros
#include <cyg/infra/cyg_ass.h>          // CYG_FAIL

//--------------------------------------------------------------------------
// Clock control, using TMU counter 0.

#define CYGHWR_SH_RTC_TIMER_IS_TMU

#define HAL_CLOCK_INITIALIZE( _period_ )                                    \
    CYG_MACRO_START                                                         \
    register cyg_uint8 _tstr_;                                              \
                                                                            \
    /* Disable timer while programming it. */                               \
    HAL_READ_UINT8(CYGARC_REG_TSTR, _tstr_);                                \
    _tstr_ &= ~CYGARC_REG_TSTR_STR0;                                        \
    HAL_WRITE_UINT8(CYGARC_REG_TSTR, _tstr_);                               \
                                                                            \
    /* Set counter registers. */                                            \
    HAL_WRITE_UINT32(CYGARC_REG_TCOR0, (_period_));                         \
    HAL_WRITE_UINT32(CYGARC_REG_TCNT0, (_period_));                         \
                                                                            \
    /* Set interrupt on underflow and decrement frequency */                \
    HAL_WRITE_UINT16(CYGARC_REG_TCR0, CYGARC_REG_TCR_UNIE |                 \
                     ((4==CYGHWR_HAL_SH_TMU_PRESCALE_0) ?                   \
                          CYGARC_REG_TCR_TPSC_4 :                           \
                      (16==CYGHWR_HAL_SH_TMU_PRESCALE_0) ?                  \
                          CYGARC_REG_TCR_TPSC_16:                           \
                      (64==CYGHWR_HAL_SH_TMU_PRESCALE_0) ?                  \
                          CYGARC_REG_TCR_TPSC_64:CYGARC_REG_TCR_TPSC_256)); \
                                                                            \
                                                                            \
    /* Enable timer. */                                                     \
    _tstr_ |= CYGARC_REG_TSTR_STR0;                                         \
    HAL_WRITE_UINT8(CYGARC_REG_TSTR, _tstr_);                               \
                                                                            \
    CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )           \
    CYG_MACRO_START                                     \
    register cyg_uint16 _tcr_;                          \
                                                        \
    /* Clear underflow flag. */                         \
    HAL_READ_UINT16(CYGARC_REG_TCR0, _tcr_);            \
    _tcr_ &= ~CYGARC_REG_TCR_UNF;                       \
    HAL_WRITE_UINT16(CYGARC_REG_TCR0, _tcr_);           \
    HAL_READ_UINT16(CYGARC_REG_TCR0, _tcr_);            \
                                                        \
    CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )                              \
    CYG_MACRO_START                                             \
    register cyg_uint32 _result_;                               \
                                                                \
    HAL_READ_UINT32(CYGARC_REG_TCNT0, _result_);                \
                                                                \
    *(_pvalue_) = CYGNUM_KERNEL_COUNTERS_RTC_PERIOD-_result_;   \
    CYG_MACRO_END

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
#define HAL_CLOCK_LATENCY( _pvalue_ ) HAL_CLOCK_READ(_pvalue_)
#endif

externC void hal_delay_us(int);
#define HAL_DELAY_US(n) hal_delay_us(n)

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_EXCEVT_H
// End of hal_intr_excevt.h
