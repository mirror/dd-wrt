//==========================================================================
//
//      pxa2x0_misc.c
//
//      HAL misc board support code for Intel PXA2X0
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
// Author(s):    <knud.woehler@microplex.de>
// Date:         2002-09-03
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_pxa2x0.h>
#include <cyg/hal/hal_mm.h>
#include <cyg/infra/diag.h>


// Initialize the interrupt environment
externC void plf_hardware_init(void);

void hal_hardware_init(void)
{
    hal_xscale_core_init();

    *PXA2X0_ICMR = 0;           // IRQ Mask
    *PXA2X0_ICLR = 0;           // Route interrupts to IRQ
    *PXA2X0_ICCR = 1;

    *PXA2X0_GRER0 = 0;          // Disable rising edge detect
    *PXA2X0_GRER1 = 0;
    *PXA2X0_GRER2 = 0;

    *PXA2X0_GFER0 = 0;          // Disable falling edge detect
    *PXA2X0_GFER1 = 0;
    *PXA2X0_GFER2 = 0;

    *PXA2X0_GEDR0 = 0xffffffff; // Clear edge detect status
    *PXA2X0_GEDR1 = 0xffffffff;
    *PXA2X0_GEDR2 = 0x0001ffff;

    plf_hardware_init();        // Perform any platform specific initializations

    *PXA2X0_OSCR = 0;           // Let the "OS" counter run
    *PXA2X0_OSMR0 = 0;

#ifdef CYGSEM_HAL_ENABLE_DCACHE_ON_STARTUP
    HAL_DCACHE_ENABLE();        // Enable caches
#endif
#ifdef CYGSEM_HAL_ENABLE_ICACHE_ON_STARTUP
    HAL_ICACHE_ENABLE();
#endif
}

//
// GPIO support functions
//
void
_pxa2x0_set_GPIO_mode(int bit, int mode, int dir)
{
    int bank = bit / 32;
    unsigned long *gpdr, *gafr;

    gpdr = &PXA2X0_GPDR0[bank];
    gafr = &PXA2X0_GAFR0_L[(bit&0x30)>>4];
    bit %= 32;
    // Data direction registers have 1 bit per GPIO
    *gpdr = (*gpdr & ~(1<<bit)) | (dir<<bit);
    // Alternate function regusters have 2 bits per GPIO
    bit = (bit & 0x0F) * 2;
    *gafr = (*gafr & ~(3<<bit)) | (mode<<bit);
}


// Initialize the clock
static cyg_uint32  clock_period;

void hal_clock_initialize(cyg_uint32 period)
{
	*PXA2X0_OSMR0 = period;					// Load match value
	clock_period = period;
    
	*PXA2X0_OSCR = 0;						// Start the counter
    *PXA2X0_OSSR = PXA2X0_OSSR_TIMER0;		// Clear any pending interrupt
    *PXA2X0_OIER |= PXA2X0_OIER_TIMER0;		// Enable timer 0 interrupt

    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_TIMER0 );	// Unmask timer 0 interrupt
}

// This routine is called during a clock interrupt.
void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    *PXA2X0_OSMR0 = *PXA2X0_OSCR + period;	// Load new match value
    *PXA2X0_OSSR = PXA2X0_OSSR_TIMER0;		// Clear any pending interrupt
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

// Note: The "contract" for this function is that the value is the number
// of hardware clocks that have happened since the last interrupt (i.e.
// when it was reset).  This value is used to measure interrupt latencies.
// However, since the hardware counter runs freely, this routine computes
// the difference between the current clock period and the number of hardware
// ticks left before the next timer interrupt.
void hal_clock_read(cyg_uint32 *pvalue)
{
    int orig;
    HAL_DISABLE_INTERRUPTS(orig);
    *pvalue = clock_period + *PXA2X0_OSCR - *PXA2X0_OSMR0;
    HAL_RESTORE_INTERRUPTS(orig);
}

// Delay for some number of micro-seconds
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 val = 0;
    cyg_uint32 ctr = *PXA2X0_OSCR;
    while (usecs-- > 0) {
        do {
            if (ctr != *PXA2X0_OSCR) {
                val += 271267;          // 271267ps (3.6865Mhz -> 271.267ns)
                ++ctr;
            }
        } while (val < 1000000);
        val -= 1000000;
    }
}


// Interrupt handling

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
    cyg_uint32 sources, index;

    sources = *PXA2X0_ICIP;

#ifdef HAL_EXTENDED_IRQ_HANDLER
    // Use platform specific IRQ handler, if defined
    // Note: this macro should do a 'return' with the appropriate
    // interrupt number if such an extended interrupt exists.  The
    // assumption is that the line after the macro starts 'normal' processing.
    HAL_EXTENDED_IRQ_HANDLER(sources);
#endif

    if ( sources & 0xff0000 )
        index = 16;
    else if ( sources & 0xff00 )
        index = 8;
    else if ( sources & 0xff )
        index = 0;
    else // if ( sources & 0xff000000 )
        index = 24;

    do {
        if ( (1 << index) & sources ) {
            if (index == CYGNUM_HAL_INTERRUPT_GPIO) {
                // Special case of GPIO cascade.  Search for lowest set bit
                sources = *PXA2X0_GEDR0;
                index = 0;
                do {
                    if (sources & (1 << index)) {
                        return index+32;
                    }
                    index++;
                } while (index < 32);
                sources = *PXA2X0_GEDR1;
                index = 0;
                do {
                    if (sources & (1 << index)) {
                        return index+64;
                    }
                    index++;
                } while (index < 32);
                sources = *PXA2X0_GEDR2;
                index = 0;
                do {
                    if (sources & (1 << index)) {
                        return index+96;
                    }
                    index++;
                } while (index < 16);

            }
            return index;
        }
      index++;
    } while ( index & 7 );
    
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

