#ifndef CYGONCE_HAL_HAL_INTR_H
#define CYGONCE_HAL_HAL_INTR_H

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
// Author(s):    proven
// Contributors: proven, jskov, pjo, nickg
// Date:         1999-02-20
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock.
//              
// Usage:
//               #include <cyg/hal/hal_intr.h>
//               ...
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_i386.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/var_intr.h>

//--------------------------------------------------------------------------
// Exception vectors.
// Standard exception vectors supported by most IA32 CPUs

#define CYGNUM_HAL_VECTOR_DIV0                    0
#define CYGNUM_HAL_VECTOR_DEBUG                   1
#define CYGNUM_HAL_VECTOR_NMI                     2
#define CYGNUM_HAL_VECTOR_BREAKPOINT              3
#define CYGNUM_HAL_VECTOR_OVERFLOW                4
#define CYGNUM_HAL_VECTOR_BOUND                   5
#define CYGNUM_HAL_VECTOR_OPCODE                  6
#define CYGNUM_HAL_VECTOR_NO_DEVICE               7
#define CYGNUM_HAL_VECTOR_DOUBLE_FAULT            8
#define CYGNUM_HAL_VECTOR_INVALID_TSS            10
#define CYGNUM_HAL_VECTOR_SEGV                   11
#define CYGNUM_HAL_VECTOR_STACK_FAULT            12
#define CYGNUM_HAL_VECTOR_PROTECTION             13
#define CYGNUM_HAL_VECTOR_PAGE                   14
#define CYGNUM_HAL_VECTOR_FPE                    16
#define CYGNUM_HAL_VECTOR_ALIGNMENT              17

// The default size of the VSR table is 256 entries.
#ifndef CYGNUM_HAL_VSR_MIN
#define CYGNUM_HAL_VSR_MIN                        0
#define CYGNUM_HAL_VSR_MAX                       255
#define CYGNUM_HAL_VSR_COUNT                     256
#endif

// Common exception vectors.
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION CYGNUM_HAL_VECTOR_OPCODE
#define CYGNUM_HAL_EXCEPTION_INTERRUPT           CYGNUM_HAL_VECTOR_BREAKPOINT
#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS         CYGNUM_HAL_VECTOR_PROTECTION
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS         CYGNUM_HAL_VECTOR_PROTECTION
#define CYGNUM_HAL_EXCEPTION_TRAP                CYGNUM_HAL_VECTOR_DEBUG
#define CYGNUM_HAL_EXCEPTION_FPU                 CYGNUM_HAL_VECTOR_FPE
#define CYGNUM_HAL_EXCEPTION_STACK_OVERFLOW      CYGNUM_HAL_VECTOR_SEGV
#define CYGNUM_HAL_EXCEPTION_DIV_BY_ZERO         CYGNUM_HAL_VECTOR_DIV0
#define CYGNUM_HAL_EXCEPTION_OVERFLOW            CYGNUM_HAL_VECTOR_OVERFLOW

#define CYGNUM_HAL_EXCEPTION_MIN                 0
#define CYGNUM_HAL_EXCEPTION_MAX                 31
#define CYGNUM_HAL_EXCEPTION_COUNT (CYGNUM_HAL_EXCEPTION_MAX - CYGNUM_HAL_EXCEPTION_MIN + 1)

// These really are wild guesses on my part...
#define CYGNUM_HAL_VECTOR_SIGBUS                CYGNUM_HAL_EXCEPTION_DATA_ACCESS
#define CYGNUM_HAL_VECTOR_SIGFPE                CYGNUM_HAL_EXCEPTION_FPU
#define CYGNUM_HAL_VECTOR_SIGSEGV               CYGNUM_HAL_VECTOR_SEGV

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS  hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS  hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
externC volatile CYG_ADDRESS  hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//---------------------------------------------------------------------------
// Default ISR

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_DEFAULT_ISR hal_default_isr

//--------------------------------------------------------------------------
// CPU interrupt enable/disable macros

#define HAL_ENABLE_INTERRUPTS()                 \
CYG_MACRO_START                                 \
    asm ("sti") ;                               \
CYG_MACRO_END

