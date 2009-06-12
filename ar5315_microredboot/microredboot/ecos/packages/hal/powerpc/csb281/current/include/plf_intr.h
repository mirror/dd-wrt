#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      CSB281 platform specific interrupt definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Contributors: jskov, gthomas
// Date:         2000-06-13
// Purpose:      Define platform specific interrupt support
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

//----------------------------------------------------------------------------
// Platform specific interrupt mapping - interrupt vectors
#define CYGNUM_HAL_INTERRUPT_IRQ0   0x02
#define CYGNUM_HAL_INTERRUPT_IRQ1   0x03
#define CYGNUM_HAL_INTERRUPT_IRQ2   0x04
#define CYGNUM_HAL_INTERRUPT_IRQ3   0x05
#define CYGNUM_HAL_INTERRUPT_IRQ4   0x06
#define CYGNUM_HAL_INTERRUPT_UART0  0x07
#define CYGNUM_HAL_INTERRUPT_UART1  0x08
#define CYGNUM_HAL_INTERRUPT_TIMER0 0x09
#define CYGNUM_HAL_INTERRUPT_TIMER1 0x0A
#define CYGNUM_HAL_INTERRUPT_TIMER2 0x0B
#define CYGNUM_HAL_INTERRUPT_TIMER3 0x0C
#define CYGNUM_HAL_INTERRUPT_I2C    0x0D
#define CYGNUM_HAL_INTERRUPT_DMA0   0x0E
#define CYGNUM_HAL_INTERRUPT_DMA1   0x0F
#define CYGNUM_HAL_INTERRUPT_MSG    0x10
#define CYGNUM_HAL_ISR_MAX          0x10

#define CYGNUM_HAL_INTERRUPT_PCI0   CYGNUM_HAL_INTERRUPT_IRQ0  // PCI slot 0 (disabled)
#define CYGNUM_HAL_INTERRUPT_PCI1   CYGNUM_HAL_INTERRUPT_IRQ1  // PCI slot 1
#define CYGNUM_HAL_INTERRUPT_LAN    CYGNUM_HAL_INTERRUPT_IRQ2  // Onboard GD82559
#define CYGNUM_HAL_INTERRUPT_MOUSE  CYGNUM_HAL_INTERRUPT_IRQ3  // PS/2 mouse
#define CYGNUM_HAL_INTERRUPT_KBD    CYGNUM_HAL_INTERRUPT_IRQ4  // PS/2 keyboard

// Platform specific interrupt handling - using EPIC
#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

externC void hal_interrupt_mask(int);
externC void hal_interrupt_unmask(int);
externC void hal_interrupt_acknowledge(int);
externC void hal_interrupt_configure(int, int, int);
externC void hal_interrupt_set_level(int, int);

#define HAL_INTERRUPT_MASK( _vector_ )                     \
    hal_interrupt_mask( _vector_ ) 
#define HAL_INTERRUPT_UNMASK( _vector_ )                   \
    hal_interrupt_unmask( _vector_ )
#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )              \
    hal_interrupt_acknowledge( _vector_ )
#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ ) \
    hal_interrupt_configure( _vector_, _level_, _up_ )
#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )       \
    hal_interrupt_set_level( _vector_, _level_ )


//--------------------------------------------------------------------------
// Control-C support.

// Defined by the quicc driver
// #include <cyg/hal/quicc/quicc_smc1.h>


//----------------------------------------------------------------------------
// Reset.

// The CSB281 does not have a watchdog (not one we can easily use for this
// purpose anyway).
#define HAL_PLATFORM_RESET() CYG_EMPTY_STATEMENT

#define HAL_PLATFORM_RESET_ENTRY 0xfff00100

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
