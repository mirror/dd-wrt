#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      Platform specific Interrupt and clock support
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
// Contributors: jskov
// Date:         2001-06-12
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the SE77x9 board.
// Usage:
//               #include <cyg/hal/plf_intr.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>

//----------------------------------------------------------------------------
// External interrupts
#define CYGNUM_HAL_INTERRUPT_EXTERNALS_BASE CYGNUM_HAL_INTERRUPT_LVL0
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ8 CYGNUM_HAL_INTERRUPT_LVL0
#define CYGNUM_HAL_INTERRUPT_KEYBOARD  CYGNUM_HAL_INTERRUPT_LVL1
#define CYGNUM_HAL_INTERRUPT_PCMCIA2   CYGNUM_HAL_INTERRUPT_LVL2
#define CYGNUM_HAL_INTERRUPT_COM2      CYGNUM_HAL_INTERRUPT_LVL3
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ6 CYGNUM_HAL_INTERRUPT_LVL4
#define CYGNUM_HAL_INTERRUPT_MOUSE     CYGNUM_HAL_INTERRUPT_LVL5
#define CYGNUM_HAL_INTERRUPT_PCMCIA1   CYGNUM_HAL_INTERRUPT_LVL6
#define CYGNUM_HAL_INTERRUPT_COM1      CYGNUM_HAL_INTERRUPT_LVL7
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ4 CYGNUM_HAL_INTERRUPT_LVL8
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ3 CYGNUM_HAL_INTERRUPT_LVL9
#define CYGNUM_HAL_INTERRUPT_PARALLEL  CYGNUM_HAL_INTERRUPT_LVL10
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ2 CYGNUM_HAL_INTERRUPT_LVL11
#define CYGNUM_HAL_INTERRUPT_LAN       CYGNUM_HAL_INTERRUPT_LVL12
#define CYGNUM_HAL_INTERRUPT_IDE       CYGNUM_HAL_INTERRUPT_LVL13
#define CYGNUM_HAL_INTERRUPT_PCMCIA0   CYGNUM_HAL_INTERRUPT_LVL14

//----------------------------------------------------------------------------
// Interrupt controller
#define CYGARC_REG_INTC_A                   0xb1400000
#define CYGARC_REG_INTC_B                   0xb1400002
#define CYGARC_REG_INTC_C                   0xb1400004
#define CYGARC_REG_INTC_D                   0xb1400006
#define CYGARC_REG_INTC_E                   0xb1400008
#define CYGARC_REG_INTC_F                   0xb140000a
#define CYGARC_REG_INTC_G                   0xb140000c

//----------------------------------------------------------------------------
// Interrupt configuration extention macros
//
// It appears that masks do not appear linear in the INTC like on the SE7751.
// The below magic values determined from the INTC's startup values.
//               A        B       C       D       E       F      G
// 0xb1400000: 0x02a0  0x0005  0x008c  0xe030  0x0d91  0xf0b0  0x7640  0x0000

static inline void
_mask_vec(int level, cyg_uint32 reg, int shift, int lvl)
{
    cyg_uint16 msk;
    shift *= 4;
    HAL_READ_UINT16(reg, msk);
    msk &= ~(0x000f << shift);
    if (level) msk |= lvl << shift;
    HAL_WRITE_UINT16(reg, msk);
}

#define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                        \
 case CYGNUM_HAL_INTERRUPT_SLOT_IRQ8:                                            \
     _mask_vec(level, CYGARC_REG_INTC_F, 3, 0xf);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_KEYBOARD:                                             \
     _mask_vec(level, CYGARC_REG_INTC_D, 3, 0xe);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_PCMCIA2:                                              \
     _mask_vec(level, CYGARC_REG_INTC_E, 2, 0xd);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_COM2:                                                 \
     _mask_vec(level, CYGARC_REG_INTC_C, 0, 0xc);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_SLOT_IRQ6:                                            \
     _mask_vec(level, CYGARC_REG_INTC_F, 1, 0xb);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_MOUSE:                                                \
     _mask_vec(level, CYGARC_REG_INTC_A, 1, 0xa);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_PCMCIA1:                                              \
     _mask_vec(level, CYGARC_REG_INTC_E, 1, 0x9);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_COM1:                                                 \
     _mask_vec(level, CYGARC_REG_INTC_C, 1, 0x8);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_SLOT_IRQ4:                                            \
     _mask_vec(level, CYGARC_REG_INTC_G, 3, 0x7);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_SLOT_IRQ3:                                            \
     _mask_vec(level, CYGARC_REG_INTC_G, 2, 0x6);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_PARALLEL:                                             \
     _mask_vec(level, CYGARC_REG_INTC_B, 0, 0x5);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_SLOT_IRQ2:                                            \
     _mask_vec(level, CYGARC_REG_INTC_G, 1, 0x4);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_LAN:                                                  \
     _mask_vec(level, CYGARC_REG_INTC_D, 1, 0x3);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_IDE:                                                  \
     _mask_vec(level, CYGARC_REG_INTC_A, 2, 0x2);                                \
     break;                                                                      \
 case CYGNUM_HAL_INTERRUPT_PCMCIA0:                                              \
     _mask_vec(level, CYGARC_REG_INTC_E, 0, 0x1);                                \
     break;

//----------------------------------------------------------------------------
// Reset.
// Block interrupts and cause an exception. This forces a reset.
#define HAL_PLATFORM_RESET() \
    asm volatile ("ldc %0,sr;trapa #0x00;" : : "r" (CYGARC_REG_SR_BL))
    
#define HAL_PLATFORM_RESET_ENTRY 0x80000000

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
