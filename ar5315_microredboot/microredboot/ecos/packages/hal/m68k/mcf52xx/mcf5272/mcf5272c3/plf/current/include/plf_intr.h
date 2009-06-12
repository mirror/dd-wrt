#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      Platform specific interrupt and clock support
//
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

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

//--------------------------------------------------------------------------
// Interrupt vectors.

/*      The vector used by the Real time clock                              */

#define CYGNUM_HAL_INTERRUPT_RTC (CYGNUM_HAL_VECTOR_TMR4)

//---------------------------------------------------------------------------
// Clock control

/*      The mcf5272 has 4 timers 0-3.  Define the timer number that we want */
/* to use for the OS's clock.                                               */

#define CYGNUM_HAL_RTC_TIMER_NUM (3)

/*      Set the timer to generate 1 ms or 1000000 ns period interrupts.     */

#define HAL_M68K_MCF52xx_MCF5272_CLOCK_NS 1000000
#if CYGNUM_HAL_RTC_PERIOD != HAL_M68K_MCF52xx_MCF5272_CLOCK_NS
#warning Unexpected clock period for this board!!!
#endif

/*      Initialize the timer to generate an interrupt every 1 ms.  Use  the */
/* system clock divided by 16 as  the source.  Using 11*3 as the  prescaler */
/* gives a 8 us counter.  When this counter reaches 125 (1 ms) generate  an */
/* interrupt.                                                               */

#define HAL_CLOCK_INITIALIZE(_period_)                                      \
CYG_MACRO_START                                                             \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].tmr = 0x0000;                  \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].trr = 125-1;                   \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].tcn = 0;                       \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].ter = 0x0003;                  \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].tmr =                          \
    (((3*11)-1) << MCF5272_TIMER_TMR_PS_BIT) |                              \
    (0 << MCF5272_TIMER_TMR_CE_BIT) |                                       \
    (0 << MCF5272_TIMER_TMR_OM_BIT) |                                       \
    (1 << MCF5272_TIMER_TMR_ORI_BIT) |                                      \
    (1 << MCF5272_TIMER_TMR_FRR_BIT) |                                      \
    (2 << MCF5272_TIMER_TMR_CLK_BIT) |                                      \
    (1 << MCF5272_TIMER_TMR_RST_BIT);                                       \
CYG_MACRO_END

/*      We must clear the bit in the timer event register before we can get */
/* another interrupt.                                                       */

#define HAL_CLOCK_RESET( _vector_, _period_ )                               \
CYG_MACRO_START                                                             \
MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].ter = 0x0002;                  \
CYG_MACRO_END

/*      Read the current counter from the timer.                            */

#define HAL_CLOCK_READ( _pvalue_ )                                          \
CYG_MACRO_START                                                             \
*(_pvalue_) = MCF5272_SIM->timer[CYGNUM_HAL_RTC_TIMER_NUM].tcn;             \
CYG_MACRO_END

//---------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H

