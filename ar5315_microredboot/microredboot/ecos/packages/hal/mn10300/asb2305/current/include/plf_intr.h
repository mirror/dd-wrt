#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      ASB2305 Interrupt and clock support
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
// Author(s):    dhowells
// Contributors: nickg, jskov,
//               gthomas, jlarmour, dmoseley
// Date:         2001-05-17
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the AM33-2 ASB2305 board.
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

#if CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL==0
#define CYGNUM_HAL_INTERRUPT_DEBUG_UART CYGNUM_HAL_INTERRUPT_SERIAL_1_RX
#elif CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL==1
#define CYGNUM_HAL_INTERRUPT_DEBUG_UART CYGNUM_HAL_INTERRUPT_SERIAL_0_RX
#else
#error UNKNOWN DEBUG CHANNEL
#endif

extern void cyg_plf_hal_delay_us(cyg_int32 usecs);
#define HAL_DELAY_US cyg_plf_hal_delay_us

//--------------------------------------------------------------------------
// Control-C support.

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_SERIAL_1_RX

externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_CTRLC_ISR hal_ctrlc_isr

#endif

//--------------------------------------------------------------------------
// Function call support with a new stack.
// This will emulate thread creation using a new stack
//
#define HAL_ARCH_FUNCALL_NEW_STACK(func, base, size)     hal_arch_funcall_new_stack(func, base, size)
extern void hal_arch_funcall_new_stack(void (*func)(void), void* stack_base, cyg_uint32 stack_size);


//----------------------------------------------------------------------------
// Reset.
#ifndef CYGHWR_HAL_RESET_DEFINED
extern void cyg_hal_plf_reset( void );
#define CYGHWR_HAL_RESET_DEFINED
#define HAL_PLATFORM_RESET()               cyg_hal_plf_reset()

#define HAL_PLATFORM_RESET_ENTRY           0x40000000

#endif // CYGHWR_HAL_RESET_DEFINED

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
