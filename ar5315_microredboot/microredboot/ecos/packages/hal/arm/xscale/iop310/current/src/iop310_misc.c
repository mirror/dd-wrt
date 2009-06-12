//==========================================================================
//
//      iop310_misc.c
//
//      HAL misc board support code for XScale IOP310
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Contributors: msalter, gthomas
// Date:         2000-10-10
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
#include <cyg/hal/hal_iop310.h>         // Hardware definitions
#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

static cyg_uint32 nfiq_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nirq_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nmi_mcu_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nmi_patu_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nmi_satu_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nmi_pb_ISR(cyg_vector_t vector, cyg_addrword_t data);
static cyg_uint32 nmi_sb_ISR(cyg_vector_t vector, cyg_addrword_t data);

// Some initialization has already been done before we get here.
//
// Set up the interrupt environment.
// Set up the MMU so that we can use caches.
// Enable caches.
// - All done!

void hal_hardware_init(void)
{
    hal_xscale_core_init();

    // Route INTA-INTD to IRQ pin
    //   The Yavapai manual is incorrect in that a '1' value
    //   routes to the IRQ line, not a '0' value.
    *PIRSR_REG = 0x0f;

    // Disable all interrupt sources:
    *IIMR_REG = 0x7f;
    *OIMR_REG = 0x7f; // don't mask INTD which is really xint3

    // Let the platform do any specific initializations
    hal_plf_hardware_init();

    // Mask off all interrupts via xint3
    *X3MASK_REG = 0x1F;

    // Let the timer run at a default rate (for delays)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    // Set up eCos/ROM interfaces
    hal_if_init();

    // attach some builtin interrupt handlers
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_NIRQ, &nirq_ISR, CYGNUM_HAL_INTERRUPT_NIRQ, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_NIRQ);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_NFIQ, &nfiq_ISR, CYGNUM_HAL_INTERRUPT_NFIQ, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_NFIQ);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_MCU_ERR, &nmi_mcu_ISR, CYGNUM_HAL_INTERRUPT_MCU_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_MCU_ERR);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_PATU_ERR, &nmi_patu_ISR, CYGNUM_HAL_INTERRUPT_PATU_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_PATU_ERR);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SATU_ERR, &nmi_satu_ISR, CYGNUM_HAL_INTERRUPT_SATU_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SATU_ERR);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_PBDG_ERR, &nmi_pb_ISR, CYGNUM_HAL_INTERRUPT_PBDG_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_PBDG_ERR);

    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SBDG_ERR, &nmi_sb_ISR, CYGNUM_HAL_INTERRUPT_SBDG_ERR, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SBDG_ERR);

#if 0
    // Enable FIQ
    {
        unsigned rtmp = 0;

        asm volatile ("mrs %0,cpsr\n"
                      "bic %0,%0,#0x40\n"
                      "msr cpsr,%0\n"
                      : "=r"(rtmp) : );
    }
#endif

    // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();
}

/*------------------------------------------------------------------------*/

//
// Memory layout
//

externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  512<<20,
                "More than 512MB reported - that can't be right" );

    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) ) {
        regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
    return regionend;
} // hal_arm_mem_real_region_top()


// -------------------------------------------------------------------------

// Clock can come from the PMU or from an external timer.
// The external timer is the preferred choice.

#if CYGNUM_HAL_INTERRUPT_RTC == CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL

// Proper version that uses the clock counter in the PMU to do proper
// interrupts that require acknowledgement and all that good stuff.

static cyg_uint32 hal_clock_init_period; // The START value, it counts up

void hal_clock_initialize(cyg_uint32 period)
{
    // event types both zero; clear all 3 interrupts;
    // disable all 3 counter interrupts;
    // CCNT counts every processor cycle; reset all counters;
    // enable PMU.
    register cyg_uint32 init = 0x00000707;
    asm volatile (
        "mcr      p14,0,%0,c0,c0,0;" // write into PMNC
        :
        : "r"(init)
        /*:*/
        );
    // the CCNT in the PMU counts *up* then interrupts at overflow
    // ie. at 0x1_0000_0000 as it were.
    // So init to 0xffffffff - period + 1 to get the right answer.
    period = (~period) + 1;
    hal_clock_init_period = period;
    hal_clock_reset( 0, 0 );
}

