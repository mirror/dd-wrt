//==========================================================================
//
//      sa11x0_misc.c
//
//      HAL misc board support code for StrongARM SA11x0
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
// Contributors: hmt
//               Travis C. Furrer <furrer@mit.edu>
// Date:         2000-05-08
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_misc.h>           // Size constants
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>          // Cache control
#include <cyg/hal/hal_sa11x0.h>         // Hardware definitions
#include <cyg/hal/hal_mm.h>             // MMap table definitions

#include <cyg/infra/diag.h>             // diag_printf

// Most initialization has already been done before we get here.
// All we do here is set up the interrupt environment.
// FIXME: some of the stuff in hal_platform_setup could be moved here.

externC void plf_hardware_init(void);

void hal_hardware_init(void)
{
    // Mask all interrupts
    *SA11X0_ICMR = 0;
     
    // Make all interrupts do IRQ and not FIQ
    // FIXME: Change this if you use FIQs.
    *SA11X0_ICLR = 0;
     
    // Prevent masked interrupts from bringing us out of idle mode
    *SA11X0_ICCR = 1;

    // Disable all GPIO interrupt sources
    *SA11X0_GPIO_RISING_EDGE_DETECT = 0;
    *SA11X0_GPIO_FALLING_EDGE_DETECT = 0;
    *SA11X0_GPIO_EDGE_DETECT_STATUS = 0x0FFFFFFF;

    // Perform any platform specific initializations
    plf_hardware_init();

    // Let the "OS" counter run
    *SA11X0_OSCR = 0;
    *SA11X0_OSMR0 = 0;

    // Set up eCos/ROM interfaces
    hal_if_init();

    // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();
}

// -------------------------------------------------------------------------
static cyg_uint32  clock_period;

void hal_clock_initialize(cyg_uint32 period)
{
    // Load match value
    *SA11X0_OSMR0 = period;
    clock_period = period;

    // Start the counter
    *SA11X0_OSCR = 0;

    // Clear any pending interrupt
    *SA11X0_OSSR = SA11X0_OSSR_TIMER0;

    // Enable timer 0 interrupt    
    *SA11X0_OIER |= SA11X0_OIER_TIMER0;

    // Unmask timer 0 interrupt
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_TIMER0 );

    // That's all.
}

// This routine is called during a clock interrupt.

// Define this if you want to ensure that the clock is perfect (i.e. does
// not drift).  One reason to leave it turned off is that it costs some
// us per system clock interrupt for this maintenance.
#undef COMPENSATE_FOR_CLOCK_DRIFT

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
#ifdef COMPENSATE_FOR_CLOCK_DRIFT
    cyg_uint32 next = *SA11X0_OSMR0 + period;    // Next interrupt time
    *SA11X0_OSSR = SA11X0_OSSR_TIMER0;           // Clear any pending interrupt
    *SA11X0_OSMR0 = next;                        // Load new match value
    {
        cyg_uint32 ctr = *SA11X0_OSCR;
        clock_period = next - ctr;
        if ((clock_period - 1) >= period) {      // Adjust for missed interrupts
            *SA11X0_OSMR0 = ctr + period;
            *SA11X0_OSSR = SA11X0_OSSR_TIMER0;   // Clear pending interrupt
            clock_period = period;
        }
    }
#else
    *SA11X0_OSMR0 = *SA11X0_OSCR + period;       // Load new match value
    *SA11X0_OSSR = SA11X0_OSSR_TIMER0;           // Clear any pending interrupt
#endif
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
    *pvalue = clock_period + *SA11X0_OSCR - *SA11X0_OSMR0;
    HAL_RESTORE_INTERRUPTS(orig);
}

// This is to cope with the test read used by tm_basic with
// CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY defined; we read the count ASAP
// in the ISR, *before* resetting the clock.  Which returns 1tick +
// latency if we just use plain hal_clock_read().
void hal_clock_latency(cyg_uint32 *pvalue)
{
    int orig;
    HAL_DISABLE_INTERRUPTS(orig);
    *pvalue = *SA11X0_OSCR - *SA11X0_OSMR0;
    HAL_RESTORE_INTERRUPTS(orig);
}

//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 val = 0;
    cyg_uint32 ctr = *SA11X0_OSCR;
    while (usecs-- > 0) {
        do {
            if (ctr != *SA11X0_OSCR) {
                val += 271267;          // 271267ps (3.6865Mhz -> 271.267ns)
                ++ctr;
            }
        } while (val < 1000000);
        val -= 1000000;
    }
}
#ifdef CYGPKG_PROFILE_GPROF
//--------------------------------------------------------------------------
//
// Profiling support - uses a separate high-speed timer
//

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/profile/profile.h>

// Can't rely on Cyg_Interrupt class being defined.
#define Cyg_InterruptHANDLED 1

// Profiling timer ISR
static cyg_uint32 
profile_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data, HAL_SavedRegisters *regs)
{
    HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_TIMER1);
    __profile_hit(regs->pc);
    return Cyg_InterruptHANDLED;
}

