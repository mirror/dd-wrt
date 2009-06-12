/*==========================================================================
//
//      at91_misc.c
//
//      HAL misc board support code for Atmel AT91
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Contributors: gthomas, jskov, nickg, tkoeller
// Date:         2001-07-12
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
#include <cyg/hal/var_io.h>             // platform registers

// -------------------------------------------------------------------------
// Clock support

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    CYG_ADDRESS timer = AT91_TC+AT91_TC_TC0;

    CYG_ASSERT(period < 0x10000, "Invalid clock period");

    // Disable counter
    HAL_WRITE_UINT32(timer+AT91_TC_CCR, AT91_TC_CCR_CLKDIS);

    // Set registers
    HAL_WRITE_UINT32(timer+AT91_TC_CMR, AT91_TC_CMR_CPCTRG |        // Reset counter on CPC
                                        AT91_TC_CMR_CLKS_MCK32);    // 1 MHz
    HAL_WRITE_UINT32(timer+AT91_TC_RC, period);

    // Start timer
    HAL_WRITE_UINT32(timer+AT91_TC_CCR, AT91_TC_CCR_TRIG | AT91_TC_CCR_CLKEN);

    // Enable timer 0 interrupt    
    HAL_WRITE_UINT32(timer+AT91_TC_IER, AT91_TC_IER_CPC);
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    CYG_ADDRESS timer = AT91_TC+AT91_TC_TC0;
    cyg_uint32 sr;

    CYG_ASSERT(period < 0x10000, "Invalid clock period");

    HAL_READ_UINT32(timer+AT91_TC_SR, sr);  // Clear interrupt

    if (period != _period) {
        hal_clock_initialize(period);
    }
    _period = period;

}

void hal_clock_read(cyg_uint32 *pvalue)
{
    CYG_ADDRESS timer = AT91_TC+AT91_TC_TC0;
    cyg_uint32 val;

    HAL_READ_UINT32(timer+AT91_TC_CV, val);
    *pvalue = val;
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//   Use timer #2 in MCLOCK/32 mode.
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 stat;
    cyg_uint64 ticks;
#if defined(CYGHWR_HAL_ARM_AT91_JTST)
    // TC2 is reserved for AD/DA. Use TC1 instead. 
    CYG_ADDRESS timer = AT91_TC+AT91_TC_TC1;
#else
    CYG_ADDRESS timer = AT91_TC+AT91_TC_TC2;
#endif
    // Calculate how many timer ticks the required number of
    // microseconds equate to. We do this calculation in 64 bit
    // arithmetic to avoid overflow.
    ticks = (((cyg_uint64)usecs) * ((cyg_uint64)CYGNUM_HAL_ARM_AT91_CLOCK_SPEED))/32000000LL;
    
    // Disable counter
    HAL_WRITE_UINT32(timer+AT91_TC_CCR, AT91_TC_CCR_CLKDIS);

    // Set registers
    HAL_WRITE_UINT32(timer+AT91_TC_CMR, AT91_TC_CMR_CLKS_MCK32);  // 1MHz
    HAL_WRITE_UINT32(timer+AT91_TC_RA, 0);
    HAL_WRITE_UINT32(timer+AT91_TC_RC, ticks);

    // Start timer
    HAL_WRITE_UINT32(timer+AT91_TC_CCR, AT91_TC_CCR_TRIG | AT91_TC_CCR_CLKEN);

    // Wait for the compare
    do {
        HAL_READ_UINT32(timer+AT91_TC_SR, stat);
    } while ((stat & AT91_TC_SR_CPC) == 0);
}

// -------------------------------------------------------------------------
// Hardware init

void hal_hardware_init(void)
{
    unsigned i;

    // Reset all interrupts
    HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_IDCR, 0xFFFFFFFF);

    // Flush internal priority level stack
    for (i = 0; i < 8; ++i)
        HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_EOI, 0xFFFFFFFF);
    
    // Set up eCos/ROM interfaces
    hal_if_init();

}

// -------------------------------------------------------------------------
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    cyg_uint32 irq_num;
    cyg_uint32 ivr;
#ifdef CYGHWR_HAL_ARM_AT91_FIQ
    // handle fiq interrupts as irq 
    cyg_uint32 ipr,imr;

    HAL_READ_UINT32(AT91_AIC+AT91_AIC_IPR, ipr);
    HAL_READ_UINT32(AT91_AIC+AT91_AIC_IMR, imr);

    if (imr & ipr & (1 << CYGNUM_HAL_INTERRUPT_FIQ)) {
      HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_ICCR, (1 << CYGNUM_HAL_INTERRUPT_FIQ));
      return CYGNUM_HAL_INTERRUPT_FIQ;
    }
#endif
    // Calculate active interrupt (updates ISR)
    HAL_READ_UINT32(AT91_AIC+AT91_AIC_IVR, ivr);

    HAL_READ_UINT32(AT91_AIC+AT91_AIC_ISR, irq_num);

    // An invalid interrrupt source is treated as a spurious interrupt    
    if (irq_num < CYGNUM_HAL_ISR_MIN || irq_num > CYGNUM_HAL_ISR_MAX)
      irq_num = CYGNUM_HAL_INTERRUPT_NONE;
    
    return irq_num;
}

// -------------------------------------------------------------------------
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_IDCR, (1<<vector));
}

void hal_interrupt_unmask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_IECR, (1<<vector));
}

void hal_interrupt_acknowledge(int vector)
{
    // No check for valid vector here! Spurious interrupts
    // must be acknowledged, too.
    HAL_WRITE_UINT32(AT91_AIC+AT91_AIC_EOI, 0xFFFFFFFF);  
}

void hal_interrupt_configure(int vector, int level, int up)
{
    cyg_uint32 mode;

    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    if (level) {
        if (up) {
            mode = AT91_AIC_SMR_LEVEL_HI;
        } else {
            mode = AT91_AIC_SMR_LEVEL_LOW;
        }
    } else {
        if (up) {
            mode = AT91_AIC_SMR_EDGE_POS;
        } else {
            mode = AT91_AIC_SMR_EDGE_NEG;
        }
    }
    mode |= 7;  // Default priority
    HAL_WRITE_UINT32(AT91_AIC+(AT91_AIC_SMR0+(vector*4)), mode);
}

void hal_interrupt_set_level(int vector, int level)
{
    cyg_uint32 mode;

    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
    CYG_ASSERT(level >= 0 && level <= 7, "Invalid level");

    HAL_READ_UINT32(AT91_AIC+(AT91_AIC_SMR0+(vector*4)), mode);
    mode = (mode & ~AT91_AIC_SMR_PRIORITY) | level;
    HAL_WRITE_UINT32(AT91_AIC+(AT91_AIC_SMR0+(vector*4)), mode);
}

void hal_show_IRQ(int vector, int data, int handler)
{
//    UNDEFINED(__FUNCTION__);  // FIXME
}


/* Use the watchdog to generate a reset */
void hal_at91_reset_cpu(void)
{
    HAL_WRITE_UINT32(AT91_WD + AT91_WD_OMR, AT91_WD_OMR_OKEY);
    HAL_WRITE_UINT32(AT91_WD + AT91_WD_CMR, AT91_WD_CMR_CKEY);
    HAL_WRITE_UINT32(AT91_WD + AT91_WD_CR, AT91_WD_CR_RSTKEY);
    HAL_WRITE_UINT32(AT91_WD + AT91_WD_OMR, AT91_WD_OMR_OKEY | AT91_WD_OMR_RSTEN | AT91_WD_OMR_WDEN);
    while(1) CYG_EMPTY_STATEMENT;
}

//--------------------------------------------------------------------------
// EOF at91_misc.c
