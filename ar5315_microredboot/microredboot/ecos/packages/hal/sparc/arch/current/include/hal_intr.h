#ifndef CYGONCE_HAL_INTR_H
#define CYGONCE_HAL_INTR_H

//===========================================================================
//
//      hal_intr.h
//
//      HAL Interrupt and clock support
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg, gthomas, hmt
// Contributors: nickg, gthomas, hmt,
//               jlarmour
// Date:         1999-02-20, 2002-03-08
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
//===========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_sparc.h>

#include <cyg/infra/cyg_type.h>

//---------------------------------------------------------------------------
// SPARC exception vectors.
//
// A note on nomenclature:
//
// SPARC has traps: interrupts are traps, and so are exceptions.
// There are 255 of them in the hardware: this HAL's trampoline code decodes
// them into the 27 listed below as CYGNUM_HAL_VECTOR_xxx.
// They are handled uniformly in the trampoline code in the sense that
// each vector has a VSR which is called in the same way.
// Interrupts (vectors 1-15) have one VSR by default, exceptions (vectors
// 16-26) another.
// The interrupt VSR sets up a C stack and calls the corresponding ISR with
// the required arguments; this ABI is mandated by the kernel.
// The exception VSR sets up a C stack and calls the corresponding XSR
// (just an entry in the ISR[sic] table) with similar arguments, such that
// it (by default) can call the kernel's cyg_hal_deliver_exception().
// 
// So:
// CYGNUM_HAL_VSR_MAX/MIN/COUNT describe the number of VSR entries *and*
// the number of ISR (and associated data) entries (including those which
// are XSRs, just a special case of ISRs).
// CYGNUM_HAL_ISR_MAX/MIN/COUNT describe the number of interrupt sources
// and is used for bounds checking in kernel interrupt objects.
// CYGNUM_HAL_EXCEPTION_MAX/MIN/COUNT describe vector numbers which have
// by default the exception VSR and default XSR installed.


// These correspond to VSRs and the values are the ones to use for
// HAL_VSR_GET/SET

#define CYGNUM_HAL_VECTOR_RESERVED_0       0
#define CYGNUM_HAL_VECTOR_INTERRUPT_1      1 // NB: least important
#define CYGNUM_HAL_VECTOR_INTERRUPT_2      2 // (lowest priority)
#define CYGNUM_HAL_VECTOR_INTERRUPT_3      3
#define CYGNUM_HAL_VECTOR_INTERRUPT_4      4
#define CYGNUM_HAL_VECTOR_INTERRUPT_5      5
#define CYGNUM_HAL_VECTOR_INTERRUPT_6      6
#define CYGNUM_HAL_VECTOR_INTERRUPT_7      7
#define CYGNUM_HAL_VECTOR_INTERRUPT_8      8
#define CYGNUM_HAL_VECTOR_INTERRUPT_9      9
#define CYGNUM_HAL_VECTOR_INTERRUPT_10    10
#define CYGNUM_HAL_VECTOR_INTERRUPT_11    11
#define CYGNUM_HAL_VECTOR_INTERRUPT_12    12
#define CYGNUM_HAL_VECTOR_INTERRUPT_13    13
#define CYGNUM_HAL_VECTOR_INTERRUPT_14    14 // (highest priority)
#define CYGNUM_HAL_VECTOR_INTERRUPT_15    15 // NB: most important (NMI)

#define CYG_VECTOR_IS_INTERRUPT(v)        (15 >= (v))

#define CYGNUM_HAL_VECTOR_USER_TRAP       16 // Ticc instructions
#define CYGNUM_HAL_VECTOR_FETCH_ABORT     17 // trap type 1
#define CYGNUM_HAL_VECTOR_ILLEGAL_OP      18 // trap type 2
#define CYGNUM_HAL_VECTOR_PRIV_OP         19 // tt 3: privileged op
#define CYGNUM_HAL_VECTOR_NOFPCP          20 // tt 4,36: FP or coproc
#define CYGNUM_HAL_VECTOR_RESERVED_1      21 // (not used)
#define CYGNUM_HAL_VECTOR_RESERVED_2      22 // (not used)
#define CYGNUM_HAL_VECTOR_UNALIGNED       23 // tt 7: unaligned memory access
#define CYGNUM_HAL_VECTOR_TT_EIGHT        24 // tt 8: not defined
#define CYGNUM_HAL_VECTOR_DATA_ABORT      25 // tt 9: read/write failed
                                        
