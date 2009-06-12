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
// Contributors: proven, jskov, pjo, nickg, bartv
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

// Interrupt and exception handling in the synthetic target is very much
// tied up with POSIX signal handling.
//
// There are two interrupt sources to consider. The system clock is
// provided most conveniently by a timer signal SIGALRM. All other
// interrupts are handled by communication with the auxiliary program
// and involve SIGIO. The model that is actually presented to
// higher-level code is a single VSR, and 32 ISRs. ISR 0 is
// arbitrarily assigned to the clock. The remaining 31 ISRs correspond
// to devices managed through the auxiliary, effectively providing an
// interrupt controller. A single VSR suffices because it is passed
// the signal number as argument.
//
// Exceptions also correspond to signals, but are not handled through
// the same VSR: slightly different processing is needed for
// interrupts vs. exceptions, for example the latter does not involve
// calling interrupt_end(). The exceptions of interest are SIGILL,
// SIGBUS, SIGFPE, and SIGSEGV. SIGBUS and SIGSEGV are treated as a
// single exception. Obviously there are other signals but they do not
// have obvious meanings in the context of the synthetic target. NOTE:
// SIGSTKFLT may be needed as well at some point.

#define CYGNUM_HAL_INTERRUPT_RTC        0
#define CYGNUM_HAL_ISR_MIN              0
#define CYGNUM_HAL_ISR_MAX              31
#define CYGNUM_HAL_ISR_COUNT            (CYGNUM_HAL_ISR_MAX + 1)

#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION        0
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS                1
#define CYGNUM_HAL_EXCEPTION_FPU                        2

#define CYGNUM_HAL_EXCEPTION_MIN        0
#define CYGNUM_HAL_EXCEPTION_MAX        CYGNUM_HAL_EXCEPTION_FPU
#define CYGNUM_HAL_EXCEPTION_COUNT      (CYGNUM_HAL_EXCEPTION_MAX + 1)

#define CYGNUM_HAL_VECTOR_SIGNAL        0
#define CYGNUM_HAL_VSR_MIN              0
#define CYGNUM_HAL_VSR_MAX              CYGNUM_HAL_VECTOR_SIGNAL
#define CYGNUM_HAL_VSR_COUNT            (CYGNUM_HAL_VSR_MAX + 1)

// These #include's cannot happen until after the above are defined.
// There are dependencies on e.g. CYGNUM_HAL_EXCEPTION_COUNT.
// Basic data types
#include <cyg/infra/cyg_type.h>

// cyg_vector_t etc., supplied either by the kernel or the common HAL
#include <cyg/hal/drv_api.h>


// Nearly all interrupt state control happens via functions. This
// facilitates debugging, for example it is easier to set breakpoints
// that way, at the cost of performance. However performance is not a
// critical issue for the synthetic target, Instead it is intended to
// facilitate application development, and hence debugability has a
// higher priority. There is one exception: the sequence
// disable_interrupts() followed by restore_interrupts() occurs
// frequently and is worth some inlining.
//
// Note: some of the details such as the existence of a global
// variable hal_interrupts_enabled are known to the context switch
// code in the variant HAL.
typedef cyg_bool_t          CYG_INTERRUPT_STATE;
externC volatile cyg_bool_t hal_interrupts_enabled;
externC void                hal_enable_interrupts(void);
externC cyg_bool_t          hal_interrupt_in_use(cyg_vector_t);
externC void                hal_interrupt_attach(cyg_vector_t, cyg_ISR_t*, CYG_ADDRWORD, CYG_ADDRESS);
externC void                hal_interrupt_detach(cyg_vector_t, cyg_ISR_t*);
externC void                (*hal_vsr_get(cyg_vector_t))(void);
externC void                hal_vsr_set(cyg_vector_t, void (*)(void), void (**)(void));
externC void                hal_interrupt_mask(cyg_vector_t);
externC void                hal_interrupt_unmask(cyg_vector_t);
externC void                hal_interrupt_acknowledge(cyg_vector_t);
externC void                hal_interrupt_configure(cyg_vector_t, cyg_bool_t, cyg_bool_t);
externC void                hal_interrupt_set_level(cyg_vector_t, cyg_priority_t);

    
#define HAL_ENABLE_INTERRUPTS()                 \
    CYG_MACRO_START                             \
    hal_enable_interrupts();                    \
    CYG_MACRO_END

