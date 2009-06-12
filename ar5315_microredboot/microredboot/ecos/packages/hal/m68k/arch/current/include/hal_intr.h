#ifndef CYGONCE_HAL_HAL_INTR_H
#define CYGONCE_HAL_HAL_INTR_H

//==========================================================================
//
//      hal_intr.h
//
//      m68k Interrupt and clock support
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

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>

#include <cyg/hal/var_intr.h>

//--------------------------------------------------------------------------
// m68k exception vectors. These correspond to VSRs and are the values
// to use for HAL_VSR_GET/SET
#define CYGNUM_HAL_VECTOR_SSP               0
#define CYGNUM_HAL_VECTOR_RESET             1
#define CYGNUM_HAL_VECTOR_BUSERR            2
#define CYGNUM_HAL_VECTOR_ADDERR            3
#define CYGNUM_HAL_VECTOR_ILLINST           4
#define CYGNUM_HAL_VECTOR_ZERODIV           5
#define CYGNUM_HAL_VECTOR_CHKINST           6
#define CYGNUM_HAL_VECTOR_TRAPVINST         7
#define CYGNUM_HAL_VECTOR_PRIVVIOLATION     8
#define CYGNUM_HAL_VECTOR_TRACE             9
#define CYGNUM_HAL_VECTOR_L1010             10
#define CYGNUM_HAL_VECTOR_L1111             11
#define CYGNUM_HAL_VECTOR_UNINITINT         15
#define CYGNUM_HAL_VECTOR_SPURINT           24
#define CYGNUM_HAL_VECTOR_AUTOVEC1          25
#define CYGNUM_HAL_VECTOR_AUTOVEC2          26
#define CYGNUM_HAL_VECTOR_AUTOVEC3          27
#define CYGNUM_HAL_VECTOR_AUTOVEC4          28
#define CYGNUM_HAL_VECTOR_AUTOVEC5          29
#define CYGNUM_HAL_VECTOR_AUTOVEC6          30
#define CYGNUM_HAL_VECTOR_AUTOVEC7          31
#define CYGNUM_HAL_VECTOR_NMI               CYGNUM_HAL_VECTOR_AUTOVEC7
#define CYGNUM_HAL_VECTOR_TRAPFIRST         32
#define CYGNUM_HAL_VECTOR_NUMTRAPS          16
#define CYGNUM_HAL_VECTOR_TRAPLAST          (CYGNUM_HAL_VECTOR_TRAPFIRST+CYGNUM_HAL_VECTOR_NUMTRAPS-1)
#define CYGNUM_HAL_VECTOR_INTRFIRST         64
#define CYGNUM_HAL_VECTOR_NUMINTRS          192
#define CYGNUM_HAL_VECTOR_INTRLAST          (CYGNUM_HAL_VECTOR_INTRFIRST+CYGNUM_HAL_VECTOR_NUMINTRS-1)

#define CYGNUM_HAL_VSR_MIN                  CYGNUM_HAL_VECTOR_SSP
#define CYGNUM_HAL_VSR_MAX                  CYGNUM_HAL_VECTOR_INTRLAST
#define CYGNUM_HAL_VSR_COUNT                (CYGNUM_HAL_VSR_MAX+1)

//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGNUM_HAL_ISR_MAX
#define CYGNUM_HAL_ISR_MIN                   0
#define CYGNUM_HAL_ISR_MAX                   255
#define CYGNUM_HAL_ISR_COUNT                 (7+CYGNUM_HAL_VECTOR_NUMINTRS)
#endif /* CYGNUM_HAL_ISR_MAX */

#ifndef CYGNUM_HAL_EXCEPTION_COUNT
#define CYGNUM_HAL_EXCEPTION_MIN             CYGNUM_HAL_VECTOR_BUSERR
#define CYGNUM_HAL_EXCEPTION_MAX             CYGNUM_HAL_VECTOR_SPURINT
#define CYGNUM_HAL_EXCEPTION_COUNT           ((CYGNUM_HAL_EXCEPTION_MAX-CYGNUM_HAL_EXCEPTION_MIN)+1)
#endif /* CYGNUM_HAL_EXCEPTION_COUNT */

