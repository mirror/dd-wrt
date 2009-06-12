#ifndef CYGONCE_HAL_PROC_INTR_H
#define CYGONCE_HAL_PROC_INTR_H

//==========================================================================
//
//      proc_intr.h
//
//      mcf5272 Processor variant interrupt and clock support
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

// Include any platform specific interrupt definitions.
#include <cyg/hal/plf_intr.h>

// Include for the SIM address (MCF5272_SIM).
#include <cyg/hal/proc_arch.h>

//---------------------------------------------------------------------------
// Interrupt controller management

//      This chip has a programmable interrupt vector base which is  different
// from the vector base  register (VBR).   All interrupts  from the  interrupt
// controller are offsets from  the  programmable  interrupt  vector  register
// (PIVR).

#define HAL_PROG_INT_VEC_BASE 64

// Vector numbers defined by the interrupt controller.
// These are all relative to the interrupt vector base number.
#define CYGNUM_HAL_VECTOR_USR_SPUR_INT  (0 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT1       (1 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT2       (2 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT3       (3 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT4       (4 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_TMR1          (5 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_TMR2          (6 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_TMR3          (7 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_TMR4          (8 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_UART1         (9 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_UART2         (10 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_PLIP          (11 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_PLIA          (12 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB0          (13 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB1          (14 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB2          (15 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB3          (16 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB4          (17 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB5          (18 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB6          (19 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_USB7          (20 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_DMA           (21 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_ERX           (22 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_ETX           (23 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_ENTC          (24 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_QSPI          (25 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT5       (26 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_EXTINT6       (27 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_SWTO          (28 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_RES1          (29 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_RES2          (30 + HAL_PROG_INT_VEC_BASE)
#define CYGNUM_HAL_VECTOR_RES3          (31 + HAL_PROG_INT_VEC_BASE)

//---------------------------------------------------------------------------
// Interrupt controller macros.

//      Declare a mirror copy of the  interrupt control registers used to  set
// interrupt priorities.  In order to mask and unmask a specific interrupt, we
// must be able to set its priority  to  zero  and  then  restore  it  to  ist
// original priority.  We use  these  locations  to  determine  the  level  to
// restore the interrupt to in the unmask macro.

externC cyg_uint32 hal_icr_pri_mirror[4];

//      Block the interrupt associated with the given vector.  To do this,  we
// set the interrupt priority level to  zero for the specified interrupt.   To
// set the interrupt priority level,  we  simultaneously  write  a  1  to  the
// pending interrupt field.  The other interrupts are unaffected.  Disable all
// interrupts while we access the hardware registers.

#define HAL_INTERRUPT_MASK( _vector_ ) \
CYG_MACRO_START \
    cyg_uint32 _vec_offset = (_vector_) - HAL_PROG_INT_VEC_BASE - 1; \
    cyg_uint32 _icr = _vec_offset / 8; \
    cyg_uint32 _icr_msk = 0xf0000000 >> ((_vec_offset % 8) * 4); \
    CYG_INTERRUPT_STATE _intr_state; \
    HAL_DISABLE_INTERRUPTS(_intr_state); \
    MCF5272_SIM->intc.icr[_icr] &= _icr_msk ^ 0x77777777; \
    HAL_RESTORE_INTERRUPTS(_intr_state); \
CYG_MACRO_END

//      Unblock the interrupt associated  with  the  given  vector.   Set  the
// interrupt priority using the value  from the icr mirror variable.   Disable
// all interrupts while we access the hardware registers.