void hal_interrupt_mask(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_MASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_MASK(vector);
#endif
    
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2) {
        vector = CYGNUM_HAL_INTERRUPT_GPIO;
    }
    *PXA2X0_ICMR &= ~(1 << vector);
}

void hal_interrupt_unmask(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_UNMASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_UNMASK(vector);
#endif

    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2) {
        vector = CYGNUM_HAL_INTERRUPT_GPIO;
    }
    *PXA2X0_ICMR |= (1 << vector);
}

void hal_interrupt_acknowledge(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_ACKNOWLEDGE
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_ACKNOWLEDGE(vector);
#endif
	if (vector == CYGNUM_HAL_INTERRUPT_GPIO0 || vector == CYGNUM_HAL_INTERRUPT_GPIO1)
	{
		*PXA2X0_GEDR0  = (1 << (vector - 8));
	}else{
	    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO64) {
			*PXA2X0_GEDR2  = (1 << (vector - 96));
		} else if (vector >= CYGNUM_HAL_INTERRUPT_GPIO32) {
			*PXA2X0_GEDR1  = (1 << (vector - 64));
		} else if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2) {
			*PXA2X0_GEDR0  = (1 << (vector - 32));
		} else {
			// Not a GPIO interrupt
			return;
		}
	}
}

void hal_interrupt_configure(int vector, int level, int up)
{

#ifdef HAL_EXTENDED_INTERRUPT_CONFIGURE
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_CONFIGURE(vector, level, up);
#endif
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO64) {
        if (level) {
            if (up) {
                // Enable both edges
                *PXA2X0_GRER2 |= (1 << (vector - 96));
                *PXA2X0_GFER2 |= (1 << (vector - 96));
            } else {
                // Disable both edges
                *PXA2X0_GRER2 &= ~(1 << (vector - 96));
                *PXA2X0_GFER2 &= ~(1 << (vector - 96));
            }
        } else {
            // Only interested in one edge
            if (up) {
                // Set rising edge detect and clear falling edge detect.
                *PXA2X0_GRER2 |= (1 << (vector - 96));
                *PXA2X0_GFER2 &= ~(1 << (vector - 96));
            } else {
                // Set falling edge detect and clear rising edge detect.
                *PXA2X0_GFER2 |= (1 << (vector - 96));
                *PXA2X0_GRER2 &= ~(1 << (vector - 96));
            }
        }
    } else if (vector >= CYGNUM_HAL_INTERRUPT_GPIO32) {
        if (level) {
            if (up) {
                // Enable both edges
                *PXA2X0_GRER1 |= (1 << (vector - 64));
                *PXA2X0_GFER1 |= (1 << (vector - 64));
            } else {
                // Disable both edges
                *PXA2X0_GRER1 &= ~(1 << (vector - 64));
                *PXA2X0_GFER1 &= ~(1 << (vector - 64));
            }
        } else {
            // Only interested in one edge
            if (up) {
                // Set rising edge detect and clear falling edge detect.
                *PXA2X0_GRER1 |= (1 << (vector - 64));
                *PXA2X0_GFER1 &= ~(1 << (vector - 64));
            } else {
                // Set falling edge detect and clear rising edge detect.
                *PXA2X0_GFER1 |= (1 << (vector - 64));
                *PXA2X0_GRER1 &= ~(1 << (vector - 64));
            }
        }
    } else if (vector >= CYGNUM_HAL_INTERRUPT_GPIO2) {
        if (level) {
            if (up) {
                // Enable both edges
                *PXA2X0_GRER0 |= (1 << (vector - 32));
                *PXA2X0_GFER0 |= (1 << (vector - 32));
            } else {
                // Disable both edges
                *PXA2X0_GRER0 &= ~(1 << (vector - 32));
                *PXA2X0_GFER0 &= ~(1 << (vector - 32));
            }
        } else {
            // Only interested in one edge
            if (up) {
                // Set rising edge detect and clear falling edge detect.
                *PXA2X0_GRER0 |= (1 << (vector - 32));
                *PXA2X0_GFER0 &= ~(1 << (vector - 32));
            } else {
                // Set falling edge detect and clear rising edge detect.
                *PXA2X0_GFER0 |= (1 << (vector - 32));
                *PXA2X0_GRER0 &= ~(1 << (vector - 32));
            }
        }
    } else if (vector == CYGNUM_HAL_INTERRUPT_GPIO0 || vector == CYGNUM_HAL_INTERRUPT_GPIO1)
	{
        if (level) {
            if (up) {
                // Enable both edges
                *PXA2X0_GRER0 |= (1 << (vector - 8));
                *PXA2X0_GFER0 |= (1 << (vector - 8));
            } else {
                // Disable both edges
                *PXA2X0_GRER0 &= ~(1 << (vector - 8));
                *PXA2X0_GFER0 &= ~(1 << (vector - 8));
            }
        } else {
            // Only interested in one edge
            if (up) {
                // Set rising edge detect and clear falling edge detect.
                *PXA2X0_GRER0 |= (1 << (vector - 8));
                *PXA2X0_GFER0 &= ~(1 << (vector - 8));
            } else {
                // Set falling edge detect and clear rising edge detect.
                *PXA2X0_GFER0 |= (1 << (vector - 8));
                *PXA2X0_GRER0 &= ~(1 << (vector - 8));
            }
        }
	}


}

void hal_interrupt_set_level(int vector, int level)
{

#ifdef HAL_EXTENDED_INTERRUPT_SET_LEVEL
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_SET_LEVEL(vector, level);
#endif
}

