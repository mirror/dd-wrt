//==========================================================================
//
//      hal_intr.h
//
//      HAL Interrupt and clock support
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
// Author(s):    sfurman
// Contributors: 
// Date:         2003-01-24
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               both external interrupts and clock interrupts.
//              
// Usage:
//              #include <cyg/hal/hal_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_HAL_HAL_INTR_H
#define CYGONCE_HAL_HAL_INTR_H

#include <cyg/hal/hal_arch.h>

//--------------------------------------------------------------------------
// OpenRISC vectors. 

// These are the exception/interrupt causes defined by the hardware.
// These values are the ones to use for HAL_VSR_GET/SET

// Reset
#define CYGNUM_HAL_VECTOR_RESET                0x01

// Bus Error - probably invalid physical address
#define CYGNUM_HAL_VECTOR_BUS_ERROR            0x02

// Either no matching page-table entry or protection fault
// while executing load/store operation
#define CYGNUM_HAL_VECTOR_DATA_PAGE_FAULT      0x03

// Either no matching page-table entry or protection fault
// while fetching instruction
#define CYGNUM_HAL_VECTOR_INSTR_PAGE_FAULT     0x04

// Tick Timer interrupt
#define CYGNUM_HAL_VECTOR_TICK_TIMER           0x05

// Unaligned access
#define CYGNUM_HAL_VECTOR_UNALIGNED_ACCESS     0x06

// Illegal instruction
#define CYGNUM_HAL_VECTOR_RESERVED_INSTRUCTION 0x07

// External Interrupt from PIC
#define CYGNUM_HAL_VECTOR_INTERRUPT            0x08

// D-TLB Miss
#define CYGNUM_HAL_VECTOR_DTLB_MISS            0x09

// I-TLB Miss
#define CYGNUM_HAL_VECTOR_ITLB_MISS            0x0A

// Numeric overflow, etc.
#define CYGNUM_HAL_VECTOR_RANGE                0x0B

// System Call
#define CYGNUM_HAL_VECTOR_SYSTEM_CALL          0x0C

// TRAP instruction executed
#define CYGNUM_HAL_VECTOR_TRAP                 0x0E

#define CYGNUM_HAL_VSR_MIN                     CYGNUM_HAL_VECTOR_RESET
#define CYGNUM_HAL_VSR_MAX                     CYGNUM_HAL_VECTOR_TRAP
#define CYGNUM_HAL_VSR_COUNT                   (CYGNUM_HAL_VSR_MAX-CYGNUM_HAL_VSR_MIN+1)

// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS \
          CYGNUM_HAL_VECTOR_DTLB_MISS
#define CYGNUM_HAL_EXCEPTION_DATA_TLBMISS_ACCESS \
          CYGNUM_HAL_VECTOR_DTLB_MISS
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS \
          CYGNUM_HAL_VECTOR_UNALIGNED_ACCESS
#define CYGNUM_HAL_EXCEPTION_SYSTEM_CALL    CYGNUM_HAL_VECTOR_SYSTEM_CALL
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION \
          CYGNUM_HAL_VECTOR_RESERVED_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_OVERFLOW       CYGNUM_HAL_VECTOR_RANGE
#define CYGNUM_HAL_EXCEPTION_INTERRUPT      CYGNUM_HAL_VECTOR_INTERRUPT

// Min/Max exception numbers and how many there are
#define CYGNUM_HAL_EXCEPTION_MIN                CYGNUM_HAL_VSR_MIN
#define CYGNUM_HAL_EXCEPTION_MAX                CYGNUM_HAL_VSR_MAX
#define CYGNUM_HAL_EXCEPTION_COUNT           \
                 ( CYGNUM_HAL_EXCEPTION_MAX - CYGNUM_HAL_EXCEPTION_MIN + 1 )


#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED
#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

// Interrupts 0-31 are connected to the PIC
#define CYGNUM_HAL_INTERRUPT_0                0
#define CYGNUM_HAL_INTERRUPT_1                1
#define CYGNUM_HAL_INTERRUPT_2                2
#define CYGNUM_HAL_INTERRUPT_3                3
#define CYGNUM_HAL_INTERRUPT_4                4
#define CYGNUM_HAL_INTERRUPT_5                5
#define CYGNUM_HAL_INTERRUPT_6                6
#define CYGNUM_HAL_INTERRUPT_7                7
#define CYGNUM_HAL_INTERRUPT_8                8
#define CYGNUM_HAL_INTERRUPT_9                9
#define CYGNUM_HAL_INTERRUPT_10               10
#define CYGNUM_HAL_INTERRUPT_11               11
#define CYGNUM_HAL_INTERRUPT_12               12
#define CYGNUM_HAL_INTERRUPT_13               13
#define CYGNUM_HAL_INTERRUPT_14               14
#define CYGNUM_HAL_INTERRUPT_15               15
#define CYGNUM_HAL_INTERRUPT_16               16
#define CYGNUM_HAL_INTERRUPT_17               17
#define CYGNUM_HAL_INTERRUPT_18               18
#define CYGNUM_HAL_INTERRUPT_19               19
#define CYGNUM_HAL_INTERRUPT_20               20
#define CYGNUM_HAL_INTERRUPT_21               21
#define CYGNUM_HAL_INTERRUPT_22               22
#define CYGNUM_HAL_INTERRUPT_23               23
#define CYGNUM_HAL_INTERRUPT_24               24
#define CYGNUM_HAL_INTERRUPT_25               25
#define CYGNUM_HAL_INTERRUPT_26               26
#define CYGNUM_HAL_INTERRUPT_27               27
#define CYGNUM_HAL_INTERRUPT_28               28
#define CYGNUM_HAL_INTERRUPT_29               29
#define CYGNUM_HAL_INTERRUPT_30               30
#define CYGNUM_HAL_INTERRUPT_31               31

