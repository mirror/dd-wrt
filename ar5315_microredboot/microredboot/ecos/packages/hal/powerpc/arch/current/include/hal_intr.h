#ifndef CYGONCE_HAL_INTR_H
#define CYGONCE_HAL_INTR_H

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
// Copyright (C) 2002 Gary Thomas
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
//               jlarmour
// Date:         1999-02-19
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:
//               #include <cyg/hal/hal_intr.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // types

#include <cyg/hal/ppc_regs.h>           // register definitions

#include <cyg/hal/var_intr.h>           // variant extensions

//--------------------------------------------------------------------------
// PowerPC exception vectors. These correspond to VSRs and are the values
// to use for HAL_VSR_GET/SET

#define CYGNUM_HAL_VECTOR_RESERVED_0        0
#define CYGNUM_HAL_VECTOR_RESET             1
#define CYGNUM_HAL_VECTOR_MACHINE_CHECK     2
#define CYGNUM_HAL_VECTOR_DSI               3
#define CYGNUM_HAL_VECTOR_ISI               4
#define CYGNUM_HAL_VECTOR_INTERRUPT         5
#define CYGNUM_HAL_VECTOR_ALIGNMENT         6
#define CYGNUM_HAL_VECTOR_PROGRAM           7
#define CYGNUM_HAL_VECTOR_FP_UNAVAILABLE    8
#define CYGNUM_HAL_VECTOR_DECREMENTER       9
#define CYGNUM_HAL_VECTOR_RESERVED_A        10
#define CYGNUM_HAL_VECTOR_RESERVED_B        11
#define CYGNUM_HAL_VECTOR_SYSTEM_CALL       12
#define CYGNUM_HAL_VECTOR_TRACE             13
#define CYGNUM_HAL_VECTOR_FP_ASSIST         14

#define CYGNUM_HAL_VSR_MIN                   CYGNUM_HAL_VECTOR_RESERVED_0
#ifndef CYGNUM_HAL_VSR_MAX
# define CYGNUM_HAL_VSR_MAX                  CYGNUM_HAL_VECTOR_FP_ASSIST
#endif
#define CYGNUM_HAL_VSR_COUNT                 ( CYGNUM_HAL_VSR_MAX + 1 )

#ifndef CYG_VECTOR_IS_INTERRUPT
# define CYG_VECTOR_IS_INTERRUPT(v)   \
     (CYGNUM_HAL_VECTOR_INTERRUPT == (v) \
      || CYGNUM_HAL_VECTOR_DECREMENTER == (v))
#endif

// The decoded interrupts.
// Define decrementer as the first interrupt since it is guaranteed to
// be defined on all PowerPCs. External may expand into several interrupts
// depending on interrupt controller capabilities.
#define CYGNUM_HAL_INTERRUPT_DECREMENTER     0
#define CYGNUM_HAL_INTERRUPT_EXTERNAL        1

#define CYGNUM_HAL_ISR_MIN                   CYGNUM_HAL_INTERRUPT_DECREMENTER
#ifndef CYGNUM_HAL_ISR_MAX
# define CYGNUM_HAL_ISR_MAX                  CYGNUM_HAL_INTERRUPT_EXTERNAL
#endif
#define CYGNUM_HAL_ISR_COUNT                 ( CYGNUM_HAL_ISR_MAX + 1 )

#ifndef CYGHWR_HAL_EXCEPTION_VECTORS_DEFINED
// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_RESERVED_0      CYGNUM_HAL_VECTOR_RESERVED_0
#define CYGNUM_HAL_EXCEPTION_MACHINE_CHECK   CYGNUM_HAL_VECTOR_MACHINE_CHECK
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS     CYGNUM_HAL_VECTOR_DSI
#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS     CYGNUM_HAL_VECTOR_ISI
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS  \
           CYGNUM_HAL_VECTOR_ALIGNMENT
#define CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL   CYGNUM_HAL_VECTOR_FP_UNAVAILABLE
#define CYGNUM_HAL_EXCEPTION_RESERVED_A      CYGNUM_HAL_VECTOR_RESERVED_A
#define CYGNUM_HAL_EXCEPTION_RESERVED_B      CYGNUM_HAL_VECTOR_RESERVED_B
#define CYGNUM_HAL_EXCEPTION_SYSTEM_CALL     CYGNUM_HAL_VECTOR_SYSTEM_CALL
#define CYGNUM_HAL_EXCEPTION_TRACE           CYGNUM_HAL_VECTOR_TRACE
#define CYGNUM_HAL_EXCEPTION_FP_ASSIST       CYGNUM_HAL_VECTOR_FP_ASSIST

#define CYGNUM_HAL_EXCEPTION_MIN             CYGNUM_HAL_EXCEPTION_RESERVED_0
#ifndef CYGNUM_HAL_EXCEPTION_MAX
#define CYGNUM_HAL_EXCEPTION_MAX             CYGNUM_HAL_VSR_MAX
#endif

#define CYGHWR_HAL_EXCEPTION_VECTORS_DEFINED

