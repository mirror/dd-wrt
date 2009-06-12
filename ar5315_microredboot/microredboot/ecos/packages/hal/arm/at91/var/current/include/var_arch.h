#ifndef CYGONCE_HAL_VAR_ARCH_H
#define CYGONCE_HAL_VAR_ARCH_H
//=============================================================================
//
//      var_arch.h
//
//      AT91 variant architecture overrides
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Jonathan Larmour <jifl@eCosCentric.com>
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: Daniel Neri
// Date:         2003-06-24
// Purpose:      AT91 variant architecture overrides
// Description: 
// Usage:        #include <cyg/hal/hal_arch.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>

//--------------------------------------------------------------------------
// Idle thread code.
// This macro is called in the idle thread loop, and gives the HAL the
// chance to insert code. Typical idle thread behaviour might be to halt the
// processor. These implementations halt the system core clock.

#ifndef HAL_IDLE_THREAD_ACTION


#if defined(CYGHWR_HAL_ARM_AT91_R40807) || \
    defined(CYGHWR_HAL_ARM_AT91_R40008)

#define HAL_IDLE_THREAD_ACTION(_count_)                       \
CYG_MACRO_START                                               \
    HAL_WRITE_UINT32(AT91_PS+AT91_PS_CR, AT91_PS_CR_CPU);     \
CYG_MACRO_END

#elif defined(CYGHWR_HAL_ARM_AT91_M42800A) || \
      defined(CYGHWR_HAL_ARM_AT91_M55800A)

#define HAL_IDLE_THREAD_ACTION(_count_)                       \
CYG_MACRO_START                                               \
    HAL_WRITE_UINT32(AT91_PMC+AT91_PMC_SCDR, 1);              \
CYG_MACRO_END

#elif defined(CYGHWR_HAL_ARM_AT91_JTST) 
// No idle action for the JTST
#else
#error Unknown AT91 variant

#endif

#endif

//-----------------------------------------------------------------------------
// end of var_arch.h
#endif // CYGONCE_HAL_VAR_ARCH_H