#define HAL_INTERRUPT_UNMASK( _vector_ ) \
CYG_MACRO_START \
    cyg_uint32 _vec_offset = (_vector_) - HAL_PROG_INT_VEC_BASE - 1; \
    cyg_uint32 _icr = _vec_offset / 8; \
    cyg_uint32 _icr_msk_offset = ((8-1)*4) - (_vec_offset % 8) * 4; \
    cyg_uint32 _icr_msk = 0x0F << (_icr_msk_offset); \
    cyg_uint32 _icr_val; \
    CYG_INTERRUPT_STATE _intr_state; \
    HAL_DISABLE_INTERRUPTS(_intr_state); \
    _icr_val = MCF5272_SIM->intc.icr[_icr] & 0x77777777 & ~_icr_msk; \
    _icr_val |= hal_icr_pri_mirror[_icr] & _icr_msk; \
    _icr_val |= 0x08 << _icr_msk_offset; \
    MCF5272_SIM->intc.icr[_icr] = _icr_val; \
    HAL_RESTORE_INTERRUPTS(_intr_state); \
CYG_MACRO_END

//      Acknowledge  the  interrupt  by  writing  a  1  to  the  corresponding
// interrupt pending bit.  Write 0 to all other interrupt pending bits.  Leave
// all priority levels unchanged.  Disable all interrupts while we access  the
// hardware registers.

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ ) \
CYG_MACRO_START \
    cyg_uint32 _vec_offset = (_vector_) - HAL_PROG_INT_VEC_BASE - 1; \
    cyg_uint32 _icr = _vec_offset / 8; \
    cyg_uint32 _icr_msk = 0x80000000 >> ((_vec_offset % 8) * 4); \
    CYG_INTERRUPT_STATE _intr_state; \
    HAL_DISABLE_INTERRUPTS(_intr_state); \
    MCF5272_SIM->intc.icr[_icr] &= _icr_msk | 0x77777777; \
    HAL_RESTORE_INTERRUPTS(_intr_state); \
CYG_MACRO_END

//      Set the priority in the interrupt control register and the icr mirror.
// Do not copy the icr  mirror into  the icr  because some  interrupts may  be
// masked.  Disable all interrupts while we access the hardware registers.

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _prilevel_ ) \
CYG_MACRO_START \
    cyg_uint32 _vec_offset = (_vector_) - HAL_PROG_INT_VEC_BASE - 1; \
    cyg_uint32 _icr = _vec_offset / 8; \
    cyg_uint32 _icr_msk_offset = ((8-1)*4) - (_vec_offset % 8) * 4; \
    cyg_uint32 _icr_msk = 0x0F << (_icr_msk_offset); \
    cyg_uint32 _icr_val = (0x08 | (_prilevel_ & 0x07)) << _icr_msk_offset; \
    CYG_INTERRUPT_STATE _intr_state; \
    HAL_DISABLE_INTERRUPTS(_intr_state); \
    cyg_uint32 _mir_val = hal_icr_pri_mirror[_icr] & 0x77777777 & ~_icr_msk; \
    hal_icr_pri_mirror[_icr] = _mir_val | _icr_val; \
    _icr_val |= MCF5272_SIM->intc.icr[_icr] & 0x77777777 & ~_icr_msk; \
    MCF5272_SIM->intc.icr[_icr] = _icr_val; \
    HAL_RESTORE_INTERRUPTS(_intr_state); \
CYG_MACRO_END

//      Set/clear  the  interrupt  transition   register  bit.   Disable   all
// interrupts while we access the hardware registers.

//      WARNING: It seems that manual currently  has the polarity of this  bit
// wrong.

#define HAL_INTERRUPT_CONFIGURE( _vector_, _leveltriggered_, _up_ ) \
CYG_MACRO_START \
    if (!(_leveltriggered_)) \
    { \
        cyg_uint32 _vec_offset = (_vector_) - HAL_PROG_INT_VEC_BASE - 1; \
        cyg_uint32 _itr_bit = 0x80000000 >> _vec_offset; \
        CYG_INTERRUPT_STATE _intr_state; \
        HAL_DISABLE_INTERRUPTS(_intr_state); \
        if (_up_) \
        { \
            MCF5272_SIM->intc.pitr |= _itr_bit; \
        } \
        else \
        { \
            MCF5272_SIM->intc.pitr &= ~_itr_bit; \
        } \
        HAL_RESTORE_INTERRUPTS(_intr_state); \
    } \
CYG_MACRO_END

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PROC_INTR_H

