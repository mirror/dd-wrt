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
// Author(s):    nickg, gthomas
// Contributors: nickg, gthomas,
//               jlarmour
// Date:         1999-02-20
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:        #include <cyg/hal/hal_intr.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

// This is to allow a variant to decide that there is no platform-specific
// interrupts file; and that in turn can be overridden by a platform that
// refines the variant's ideas.
#ifdef    CYGBLD_HAL_PLF_INTS_H
# include CYGBLD_HAL_PLF_INTS_H // should include variant data as required
#else 
# ifdef    CYGBLD_HAL_VAR_INTS_H
#  include CYGBLD_HAL_VAR_INTS_H
# else
#  include <cyg/hal/hal_platform_ints.h> // default less-complex platforms
# endif
#endif

// Spurious interrupt (no interrupt source could be found)
#define CYGNUM_HAL_INTERRUPT_NONE -1

//--------------------------------------------------------------------------
// ARM exception vectors.

// These vectors correspond to VSRs. These values are the ones to use for
// HAL_VSR_GET/SET

#define CYGNUM_HAL_VECTOR_RESET                0
#define CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION    1
#define CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT   2
#define CYGNUM_HAL_VECTOR_ABORT_PREFETCH       3
#define CYGNUM_HAL_VECTOR_ABORT_DATA           4
#define CYGNUM_HAL_VECTOR_reserved             5
#define CYGNUM_HAL_VECTOR_IRQ                  6
#define CYGNUM_HAL_VECTOR_FIQ                  7

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                     7
#define CYGNUM_HAL_VSR_COUNT                   8

// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION \
          CYGNUM_HAL_VECTOR_UNDEF_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_INTERRUPT \
          CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT
#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS    CYGNUM_HAL_VECTOR_ABORT_PREFETCH
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS    CYGNUM_HAL_VECTOR_ABORT_DATA

#define CYGNUM_HAL_EXCEPTION_MIN     CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_MAX     CYGNUM_HAL_EXCEPTION_DATA_ACCESS
#define CYGNUM_HAL_EXCEPTION_COUNT   (CYGNUM_HAL_EXCEPTION_MAX - \
                                      CYGNUM_HAL_EXCEPTION_MIN + 1)

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
externC CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

// Platform setup memory size (0 if unknown by hardware)
externC CYG_ADDRWORD   hal_dram_size;
// what, if anything, this means, is platform dependent:
externC CYG_ADDRWORD   hal_dram_type; 

#if CYGINT_HAL_ARM_MEM_REAL_REGION_TOP

externC cyg_uint8 *hal_arm_mem_real_region_top( cyg_uint8 *_regionend_ );
                                                
# define HAL_MEM_REAL_REGION_TOP( _regionend_ ) \
    hal_arm_mem_real_region_top( _regionend_ )
#endif

//--------------------------------------------------------------------------
// Default ISR
// The #define is used to test whether this routine exists, and to allow
// code outside the HAL to call it.
 
externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_DEFAULT_ISR hal_default_isr

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//--------------------------------------------------------------------------
// Interrupt control macros

#ifndef __thumb__
// Note: This disables both FIQ and IRQ interrupts!
#define HAL_DISABLE_INTERRUPTS(_old_)           \
    asm volatile (                              \
        "mrs %0,cpsr;"                          \
        "mrs r4,cpsr;"                          \
        "orr r4,r4,#0xC0;"                      \
        "msr cpsr,r4"                           \
        : "=r"(_old_)                           \
        :                                       \
        : "r4"                                  \
        );

#define HAL_ENABLE_INTERRUPTS()                 \
    asm volatile (                              \
        "mrs r3,cpsr;"                          \
        "bic r3,r3,#0xC0;"                      \
        "msr cpsr,r3"                           \
        :                                       \
        :                                       \
        : "r3"                                  \
        );

#define HAL_RESTORE_INTERRUPTS(_old_)           \
    asm volatile (                              \
        "mrs r3,cpsr;"                          \
        "and r4,%0,#0xC0;"                      \
        "bic r3,r3,#0xC0;"                      \
        "orr r3,r3,r4;"                         \
        "msr cpsr,r3"                           \
        :                                       \
        : "r"(_old_)                            \
        : "r3", "r4"                            \
        );

#define HAL_QUERY_INTERRUPTS(_old_)             \
    asm volatile (                              \
        "mrs r4,cpsr;"                          \
        "and r4,r4,#0xC0;"                      \
        "eor %0,r4,#0xC0;"                      \
        : "=r"(_old_)                           \
        :                                       \
        : "r4"                                  \
        );
#else // __thumb__

// Thumb mode does not have access to the PSR registers;  

#if 0 // These don't seem to always work
#define HAL_DISABLE_INTERRUPTS(_old_)                   \
    asm volatile (                                      \
        "ldr r4,=10f;"                                  \
        "bx r4;"                                        \
        ".code 32;"                                     \
        "10:;"                                          \
        "mrs %0,cpsr;"                                  \
        "mrs r4,cpsr;"                                  \
        "orr r4,r4,#0xC0;"                              \
        "msr cpsr,r4;"                                  \
        "ldr r4,=10f+1;"                                \
        "bx  r4;"                                       \
        ".code 16;"                                     \
        "10:;"                                          \
        : "=r"(_old_)                                   \
        :                                               \
        : "r4"                                          \
        );

