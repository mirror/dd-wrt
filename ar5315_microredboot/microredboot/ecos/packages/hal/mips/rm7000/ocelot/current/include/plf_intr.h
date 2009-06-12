#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      Ocelot Interrupt and clock support
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
// Contributors: jskov, nickg
// Date:         2000-05-09
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the REF4955 board.
//              
// Usage:
//              #include <cyg/hal/plf_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

// The first 10 correspond to the interrupt lines in the status/cause regs
#define CYGNUM_HAL_INTERRUPT_ETH0               0
#define CYGNUM_HAL_INTERRUPT_ETH1               1
#define CYGNUM_HAL_INTERRUPT_UART1              2
#define CYGNUM_HAL_INTERRUPT_21555              3
#define CYGNUM_HAL_INTERRUPT_GALILEO            4
#define CYGNUM_HAL_INTERRUPT_COMPARE            5
#define CYGNUM_HAL_INTERRUPT_PMC1               6
#define CYGNUM_HAL_INTERRUPT_PMC2               7
#define CYGNUM_HAL_INTERRUPT_CPCI               8
#define CYGNUM_HAL_INTERRUPT_UART2              9

// PCI interrupts are hardwired for the devices connected to the bus
#define CYGNUM_HAL_INTERRUPT_PCI_INTA           CYGNUM_HAL_INTERRUPT_GALILEO
#define CYGNUM_HAL_INTERRUPT_PCI_INTB           CYGNUM_HAL_INTERRUPT_ETH0
#define CYGNUM_HAL_INTERRUPT_PCI_INTC           CYGNUM_HAL_INTERRUPT_GALILEO
#define CYGNUM_HAL_INTERRUPT_PCI_INTD           CYGNUM_HAL_INTERRUPT_GALILEO

// Min/Max ISR numbers and how many there are
#define CYGNUM_HAL_ISR_MIN                     CYGNUM_HAL_INTERRUPT_ETH0
#define CYGNUM_HAL_ISR_MAX                     CYGNUM_HAL_INTERRUPT_UART2
#define CYGNUM_HAL_ISR_COUNT                   (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC            CYGNUM_HAL_INTERRUPT_COMPARE

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif

//--------------------------------------------------------------------------
// Interrupt controller information

// V320USC 
#define CYGARC_REG_INT_STAT   0xb80000ec

#define CYGARC_REG_INT_CFG0   0xb80000e0
#define CYGARC_REG_INT_CFG1   0xb80000e4
#define CYGARC_REG_INT_CFG2   0xb80000e8
#define CYGARC_REG_INT_CFG3   0xb8000158

#define CYGARC_REG_INT_CFG_INT0 0x00000100
#define CYGARC_REG_INT_CFG_INT1 0x00000200
#define CYGARC_REG_INT_CFG_INT2 0x00000400
#define CYGARC_REG_INT_CFG_INT3 0x00000800


// FPGA
#define CYGARC_REG_PCI_STAT   0xb5300000
#define CYGARC_REG_PCI_MASK   0xb5300030

#define CYGARC_REG_IO_STAT    0xb5300010
#define CYGARC_REG_IO_MASK    0xb5300040


//----------------------------------------------------------------------------
// Reset.
// Uses Secondary Reset Bit in 21555. Don't know where it is mapped though.
#define CYGARC_REG_BOARD_RESET 0x????????

#define HAL_PLATFORM_RESET() /* HAL_WRITE_UINT8(CYGARC_REG_BOARD_RESET,1) */

#define HAL_PLATFORM_RESET_ENTRY 0xbfc00000

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
