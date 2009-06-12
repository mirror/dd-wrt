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
#  include <cyg/hal/plf_ints.h> // default less-complex platforms
# endif
#endif

// Spurious interrupt (no interrupt source could be found)
#define CYGNUM_HAL_INTERRUPT_NONE -1

//--------------------------------------------------------------------------
// FUJITSU exception vectors.

// The Fujitsu FR-V architecture supports up to 256 interrupt/exceptions.
// Each vectors to a specific VSR which is 16 bytes (4 instructions) long.

// These vectors correspond to VSRs. These values are the ones to use for
// HAL_VSR_GET/SET

#define CYGNUM_HAL_VECTOR_RESET                       0x00
#define CYGNUM_HAL_VECTOR_INSTR_ACCESS_MMU_MISS       0x01
#define CYGNUM_HAL_VECTOR_INSTR_ACCESS_ERROR          0x02
#define CYGNUM_HAL_VECTOR_INSTR_ACCESS_EXCEPTION      0x03
#define CYGNUM_HAL_VECTOR_PRIVELEDGED_INSTRUCTION     0x06
#define CYGNUM_HAL_VECTOR_ILLEGAL_INSTRUCTION         0x07
#define CYGNUM_HAL_VECTOR_REGISTER_EXCEPTION          0x08
#define CYGNUM_HAL_VECTOR_FP_DISABLED                 0x0A
#define CYGNUM_HAL_VECTOR_MP_DISABLED                 0x0B
#define CYGNUM_HAL_VECTOR_FP_EXCEPTION                0x0D
#define CYGNUM_HAL_VECTOR_MP_EXCEPTION                0x0E
#define CYGNUM_HAL_VECTOR_MEMORY_ADDRESS_NOT_ALIGNED  0x10
#define CYGNUM_HAL_VECTOR_DATA_ACCESS_ERROR           0x11
#define CYGNUM_HAL_VECTOR_DATA_ACCESS_MMU_MISS        0x12
#define CYGNUM_HAL_VECTOR_DATA_ACCESS_EXCEPTION       0x13
#define CYGNUM_HAL_VECTOR_DATA_STORE_ERROR            0x14
#define CYGNUM_HAL_VECTOR_DIVISION_EXCEPTION          0x17
#define CYGNUM_HAL_VECTOR_COMMIT_EXCEPTION            0x19
#define CYGNUM_HAL_VECTOR_COMPOUND_EXCEPTION          0x20
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1  0x21
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_2  0x22
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_3  0x23
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_4  0x24
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_5  0x25
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_6  0x26
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_7  0x27
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_8  0x28
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_9  0x29
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_10 0x2A
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_11 0x2B
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_12 0x2C
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_13 0x2D
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_14 0x2E
#define CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15 0x2F
#define CYGNUM_HAL_VECTOR_SYSCALL                     0x80  // tira gr0,#0
#define CYGNUM_HAL_VECTOR_BREAKPOINT_TRAP             0x81  // tira gr0,#1
#define CYGNUM_HAL_VECTOR_BREAKPOINT                  0xFF  // break

#define CYGNUM_HAL_VSR_MIN                     0
#define CYGNUM_HAL_VSR_MAX                   255
#define CYGNUM_HAL_VSR_COUNT                 256
#define CYGNUM_HAL_ISR_COUNT                 256  // 1-1 mapping

// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()

#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION \
          CYGNUM_HAL_VECTOR_ILLEGAL_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_INTERRUPT \
          CYGNUM_HAL_VECTOR_SOFTWARE_INTERRUPT

#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS    CYGNUM_HAL_VECTOR_INSTR_ACCESS_ERROR
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS    CYGNUM_HAL_VECTOR_DATA_ACCESS_ERROR