void
hal_enable_profile_timer(int resolution)
{
    // Run periodic timer interrupt for profile 
    // The resolution is specified in us, the hardware generates 3.6864
    // ticks/us
    int period = (resolution*36864) / 10000;

    // Attach ISR.
    HAL_INTERRUPT_ATTACH(CYGNUM_HAL_INTERRUPT_TIMER1, &profile_isr, 0x1111, 0);
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_TIMER1);

    // Set period.
    *SA11X0_OSMR1 = period;
}
#endif

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
    cyg_uint32 sources, index;

#ifdef HAL_EXTENDED_IRQ_HANDLER
    // Use platform specific IRQ handler, if defined
    // Note: this macro should do a 'return' with the appropriate
    // interrupt number if such an extended interrupt exists.  The
    // assumption is that the line after the macro starts 'normal' processing.
    HAL_EXTENDED_IRQ_HANDLER(index);
#endif

#if 0 // test FIQ and print alert if active - really for debugging
    sources = *SA11X0_ICFP;
    if ( 0 != sources )
        diag_printf( "FIQ source active!!! - fiqstatus %08x irqstatus %08x\n",
                     sources, *SA11X0_ICIP );
    else
#endif // Scan FIQ sources also

    sources = *SA11X0_ICIP;

// FIXME
// if we come to support FIQ properly...
//    if ( 0 == sources )
//        sources = *SA11X0_ICFP;

    // Nothing wrong with scanning them in any order we choose...
    // So here we try to make the serial devices steal fewer cycles.
    // So, knowing this is an ARM:
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
                sources = *SA11X0_GPIO_EDGE_DETECT_STATUS & 0x0FFFF800;
                index = 11;
                do {
                    if (sources & (1 << index)) {
                        index += 32;
                        break;
                    }
                    index++;
                } while (index < 28);
            }
            return index;
        }
      index++;
    } while ( index & 7 );
    
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_MASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_MASK(vector);
#endif

    // Non-GPIO interrupt sources can be masked separately.
    // Note: masking any non-unique GPIO signal (31..11) results in
    // all GPIO signals (31..11) being masked as only the "lump"
    // source will be changed.
    
    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO11) {
        vector = CYGNUM_HAL_INTERRUPT_GPIO;
    }
    *SA11X0_ICMR &= ~(1 << vector);
}

void hal_interrupt_unmask(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_UNMASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_UNMASK(vector);
#endif

    if (vector >= CYGNUM_HAL_INTERRUPT_GPIO11) {
        vector = CYGNUM_HAL_INTERRUPT_GPIO;
    }
    *SA11X0_ICMR |= (1 << vector);
}