//--------------------------------------------------------------------------
// Static data used by HAL

// ISR tables
externC volatile CYG_ADDRESS  cyg_hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRWORD cyg_hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
externC volatile CYG_ADDRESS  cyg_hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

// VSR table
externC volatile CYG_ADDRESS  cyg_hal_vsr_table[CYGNUM_HAL_VSR_COUNT];

//---------------------------------------------------------------------------
// Translate a vector number into an ISR table index.

#define HAL_TRANSLATE_VECTOR(_vector_,_index_)                              \
    CYG_MACRO_START                                                         \
    switch ((_vector_))                                                     \
    {                                                                       \
    case CYGNUM_HAL_VECTOR_AUTOVEC1 ... CYGNUM_HAL_VECTOR_AUTOVEC7:         \
        (_index_) = ((_vector_) - CYGNUM_HAL_VECTOR_AUTOVEC1);              \
        break;                                                              \
    case CYGNUM_HAL_VECTOR_INTRFIRST ... CYGNUM_HAL_VECTOR_INTRLAST:        \
        (_index_) = ((_vector_)                                             \
                     - CYGNUM_HAL_VECTOR_INTRFIRST                          \
                     + (CYGNUM_HAL_VECTOR_AUTOVEC7                          \
                        - CYGNUM_HAL_VECTOR_AUTOVEC1                        \
                        + 1));                                              \
        break;                                                              \
    default:                                                                \
        CYG_FAIL("Unknown Interrupt!!!");                                   \
        (_index_) = (typeof(_index_))-1;                                                     \
    }                                                                       \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt state storage

typedef cyg_uint16 CYG_INTERRUPT_STATE;

//---------------------------------------------------------------------------
// Interrupt and VSR attachment macros

externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
externC void hal_default_exception_handler(CYG_WORD vector,
                                           HAL_SavedRegisters *regs);

#define HAL_INTERRUPT_IN_USE( _vector_, _state_)        \
    CYG_MACRO_START                                     \
    cyg_uint32 _index_;                                 \
    HAL_TRANSLATE_VECTOR ((_vector_), _index_);         \
                                                        \
    if (cyg_hal_interrupt_handlers[_index_]             \
        ==(CYG_ADDRESS)&hal_default_isr)                \
        (_state_) = 0;                                  \
    else                                                \
        (_state_) = 1;                                  \
    CYG_MACRO_END

#define HAL_INTERRUPT_ATTACH( _vector_, _isr_, _data_, _object_ )       \
    CYG_MACRO_START                                                     \
    cyg_uint32 _index_;                                                 \
    HAL_TRANSLATE_VECTOR((_vector_), _index_);                          \
                                                                        \
    if (cyg_hal_interrupt_handlers[_index_]                             \
        ==(CYG_ADDRESS)&hal_default_isr)                                \
    {                                                                   \
        cyg_hal_interrupt_handlers[_index_] = (CYG_ADDRESS)(_isr_);     \
        cyg_hal_interrupt_data[_index_] = (CYG_ADDRWORD)(_data_);       \
        cyg_hal_interrupt_objects[_index_] = (CYG_ADDRESS)(_object_);   \
    }                                                                   \
    CYG_MACRO_END

#define HAL_INTERRUPT_DETACH( _vector_, _isr_ ) \
CYG_MACRO_START \
    cyg_uint32 _index_; \
    HAL_INTERRUPT_MASK(_vector_); \
    HAL_TRANSLATE_VECTOR((_vector_), _index_); \
    if (cyg_hal_interrupt_handlers[_index_] \
        == (CYG_ADDRESS)(_isr_)) \
    { \
        cyg_hal_interrupt_handlers[_index_] = \
            (CYG_ADDRESS)&hal_default_isr; \
        cyg_hal_interrupt_data[_index_] = 0; \
        cyg_hal_interrupt_objects[_index_] = 0; \
    } \
