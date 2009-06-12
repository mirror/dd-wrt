#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      IDT32438 Interrupt and clock support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    vsubbiah
// Contributors:
// Date:         2005-11-01
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the IDT3248 chip.
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


#define CYGNUM_HAL_INTERRUPT_UART_VEC          70

//----------------------------------------------------------------------------
// Reset.
#ifndef CYGHWR_HAL_RESET_DEFINED

#define CYGHWR_HAL_RESET_DEFINED
#define HAL_PLATFORM_RESET()            hal_idt32438_reset()
#define HAL_PLATFORM_RESET_ENTRY        0xbfc00000

#endif // CYGHWR_HAL_RESET_DEFINED

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