// This routine is called during a clock interrupt.
// (before acknowledging the interrupt)
void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    asm volatile (
        "mrc      p14,0,r0,c1,c0,0;" // read from CCNT - how long since OVFL
        "add      %0, %0, r0;"       // synchronize with previous overflow
        "mcr      p14,0,%0,c1,c0,0;" // write into CCNT
        :
        : "r"(hal_clock_init_period)
        : "r0"
        );
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    register cyg_uint32 now;
    asm volatile (
        "mrc      p14,0,%0,c1,c0,0;" // read from CCNT
        : "=r"(now)
        :
        /*:*/
        );
    *pvalue = now - hal_clock_init_period;
}

// Delay for some usecs.
void hal_delay_us(cyg_uint32 delay)
{
  int i;
  // the loop is going to take 3 ticks.  At 600 MHz, to give uS, multiply
  // by 600/3 = 200. No volatile is needed on i; gcc recognizes delay
  // loops and does NOT elide them.
  for ( i = 200 * delay; i ; i--)
    ;
}

#else // external timer

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    _period = period;

    // disable timer
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_CNT_DISAB();

    *TIMER_LA0_REG_ADDR = period;
    *TIMER_LA1_REG_ADDR = period >> 8;
    *TIMER_LA2_REG_ADDR = period >> 16;

    EXT_TIMER_INT_ENAB();
    EXT_TIMER_CNT_ENAB();
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
#define MIN_TICKS (500)
#define MAX_TICKS (0xffffff) // 24-bit timer

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
        else if ( newp > MAX_TICKS ) {
            newp = MAX_TICKS;
            // recalculate to get the exact delay for this integral hz
            // and hunt hz up to an acceptable value if necessary
            i = period * old_hz / newp;
            if ( i ) do {
                newp = period * old_hz / i;
                i++;
            } while (newp > MAX_TICKS && i);
        }

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

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    // to clear the timer interrupt, clear the timer interrupt
    // enable, then re-set the int. enable bit
    EXT_TIMER_INT_DISAB();
    EXT_TIMER_INT_ENAB();
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint8  cnt0, cnt1, cnt2, cnt3;
    cyg_uint32 timer_val;;

    // first read latches the count
    // Actually, it looks like there is a hardware problem where
    // invalid counts get latched. This do while loop appears
    // to get around the problem.
    do {
	cnt0 = *TIMER_LA0_REG_ADDR & TIMER_COUNT_MASK;
    } while (cnt0 == 0);
    cnt1 = *TIMER_LA1_REG_ADDR & TIMER_COUNT_MASK;
    cnt2 = *TIMER_LA2_REG_ADDR & TIMER_COUNT_MASK;
    cnt3 = *TIMER_LA3_REG_ADDR & 0xf;	/* only 4 bits in most sig. */

    /* now build up the count value */
    timer_val  =  ((cnt0 & 0x40) >> 1) | (cnt0 & 0x1f);
    timer_val |= (((cnt1 & 0x40) >> 1) | (cnt1 & 0x1f)) << 6;
    timer_val |= (((cnt2 & 0x40) >> 1) | (cnt2 & 0x1f)) << 12;
    timer_val |= cnt3 << 18;

    *pvalue = timer_val;
}

// Delay for some usecs.
void hal_delay_us(cyg_uint32 delay)
{
#define _CNT_MASK 0x3fffff
#define _TICKS_PER_USEC (EXT_TIMER_CLK_FREQ / 1000000)
    cyg_uint32 now, last, diff, ticks;

    hal_clock_read(&last);
    diff = ticks = 0;

    while (delay > ticks) {
	hal_clock_read(&now);

	if (now < last)
	    diff += ((_period - last) + now);
	else
	    diff += (now - last);

	last = now;

	if (diff >= _TICKS_PER_USEC) {
	    ticks += (diff / _TICKS_PER_USEC);
	    diff %= _TICKS_PER_USEC;
	}
    }
}