#define HAL_ENABLE_INTERRUPTS()                         \
    asm volatile (                                      \
        "ldr r3,=10f;"                                  \
        "bx r3;"                                        \
        ".code 32;"                                     \
        "10:;"                                          \
        "mrs r3,cpsr;"                                  \
        "bic r3,r3,#0xC0;"                              \
        "msr cpsr,r3;"                                  \
        "ldr r3,=10f+1;"                                \
        "bx  r3;"                                       \
        ".code 16;"                                     \
        "10:;"                                          \
        :                                               \
        :                                               \
        : "r3"                                          \
        );

#define HAL_RESTORE_INTERRUPTS(_old_)                   \
    asm volatile (                                      \
        "ldr r3,=10f;"                                  \
        "bx r3;"                                        \
        ".code 32;"                                     \
        "10:;"                                          \
        "mrs r3,cpsr;"                                  \
        "and r4,%0,#0xC0;"                              \
        "bic r3,r3,#0xC0;"                              \
        "orr r3,r3,r4;"                                 \
        "msr cpsr,r3;"                                  \
        "ldr r3,=10f+1;"                                \
        "bx  r3;"                                       \
        ".code 16;"                                     \
        "10:;"                                          \
        :                                               \
        : "r"(_old_)                                    \
        : "r3", "r4"                                    \
        );

#define HAL_QUERY_INTERRUPTS(_old_)                     \
    asm volatile (                                      \
        "ldr r4,=10f;"                                  \
        "bx r4;"                                        \
        ".code 32;"                                     \
        "10:;"                                          \
        "mrs r4,cpsr;"                                  \
        "and r4,r4,#0xC0;"                              \
        "eor %0,r4,#0xC0;"                              \
        "ldr r4,=10f+1;"                                \
        "bx  r4;"                                       \
        ".code 16;"                                     \
        "10:;"                                          \
        : "=r"(_old_)                                   \
        :                                               \
        : "r4"                                          \
        );
#else

externC cyg_uint32 hal_disable_interrupts(void);
externC void       hal_enable_interrupts(void);
externC void       hal_restore_interrupts(cyg_uint32);
externC cyg_uint32 hal_query_interrupts(void);

#define HAL_DISABLE_INTERRUPTS(_old_)                   \
    _old_ = hal_disable_interrupts();

#define HAL_ENABLE_INTERRUPTS()                         \
    hal_enable_interrupts();

#define HAL_RESTORE_INTERRUPTS(_old_)                   \
    hal_restore_interrupts(_old_);

#define HAL_QUERY_INTERRUPTS(_old_)                     \
    _old_ = hal_query_interrupts();
#endif

#endif // __thumb__

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

#ifndef HAL_TRANSLATE_VECTOR
#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
    (_index_) = (_vector_)
#endif

//--------------------------------------------------------------------------
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                          \
    CYG_MACRO_START                                                       \
    cyg_uint32 _index_;                                                   \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                           \
                                                                          \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr ) \
        (_state_) = 0;                                                    \
    else                                                                  \
        (_state_) = 1;                                                    \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )          \
    CYG_MACRO_START                                                        \
    if( hal_interrupt_handlers[_vector_] == (CYG_ADDRESS)hal_default_isr ) \
    {                                                                      \
        hal_interrupt_handlers[_vector_] = (CYG_ADDRESS)_isr_;             \
        hal_interrupt_data[_vector_] = (CYG_ADDRWORD) _data_;              \
        hal_interrupt_objects[_vector_] = (CYG_ADDRESS)_object_;           \
    }                                                                      \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                            \
    CYG_MACRO_START                                                        \
    if( hal_interrupt_handlers[_vector_] == (CYG_ADDRESS)_isr_ )           \
    {                                                                      \
        hal_interrupt_handlers[_vector_] = (CYG_ADDRESS)hal_default_isr;   \
        hal_interrupt_data[_vector_] = 0;                                  \
        hal_interrupt_objects[_vector_] = 0;                               \
    }                                                                      \
    CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                         \
    *(CYG_ADDRESS *)(_pvsr_) = hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ )               \
    CYG_MACRO_START                                             \
    if( _poldvsr_ != NULL )                                     \
        *(CYG_ADDRESS *)_poldvsr_ = hal_vsr_table[_vector_];    \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;               \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt controller access

externC void hal_interrupt_mask(int);
externC void hal_interrupt_unmask(int);
externC void hal_interrupt_acknowledge(int);
externC void hal_interrupt_configure(int, int, int);
externC void hal_interrupt_set_level(int, int);

#define HAL_INTERRUPT_MASK( _vector_ )                     \
    hal_interrupt_mask( _vector_ ) 
#define HAL_INTERRUPT_UNMASK( _vector_ )                   \
    hal_interrupt_unmask( _vector_ )
#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )              \
    hal_interrupt_acknowledge( _vector_ )
#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ ) \
    hal_interrupt_configure( _vector_, _level_, _up_ )
#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )       \
    hal_interrupt_set_level( _vector_, _level_ )

//--------------------------------------------------------------------------
// Clock control

externC void hal_clock_initialize(cyg_uint32);
externC void hal_clock_read(cyg_uint32 *);
externC void hal_clock_reset(cyg_uint32, cyg_uint32);

#define HAL_CLOCK_INITIALIZE( _period_ )   hal_clock_initialize( _period_ )
#define HAL_CLOCK_RESET( _vec_, _period_ ) hal_clock_reset( _vec_, _period_ )
#define HAL_CLOCK_READ( _pvalue_ )         hal_clock_read( _pvalue_ )
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
# ifndef HAL_CLOCK_LATENCY
#  define HAL_CLOCK_LATENCY( _pvalue_ )    HAL_CLOCK_READ( (cyg_uint32 *)_pvalue_ )
# endif
#endif

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_H
// End of hal_intr.h
