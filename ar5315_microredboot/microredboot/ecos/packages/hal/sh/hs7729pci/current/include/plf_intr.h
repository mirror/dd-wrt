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
// Date:         2001-05-25
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the HS7729PCI board.
// Usage:
//               #include <cyg/hal/plf_intr.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

//----------------------------------------------------------------------------
// External interrupts
#define CYGNUM_HAL_INTERRUPT_EXTERNALS_BASE CYGNUM_HAL_INTERRUPT_LVL0
#define CYGNUM_HAL_INTERRUPT_PCI            CYGNUM_HAL_INTERRUPT_LVL0
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ4       CYGNUM_HAL_INTERRUPT_LVL1
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ1      CYGNUM_HAL_INTERRUPT_LVL2
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ12      CYGNUM_HAL_INTERRUPT_LVL3
#define CYGNUM_HAL_INTERRUPT_PCMCIA_IRQ0    CYGNUM_HAL_INTERRUPT_LVL4
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ5       CYGNUM_HAL_INTERRUPT_LVL5
#define CYGNUM_HAL_INTERRUPT_USB1           CYGNUM_HAL_INTERRUPT_LVL6
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ8       CYGNUM_HAL_INTERRUPT_LVL7
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ1       CYGNUM_HAL_INTERRUPT_LVL8
#define CYGNUM_HAL_INTERRUPT_RESERVED1      CYGNUM_HAL_INTERRUPT_LVL9
#define CYGNUM_HAL_INTERRUPT_PCMCIA_IRQ2    CYGNUM_HAL_INTERRUPT_LVL10
#define CYGNUM_HAL_INTERRUPT_USB2           CYGNUM_HAL_INTERRUPT_LVL11
#define CYGNUM_HAL_INTERRUPT_RESERVED2      CYGNUM_HAL_INTERRUPT_LVL12
#define CYGNUM_HAL_INTERRUPT_SLOT_IRQ5      CYGNUM_HAL_INTERRUPT_LVL13
#define CYGNUM_HAL_INTERRUPT_UIO_IRQ3       CYGNUM_HAL_INTERRUPT_LVL14

// Decoded interrupts - these follow the INTC v3 vectors defined in
// var_intr.h
#define CYGNUM_HAL_INTERRUPT_PCIA           66
#define CYGNUM_HAL_INTERRUPT_PCIB           67
#define CYGNUM_HAL_INTERRUPT_PCIC           68
#define CYGNUM_HAL_INTERRUPT_PCID           69

#define CYGNUM_HAL_ISR_PLF_MAX              CYGNUM_HAL_INTERRUPT_PCID

//----------------------------------------------------------------------------
// Interrupt configuration extention macros
#define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                               \
 case CYGNUM_HAL_INTERRUPT_NMI:                                                         \
     /* fall through */                                                                 \
 case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:                         \
     /* Cannot change levels */                                                         \
     break;                                                                             \
 case CYGNUM_HAL_INTERRUPT_PCIA ... CYGNUM_HAL_INTERRUPT_PCID:                          \
  {                                                                                     \
      cyg_uint32 msk;                                                                   \
      HAL_READ_UINT32(CYGARC_REG_SD0001_INT_ENABLE, msk);                               \
      msk &= ~(CYGARC_REG_SD0001_INT_INTA << ((vec) - CYGNUM_HAL_INTERRUPT_PCIA));      \
      msk |= CYGARC_REG_SD0001_INT_EN;                                                  \
      if ((level))                                                                      \
          msk |= CYGARC_REG_SD0001_INT_INTA << ((vec) - CYGNUM_HAL_INTERRUPT_PCIA);     \
      HAL_WRITE_UINT32(CYGARC_REG_SD0001_INT_ENABLE, msk);                              \
      break;                                                                            \
  }

#define CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vec)                                       \
  CYG_MACRO_START                                                                       \
  if (vec >= CYGNUM_HAL_INTERRUPT_PCIA && vec <= CYGNUM_HAL_INTERRUPT_PCID) {           \
      cyg_uint32 sts = CYGARC_REG_SD0001_INT_INTA << (vec - CYGNUM_HAL_INTERRUPT_PCIA); \
      HAL_WRITE_UINT32(CYGARC_REG_SD0001_INT_STS1, sts);                                \
  }                                                                                     \
  CYG_MACRO_END

//----------------------------------------------------------------------------
// Reset.
// Block interrupts and cause an exception. This forces a reset.
#define HAL_PLATFORM_RESET() \
    asm volatile ("ldc %0,sr;trapa #0x00;" : : "r" (CYGARC_REG_SR_BL))
    
#define HAL_PLATFORM_RESET_ENTRY 0x80000000

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