#endif // CYGHWR_HAL_EXCEPTION_VECTORS_DEFINED

// FIXME: This is still rather ugly. Should probably be made variant
//        specific using a decode_hal_exception macro or somesuch.
// decoded exception vectors
#define CYGNUM_HAL_EXCEPTION_TRAP                     (-1)
#define CYGNUM_HAL_EXCEPTION_PRIVILEGED_INSTRUCTION   (-2)
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION      (-3)
#define CYGNUM_HAL_EXCEPTION_FPU                      (-4)

#undef  CYGNUM_HAL_EXCEPTION_MIN
#define CYGNUM_HAL_EXCEPTION_MIN             CYGNUM_HAL_EXCEPTION_FPU


#define CYGNUM_HAL_EXCEPTION_COUNT           \
                 ( CYGNUM_HAL_EXCEPTION_MAX - CYGNUM_HAL_EXCEPTION_MIN + 1 )

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];
// VSR table
externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

//--------------------------------------------------------------------------
// Default ISRs
// The #define is used to test whether this routine exists, and to allow
// us to call it.

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
externC cyg_uint32 hal_default_decrementer_isr(CYG_ADDRWORD vector, 
                                               CYG_ADDRWORD data);

#define HAL_DEFAULT_ISR hal_default_isr

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//--------------------------------------------------------------------------
// Interrupt control macros

#define HAL_DISABLE_INTERRUPTS(_old_)                   \
    CYG_MACRO_START                                     \
    cyg_uint32 tmp1, tmp2;                              \
    asm volatile (                                      \
        "mfmsr  %0;"                                    \
        "mr     %2,%0;"                                 \
        "li     %1,0;"                                  \
        "rlwimi %2,%1,0,16,16;"                         \
        "mtmsr  %2;"                                    \
        : "=r"(_old_), "=r" (tmp1), "=r" (tmp2));       \
    CYG_MACRO_END

#define HAL_ENABLE_INTERRUPTS()         \
    CYG_MACRO_START                     \
    cyg_uint32 tmp1, tmp2;              \
    asm volatile (                      \
        "mfmsr  %0;"                    \
        "ori    %1,%1,0x8000;"          \
        "rlwimi %0,%1,0,16,16;"         \
        "mtmsr  %0;"                    \
        : "=r" (tmp1), "=r" (tmp2));    \
    CYG_MACRO_END

#define HAL_RESTORE_INTERRUPTS(_old_)   \
    CYG_MACRO_START                     \
    cyg_uint32 tmp;                     \
    asm volatile (                      \
        "mfmsr  %0;"                    \
        "rlwimi %0,%1,0,16,16;"         \
        "mtmsr  %0;"                    \
        : "=&r" (tmp)                   \
        : "r" (_old_));                 \
    CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS(_old_)     \
    CYG_MACRO_START                     \
    cyg_uint32 tmp;                     \
    asm volatile (                      \
        "mfmsr  %0;"                    \
        "lis    %1,0;"                  \
        "ori    %1,%1,0x8000;"          \
        "and    %0,%0,%1;"              \
        : "=&r"(_old_), "=r" (tmp));     \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Machine check manipulation
#define HAL_DISABLE_MACHINE_CHECK(_old_)                \
    CYG_MACRO_START                                     \
    cyg_uint32 tmp1, tmp2;                              \
    asm volatile (                                      \
        "mfmsr  %0;"                                    \
        "mr     %2,%0;"                                 \
        "li     %1,0;"                                  \
        "rlwimi %2,%1,0,19,19;"                         \
        "mtmsr  %2;"                                    \
        : "=r"(_old_), "=r" (tmp1), "=r" (tmp2));       \
    CYG_MACRO_END

#define HAL_ENABLE_MACHINE_CHECK()      \
    CYG_MACRO_START                     \
    cyg_uint32 tmp1, tmp2;              \
    asm volatile (                      \
        "mfmsr  %0;"                    \
        "lis    %1,%1,0x0001;"          \
        "rlwimi %0,%1,0,19,19;"         \
        "mtmsr  %0;"                    \
        : "=r" (tmp1), "=r" (tmp2));    \
    CYG_MACRO_END

#define HAL_QUERY_MACHINE_CHECK(_old_)  \
    CYG_MACRO_START                     \
    cyg_uint32 tmp;                     \
    asm volatile (                      \
        "mfmsr  %0;"                    \
        "lis    %1,0x0001;"             \
        "and    %0,%0,%1;"              \
        : "=&r"(_old_), "=r" (tmp));     \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Vector translation.

#ifndef HAL_TRANSLATE_VECTOR
// Basic PowerPC configuration only has two vectors; decrementer and
// external. Isr tables/chaining use same vector decoder.
#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
    (_index_) = (_vector_)
#endif