#define CYGNUM_HAL_VECTOR_OTHERS          26 // any others

#define CYGNUM_HAL_VSR_MIN                 0
#define CYGNUM_HAL_VSR_MAX                26
#define CYGNUM_HAL_VSR_COUNT              27

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Interrupt vectors. These are the values used with HAL_INTERRUPT_ATTACH()
// et al

#define CYGNUM_HAL_INTERRUPT_RESERVED_0   CYGNUM_HAL_VECTOR_RESERVED_0
#define CYGNUM_HAL_INTERRUPT_1            CYGNUM_HAL_VECTOR_INTERRUPT_1
#define CYGNUM_HAL_INTERRUPT_2            CYGNUM_HAL_VECTOR_INTERRUPT_2
#define CYGNUM_HAL_INTERRUPT_3            CYGNUM_HAL_VECTOR_INTERRUPT_3
#define CYGNUM_HAL_INTERRUPT_4            CYGNUM_HAL_VECTOR_INTERRUPT_4
#define CYGNUM_HAL_INTERRUPT_5            CYGNUM_HAL_VECTOR_INTERRUPT_5
#define CYGNUM_HAL_INTERRUPT_6            CYGNUM_HAL_VECTOR_INTERRUPT_6
#define CYGNUM_HAL_INTERRUPT_7            CYGNUM_HAL_VECTOR_INTERRUPT_7
#define CYGNUM_HAL_INTERRUPT_8            CYGNUM_HAL_VECTOR_INTERRUPT_8
#define CYGNUM_HAL_INTERRUPT_9            CYGNUM_HAL_VECTOR_INTERRUPT_9
#define CYGNUM_HAL_INTERRUPT_10           CYGNUM_HAL_VECTOR_INTERRUPT_10
#define CYGNUM_HAL_INTERRUPT_11           CYGNUM_HAL_VECTOR_INTERRUPT_11
#define CYGNUM_HAL_INTERRUPT_12           CYGNUM_HAL_VECTOR_INTERRUPT_12
#define CYGNUM_HAL_INTERRUPT_13           CYGNUM_HAL_VECTOR_INTERRUPT_13
#define CYGNUM_HAL_INTERRUPT_14           CYGNUM_HAL_VECTOR_INTERRUPT_14
#define CYGNUM_HAL_INTERRUPT_15           CYGNUM_HAL_VECTOR_INTERRUPT_15

#define CYGNUM_HAL_ISR_MIN                 0
#define CYGNUM_HAL_ISR_MAX                15
#define CYGNUM_HAL_ISR_COUNT              16

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Exception vectors. These are the values used when passed out to an
// external exception handler using cyg_hal_deliver_exception()
// They can also be used with HAL_INTERRUPT_ATTACH() et al to install
// different XSRs.

#define CYGNUM_HAL_EXCEPTION_TRAP               CYGNUM_HAL_VECTOR_USER_TRAP
#define CYGNUM_HAL_EXCEPTION_CODE_ACCESS        CYGNUM_HAL_VECTOR_FETCH_ABORT
#define CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION \
                                                CYGNUM_HAL_VECTOR_ILLEGAL_OP
#define CYGNUM_HAL_EXCEPTION_PRIVILEGED_INSTRUCTION \
                                                CYGNUM_HAL_VECTOR_PRIV_OP
#define CYGNUM_HAL_EXCEPTION_FPU_NOT_AVAIL      CYGNUM_HAL_VECTOR_NOFPCP
#define CYGNUM_HAL_EXCEPTION_RESERVED1          CYGNUM_HAL_VECTOR_RESERVED1
#define CYGNUM_HAL_EXCEPTION_RESERVED2          CYGNUM_HAL_VECTOR_RESERVED2
#define CYGNUM_HAL_EXCEPTION_DATA_UNALIGNED_ACCESS \
                                                CYGNUM_HAL_VECTOR_UNALIGNED
