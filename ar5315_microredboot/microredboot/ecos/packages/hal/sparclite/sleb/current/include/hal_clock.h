#ifndef CYGONCE_HAL_CLOCK_H
#define CYGONCE_HAL_CLOCK_H

//=============================================================================
//
//      hal_clock.h
//
//      HAL clock support
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, gthomas, hmt
// Contributors:        nickg, gthomas, hmt
// Date:        1999-01-28
// Purpose:     Define clock support
// Description: The macros defined here provide the HAL APIs for handling
//              the clock.
//              
// Usage:
//              #include <cyg/hal/hal_intr.h> // which includes this file
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

// #include <cyg/hal/hal_io.h>

#include <cyg/hal/hal_hwio.h> // HAL_SPARC_86940_READ/WRITE

//-----------------------------------------------------------------------------
// Clock control


// The vector used by the Real time clock is defined in hal_xpic.h

extern cyg_int32 cyg_hal_sparclite_clock_period;

//-----------------------------------------------------------------------------

#define HAL_SPARC_86940_REG_TIMER1_PRESCALER ( 0x14 * 4 )
#define HAL_SPARC_86940_REG_TIMER1_CONTROL   ( 0x15 * 4 )
#define HAL_SPARC_86940_REG_TIMER1_RELOAD    ( 0x16 * 4 )
#define HAL_SPARC_86940_REG_TIMER1_COUNT     ( 0x17 * 4 )



#define HAL_SPARC_86940_TIMER1_PRESCALER_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TIMER1_PRESCALER, v )

#define HAL_SPARC_86940_TIMER1_PRESCALER_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TIMER1_PRESCALER, r )

#define HAL_SPARC_86940_TIMER1_CONTROL_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TIMER1_CONTROL, v )

#define HAL_SPARC_86940_TIMER1_CONTROL_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TIMER1_CONTROL, r )

#define HAL_SPARC_86940_TIMER1_RELOAD_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_TIMER1_RELOAD, v )

#define HAL_SPARC_86940_TIMER1_RELOAD_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TIMER1_RELOAD, r )

#define HAL_SPARC_86940_TIMER1_COUNT_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_TIMER1_COUNT, r )

//-----------------------------------------------------------------------------

// Initialize the clock to 1MHz whatever the system clock speed.  This
// requires calculation...

externC void hal_clock_initialize( cyg_uint32 p );
#define HAL_CLOCK_INITIALIZE( _period_ ) hal_clock_initialize( _period_ )

// This is the easiest way to clear the interrupt.
#define HAL_CLOCK_RESET( _vector_, _period_ ) CYG_MACRO_START            \
  cyg_uint32 _scratch_;                                                  \
  HAL_SPARC_86940_TIMER1_COUNT_READ( _scratch_ );                        \
CYG_MACRO_END

#define HAL_CLOCK_READ( _pvalue_ ) CYG_MACRO_START                       \
  cyg_uint32 _read_;                                                     \
  HAL_SPARC_86940_TIMER1_COUNT_READ( _read_ );                           \
  *((cyg_uint32 *)(_pvalue_)) = cyg_hal_sparclite_clock_period - _read_; \
CYG_MACRO_END

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
#define HAL_CLOCK_LATENCY( _pvalue_ )         HAL_CLOCK_READ( _pvalue_ )
#endif

//-----------------------------------------------------------------------------

#endif // ifndef CYGONCE_HAL_CLOCK_H
// End of hal_clock.h