#endif

// -------------------------------------------------------------------------

typedef cyg_uint32 cyg_ISR(cyg_uint32 vector, CYG_ADDRWORD data);

extern void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );

static inline cyg_uint32
hal_call_isr (cyg_uint32 vector)
{
    cyg_ISR *isr;
    CYG_ADDRWORD data;
    cyg_uint32 isr_ret;

    isr = (cyg_ISR*) hal_interrupt_handlers[vector];
    data = hal_interrupt_data[vector];

    isr_ret = (*isr) (vector, data);

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    if (isr_ret & CYG_ISR_CALL_DSR) {
        cyg_interrupt_post_dsr (hal_interrupt_objects[vector]);
    }
#endif

    return isr_ret & ~CYG_ISR_CALL_DSR;
}

void _scrub_ecc(unsigned p)
{
    asm volatile ("ldrb r4, [%0]\n"
		  "strb r4, [%0]\n" : : "r"(p) );
}

static cyg_uint32 nmi_mcu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 eccr_reg;

    // Read current state of ECC register
    eccr_reg = *ECCR_REG;

    // Turn off all ecc error reporting
    *ECCR_REG = 0x4;

    // Check for ECC Error 0
    if(*MCISR_REG & 0x1) {
	
#ifdef DEBUG_NMI
	diag_printf("ELOG0 = 0x%X\n", *ELOG0_REG);
	diag_printf("ECC Error Detected at Address 0x%X\n",*ECAR0_REG);		
#endif
	
	// Check for single-bit error
        if(!(*ELOG0_REG & 0x00000100)) {
	    // call ECC restoration function
	    _scrub_ecc(*ECAR0_REG);

	    // Clear the MCISR
	    *MCISR_REG = 0x1;
        } else {
#ifdef DEBUG_NMI
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error 1
    if(*MCISR_REG & 0x2) {

#ifdef DEBUG_NMI
	diag_printf("ELOG0 = 0x%X\n",*ELOG1_REG);
	diag_printf("ECC Error Detected at Address 0x%X\n",*ECAR1_REG);	
#endif
        
	// Check for single-bit error
        if(!(*ELOG1_REG & 0x00000100))  {
	    // call ECC restoration function
	    _scrub_ecc(*ECAR1_REG);
 
	    // Clear the MCISR
	    *MCISR_REG = 0x2;
	}
	else {
#ifdef DEBUG_NMI
            diag_printf("Multi-bit or nibble error\n");
#endif
	}
    }

    // Check for ECC Error N
    if(*MCISR_REG & 0x4) {
	// Clear the MCISR
	*MCISR_REG = 0x4;
	diag_printf("Uncorrectable error during RMW\n");
    }
    
    // Restore ECCR register
    *ECCR_REG = eccr_reg;

    // clear the interrupt condition
    *MCISR_REG = *MCISR_REG & 7;

    return CYG_ISR_HANDLED;
}

static cyg_uint32 nmi_patu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 status;

    status = *PATUISR_REG;

#ifdef DEBUG_NMI
    if (status & 0x001) diag_printf ("PPCI Master Parity Error\n");
    if (status & 0x002) diag_printf ("PPCI Target Abort (target)\n");
    if (status & 0x004) diag_printf ("PPCI Target Abort (master)\n");
    if (status & 0x008) diag_printf ("PPCI Master Abort\n");
    if (status & 0x010) diag_printf ("Primary P_SERR# Detected\n");
    if (status & 0x080) diag_printf ("Internal Bus Master Abort\n");
    if (status & 0x100) diag_printf ("PATU BIST Interrupt\n");
    if (status & 0x200) diag_printf ("PPCI Parity Error Detected\n");
    if (status & 0x400) diag_printf ("Primary P_SERR# Asserted\n");
#endif

    *PATUISR_REG = status & 0x79f;
    *PATUSR_REG |= 0xf900;

    return CYG_ISR_HANDLED;
}