#define CYGNUM_HAL_EXCEPTION_MIN     CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION
#define CYGNUM_HAL_EXCEPTION_MAX     CYGNUM_HAL_EXCEPTION_DATA_ACCESS
#define CYGNUM_HAL_EXCEPTION_COUNT   (CYGNUM_HAL_EXCEPTION_MAX - \
                                      CYGNUM_HAL_EXCEPTION_MIN + 1)

#define CYGNUM_HAL_ISR_MIN CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1
#define CYGNUM_HAL_ISR_MAX CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15

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

#if CYGINT_HAL_FRV_MEM_REAL_REGION_TOP

externC cyg_uint8 *hal_frv_mem_real_region_top( cyg_uint8 *_regionend_ );
                                                
# define HAL_MEM_REAL_REGION_TOP( _regionend_ ) \
    hal_frv_mem_real_region_top( _regionend_ )
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

externC cyg_uint32 hal_disable_interrupts(void);
externC void       hal_enable_interrupts(void);
externC void       hal_restore_interrupts(cyg_uint32);
externC cyg_uint32 hal_query_interrupts(void);

// On this processor, interrupts are controlled by level.  Since eCos
// only has the notion of "off" and "on", this will be emulated by
// NONE-level and ALL-level.
#define HAL_DISABLE_INTERRUPTS(_old_)           \
CYG_MACRO_START                                 \
    register cyg_uint32 reg;                    \
    asm volatile (                              \
        "movsg psr,%0\n"                        \
        "\tsetlos (0x0F<<3),gr5\n"              \
        "\tor %0,gr5,gr5\n"                     \
        "\tmovgs gr5,psr\n"                     \
        : "=r" (reg)                            \
        :                                       \
        : "gr5" /* Clobber list */              \
        );                                      \
    (_old_) = (reg);                            \
CYG_MACRO_END

#define HAL_ENABLE_INTERRUPTS()                 \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "movsg psr,gr4\n"                       \
        "\tsetlos (0x0F<<3),gr5\n"              \
        "\tnot gr5,gr5\n"                       \
        "\tand gr4,gr5,gr5\n"                   \
        "\tmovgs gr5,psr\n"                     \
        :                                       \
        :                                       \
        : "gr4","gr5" /* Clobber list */        \
        );                                      \
CYG_MACRO_END

// This should work, but breaks compiler
#if 0
#define HAL_RESTORE_INTERRUPTS(_old_)           \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "movsg psr,gr4\n"                       \
        "\tsetlos 1,gr5\n"                      \
        "\tand %0,gr5,gr5\n"                    \
        "\tor gr5,gr4,gr4\n"                    \
        "\tmovgs gr4,psr\n"                     \
        :                                       \
        : "g" (_old_)                           \
        : "gr4","gr5" /* Clobber list */        \
        );                                      \
CYG_MACRO_END
#else
#define HAL_RESTORE_INTERRUPTS(_old_)           \
   hal_restore_interrupts(_old_)
#endif

#define HAL_QUERY_INTERRUPTS(_old_)             \
  _old_ = hal_query_interrupts()

//--------------------------------------------------------------------------
// Routine to execute DSRs using separate interrupt stack

#ifdef  CYGIMP_HAL_COMMON_INTERRUPTS_USE_INTERRUPT_STACK
externC void hal_interrupt_stack_call_pending_DSRs(void);
#define HAL_INTERRUPT_STACK_CALL_PENDING_DSRS() \
    hal_interrupt_stack_call_pending_DSRs()

#if 0 // Interrupt stacks not implemented yet
// these are offered solely for stack usage testing
// if they are not defined, then there is no interrupt stack.
#define HAL_INTERRUPT_STACK_BASE cyg_interrupt_stack_base
#define HAL_INTERRUPT_STACK_TOP  cyg_interrupt_stack
// use them to declare these extern however you want:
//       extern char HAL_INTERRUPT_STACK_BASE[];
//       extern char HAL_INTERRUPT_STACK_TOP[];
// is recommended
#endif // 0
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
