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
// Author(s):    jskov
// Contributors: jskov,
// Date:         1999-04-24
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

#include <cyg/hal/var_intr.h>

// More include statements below. First part of this file must be
// usable for both assembly and C files, so only use defines here.

#include CYGBLD_HAL_VAR_INTR_MODEL_H

#ifndef __ASSEMBLER__

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/sh_regs.h>            // register definitions
#include <cyg/hal/hal_io.h>             // io macros
#include <cyg/infra/cyg_ass.h>          // CYG_FAIL

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];
// VSR table
externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

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
//
// Note that these macros control interrupt state by setting the Imask
// of the SR rather than the (more obvious) BL. This is because a CPU
// reset is forced if execptions (such as breakpoints) are generated
// while the BL flag is set.

#define HAL_DISABLE_INTERRUPTS(_old_)           \
    CYG_MACRO_START                             \
    cyg_uint32 _tmp_;                           \
    asm volatile (                              \
        "stc    sr,%1    \n\t"                  \
        "mov    %2,%0    \n\t"                  \
        "and    %1,%0    \n\t"                  \
        "or     %2,%1    \n\t"                  \
        "ldc    %1,sr    \n\t"                  \
        : "=&r"(_old_), "=&r" (_tmp_)           \
        : "r" (CYGARC_REG_SR_IMASK)             \
        );                                      \
    CYG_MACRO_END

#define HAL_ENABLE_INTERRUPTS()                 \
    CYG_MACRO_START                             \
    cyg_uint32 _tmp_;                           \
    asm volatile (                              \
        "stc    sr,%0    \n\t"                  \
        "and    %1,%0    \n\t"                  \
        "ldc    %0,sr    \n\t"                  \
        : "=&r" (_tmp_)                         \
        : "r" (~CYGARC_REG_SR_IMASK)            \
        );                                      \
    CYG_MACRO_END

#define HAL_RESTORE_INTERRUPTS(_old_)                   \
    CYG_MACRO_START                                     \
    cyg_uint32 _tmp1_, _tmp2_;                          \
    asm volatile (                                      \
        "stc    sr,%0    \n\t"                          \
        "and    %3,%0    \n\t"                          \
        "not    %3,%1    \n\t"                          \
        "and    %2,%1    \n\t"                          \
        "or     %1,%0    \n\t"                          \
        "ldc    %0,sr    \n\t"                          \
        : "=&r" (_tmp1_), "=&r" (_tmp2_)                \
        : "r" (_old_), "r" (~CYGARC_REG_SR_IMASK)       \
        );                                              \
    CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS(_old_)             \
    CYG_MACRO_START                             \
    asm volatile (                              \
        "stc    sr,%0    \n\t"                  \
        "and    %1,%0    \n\t"                  \
        : "=&r"(_old_)                          \
        : "r" (CYGARC_REG_SR_IMASK)             \
        );                                      \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Vector translation.

#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

# define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = 0

#else

# define HAL_TRANSLATE_VECTOR(_vector_,_index_) (_index_) = (_vector_)

#endif

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

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )         \
    CYG_MACRO_START                                                       \
    cyg_uint32 _index_;                                                   \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                           \
                                                                          \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)hal_default_isr ) \
    {                                                                     \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)_isr_;             \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD) _data_;              \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)_object_;           \
    }                                                                     \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ )                         \
    CYG_MACRO_START                                                     \
    cyg_uint32 _index_;                                                 \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                         \
                                                                        \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)_isr_ )         \
    {                                                                   \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)hal_default_isr; \
        hal_interrupt_data[_index_] = 0;                                \
        hal_interrupt_objects[_index_] = 0;                             \
    }                                                                   \
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
                              ? (CYG_ADDRESS)cyg_hal_default_interrupt_vsr    \
                              : (CYG_ADDRESS)cyg_hal_default_exception_vsr ); \
    CYG_MACRO_END


//--------------------------------------------------------------------------
// Interrupt controller(s) access
//

externC void hal_interrupt_set_level(int, int);
#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )                    \
    hal_interrupt_set_level(_vector_, _level_);

externC void hal_interrupt_mask(int);
#define HAL_INTERRUPT_MASK( _vector_ )                                  \
    hal_interrupt_mask(_vector_);

externC void hal_interrupt_unmask(int);
#define HAL_INTERRUPT_UNMASK( _vector_ )                                \
    hal_interrupt_unmask(_vector_);

externC void hal_interrupt_acknowledge(int);
#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                           \
    hal_interrupt_acknowledge(_vector_);

externC void hal_interrupt_configure(int, int, int);
#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )              \
    hal_interrupt_configure(_vector_, _level_, _up_);

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_H
// End of hal_intr.h