static cyg_uint32 nmi_satu_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 status;

    status = *SATUISR_REG;

#ifdef DEBUG_NMI
    if (status & 0x001) diag_printf ("SPCI Master Parity Error\n");
    if (status & 0x002) diag_printf ("SPCI Target Abort (target)\n");
    if (status & 0x004) diag_printf ("SPCI Target Abort (master)\n");
    if (status & 0x008) diag_printf ("SPCI Master Abort\n");
    if (status & 0x010) diag_printf ("Secondary P_SERR# Detected\n");
    if (status & 0x080) diag_printf ("Internal Bus Master Abort\n");
    if (status & 0x200) diag_printf ("SPCI Parity Error Detected\n");
    if (status & 0x400) diag_printf ("Secondary P_SERR# Asserted\n");
#endif

    *SATUISR_REG = status & 0x69f;
    *SATUSR_REG |= 0xf900;

    return CYG_ISR_HANDLED;
}

static cyg_uint32 nmi_pb_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 status;

    status = *PBISR_REG;

#ifdef DEBUG_NMI
    if (status & 0x001) diag_printf ("PPCI Master Parity Error\n");
    if (status & 0x002) diag_printf ("PPCI Target Abort (target)\n");
    if (status & 0x004) diag_printf ("PPCI Target Abort (master)\n");
    if (status & 0x008) diag_printf ("PPCI Master Abort\n");
    if (status & 0x010) diag_printf ("Primary P_SERR# Asserted\n");
    if (status & 0x020) diag_printf ("PPCI Parity Error Detected\n");
#endif

    *PBISR_REG = status & 0x3f;
    *PSR_REG |= 0xf900;

    return CYG_ISR_HANDLED;
}


static cyg_uint32 nmi_sb_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 status;

    status = *SBISR_REG;

    *SBISR_REG = status & 0x7f;
    *SSR_REG |= 0xf900;

    return CYG_ISR_HANDLED;
}


static cyg_uint32 nfiq_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 sources;
    int i, isr_ret;

    // Check NMI
    sources = *NISR_REG;
    for (i = 0; i < 12; i++) {
	if (sources & (1<<i)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_MCU_ERR + i);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "Interrupt not handled");
	    return isr_ret;
	}
    }
    return 0;
}

