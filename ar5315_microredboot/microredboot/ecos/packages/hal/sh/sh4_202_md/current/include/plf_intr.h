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
// Copyright (C) 2003 Nick Garnett 
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
// Author(s):    nickg
// Contributors: nickg
// Date:         2003-08-20
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the SuperH SH4-202 MicroDev
//               CPU board.
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
// External interrupts.

#define CYGNUM_HAL_INTERRUPT_ETH           CYGNUM_HAL_INTERRUPT_LVL3

//----------------------------------------------------------------------------
// FPGA Interrupt controller


#define	FPGA_INTC_BASE		0xA6110000ul			/* INTC base address on CPU-board FPGA */
#define	FPGA_INTENB_REG		(FPGA_INTC_BASE+0ul)		/* Interrupt Enable Register on INTC on CPU-board FPGA */
#define FPGA_INTDSB_REG         (FPGA_INTC_BASE+8ul)            /* Interrupt Disable Register on INTC on CPU-board FPGA */
#define	FPGA_INTENB_MASK(n)	(1ul<<(n))			/* Interupt mask to enable Ethernet on INTC in CPU-board FPGA */
#define	FPGA_INTPRI_REG(n)	(FPGA_INTC_BASE+0x10+((n)/8)*8)	/* Interrupt Priority Register on INTC on CPU-board FPGA */
#define	FPGA_INTPRI_LEVEL(n,x)	((x)<<(((n)%8)*4))		/* FPGA_INTPRI_LEVEL(int_number, int_level) */
#define	FPGA_INTPRI_MASK(n)	(FPGA_INTPRI_LEVEL((n),0xful))	/* Interrupt Priority Mask on INTC on CPU-board FPGA */

#define	FPGA_ETHERNET_INT	(18)				/* Interrupt number for Ethernet in INTC on CPU-board FPGA */
#define	ETHERNET_INT_PRIORITY	(0xc)				/* Interrupt Priority of Ethenet IRQ */

//----------------------------------------------------------------------------
// Interrupt configuration extension macros

#define CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vec, level)                               \
    case CYGNUM_HAL_INTERRUPT_ETH:                                                      \
        {                                                                               \
            volatile cyg_uint32* const intEnableReg = (cyg_uint32*)FPGA_INTENB_REG;     \
            volatile cyg_uint32* const intDisableReg = (cyg_uint32*)FPGA_INTDSB_REG;    \
                                                                                        \
            if( level )                                                                 \
                *intEnableReg |= FPGA_INTENB_MASK(FPGA_ETHERNET_INT);                   \
            else                                                                        \
                *intDisableReg |= FPGA_INTENB_MASK(FPGA_ETHERNET_INT);                  \
        }                                                                               \
        break;                                                                          \
    case CYGNUM_HAL_INTERRUPT_NMI:                                                      \
        /* fall through */                                                              \
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL2:                       \
        /* fall through */                                                              \
    case CYGNUM_HAL_INTERRUPT_LVL4 ... CYGNUM_HAL_INTERRUPT_LVL14:                      \
        /* Cannot change levels */                                                      \
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