// By SW convention, interrupt #32 is the tick timer
#define CYGNUM_HAL_INTERRUPT_32               32

// The interrupt vector used by the RTC, aka tick timer
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_32

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     0
#define CYGNUM_HAL_ISR_MAX                     32
#define CYGNUM_HAL_ISR_COUNT                   33

#endif

#ifndef __ASSEMBLER__
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_io.h>

#include <cyg/hal/plf_intr.h>

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_MAX+1];

//--------------------------------------------------------------------------
// Default ISR
// The #define is used to test whether this routine exists, and to allow
// us to call it.

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_DEFAULT_ISR hal_default_isr

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//--------------------------------------------------------------------------
// Interrupt control macros
#ifndef CYGHWR_HAL_INTERRUPT_ENABLE_DISABLE_RESTORE_DEFINED

// Clear both tick timer and external interrupts in the Supervisor Register
#define HAL_DISABLE_INTERRUPTS(_old_)                     \
    CYG_MACRO_START                                       \
    _old_ = MFSPR(SPR_SR);                                \
    MTSPR(SPR_SR, _old_ & ~(SPR_SR_IEE|SPR_SR_TEE));      \
    CYG_MACRO_END

// Enable both tick timer and external interrupts in the Supervisor Register
#define HAL_ENABLE_INTERRUPTS()                           \
    MTSPR(SPR_SR, MFSPR(SPR_SR) | (SPR_SR_IEE|SPR_SR_TEE))

// Copy interrupt flags from argument into Supervisor Register
#define HAL_RESTORE_INTERRUPTS(_old_)                     \
    CYG_MACRO_START                                       \
    cyg_uint32 t1,t2;                                     \
    t1 = MFSPR(SPR_SR) & ~(SPR_SR_IEE|SPR_SR_TEE);        \
    t2 = (_old_) & (SPR_SR_IEE|SPR_SR_TEE);               \
    MTSPR(SPR_SR, t1 | t2);                               \
    CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS( _state_ )                   \
    CYG_MACRO_START                                       \
    _state = MFSPR(SPR_SR);                               \
    CYG_MACRO_END

#endif // CYGHWR_HAL_INTERRUPT_ENABLE_DISABLE_RESTORE_DEFINED

//--------------------------------------------------------------------------
// Routine to execute DSRs using separate interrupt stack

#ifdef  CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
externC void hal_interrupt_stack_call_pending_DSRs(void);
#define HAL_INTERRUPT_STACK_CALL_PENDING_DSRS() \
    hal_interrupt_stack_call_pending_DSRs()

// these are offered solely for stack usage testing
// if they are not defined, then there is no interrupt stack.
#define HAL_INTERRUPT_STACK_BASE cyg_interrupt_stack_base
#define HAL_INTERRUPT_STACK_TOP  cyg_interrupt_stack
// use them to declare these extern however you want:
//       extern char HAL_INTERRUPT_STACK_BASE[];
//       extern char HAL_INTERRUPT_STACK_TOP[];
// is recommended
#endif

//--------------------------------------------------------------------------
// Vector translation.
// For chained interrupts we only have a single vector though which all
// are passed. For unchained interrupts we have a vector per interrupt.

#ifndef HAL_TRANSLATE_VECTOR

#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = 0

#else

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = (_vector_)

#endif

#endif

//--------------------------------------------------------------------------
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                          \
    CYG_MACRO_START                                                       \
    cyg_uint32 _index_;                                                   \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                           \
                                                                          \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR ) \
        (_state_) = 0;                                                    \
    else                                                                  \
        (_state_) = 1;                                                    \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )           \
{                                                                           \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                              \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR )   \
    {                                                                       \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)_isr_;               \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD)_data_;                 \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)_object_;             \
    }                                                                       \
}

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                         \
{                                                                       \
    cyg_uint32 _index_;                                                 \
    HAL_TRANSLATE_VECTOR( _vector_, _index_ );                          \
                                                                        \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)_isr_ )         \
    {                                                                   \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)HAL_DEFAULT_ISR; \
        hal_interrupt_data[_index_] = 0;                                \
        hal_interrupt_objects[_index_] = 0;                             \
    }                                                                   \
}