static cyg_uint32 nirq_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 sources;
    int i, isr_ret;
    cyg_uint32 xint3_isr, xint3_mask;

    // Check XINT3
    sources = (xint3_isr = *X3ISR_REG) & ~(xint3_mask = *X3MASK_REG);
    for (i = 0; i <= CYGNUM_HAL_INTERRUPT_XINT3_BITS; i++) {
	if (sources & (1 << i)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_XINT3_BIT0 + i);
            if ((isr_ret & CYG_ISR_HANDLED) == 0) {
                diag_printf("XINT3 int not handled - ISR: %02x, MASK: %02x\n", xint3_isr, ~xint3_mask);
            }
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "XINT3 Interrupt not handled");
	    return isr_ret;
	}
    }
    // What to do about S_INTA-S_INTC?

    // Check XINT6
    sources = *X6ISR_REG;
    for (i = 0; i < 3; i++) {
	// check DMA irqs
	if (sources & (1<<i)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_DMA_0 + i);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "DMA Interrupt not handled");
	    return isr_ret;
	}
    }
    if (sources & 0x10) {
	// performance monitor
	_80312_EMISR = *EMISR_REG;
	if (_80312_EMISR & 1) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_GTSC);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "GTSC Interrupt not handled");
	}
	if (_80312_EMISR & 0x7ffe) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_PEC);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "PEC Interrupt not handled");
	}
	return 0;
    }
    if (sources & 0x20) {
	// Application Accelerator Unit
	isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_AAIP);
	CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "AAIP Interrupt not handled");
	return isr_ret;
    }
    
    // Check XINT7
    sources = *X7ISR_REG;
    if (sources & 2) {
	// I2C Unit
	cyg_uint32 i2c_sources = *ISR_REG;
	
	if (i2c_sources & (1<<7)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_RX_FULL);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C RX FULL Interrupt not handled");
	}
	if (i2c_sources & (1<<6)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C TX EMPTY Interrupt not handled");
	}
	if (i2c_sources & (1<<10)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_BUS_ERR);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C BUS ERR Interrupt not handled");
	}
	if (i2c_sources & (1<<4)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_STOP);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C STOP Interrupt not handled");
	}
	if (i2c_sources & (1<<5)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_LOSS);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C LOSS Interrupt not handled");
	}
	if (i2c_sources & (1<<9)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_I2C_ADDRESS);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "I2C ADDRESS Interrupt not handled");
	}
	return 0;
    }
    if (sources & 4) {
	// Messaging Unit
	cyg_uint32 inb_sources = *IISR_REG;

	if (inb_sources & (1<<0)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_MESSAGE_0);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "MESSAGE 0 Interrupt not handled");
	}
	if (inb_sources & (1<<1)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_MESSAGE_1);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "MESSAGE 1 Interrupt not handled");
	}
	if (inb_sources & (1<<2)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_DOORBELL);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "DOORBELL Interrupt not handled");
	}
	if (inb_sources & (1<<4)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_QUEUE_POST);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "QUEUE POST Interrupt not handled");
	}
	if (inb_sources & (1<<6)) {
	    isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_INDEX_REGISTER);
	    CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "INDEX REGISTER Interrupt not handled");
	}
	return 0;
    }
    if (sources & 8) {
	// BIST
	isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_BIST);
	CYG_ASSERT (isr_ret & CYG_ISR_HANDLED, "BIST Interrupt not handled");
    }

    return 0;
}

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
    int sources, masks;

    asm volatile ( // read the interrupt source reg INTSRC
        "mrc      p13,0,%0,c4,c0,0;"
        : "=r"(sources)
        : 
      /*:*/
        );
    asm volatile ( // read the interrupt control reg INTCTL
        "mrc      p13,0,%0,c0,c0,0;"
        : "=r"(masks)
        : 
      /*:*/
        );
    // is a source both unmasked and active?
    if ( (0 != (1 & masks)) && (0 != ((8 << 28) & sources)) )
        return CYGNUM_HAL_INTERRUPT_NFIQ;
    if ( (0 != (2 & masks)) && (0 != ((4 << 28) & sources)) )
        return CYGNUM_HAL_INTERRUPT_NIRQ;
    if ( (0 != (8 & masks)) && (0 != ((2 << 28) & sources)) )
        return CYGNUM_HAL_INTERRUPT_BCU_INTERRUPT;
    if ( (0 != (4 & masks)) && (0 != ((1 << 28) & sources)) ) {
        // more complicated; it's the PMU.
        asm volatile ( // read the PMNC perfmon control reg
            "mrc      p14,0,%0,c0,c0,0;"
            : "=r"(sources)
            : 
            /*:*/
            );
        // sources is now the PMNC performance monitor control register
        // enable bits are 4..6, status bits are 8..10
        sources = (sources >> 4) & (sources >> 8);
        if ( 1 & sources )
            return CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL;
        if ( 2 & sources )
            return CYGNUM_HAL_INTERRUPT_PMU_PMN1_OVFL;
        if ( 4 & sources )
            return CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL;
    }

    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    int mask = 0;
    int submask = 0;
    switch ( vector ) {
    case CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_PMN1_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL:
        submask = vector - CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL; // 0 to 2
        // select interrupt enable bit and also enable the perfmon per se
        submask = (1 << (submask + 4)); // bits 4-6 are masks
        asm volatile (
            "mrc      p14,0,r1,c0,c0,0;"
            "bic      r1, r1, #0x700;" // clear the overflow/interrupt flags
            "bic      r1, r1, #0x006;" // clear the reset bits
            "bic      %0, r1, %0;"     // preserve r1; better for debugging
            "tsts     %0, #0x070;"     // are all 3 sources now off?
            "biceq    %0, %0, #1;"     // if so, disable entirely.
            "mcr      p14,0,%0,c0,c0,0;"
            :
            : "r"(submask)
            : "r1"
            );
        mask = 4;
        break;
    case CYGNUM_HAL_INTERRUPT_BCU_INTERRUPT:
        // Nothing specific to do here
        mask = 8;
        break;
    case CYGNUM_HAL_INTERRUPT_NIRQ         :
        mask = 2;
        break;
    case CYGNUM_HAL_INTERRUPT_NFIQ         :
        mask = 1;
        break;
    case CYGNUM_HAL_INTERRUPT_GTSC:
        *GTMR_REG &= ~1;
        return;
    case CYGNUM_HAL_INTERRUPT_PEC:
        *ESR_REG &= ~(1<<16);
        return;
    case CYGNUM_HAL_INTERRUPT_AAIP:
        *ADCR_REG &= ~1;
        return;
    case CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY ... CYGNUM_HAL_INTERRUPT_I2C_ADDRESS:
        *ICR_REG &= ~(1<<(vector - CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY));
	return;
    case CYGNUM_HAL_INTERRUPT_MESSAGE_0 ... CYGNUM_HAL_INTERRUPT_INDEX_REGISTER:
        *IIMR_REG &= ~(1<<(vector - CYGNUM_HAL_INTERRUPT_MESSAGE_0));
	return;
    case CYGNUM_HAL_INTERRUPT_BIST:
        *ATUCR_REG &= ~(1<<3);
	return;
    case CYGNUM_HAL_INTERRUPT_P_SERR:  // FIQ
        *ATUCR_REG &= ~(1<<9);
	return;
    case CYGNUM_HAL_INTERRUPT_S_SERR:  // FIQ
        *ATUCR_REG &= ~(1<<10);
	return;
    case CYGNUM_HAL_INTERRUPT_XINT3_BIT0 ... CYGNUM_HAL_INTERRUPT_XINT3_BIT4:
        *X3MASK_REG |= (1<<(vector - CYGNUM_HAL_INTERRUPT_XINT3_BIT0));
	return;

#ifdef CYGNUM_HAL_INTERRUPT_PCI_S_INTC	
    // The hardware doesn't (yet?) provide masking or status for these
    // even though they can trigger cpu interrupts. ISRs will need to
    // poll the device to see if the device actually triggered the
    // interrupt.
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTC:
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTB:
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTA:
    default:
        /* do nothing */
        return;
#endif
    }
    asm volatile (
        "mrc      p13,0,r1,c0,c0,0;"
        "bic      r1, r1, %0;"
        "mcr      p13,0,r1,c0,c0,0;"
        :
        : "r"(mask)
        : "r1"
        );
}

