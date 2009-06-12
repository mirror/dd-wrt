//==========================================================================
//
//      gps4020_misc.c
//
//      HAL misc board support code for ARM GPS4020-1
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-02-20
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>             // calling interface
#include <cyg/hal/hal_misc.h>           // helper functions
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#include <cyg/hal/drv_api.h>            // HAL ISR support
#endif

#include <cyg/hal/gps4020.h>

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    volatile struct _gps4020_timer *tc = (volatile struct _gps4020_timer *)GPS4020_TC1;
    
    // Start timer in "reload" mode to set counter value
    tc->tc[0].control = TC_CTL_SCR_HALT;  // Disable timer
    tc->tc[0].reload = period;
    tc->tc[0].control = TC_CTL_IE | TC_CTL_SCR_COUNT | TC_CTL_MODE_RELOAD | TC_CTL_HEP | TC_CLOCK_BASE;
    _period = period;
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    hal_clock_initialize(period);
    _gps4020_watchdog(false);
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    volatile struct _gps4020_timer *tc = (volatile struct _gps4020_timer *)GPS4020_TC1;
    *pvalue = _period - tc->tc[0].current;
}

//
// Delay for some number of micro-seconds
//   Use timer #2 of TC2
//
void hal_delay_us(cyg_int32 usecs)
{
    volatile struct _gps4020_timer *tc = (volatile struct _gps4020_timer *)GPS4020_TC2;
    unsigned long val1, val2;
    int timeout;
    
    // Start timer in "reload" mode to set counter value
    tc->tc[1].control = TC_CTL_SCR_HALT;  // Disable timer
    tc->tc[1].reload = usecs;
    tc->tc[1].control = TC_CTL_SCR_COUNT | TC_CTL_MODE_RELOAD | TC_CTL_HEP | TC_CLOCK_BASE;
    timeout = 10;
    val1 = tc->tc[1].current;
    while ((tc->tc[1].control & TC_CTL_OS) == 0) {
        if (--timeout == 0) {
            val2 = tc->tc[1].current;
            if (val1 == val2) {
                // Timer is stuck - use a cruder method!
                while (--usecs > 0) ;
                return;
            }
        }
    }
}

void hal_hardware_init(void)
{
    volatile struct _gps4020_intc *intc = (volatile struct _gps4020_intc *)GPS4020_INTC;

    // Clear and reset all interrupt sources
    intc->enable = 0;
    //                              3322 2222 2222 1111 1111 1100 0000 0000
    //                              1098 7654 3210 9876 5432 1098 7654 3210
    intc->polarity = 0x1C01E01F; // 0001 1100 0000 0001 1110 0000 0001 1111;
    intc->trigger  = 0x000C00C0; // 0000 0000 0000 1100 0000 0000 1100 0000;
    // Set up eCos/ROM interfaces
    hal_if_init();
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    volatile struct _gps4020_intc *intc = (volatile struct _gps4020_intc *)GPS4020_INTC;
    int vec = (intc->IRQ_encoded >> 2);
    return vec;
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    volatile struct _gps4020_intc *intc = (volatile struct _gps4020_intc *)GPS4020_INTC;
    intc->enable &= ~(1<<vector);
}

void hal_interrupt_unmask(int vector)
{
    volatile struct _gps4020_intc *intc = (volatile struct _gps4020_intc *)GPS4020_INTC;
    intc->enable |= (1<<vector);
}

void hal_interrupt_acknowledge(int vector)
{
    volatile struct _gps4020_intc *intc = (volatile struct _gps4020_intc *)GPS4020_INTC;
    intc->reset = (1<<vector);
}

void hal_interrupt_configure(int vector, int level, int up)
{
//    diag_printf("%s(%d,%d,%d)\n", __PRETTY_FUNCTION__, vector, level, up);
}

void hal_interrupt_set_level(int vector, int level)
{
//    diag_printf("%s(%d,%d)\n", __PRETTY_FUNCTION__, vector, level);
}

//-----------------------------------------------------------------------------
// Reset board

void
hal_gps4020_reset(void)
{
    volatile struct _gps4020_watchdog *wdg = (volatile struct _gps4020_watchdog *)GPS4020_WATCHDOG;

    wdg->period = 1;  // Almost as fast as possible
    wdg->reset = GPS4020_WATCHDOG_RESET;
    // Wait for it...
    for(;;);
}

// Watchdog support
void
_gps4020_watchdog(bool is_idle)
{
    volatile struct _gps4020_watchdog *wdg = (volatile struct _gps4020_watchdog *)GPS4020_WATCHDOG;
    wdg->reset = GPS4020_WATCHDOG_RESET;
}

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
RedBoot_idle(_gps4020_watchdog, RedBoot_AFTER_NETIO);
#endif

/*------------------------------------------------------------------------*/
// EOF hal_misc.c