#define HAL_DISABLE_INTERRUPTS(_old_)           \
    CYG_MACRO_START                             \
    _old_ = hal_interrupts_enabled;             \
    hal_interrupts_enabled = false;             \
    CYG_MACRO_END

#define HAL_RESTORE_INTERRUPTS(_old_)           \
    CYG_MACRO_START                             \
    if (!_old_) {                               \
        hal_interrupts_enabled = false;         \
    } else if (!hal_interrupts_enabled) {       \
        hal_enable_interrupts();                \
    }                                           \
    CYG_MACRO_END

#define HAL_QUERY_INTERRUPTS(_old_)             \
    CYG_MACRO_START                             \
    _old_ = hal_interrupts_enabled;             \
    CYG_MACRO_END

#define HAL_TRANSLATE_VECTOR(_vector_, _index_) \
    CYG_MACRO_START                             \
    (_index_) = (_vector_);                     \
    CYG_MACRO_END

#define HAL_INTERRUPT_IN_USE(_vector_, _state_)                         \
    CYG_MACRO_START                                                     \
    (_state_) = hal_interrupt_in_use(_vector_);                         \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH(_vector_, _isr_, _data_, _object_ )        \
    CYG_MACRO_START                                                     \
    hal_interrupt_attach(_vector_, _isr_, (CYG_ADDRWORD) _data_, (CYG_ADDRESS) _object_); \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH(_vector_, _isr_)                           \
    CYG_MACRO_START                                                     \
    hal_interrupt_detach(_vector_, _isr_);                              \
    CYG_MACRO_END

#define HAL_VSR_GET(_vector_, _vsr_)                                    \
    CYG_MACRO_START                                                     \
    (*_vsr_) = hal_vsr_get(_vector_);                                   \
    CYG_MACRO_END

#define HAL_VSR_SET(_vector_, _vsr_, _poldvsr_)                         \
    CYG_MACRO_START                                                     \
    hal_vsr_set(_vector_, _vsr_, _poldvsr_);                            \
    CYG_MACRO_END

#define HAL_INTERRUPT_MASK(_vector_)            \
    CYG_MACRO_START                             \
    hal_interrupt_mask(_vector_);               \
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK(_vector_)          \
    CYG_MACRO_START                             \
    hal_interrupt_unmask(_vector_);             \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE(_vector_)     \
    CYG_MACRO_START                             \
    hal_interrupt_acknowledge(_vector_);        \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE(_vector_, _level_, _up_)        \
    CYG_MACRO_START                                             \
    hal_interrupt_configure(_vector_, _level_, _up_);           \
    CYG_MACRO_END

#define HAL_INTERRUPT_SET_LEVEL(_vector_, _level_)              \
    CYG_MACRO_START                                             \
    hal_interrupt_set_level(_vector_, _level_);                 \
    CYG_MACRO_END

// Additional data exported by the synthetic target interrupt handling
// subsystem. These two variables correspond to typical interrupt
// status and mask registers.
extern volatile cyg_uint32   synth_pending_isrs;
extern volatile cyg_uint32   synth_masked_isrs;

// ----------------------------------------------------------------------------
// The clock support
externC void            hal_clock_initialize(cyg_uint32);
externC cyg_uint32      hal_clock_read(void);

#define HAL_CLOCK_INITIALIZE( _period_ )                        \
    CYG_MACRO_START                                             \
    hal_clock_initialize(_period_);                             \
    CYG_MACRO_END

// No special action is needed for reset.
#define HAL_CLOCK_RESET( _vector_, _period_ )                   \
    CYG_EMPTY_STATEMENT

#define HAL_CLOCK_READ(_pvalue_)                                \
    CYG_MACRO_START                                             \
    *(_pvalue_) = hal_clock_read();                             \
    CYG_MACRO_END

// ----------------------------------------------------------------------------
// Resetting the Synth target is not possible, but existing the process is.
externC void            cyg_hal_sys_exit(int); 
#define HAL_PLATFORM_RESET()                                    \
    CYG_MACRO_START                                             \
    cyg_hal_sys_exit(0);                                        \
    CYG_MACRO_END

// ----------------------------------------------------------------------------
// Test case exit support.
#define CYGHWR_TEST_PROGRAM_EXIT()                              \
    CYG_MACRO_START                                             \
    cyg_hal_sys_exit(0);                                        \
    CYG_MACRO_END
//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H
// End of hal_intr.h