#define HAL_VSR_GET( _vector_, _pvsr_ )                 \
    *(_pvsr_) = (void (*)())hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ ) CYG_MACRO_START         \
    if( (void*)_poldvsr_ != NULL)                                         \
        *(CYG_ADDRESS *)_poldvsr_ = (CYG_ADDRESS)hal_vsr_table[_vector_]; \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;                         \
CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler.  But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon.  This macro undoes that so that eCos handles the
// exception.  So use it with care.

externC void cyg_hal_default_exception_vsr(void);
externC void cyg_hal_default_interrupt_vsr(void);

#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ ) CYG_MACRO_START  \
    HAL_VSR_SET( _vector_, _vector_ == CYGNUM_HAL_VECTOR_INTERRUPT          \
                              ? (CYG_ADDRESS)cyg_hal_default_interrupt_vsr  \
                              : (CYG_ADDRESS)cyg_hal_default_exception_vsr, \
                 _poldvsr_ );                                               \
CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt controller access

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED
#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

// Mask (disable) interrupts from specified source
#define HAL_INTERRUPT_MASK( _vector_ )            \
CYG_MACRO_START                                   \
    int mask;                                     \
    if ((_vector_) == CYGNUM_HAL_INTERRUPT_RTC) { \
        /* The tick timer interrupt isn't */      \
        /* controlled by the PIC; It has its own*/\
        /* enable bit in the SR. */               \
        MTSPR(SPR_SR, MFSPR(SPR_SR)& ~SPR_SR_TEE);\
    } else {                                      \
        mask = ~(1 << (_vector_));                \
        MTSPR(SPR_PICMR, MFSPR(SPR_PICMR)& mask); \
    }                                             \
CYG_MACRO_END

// Allow interrupts from specified source
#define HAL_INTERRUPT_UNMASK( _vector_ )          \
CYG_MACRO_START                                   \
    int bit;                                      \
    if ((_vector_) == CYGNUM_HAL_INTERRUPT_RTC) { \
        /* The tick timer interrupt isn't */      \
        /* controlled by the PIC; It has its own*/\
        /* enable bit in the SR. */               \
        MTSPR(SPR_SR, MFSPR(SPR_SR) | SPR_SR_TEE);\
    } else {                                      \
        bit = (1 << (_vector_));                  \
        MTSPR(SPR_PICMR, MFSPR(SPR_PICMR) | bit); \
    }                                             \
CYG_MACRO_END

// Reset interrupt request in the PIC for specified device
#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )     \
CYG_MACRO_START                                   \
    int mask;                                     \
    if ((_vector_) != CYGNUM_HAL_INTERRUPT_RTC) { \
        mask = ~(1 << (_vector_));                \
        MTSPR(SPR_PICSR, MFSPR(SPR_PICSR) & mask);\
    }                                             \
CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ ) CYG_EMPTY_STATEMENT

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )  CYG_EMPTY_STATEMENT

#endif

//--------------------------------------------------------------------------
// Clock control.

externC CYG_WORD32 cyg_hal_clock_period;
#define CYGHWR_HAL_CLOCK_PERIOD_DEFINED

// Start tick timer interrupts
#define HAL_CLOCK_INITIALIZE( _period_ )        \
CYG_MACRO_START                                 \
{                                               \
    int ttmr_new = _period_ | 0x60000000;       \
    MTSPR(SPR_TTMR, 0);                         \
    MTSPR(SPR_TTCR, 0);                         \
    MTSPR(SPR_TTMR, ttmr_new);                  \
    cyg_hal_clock_period = _period_;            \
}                                               \
CYG_MACRO_END

// Acknowledge clock timer interrupt
#define HAL_CLOCK_RESET( _vector_, _period_ )   \
CYG_MACRO_START                                 \
    int ttmr_new = _period_ | 0x60000000;       \
    MTSPR(SPR_TTMR, ttmr_new);                  \
CYG_MACRO_END

// Read the current value of the tick timer
#define HAL_CLOCK_READ( _pvalue_ )              \
CYG_MACRO_START                                 \
    *(_pvalue_) = MFSPR(SPR_TTCR);              \
CYG_MACRO_END

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && \
    !defined(HAL_CLOCK_LATENCY)
#define HAL_CLOCK_LATENCY( _pvalue_ )                   \
CYG_MACRO_START                                         \
    register CYG_WORD32 _cval_;                         \
    HAL_CLOCK_READ(&_cval_);                            \
    *(_pvalue_) = _cval_ - cyg_hal_clock_period;        \
CYG_MACRO_END
#endif


//--------------------------------------------------------------------------
// Microsecond delay function provided in hal_misc.c
externC void hal_delay_us(int us);

#define HAL_DELAY_US(n)          hal_delay_us(n)

#endif /* #ifndef __ASSEMBLER__ */

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H
// End of hal_intr.h
