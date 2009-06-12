//==========================================================================
//
//      ixp425_misc.c
//
//      HAL misc board support code for Intel IXP425 Network Processor
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-12-08
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_stub.h>           // Stub macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_io.h>
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
// FIXME: some of the stuff in hal_platform_setup could be moved here.

externC void plf_hardware_init(void);

void
hal_hardware_init(void)
{
    hal_xscale_core_init();

    // all interrupt sources to IRQ and disabled
    *IXP425_INTR_SEL = 0;
    *IXP425_INTR_EN = 0;

    // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();

    // Let the timer run at a default rate (for delays)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    // Set up eCos/ROM interfaces
    hal_if_init();

    // Perform any platform specific initializations
    plf_hardware_init();

#ifdef CYGPKG_IO_PCI
    cyg_hal_plf_pci_init();
#endif
}

// -------------------------------------------------------------------------
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int
hal_IRQ_handler(void)
{
    cyg_uint32 sources;
    int index;

    sources = *IXP425_INTR_IRQ_ST;
    if (sources) {
	HAL_LSBIT_INDEX(index, sources);
	return index;
    }

    sources = *IXP425_INTR_FIQ_ST;
    if (sources) {
	HAL_LSBIT_INDEX(index, sources);
	return index;
    }

    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

void
hal_interrupt_mask(int vector)
{
    unsigned val;

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_MASK(vector);
	return;
    }
#endif

    val = *IXP425_INTR_EN & ~(1 << vector);

    // If unmasking NPE interrupt, also unmask QM1 which is
    // shared by all NPE ports.
    if (vector == CYGNUM_HAL_INTERRUPT_NPEB ||
	vector == CYGNUM_HAL_INTERRUPT_NPEC) {
	if (!(val & (CYGNUM_HAL_INTERRUPT_NPEB | CYGNUM_HAL_INTERRUPT_NPEC)))
	    val &= ~(1 << CYGNUM_HAL_INTERRUPT_QM1);
    }
    
    *IXP425_INTR_EN = val;
}

void
hal_interrupt_unmask(int vector)
{
    unsigned val;

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_UNMASK(vector);
	return;
    }
#endif

    val = *IXP425_INTR_EN | (1 << vector);

    // If all NPE interrupts are masked, also mask QM1 which is
    // shared by all NPE ports.
    if (vector == CYGNUM_HAL_INTERRUPT_NPEB ||
	vector == CYGNUM_HAL_INTERRUPT_NPEC) {
	val |= (1 << CYGNUM_HAL_INTERRUPT_QM1);
    }

    *IXP425_INTR_EN = val;
}

void
hal_interrupt_acknowledge(int vector)
{
    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_ACKNOWLEDGE(vector);
	return;
    }
#endif

    if (vector == CYGNUM_HAL_INTERRUPT_GPIO0 ||
	vector == CYGNUM_HAL_INTERRUPT_GPIO1) {
	*IXP425_GPISR = 1 << (vector - CYGNUM_HAL_INTERRUPT_GPIO0);
	return;
    }
    
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2 &&
	vector <= CYGNUM_HAL_INTERRUPT_GPIO12) {
	*IXP425_GPISR = 4 << (vector - CYGNUM_HAL_INTERRUPT_GPIO2);
	return;
    }
}


// This function is used to configure the GPIO interrupts (and possibly
// some platform-specific interrupts).  All of the GPIO pins can potentially
// generate an interrupt.

// GPIO interrupts are configured as:
//
//    level  up  interrupt on
//    -----  --  ------------
//      0     0  Falling Edge
//      0     1  Rising Edge
//      0    -1  Either Edge
//      1     0  Low level
//      1     1  High level
//
void
hal_interrupt_configure(int vector, int level, int up)
{
    int shift, ival = 0;

    if (vector < CYGNUM_HAL_ISR_MIN || CYGNUM_HAL_ISR_MAX < vector)
	return;

#if CYGNUM_HAL_ISR_MAX > CYGNUM_HAL_VAR_ISR_MAX
    if (vector > CYGNUM_HAL_VAR_ISR_MAX) {
	HAL_PLF_INTERRUPT_CONFIGURE(vector, level, up);
	return;
    }
#endif

    if (level) {
	ival = up ? 0 : 1 ;
    } else {
	if (up == 0)
	    ival = 3;
	else if (up == 1)
	    ival = 2;
	else if (up == -1)
	    ival = 4;
    }

    if (vector == CYGNUM_HAL_INTERRUPT_GPIO0 ||
	vector == CYGNUM_HAL_INTERRUPT_GPIO1) {
	vector -= CYGNUM_HAL_INTERRUPT_GPIO0;
	shift = vector * 3;
	*IXP425_GPIT1R = (*IXP425_GPIT1R & ~(7 << shift)) | (ival << shift);
	*IXP425_GPISR |= (1 << vector);
	return;
    }
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2 &&
	vector <= CYGNUM_HAL_INTERRUPT_GPIO7) {
	vector = vector - CYGNUM_HAL_INTERRUPT_GPIO2 + 2;
	shift = vector * 3;
	*IXP425_GPIT1R = (*IXP425_GPIT1R & ~(7 << shift)) | (ival << shift);
	*IXP425_GPISR |= (1 << vector);
	return;
    }
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO8 &&
	vector <= CYGNUM_HAL_INTERRUPT_GPIO12) {
	vector -= CYGNUM_HAL_INTERRUPT_GPIO8;
	shift = vector * 3;
	*IXP425_GPIT2R = (*IXP425_GPIT2R & ~(7 << shift)) | (ival << shift);
	*IXP425_GPISR |= (1 << vector);
	return;
    }
}