CYG_MACRO_END

#define HAL_VSR_GET( _vector_, _pvsr_ )                         \
    *((CYG_ADDRESS *)(_pvsr_)) = cyg_hal_vsr_table[(_vector_)];


#define HAL_VSR_SET( _vector_, _vsr_, _poldvsr_ )                       \
    CYG_MACRO_START                                                     \
    if( (_poldvsr_) != NULL )                                           \
        *(CYG_ADDRESS *)(_poldvsr_) = cyg_hal_vsr_table[(_vector_)];    \
    cyg_hal_vsr_table[(_vector_)] = (CYG_ADDRESS)(_vsr_);               \
    CYG_MACRO_END

//--------------------------------------------------------------------------
// Interrupt control macros

//      The following interrupt control  macros are  the default  for the  68k
// architecture.   Some  architectures  will  override  these  definitions  by
// defining  them  in  their  var_intr.h  file.   Some  architectures  support
// instructions  like  andi.w  #xxxx,%sr   but  others   (Coldfire)  do   not.
// Architectures that support  these other  instructions will  want to  define
// their own macros.

#define HAL_M68K_SET_SR(__newsr__)                                          \
    CYG_MACRO_START                                                         \
    asm volatile ("move.w   %0,%%sr\n"                                      \
                  :                                                         \
                  : "g" ((CYG_INTERRUPT_STATE)(__newsr__)));                \
    CYG_MACRO_END

#ifndef HAL_ENABLE_INTERRUPTS
#define HAL_ENABLE_INTERRUPTS()                                             \
    CYG_MACRO_START                                                         \
    CYG_INTERRUPT_STATE _msk_;                                              \
    HAL_QUERY_INTERRUPTS(_msk_);                                            \
    HAL_M68K_SET_SR((_msk_ & (CYG_INTERRUPT_STATE)0xf8ff));                 \
    CYG_MACRO_END
#endif // HAL_ENABLE_INTERRUPTS

#ifndef HAL_DISABLE_INTERRUPTS
#define HAL_DISABLE_INTERRUPTS(_old_)                                       \
    CYG_MACRO_START                                                         \
    HAL_QUERY_INTERRUPTS(_old_);                                            \
    HAL_M68K_SET_SR((_old_ | (CYG_INTERRUPT_STATE)0x0700));                 \
    CYG_MACRO_END
#endif //HAL_DISABLE_INTERRUPTS

#define HAL_RESTORE_INTERRUPTS(_prev_)                                      \
    CYG_MACRO_START                                                         \
    CYG_INTERRUPT_STATE _msk_;                                              \
    HAL_QUERY_INTERRUPTS(_msk_);                                            \
    _msk_ &= (CYG_INTERRUPT_STATE)0xf8ff;                                   \
    _msk_ |= (((CYG_INTERRUPT_STATE)(_prev_))                               \
              & (CYG_INTERRUPT_STATE)0x0700);                               \
    asm volatile ("move.w   %0,%%sr\n"                                      \
                  :                                                         \
                  : "g" (_msk_));                                           \
    CYG_MACRO_END

// Use the extra assignment to avoid warnings.
// The compiler should optimize it out.
#define HAL_QUERY_INTERRUPTS(__oldmask__)                                   \
    CYG_MACRO_START                                                         \
    CYG_INTERRUPT_STATE _omsk_ = (CYG_INTERRUPT_STATE)(__oldmask__);        \
    asm volatile ("move.w   %%sr,%0\n"                                      \
                  : "=g" (_omsk_)                                           \
                  : );                                                      \
    (__oldmask__) = (typeof(__oldmask__))_omsk_;                            \
    CYG_MACRO_END

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_HAL_INTR_H