#define CYGNUM_HAL_EXCEPTION_TT_EIGHT           CYGNUM_HAL_VECTOR_TT_EIGHT
#define CYGNUM_HAL_EXCEPTION_DATA_ACCESS        CYGNUM_HAL_VECTOR_DATA_ABORT
#define CYGNUM_HAL_EXCEPTION_OTHERS             CYGNUM_HAL_VECTOR_OTHERS


#define CYGNUM_HAL_EXCEPTION_MIN          16
#define CYGNUM_HAL_EXCEPTION_MAX          (16 + 10)
#define CYGNUM_HAL_EXCEPTION_COUNT        (1 + CYGNUM_HAL_EXCEPTION_MAX - \
                                           CYGNUM_HAL_EXCEPTION_MIN)

//---------------------------------------------------------------------------
// (Null) Translation from a wider space of interrupt sources:

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) _index_ = (_vector_)

//---------------------------------------------------------------------------
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
// Static data used by HAL

// VSR table
externC volatile CYG_ADDRESS    hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

// ISR + XSR tables - so VSR count.
externC volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_VSR_COUNT];
externC volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_VSR_COUNT];
externC volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_VSR_COUNT];
// (interrupt_objects only used in the interrupt case _but_ the interrupt
//  attach &co macros write it, so keep it full-sized)

//---------------------------------------------------------------------------
// Default ISRs for exception/interrupt handing.

// note that these have the same ABI apart from the extra SP parameter
// for exceptions.

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
// return code from ISR is passed to interrupt_end() in the kernel.

externC void cyg_hal_exception_handler(CYG_ADDRWORD vector,
                                       CYG_ADDRWORD data,
                                       CYG_ADDRWORD stackpointer);

//---------------------------------------------------------------------------
// Default VSRs for exception/interrupt handing.

// note that these do not have a C ABI as such; they are *vector* service
// routines and are written in assembler.

externC void hal_default_exception_vsr( void );
externC void hal_default_interrupt_vsr( void );

//---------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint32 CYG_INTERRUPT_STATE;

//---------------------------------------------------------------------------
// Interrupt control macros

// THIS ONE IS NOT A STANDARD HAL ENTRY (HAL_DISABLE_TRAPS)
// (so should be unused externally)
#define HAL_DISABLE_TRAPS(_old_)                \
    asm volatile (                              \
        "rd     %%psr, %0;"                     \
        "andn   %0, 0x20, %%l7;"                \
        "wr     %%l7, %%psr;"                   \
        "nop; nop; nop"                         \
        : "=r"(_old_)                           \
        :                                       \
        : "l7"                                  \
        );

// THIS ONE IS NOT A STANDARD HAL ENTRY (HAL_QUERY_TRAPS)
// (so should be unused externally)
#define HAL_QUERY_TRAPS(_old_)                  \
    asm volatile (                              \
        "rd     %%psr, %%l7;"                   \
        "and    %%l7, 0x020, %0"                \
        : "=r"(_old_)                           \
        :                                       \
        : "l7"                                  \
        );

#define HAL_DISABLE_INTERRUPTS(_old_)           \
    asm volatile (                              \
        "rd     %%psr, %0;"                     \
        "or     %0, 0xf00, %%l7;"               \
        "wr     %%l7, %%psr;"                   \
        "nop; nop; nop"                         \
        : "=r"(_old_)                           \
        :                                       \
        : "l7"                                  \
        );

#define HAL_ENABLE_INTERRUPTS()                 \
    asm volatile (                              \
        "rd     %%psr, %%l7;"                   \
        "andn   %%l7, 0xf00, %%l7;"             \
        "or     %%l7, 0x020, %%l7;"             \
        "wr     %%l7, %%psr;"                   \
        "nop; nop; nop"                         \
        :                                       \
        :                                       \
        : "l7"                                  \
        );

#define HAL_RESTORE_INTERRUPTS(_old_)           \
    asm volatile (                              \
        "rd     %%psr, %%l7;"                   \
        "andn   %%l7, 0xf20, %%l7;"             \
        "and    %0 , 0xf20, %%l6;"              \
        "wr     %%l6, %%l7, %%psr;"             \
        "nop; nop; nop"                         \
        :                                       \
        : "r"(_old_)                            \
        : "l6","l7"                             \
        );