// Should do something with priority here.
void
hal_interrupt_set_level(int vector, int level)
{
}


/*------------------------------------------------------------------------*/
// RTC Support

static cyg_uint32 _period;

#define CLOCK_MULTIPLIER 66

void
hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 tmr_period;

    _period = period;

    tmr_period = (period * CLOCK_MULTIPLIER) & OST_TIM_RL_MASK;
    
    // clear pending interrupt
    *IXP425_OST_STS = OST_STS_T0INT;

    // set reload value and enable
    *IXP425_OST_TIM0_RL = tmr_period | OST_TIM_RL_ENABLE;
}


// Dynamically set the timer interrupt rate.
// Not for eCos application use at all, just special GPROF code in RedBoot.

void
hal_clock_reinitialize(          int *pfreq,    /* inout */
                        unsigned int *pperiod,  /* inout */
                        unsigned int old_hz )   /* in */
{
    unsigned int newp = 0, period, i = 0;
    int hz;
    int do_set_hw;

// Arbitrary choice somewhat - so the CPU can make
// progress with the clock set like this, we hope.
#define MIN_TICKS (2000)
#define MAX_TICKS  N/A: 32-bit counter

    if ( ! pfreq || ! pperiod )
        return; // we cannot even report a problem!

    hz = *pfreq;
    period = *pperiod;

// Requested HZ:
// 0         => tell me the current value (no change, implemented in caller)
// - 1       => tell me the slowest (no change)
// - 2       => tell me the default (no change, implemented in caller)
// -nnn      => tell me what you would choose for nnn (no change)
// MIN_INT   => tell me the fastest (no change)
//        
// 1         => tell me the slowest (sets the clock)
// MAX_INT   => tell me the fastest (sets the clock)

    do_set_hw = (hz > 0);
    if ( hz < 0 )
        hz = -hz;

    // Be paranoid about bad args, and very defensive about underflows
    if ( 0 < hz && 0 < period && 0 < old_hz ) {

        newp = period * old_hz / (unsigned)hz;

        if ( newp < MIN_TICKS ) {
            newp = MIN_TICKS;
            // recalculate to get the exact delay for this integral hz
            // and hunt hz down to an acceptable value if necessary
            i = period * old_hz / newp;
            if ( i ) do {
                newp = period * old_hz / i;
                i--;
            } while (newp < MIN_TICKS && i);
        }
        // So long as period * old_hz fits in 32 bits, there is no need to
        // worry about overflow; hz >= 1 in the initial divide.  If the
        // clock cannot do a whole second (period * old_hz >= 2^32), we
        // will get overflow here, and random returned HZ values.

        // Recalculate the actual value installed.
        i = period * old_hz / newp;
    }

    *pfreq = i;
    *pperiod = newp;

    if ( do_set_hw ) {
        hal_clock_initialize( newp );
    }
}

// This routine is called during a clock interrupt.
void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    // clear pending interrupt
    *IXP425_OST_STS = OST_STS_T0INT;
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)
void
hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 timer_val;

    // Translate timer value back into microseconds
    timer_val = *IXP425_OST_TIM0 / CLOCK_MULTIPLIER;
    
    *pvalue = _period - timer_val;
}

// Delay for some usecs.
void
hal_delay_us(cyg_int32 delay)
{
#define _TICKS_PER_USEC CLOCK_MULTIPLIER
    cyg_uint32 now, prev, diff, usecs;
    cyg_uint32 tmr_period = _period * CLOCK_MULTIPLIER;

    diff = usecs = 0;
    prev = *IXP425_OST_TIM0;

    while (delay > usecs) {
	now = *IXP425_OST_TIM0;

	if (prev < now)
	    diff += (prev + (tmr_period - now));
	else
	    diff += (prev - now);

	prev = now;

	if (diff >= _TICKS_PER_USEC) {
	    usecs += (diff / _TICKS_PER_USEC);
	    diff %= _TICKS_PER_USEC;
	}
    }
}

/*------------------------------------------------------------------------*/
// EOF ixp425_misc.c