#define HAL_DISABLE_INTERRUPTS(_old_)           \
CYG_MACRO_START                                 \
    register int x ;                            \
    asm volatile (                              \
        "pushfl ;"                              \
        "popl %0 ;"                             \
        "cli"                                   \
        :	"=r" (x)                        \
        ) ;                                     \
    (_old_) = (x & 0x200);                      \
CYG_MACRO_END

#define HAL_RESTORE_INTERRUPTS(_old_)           \
CYG_MACRO_START                                 \
    register int x = _old_;                     \
    asm volatile ( "pushfl ;"                   \
                   "popl %%eax ;"               \
                   "andl $0xFFFFFDFF,%%eax;"    \
                   "orl  %0,%%eax;"             \
                   "pushl %%eax;"               \
                   "popfl ;"                    \
                   : /* No outputs */           \
                   : "r"(x)                     \
                   : "eax"                      \
                 );                             \
CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS(_old_)             \
CYG_MACRO_START                                 \
    register int x ;                            \
    asm volatile ("pushfl ;"                    \
         "popl %0"                              \
         :	"=r" (x)                        \
        );                                      \
    (_old_) = (x & 0x200);                      \
CYG_MACRO_END


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

//---------------------------------------------------------------------------
// Interrupt and VSR attachment macros


#define HAL_INTERRUPT_IN_USE( _vector_, _state_)        \
    CYG_MACRO_START                                     \
    cyg_uint32 _index_;                                 \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);         \
                                                        \
    if (hal_interrupt_handlers[_index_]                 \
        ==(CYG_ADDRESS)HAL_DEFAULT_ISR)                 \
        (_state_) = 0;                                  \
    else                                                \
        (_state_) = 1;                                  \
    CYG_MACRO_END

#ifndef HAL_INTERRUPT_ATTACH
externC void __default_interrupt_vsr(void);
#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )               \
    CYG_MACRO_START                                                             \
    cyg_uint32 _index_;                                                         \
    HAL_TRANSLATE_VECTOR((_vector_), _index_);                                  \
                                                                                \
    HAL_VSR_SET( _vector_, &__default_interrupt_vsr , NULL);                    \
    if( hal_interrupt_handlers[_index_] == (CYG_ADDRESS)HAL_DEFAULT_ISR )       \
    {                                                                           \
        hal_interrupt_handlers[_index_] = (CYG_ADDRESS)(_isr_);                 \
        hal_interrupt_data[_index_] = (CYG_ADDRWORD)(_data_);                   \
        hal_interrupt_objects[_index_] = (CYG_ADDRESS)(_object_);               \
    }                                                                           \
    CYG_MACRO_END
#endif /* HAL_INTERRUPT_ATTACH */

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ ) \
    CYG_MACRO_START                             \
    cyg_uint32 _index_;                         \
    HAL_TRANSLATE_VECTOR((_vector_), _index_);  \
                                                \
    if (hal_interrupt_handlers[_index_]         \
        == (CYG_ADDRESS)(_isr_))                \
    {                                           \
        hal_interrupt_handlers[_index_] =       \
            (CYG_ADDRESS)HAL_DEFAULT_ISR;       \
        hal_interrupt_data[_index_] = 0;        \
        hal_interrupt_objects[_index_] = 0;     \
    }                                           \
    CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                         \
    *((CYG_ADDRESS *)(_pvsr_)) = hal_vsr_table[(_vector_)];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ )               \
    CYG_MACRO_START                                             \
    CYG_ADDRESS *__poldvsr = (CYG_ADDRESS *)(_poldvsr_);        \
    if( __poldvsr != NULL )                                     \
        *__poldvsr = hal_vsr_table[(_vector_)];                 \
    hal_vsr_table[(_vector_)] = (CYG_ADDRESS)(_vsr_);           \
    CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler.  But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon.  This macro undoes that so that eCos handles the
// exception.  So use it with care.

externC void __default_exception_vsr(void);
externC void __default_interrupt_vsr(void);

#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ )                  \
CYG_MACRO_START                                                             \
    HAL_VSR_SET( _vector_, _vector_ > CYGNUM_HAL_EXCEPTION_MAX              \
                              ? (CYG_ADDRESS)__default_interrupt_vsr        \
                                : (CYG_ADDRESS)__default_exception_vsr,     \
                 _poldvsr_ );                                               \
CYG_MACRO_END

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H
// End of hal_intr.h