void hal_interrupt_unmask(int vector)
{
    int mask = 0;
    int submask = 0;
    switch ( vector ) {
    case CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_PMN1_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL:
        submask = vector - CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL; // 0 to 2
        // select interrupt enable bit and also enable the perfmon per se
        submask = 1 + (1 << (submask + 4)); // bits 4-6 are masks
        asm volatile (
            "mrc      p14,0,r1,c0,c0,0;"
            "bic      r1, r1, #0x700;"   // clear the overflow/interrupt flags
            "bic      r1, r1, #0x006;"   // clear the reset bits
            "orr      %0, r1, %0;"       // preserve r1; better for debugging
            "mcr      p14,0,%0,c0,c0,0;"
            "mrc      p13,0,r2,c8,c0,0;" // steer PMU interrupt to IRQ
            "and      r2, r2, #2;"       // preserve the other bit (BCU steer)
            "mcr      p13,0,r2,c8,c0,0;"
            :
            : "r"(submask)
            : "r1","r2"
            );
        mask = 4;
        break;
    case CYGNUM_HAL_INTERRUPT_BCU_INTERRUPT:
         asm volatile (
            "mrc      p13,0,r2,c8,c0,0;" // steer BCU interrupt to IRQ
            "and      r2, r2, #1;"       // preserve the other bit (PMU steer)
            "mcr      p13,0,r2,c8,c0,0;"
            :
            : 
            : "r2"
            );
        mask = 8;
        break;
    case CYGNUM_HAL_INTERRUPT_NIRQ         :
        mask = 2;
        break;
    case CYGNUM_HAL_INTERRUPT_NFIQ         :
        mask = 1;
        break;
    case CYGNUM_HAL_INTERRUPT_GTSC:
        *GTMR_REG |= 1;
        return;
    case CYGNUM_HAL_INTERRUPT_PEC:
        *ESR_REG |= (1<<16);
        return;
    case CYGNUM_HAL_INTERRUPT_AAIP:
        *ADCR_REG |= 1;
        return;
    case CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY ... CYGNUM_HAL_INTERRUPT_I2C_ADDRESS:
        *ICR_REG |= (1<<(vector - CYGNUM_HAL_INTERRUPT_I2C_TX_EMPTY));
	return;
    case CYGNUM_HAL_INTERRUPT_MESSAGE_0 ... CYGNUM_HAL_INTERRUPT_INDEX_REGISTER:
        *IIMR_REG |= (1<<(vector - CYGNUM_HAL_INTERRUPT_MESSAGE_0));
	return;
    case CYGNUM_HAL_INTERRUPT_BIST:
        *ATUCR_REG |= (1<<3);
	return;
    case CYGNUM_HAL_INTERRUPT_P_SERR:  // FIQ
        *ATUCR_REG |= (1<<9);
	return;
    case CYGNUM_HAL_INTERRUPT_S_SERR:  // FIQ
        *ATUCR_REG |= (1<<10);
	return;
    case CYGNUM_HAL_INTERRUPT_XINT3_BIT0 ... CYGNUM_HAL_INTERRUPT_XINT3_BIT4:
        *X3MASK_REG &= ~(1<<(vector - CYGNUM_HAL_INTERRUPT_XINT3_BIT0));
	return;

#ifdef CYGNUM_HAL_INTERRUPT_PCI_S_INTC	
    // The hardware doesn't (yet?) provide masking or status for these
    // even though they can trigger cpu interrupts. ISRs will need to
    // poll the device to see if the device actually triggered the
    // interrupt.
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTC:
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTB:
    case CYGNUM_HAL_INTERRUPT_PCI_S_INTA:
    default:
        /* do nothing */
        return;
#endif
    }
    asm volatile (
        "mrc      p13,0,r1,c0,c0,0;"
        "orr      %0, r1, %0;"
        "mcr      p13,0,%0,c0,c0,0;"
        :
        : "r"(mask)
        : "r1"
        );
}

