//==========================================================================
//
//      e7t_misc.c
//
//      HAL misc board support code for ARM E7T
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2001-03-16
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
#include <cyg/hal/plf_io.h>             // platform registers

static cyg_uint32 _period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
static cyg_interrupt abort_interrupt;
static cyg_handle_t  abort_interrupt_handle;

// This ISR is called only for the Abort button interrupt
static int
e7t_abort_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_hal_user_break((CYG_ADDRWORD*)regs);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT0);
    return 0;  // No need to run DSR
}
#endif

void hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 tmod;

    // Disable timer 0
    HAL_READ_UINT32(E7T_TMOD, tmod);
    tmod &= ~(E7T_TMOD_TE0);
    HAL_WRITE_UINT32(E7T_TMOD, 0);

    tmod &= ~(E7T_TMOD_TMD0 | E7T_TMOD_TCLR0);
    tmod |= E7T_TMOD_TE0;

    // Set counter
    HAL_WRITE_UINT32(E7T_TDATA0, period);

    // And enable timer
    HAL_WRITE_UINT32(E7T_TMOD, tmod);

    _period = period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,
                             99,           // Priority
                             0,            // Data item passed to interrupt handler
                             e7t_abort_isr,
                             0,
                             &abort_interrupt_handle,
                             &abort_interrupt);
    cyg_drv_interrupt_attach(abort_interrupt_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;

    HAL_READ_UINT32(E7T_TCNT0, value);
    *pvalue = _period - value;
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 count;
    cyg_uint32 ticks = ((CYGNUM_HAL_RTC_PERIOD*CYGNUM_HAL_RTC_DENOMINATOR)/1000000) * usecs;
    cyg_uint32 tmod;

    // Disable timer 1
    HAL_READ_UINT32(E7T_TMOD, tmod);
    tmod &= ~(E7T_TMOD_TE1);
    HAL_WRITE_UINT32(E7T_TMOD, tmod);

    tmod &= ~(E7T_TMOD_TMD1 | E7T_TMOD_TCLR1);
    tmod |= E7T_TMOD_TE1;

    // Clear pending flag
    HAL_WRITE_UINT32(E7T_INTPND, (1 << CYGNUM_HAL_INTERRUPT_TIMER1));

    // Set counter
    HAL_WRITE_UINT32(E7T_TDATA1, ticks);

    // And enable timer
    HAL_WRITE_UINT32(E7T_TMOD, tmod);

    // Wait for timer to underflow. Can't test the timer completion
    // bit without actually enabling the interrupt. So instead watch
    // the counter.
    ticks /= 2;                         // wait for this threshold

    // Wait till timer counts below threshold
    do {
        HAL_READ_UINT32(E7T_TCNT1, count);
    } while (count >= ticks);
    // then wait for it to be reloaded
    do {
        HAL_READ_UINT32(E7T_TCNT1, count);
    } while (count < ticks);

    // Then disable timer 1 again
    tmod &= ~E7T_TMOD_TE1;
    HAL_WRITE_UINT32(E7T_TMOD, tmod);
}

// -------------------------------------------------------------------------
// Hardware init
void hal_hardware_init(void)
{
    cyg_uint32 intmask;

    // Set up eCos/ROM interfaces
    hal_if_init();

    // Enable cache
    HAL_WRITE_UINT32(E7T_SYSCFG, 
                     0x07FFFF80|E7T_SYSCFG_CM_0R_8C|E7T_SYSCFG_WE);
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();

    // Clear global interrupt mask bit
    HAL_READ_UINT32(E7T_INTMSK, intmask);
    intmask &= ~E7T_INTMSK_GLOBAL;
    HAL_WRITE_UINT32(E7T_INTMSK, intmask);
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    // Do hardware-level IRQ handling
    cyg_uint32 irq_status;
    HAL_READ_UINT32(E7T_INTOFFSET_IRQ, irq_status);
    irq_status = irq_status / 4;
    if (CYGNUM_HAL_ISR_MAX >= irq_status)
        return irq_status;
    // It's a bit bogus to test for FIQs after IRQs, but we use the
    // latter more, so don't impose the overhead of checking for FIQs
    HAL_READ_UINT32(E7T_INTOFFSET_FIQ, irq_status);
    irq_status = irq_status / 4;
    if (CYGNUM_HAL_ISR_MAX >= irq_status)
        return irq_status;
    return CYGNUM_HAL_INTERRUPT_NONE;
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(E7T_INTMSK, mask);
    old_mask = mask;
    mask |= (1<<vector);
    HAL_WRITE_UINT32(E7T_INTMSK, mask);
}

void hal_interrupt_unmask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(E7T_INTMSK, mask);
    old_mask = mask;
    mask &= ~(1<<vector);
    HAL_WRITE_UINT32(E7T_INTMSK, mask);
}

void hal_interrupt_acknowledge(int vector)
{
    HAL_WRITE_UINT32(E7T_INTPND, (1<<vector));
}

void hal_interrupt_configure(int vector, int level, int up)
{
}

void hal_interrupt_set_level(int vector, int level)
{
}

void hal_show_IRQ(int vector, int data, int handler)
{
}

//--------------------------------------------------------------------------
// EOF hal_misc.c