//--------------------------------------------------------------------------
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK

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
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                             \
    CYG_MACRO_START                                                          \
    cyg_uint32 _index_;                                                      \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                              \
                                                                             \
    if((hal_interrupt_handlers[_index_]                                      \
            == (CYG_ADDRESS)hal_default_decrementer_isr)                     \
       || (hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr)) \
        (_state_) = 0;                                                       \
    else                                                                     \
        (_state_) = 1;                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )            \
    CYG_MACRO_START                                                          \
    cyg_uint32 _index_;                                                      \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                              \
                                                                             \
    if((hal_interrupt_handlers[_index_]                                      \
            == (CYG_ADDRESS)hal_default_decrementer_isr)                     \
       || (hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr)) \
    {                                                                        \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)_isr_;                \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD) _data_;                 \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)_object_;              \
    }                                                                        \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                             \
    CYG_MACRO_START                                                         \
    cyg_uint32 _index_;                                                     \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                             \
                                                                            \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)_isr_ )             \
    {                                                                       \
        if (CYGNUM_HAL_INTERRUPT_DECREMENTER == (_vector_))                 \
            hal_interrupt_handlers[_index_] =                               \
                (CYG_ADDRESS)hal_default_decrementer_isr;                   \
        else                                                                \
            hal_interrupt_handlers[_index_] = (CYG_ADDRESS)hal_default_isr; \
        hal_interrupt_data[_index_] = 0;                                    \
        hal_interrupt_objects[_index_] = 0;                                 \
    }                                                                       \
    CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                                 \
    *(CYG_ADDRESS *)(_pvsr_) = hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ )               \
    CYG_MACRO_START                                             \
    if( _poldvsr_ != NULL )                                     \
        *(CYG_ADDRESS *)_poldvsr_ = hal_vsr_table[_vector_];    \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;               \
    CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler.  But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon.  This macro undoes that so that eCos handles the
// exception.  So use it with care.
externC void cyg_hal_default_interrupt_vsr( void );
externC void cyg_hal_default_exception_vsr( void );
#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ )                    \
    CYG_MACRO_START                                                           \
    if( (void*)_poldvsr_ != (void*)NULL )                                     \
        *(CYG_ADDRESS *)_poldvsr_ = hal_vsr_table[_vector_];                  \
    hal_vsr_table[_vector_] = ( CYG_VECTOR_IS_INTERRUPT( _vector_ )           \
                               ? (CYG_ADDRESS)cyg_hal_default_interrupt_vsr   \
                              : (CYG_ADDRESS)cyg_hal_default_exception_vsr ); \
    CYG_MACRO_END


#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#define HAL_INTERRUPT_MASK( _vector_ )

#define HAL_INTERRUPT_UNMASK( _vector_ )

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#endif

//--------------------------------------------------------------------------
// Clock control

#ifndef CYGHWR_HAL_CLOCK_DEFINED
// Note: variant or platform allowed to override these definitions

#define HAL_CLOCK_INITIALIZE( _period_ )        \
    CYG_MACRO_START                             \
    asm volatile (                              \
        "mtdec %0;"                             \
        :                                       \
        : "r"(_period_)                         \
        );                                      \
    CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ )   \
    CYG_MACRO_START                             \
    cyg_uint32 tmp;                             \
    asm volatile (                              \
        "mfdec  %0;"                            \
        "add.   %0,%0,%1;"                      \
        "bgt    1f;"                            \
        "mr     %0,%1;"                         \
        "1: mtdec %0;"                          \
        : "=&r" (tmp)                           \
        : "r"(_period_)                         \
        : "cc"                                  \
        );                                      \
    CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ )                              \
    CYG_MACRO_START                                             \
    register cyg_uint32 result;                                 \
    asm volatile(                                               \
        "mfdec  %0;"                                            \
        : "=r"(result)                                          \
        );                                                      \
    *(_pvalue_) = CYGNUM_KERNEL_COUNTERS_RTC_PERIOD-result;     \
    CYG_MACRO_END

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
#define HAL_CLOCK_LATENCY( _pvalue_ )                           \
    CYG_MACRO_START                                             \
    register cyg_int32 result;                                  \
    asm volatile(                                               \
        "mfdec  %0;"                                            \
        : "=r"(result)                                          \
        );                                                      \
    /* Pending DEC interrupts cannot be discarded. If dec is */ \
    /* positive it''s because a DEC interrupt occured while  */ \
    /* eCos was getting ready to run. Just return 0 in that  */ \
    /* case.                                                 */ \
    if (result > 0)                                             \
        result = 0;                                             \
    *(_pvalue_) = -result;                                      \
    CYG_MACRO_END
#endif

#ifndef HAL_DELAY_US
externC void hal_delay_us(int);
#define HAL_DELAY_US(n) hal_delay_us(n)
#endif

// The vector used by the Real time clock
#ifndef CYGNUM_HAL_INTERRUPT_RTC
#define CYGNUM_HAL_INTERRUPT_RTC             CYGNUM_HAL_INTERRUPT_DECREMENTER
#endif // CYGNUM_HAL_INTERRUPT_RTC

#endif // CYGHWR_HAL_CLOCK_DEFINED

//--------------------------------------------------------------------------
// Variant functions
externC void hal_variant_IRQ_init(void);

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_H
// End of hal_intr.h