void hal_interrupt_acknowledge(int vector)
{
    int submask = 0;
    switch ( vector ) {
    case CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_PMN1_OVFL:
    case CYGNUM_HAL_INTERRUPT_PMU_CCNT_OVFL:
        submask = vector - CYGNUM_HAL_INTERRUPT_PMU_PMN0_OVFL; // 0 to 2
        // select interrupt enable bit and also enable the perfmon per se
        submask = (1 << (submask + 8)); // bits 8-10 are status; write 1 clr
        // Careful not to ack other interrupts or zero any counters:
        asm volatile (
            "mrc      p14,0,r1,c0,c0,0;"
            "bic      r1, r1, #0x700;" // clear the overflow/interrupt flags
            "bic      r1, r1, #0x006;" // clear the reset bits
            "orr      %0, r1, %0;"     // preserve r1; better for debugging
            "mcr      p14,0,%0,c0,c0,0;"
            :
            : "r"(submask)
            : "r1"
            );
        break;
    case CYGNUM_HAL_INTERRUPT_BCU_INTERRUPT:
    case CYGNUM_HAL_INTERRUPT_NIRQ         :
    case CYGNUM_HAL_INTERRUPT_NFIQ         :
    default:
        /* do nothing */
        return;
    }
}

void hal_interrupt_configure(int vector, int level, int up)
{
}

void hal_interrupt_set_level(int vector, int level)
{
}

/*------------------------------------------------------------------------*/
// EOF iop310_misc.c
