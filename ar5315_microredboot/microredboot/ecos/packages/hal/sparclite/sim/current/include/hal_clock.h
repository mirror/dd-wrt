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

#include <cyg/hal/hal_io.h>

//-----------------------------------------------------------------------------
// Clock control

// in erc32 simulator:
//  4 = UART A
//  5 = UART B
//  7 = UART error
// 12 = GPT (general purpose timer)
// 13 = RTC (realtime clock)
// 15 = watchdog

// in erc32 simulator:

// The vector used by the Real time clock is defined in hal_xpic.h

// We could place conditional code here to choose one clock or the other
// depending on the selected interrupt vector... but pro tem: (pun intended)

#define SPARC_MEC_RTC_CLOCK_SCALE (5)

/* These must be accessed word-wide to work! */

#define SPARC_MEC_RTC              (0x01f80080)

#define SPARC_MEC_RTC_COUNTER    (SPARC_MEC_RTC +    0)
#define SPARC_MEC_RTC_SCALER     (SPARC_MEC_RTC +    4)
                                                       
#define SPARC_MEC_GPT_COUNTER    (SPARC_MEC_RTC +    8)
#define SPARC_MEC_GPT_SCALER     (SPARC_MEC_RTC + 0x0c)

/* MEC timer control register bits */
#define SPARC_MEC_TCR_GACR 1      /* Continuous Running */
#define SPARC_MEC_TCR_GACL 2      /* Counter Load */      
#define SPARC_MEC_TCR_GASE 4      /* System Enable */     
#define SPARC_MEC_TCR_GASL 8      /* not used */          
#define SPARC_MEC_TCR_TCRCR 0x100 /* Continuous Running */
#define SPARC_MEC_TCR_TCRCL 0x200 /* Counter Load */
#define SPARC_MEC_TCR_TCRSE 0x400 /* System Enable */
#define SPARC_MEC_TCR_TCRSL 0x800 /* not used */

#define SPARC_MEC_RTC_CONTROL    (SPARC_MEC_RTC + 0x18)

externC cyg_int32 cyg_hal_sparclite_clock_period;

#define HAL_CLOCK_INITIALIZE( _period_ ) CYG_MACRO_START              \
  HAL_WRITE_UINT32( SPARC_MEC_RTC_SCALER, SPARC_MEC_RTC_CLOCK_SCALE );\
  cyg_hal_sparclite_clock_period = (_period_);                        \
  HAL_WRITE_UINT32( SPARC_MEC_RTC_COUNTER, (_period_) );              \
  HAL_WRITE_UINT32( SPARC_MEC_RTC_CONTROL,                            \
                           (SPARC_MEC_TCR_TCRCR |                     \
                            SPARC_MEC_TCR_TCRCL |                     \
                            SPARC_MEC_TCR_TCRSE) );                   \
CYG_MACRO_END

#define HAL_CLOCK_RESET( _vector_, _period_ ) /* nowt, it is freerunning */

#define HAL_CLOCK_READ( _pvalue_ ) CYG_MACRO_START                    \
   cyg_uint32 _read_;                                                 \
   HAL_READ_UINT32( SPARC_MEC_RTC_COUNTER, _read_ );                  \
   *((cyg_uint32 *)(_pvalue_)) =                                      \
                 (cyg_hal_sparclite_clock_period - _read_ );          \
CYG_MACRO_END


#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
#define HAL_CLOCK_LATENCY( _pvalue_ )         HAL_CLOCK_READ( _pvalue_ )
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CLOCK_H
// End of hal_clock.h