void hal_interrupt_acknowledge(int vector)
{

#ifdef HAL_EXTENDED_INTERRUPT_UNMASK
    // Use platform specific handling, if defined
    // Note: this macro should do a 'return' for "extended" values of 'vector'
    // Normal vectors are handled by code subsequent to the macro call.
    HAL_EXTENDED_INTERRUPT_ACKNOWLEDGE(vector);
#endif

    // GPIO interrupts are driven by an edge detection mechanism.  This
    // is latching so these interrupts must be acknowledged directly.
    // All other interrupts simply go away when the interrupting unit
    // has been serviced by the ISR.
    if ((vector < CYGNUM_HAL_INTERRUPT_GPIO) || 
        (vector >= CYGNUM_HAL_INTERRUPT_GPIO11)) {
        *SA11X0_GPIO_EDGE_DETECT_STATUS  = (1 << (vector & 0x1F));
    } else {
        // Not a GPIO interrupt
        return;
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

    // This function can be used to configure the GPIO interrupts.  All
    // of these pins can potentially generate an interrupt, but only
    // 0..10 are unique.  Thus the discontinuity in the numbers.
    // Also, if 'level' is true, then both edges are enabled if 'up' is
    // true, otherwise they will be disabled.
    // Non GPIO sources are ignored.
    if ((vector < CYGNUM_HAL_INTERRUPT_GPIO) || 
        (vector >= CYGNUM_HAL_INTERRUPT_GPIO11)) {
        if (level) {
            if (up) {
                // Enable both edges
                *SA11X0_GPIO_RISING_EDGE_DETECT |= (1 << (vector & 0x1F));
                *SA11X0_GPIO_FALLING_EDGE_DETECT |= (1 << (vector & 0x1F));
            } else {
                // Disable both edges
                *SA11X0_GPIO_RISING_EDGE_DETECT &= ~(1 << (vector & 0x1F));
                *SA11X0_GPIO_FALLING_EDGE_DETECT &= ~(1 << (vector & 0x1F));
            }
        } else {
            // Only interested in one edge
            if (up) {
                // Set rising edge detect and clear falling edge detect.
                *SA11X0_GPIO_RISING_EDGE_DETECT |= (1 << (vector & 0x1F));
                *SA11X0_GPIO_FALLING_EDGE_DETECT &= ~(1 << (vector & 0x1F));
            } else {
                // Set falling edge detect and clear rising edge detect.
                *SA11X0_GPIO_FALLING_EDGE_DETECT |= (1 << (vector & 0x1F));
                *SA11X0_GPIO_RISING_EDGE_DETECT &= ~(1 << (vector & 0x1F));
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

    // Interrupt priorities are not configurable on the SA11X0.
}

/*------------------------------------------------------------------------*/
// These routines are for testing the equivalent efficient macros of the
// same names.  They actually inspect the MMap installed and tell the
// truth - including about the validity of the address at all.

cyg_uint32 hal_virt_to_phys_address( cyg_uint32 vaddr )
{
    register cyg_uint32 *ttb_base;
    cyg_uint32 noise;
    register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;

    // Get the TTB register
    asm volatile ("mrc  p15,0,%0,c2,c0,0;"
                  "mov  %0, %0, lsr #14;" // Lower 14 bits are undefined
                  "mov  %0, %0, asl #14;" // ...so clear them
                  : "=r"(ttb_base)
                  :
                  /*:*/);

    noise = vaddr & (SZ_1M - 1);
    vaddr /= SZ_1M; // Page size/Entry size is Mb.

    desc.word = *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS( ttb_base, vaddr );

    // Is this a valid entry that we understand?
    if ( ARM_MMU_FIRST_LEVEL_SECTION_ID == desc.section.id )
        return noise + desc.section.base_address * SZ_1M;

    return 0; // Not available.
}

cyg_uint32 hal_phys_to_virt_address( cyg_uint32 paddr )
{
    cyg_uint32 *ttb_base;
    cyg_uint32 i, noise;
    register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;
    cyg_bool identity_found = false;

    // Get the TTB register
    asm volatile ("mrc  p15,0,%0,c2,c0,0;"
                  "mov  %0, %0, lsr #14;" // Lower 14 bits are undefined
                  "mov  %0, %0, asl #14;" // ...so clear them
                  : "=r"(ttb_base)
                  :
                  /*:*/);


    noise = paddr & (SZ_1M - 1);
    paddr /= SZ_1M; // Page size/Entry size is Mb.

    for ( i = 0; i <= 0xfff; i++ ) {
        desc.word = *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS( ttb_base, i );

        // Is this a valid entry that we understand?
        if ( ARM_MMU_FIRST_LEVEL_SECTION_ID == desc.section.id ) {
            if ( paddr == desc.section.base_address ) {
                // Then the virtual address is i (in Mb).
                if ( i == paddr ) {
                    // We found a direct map first.  Do not report that
                    // immediately because it may be double mapped to a
                    // distinct virtual address, which we should return in
                    // preference.  But remember that we saw it.
                    identity_found = true;
                    continue;
                }
                // Otherwise report that we found it:
                return noise + i * SZ_1M;
            }
        }
    }
    // No non-identity matches were found.
    if ( identity_found )
        return noise + paddr * SZ_1M;

    return 0; // Not available.
}

cyg_uint32 hal_virt_to_uncached_address( cyg_uint32 vaddr )
{
    cyg_uint32 *ttb_base;
    cyg_uint32 noise, paddr, i;
    register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;

    // Get the TTB register
    asm volatile ("mrc  p15,0,%0,c2,c0,0;"
                  "mov  %0, %0, lsr #14;" // Lower 14 bits are undefined
                  "mov  %0, %0, asl #14;" // ...so clear them
                  : "=r"(ttb_base)
                  :
                  /*:*/);


    noise = vaddr & (SZ_1M - 1);
    vaddr /= SZ_1M; // Page size/Entry size is Mb.

    desc.word = *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS( ttb_base, vaddr );

    // Is this a valid entry that we understand?
    if ( ARM_MMU_FIRST_LEVEL_SECTION_ID != desc.section.id )
        return 0; // Not available.

    // Is this very address uncacheable already?
    if ( ARM_UNCACHEABLE == desc.section.c )
        return noise + vaddr * SZ_1M;

    paddr = desc.section.base_address;

    // We could look straight at a direct mapped slot for the physical
    // address as per convention...

    // Now scan through for a virtual address that maps to the same
    // physical memory, but uncached.
    for ( i = 0; i <= 0xfff; i++ ) {
        desc.word = *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS( ttb_base, i );

        // Is this a valid entry that we understand?
        if ( ARM_MMU_FIRST_LEVEL_SECTION_ID == desc.section.id )
            if ( paddr == desc.section.base_address )
                // Then the virtual address is i (in Mb).
                if ( ARM_UNCACHEABLE == desc.section.c )
                    // Then this one is not cacheable.
                    return noise + i * SZ_1M;
    }

    return 0; // Not available.
}


/*------------------------------------------------------------------------*/
// EOF sa11x0_misc.c