#define HAL_QUERY_INTERRUPTS(_old_)             \
    asm volatile (                              \
        "rd     %%psr, %%l7;"                   \
        "and    %%l7, 0xf00, %%l7;"             \
        "xor    %%l7, 0xf00, %0"                \
        : "=r"(_old_)                           \
        :                                       \
        : "l7"                                  \
        );


//---------------------------------------------------------------------------
// Interrupt and VSR attachment macros

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)                             \
    CYG_MACRO_START                                                          \
    cyg_uint32 _index_;                                                      \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);                              \
                                                                             \
    if( (CYG_ADDRESS)hal_default_isr  == hal_interrupt_handlers[_vector_] || \
        (CYG_ADDRESS)cyg_hal_exception_handler ==                            \
        hal_interrupt_handlers[_vector_] ) {                                 \
        (_state_) = 0;                                                       \
    } else {                                                                 \
        (_state_) = 1;                                                       \
    }                                                                        \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )           \
    CYG_MACRO_START                                                         \
    if( (CYG_ADDRESS)hal_default_isr  == hal_interrupt_handlers[_vector_] ||\
        (CYG_ADDRESS)cyg_hal_exception_handler ==                           \
        hal_interrupt_handlers[_vector_] )                                  \
    {                                                                       \
        hal_interrupt_handlers[_vector_] = (CYG_ADDRESS)_isr_;              \
        hal_interrupt_data[_vector_] = (CYG_ADDRWORD) _data_;               \
        hal_interrupt_objects[_vector_] = (CYG_ADDRESS)_object_;            \
    }                                                                       \
CYG_MACRO_END                                                           
                                                                            
#define HAL_INTERRUPT_DETACH( _vector_, _isr_ ) CYG_MACRO_START             \
    if( hal_interrupt_handlers[_vector_] == (CYG_ADDRESS)_isr_ )            \
    {                                                                       \
        hal_interrupt_handlers[_vector_] =                                  \
           (CYG_VECTOR_IS_INTERRUPT( _vector_ )                             \
              ? (CYG_ADDRESS)hal_default_isr                                \
              : (CYG_ADDRESS)cyg_hal_exception_handler);                    \
        hal_interrupt_data[_vector_] = 0;                                   \
        hal_interrupt_objects[_vector_] = 0;                                \
    }                                                                       \
CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                                     \
    *(CYG_ADDRESS *)(_pvsr_) = hal_vsr_table[_vector_];
    

#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ ) CYG_MACRO_START           \
    if( _poldvsr_ != NULL )                                                 \
        *(CYG_ADDRESS *)_poldvsr_ = hal_vsr_table[_vector_];                \
    hal_vsr_table[_vector_] = (CYG_ADDRESS)_vsr_;                           \
CYG_MACRO_END

// This is an ugly name, but what it means is: grab the VSR back to eCos
// internal handling, or if you like, the default handler.  But if
// cooperating with GDB and CygMon, the default behaviour is to pass most
// exceptions to CygMon.  This macro undoes that so that eCos handles the
// exception.  So use it with care.

#define HAL_VSR_SET_TO_ECOS_HANDLER( _vector_, _poldvsr_ ) CYG_MACRO_START  \
    if( _poldvsr_ != NULL )                                                 \
        *(CYG_ADDRESS *)_poldvsr_ = hal_vsr_table[_vector_];                \
    hal_vsr_table[_vector_] = ( CYG_VECTOR_IS_INTERRUPT( _vector_ )         \
                              ? (CYG_ADDRESS)hal_default_interrupt_vsr      \
                              : (CYG_ADDRESS)hal_default_exception_vsr );   \
CYG_MACRO_END



//---------------------------------------------------------------------------

// Which PIC (if any) is available is dependent on the board.
// This sets up that stuff:

#include <cyg/hal/hal_xpic.h>

// Ditto the clock(s)
// This defines all the clock macros the kernel requires:

#include <cyg/hal/hal_clock.h>

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_INTR_H
// End of hal_intr.h
