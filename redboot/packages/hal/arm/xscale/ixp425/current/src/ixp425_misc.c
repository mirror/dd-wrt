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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Red Hat, Inc.
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

externC void plf_hardware_init(void);

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
static cyg_uint32 mcu_ISR(cyg_vector_t vector, cyg_addrword_t data);
#endif

void
hal_hardware_init(void)
{
    hal_xscale_core_init();

    // all interrupt sources to IRQ and disabled
    *IXP425_INTR_SEL = 0;
    *IXP425_INTR_EN = 0;

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    *IXP425_INTR_SEL2 = 0;
    *IXP425_INTR_EN2 = 0;
#endif

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

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    // attach interrupt handler for MCU (ECC) errors
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_MCU_ERR, &mcu_ISR, CYGNUM_HAL_INTERRUPT_MCU_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_MCU_ERR);
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

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    sources = *IXP425_INTR_IRQ_ST2;
    if (sources) {
	HAL_LSBIT_INDEX(index, sources);
	return index + 32;
    }
#endif

    sources = *IXP425_INTR_FIQ_ST;
    if (sources) {
	HAL_LSBIT_INDEX(index, sources);
	return index;
    }

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    sources = *IXP425_INTR_FIQ_ST2;
    if (sources) {
	HAL_LSBIT_INDEX(index, sources);
	return index + 32;
    }
#endif

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

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    if (vector >= 32) {
	*IXP425_INTR_EN2 &= ~(1 << (vector-32));
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

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
    if (vector >= 32) {
	*IXP425_INTR_EN2 |= (1 << (vector-32));
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

#ifdef CYGHWR_HAL_ARM_XSCALE_CPU_IXP46x
/*------------------------------------------------------------------------*/
// ECC Support

static inline void
_scrub_ecc(unsigned p)
{
#if 0
    cyg_uint32 iacr;

    // The following ldr/str pair need to be atomic on the bus. Since
    // the XScale core doesn't support atomic RMW, we have to disable
    // arbitration to prevent other bus masters from taking the bus
    // between the the ldr and str.

    // Disable internal bus arbitration for everything except the CPU
    iacr = *ARB_IACR;
    *ARB_IACR = IACR_ATU(IACR_PRI_OFF)  | IACR_DMA0(IACR_PRI_OFF) |
  	        IACR_DMA1(IACR_PRI_OFF) | IACR_AAU(IACR_PRI_OFF)  |
	        IACR_PBI(IACR_PRI_OFF)  | IACR_CORE(IACR_PRI_HIGH);

#endif

    // drain write buffer
    asm volatile ("mrc  p15,0,r1,c7,c10,4\n");
    CPWAIT();

    asm volatile ("ldrb r4, [%0]\n"
		  "strb r4, [%0]\n" : : "r"(p) : "r4");

#if 0
    // Restore normal internal bus arbitration priorities
    *ARB_IACR = iacr;
#endif
}

static cyg_uint32
mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 eccr_reg, mcisr_reg;

    // Read current state of ECC register
    eccr_reg = *IXP_DDR_ECCR;

    // and the interrupt status
    mcisr_reg = *IXP_DDR_MCISR;

    // Turn off all ecc error reporting
    *IXP_DDR_ECCR = 0xc;

#ifdef DEBUG_ECC
    diag_printf("mcu_ISR entry: ECCR = 0x%X, MCISR = 0x%X\n", eccr_reg, mcisr_reg);
#endif

    // Check for ECC Error 0
    if(mcisr_reg & 1) {
	
#ifdef DEBUG_ECC
	diag_printf("ELOG0 = 0x%X\n", *IXP_DDR_ELOG0);
	diag_printf("ECC Error Detected at Address 0x%X\n",*IXP_DDR_ECAR0);
#endif
	
	// Check for single-bit error
        if(!(*IXP_DDR_ELOG0 & 0x00000100)) {
	    // call ECC restoration function
	    _scrub_ecc((*IXP_DDR_ECAR0 - SDRAM_PHYS_BASE) + SDRAM_UNCACHED_BASE);

	    // Clear the MCISR
	    *IXP_DDR_MCISR = 1;
        } else {
#ifdef DEBUG_ECC
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error 1
    if(mcisr_reg & 2) {

#ifdef DEBUG_ECC
	diag_printf("ELOG1 = 0x%X\n",*IXP_DDR_ELOG1);
	diag_printf("ECC Error Detected at Address 0x%X\n",*IXP_DDR_ECAR1);	
#endif
        
	// Check for single-bit error
        if(!(*IXP_DDR_ELOG1 & 0x00000100))  {
	    // call ECC restoration function
	    _scrub_ecc((*IXP_DDR_ECAR1 - SDRAM_PHYS_BASE) + SDRAM_UNCACHED_BASE);
 
	    // Clear the MCISR
	    *IXP_DDR_MCISR = 2;
	}
	else {
#ifdef DEBUG_ECC
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error N
    if(mcisr_reg & 4) {
	// Clear the MCISR
	*IXP_DDR_MCISR = 4;
	diag_printf("Uncorrectable error during RMW\n");
    }
    
    // Restore ECCR register
    *IXP_DDR_ECCR = eccr_reg;

#ifdef DEBUG_ECC
    diag_printf("mcu_ISR exit: MCISR = 0x%X\n", *IXP_DDR_MCISR);
#endif

    return CYG_ISR_HANDLED;
}
#endif

//
// Memory layout - runtime variations of all kinds.
//
externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( 0 == (hal_dram_size & 0xfffff),
                "hal_dram_size not whole Mb" );
    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) ) {
        regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
    // Also, we must check for the top of the heap having moved.  This is
    // because the heap does not abut the top of memory.
#ifdef CYGMEM_SECTION_heap1
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_SECTION_heap1 + CYGMEM_SECTION_heap1_SIZE) ) {
        if ( regionend > (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size )
            regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
#endif
    return regionend;
}


/*------------------------------------------------------------------------*/
// EOF ixp425_misc.c

