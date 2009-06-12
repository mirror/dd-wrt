//==========================================================================
//
//      cma230_misc.c
//
//      HAL misc board support code for ARM CMA230-1
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

#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_cma230.h>         // Hardware definitions

static cyg_uint32 _period;

// Use Timer/Counter #2 for system clock

void hal_clock_initialize(cyg_uint32 period)
{
    // Initialize counter
    *(volatile cyg_uint16 *)CMA230_TC_ENABLE = 0;  // Disable timer
    *(volatile cyg_uint16 *)CMA230_TC_CLEAR = 0;   // Resets counter
    *(volatile cyg_uint16 *)CMA230_TC_PRELOAD = period;
    *(volatile cyg_uint16 *)CMA230_TC_ENABLE = 1;  // Starts timer    
    _period = period;
}

// This routine is called during a clock interrupt.

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    if (period != _period) {
        *(volatile cyg_uint16 *)CMA230_TC_PRELOAD = period;
        _period = period;
    }
}

// Read the current value of the clock, returning the number of hardware "ticks"
// that have occurred (i.e. how far away the current value is from the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    volatile cyg_int16 *tcct = (volatile cyg_int16 *)CMA230_TC_COUNT;
    static cyg_int32 clock_val;
    clock_val = *tcct;                 // Register has only 16 bits
    *pvalue = (cyg_uint32)(_period - clock_val);    // 'clock_val' counts down and wraps
}

//
// Early stage hardware initialization
//   Some initialization has already been done before we get here.  For now
// just set up the interrupt environment.

// Note: The hardware interrupt mask (read) doesn't seem to give reliable results.
static cyg_uint8 _imrr;
#undef  CMA230_IMRr
#define CMA230_IMRr (&_imrr)

void hal_hardware_init(void)
{
#if 0
    // Clear and initialize instruction cache
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_ICACHE_ENABLE();
#endif
    // Any hardware/platform initialization that needs to be done.
    // Reset all interrupt masks (disable all interrupt sources)
    *(volatile cyg_uint8 *)CMA230_IMRw = 0;
    *(volatile cyg_uint8 *)CMA230_CLR = 0xFF;  // Clear all current interrupts

    // Set up eCos/ROM interfaces
    hal_if_init();
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

#if 0
// TEMP
int tot_ints;
cyg_uint32 int_PC[2048];
// TEMP
#endif

int hal_IRQ_handler(HAL_SavedRegisters *regs)
{
    volatile cyg_uint8 isr = *(volatile cyg_uint8 *)CMA230_ISR;    
    volatile cyg_uint8 *imrr = (volatile cyg_uint8 *)CMA230_IMRr;
    int vector;
    isr &= *imrr;  // The Interrupt Source Register shows _all_ current
                   // interrupt sources, not just the enabled ones
#if 0
// TEMP
    int_PC[tot_ints++] = 0xFFFFFFFF;
    int_PC[tot_ints++] = isr;
    int_PC[tot_ints++] = regs->cpsr;
    int_PC[tot_ints++] = regs->pc;
    if (tot_ints == 2048) tot_ints = 0;
// TEMP
#endif

    for (vector = 0;  vector < 8;  vector++) {
        if (isr & (1<<vector)) {
            return (vector+1);
        }
    }
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

// Disable (mask) an interrupt
void hal_interrupt_mask(int vector)
{
    volatile cyg_uint8 *imrr = (volatile cyg_uint8 *)CMA230_IMRr;
    volatile cyg_uint8 *imrw = (volatile cyg_uint8 *)CMA230_IMRw;
    cyg_uint8 new_mask = *imrr & ~(1<<(vector-1));
    *imrw = new_mask;
// TEMP
    *imrr = new_mask;
// TEMP
//    diag_printf("Mask interrupt #%d - mask: %x\n", vector, *imrr);
}

// Enable (unmask) an interrupt
void hal_interrupt_unmask(int vector)
{
    volatile cyg_uint8 *imrr = (volatile cyg_uint8 *)CMA230_IMRr;
    volatile cyg_uint8 *imrw = (volatile cyg_uint8 *)CMA230_IMRw;
    cyg_uint8 new_mask = *imrr | (1<<(vector-1));
    *imrw = new_mask;
// TEMP
    *imrr = new_mask;
// TEMP
//    diag_printf("Unmask interrupt #%d - mask: %x\n", vector, *imrr);
}

void hal_interrupt_acknowledge(int vector)
{
    // These two vectors are "sticky" and must be reset
    if ((vector == CYGNUM_HAL_INTERRUPT_TIMER) ||
        (vector == CYGNUM_HAL_INTERRUPT_ABORT)) {
        *(volatile cyg_uint8 *)CMA230_CLR = (1<<(vector-1));
    }
}

void hal_interrupt_configure(int vector, int level, int up)
{
    // No interrupts are configurable on this hardware
}

void hal_interrupt_set_level(int vector, int level)
{
    // No interrupts are configurable on this hardware
}

/*------------------------------------------------------------------------*/
// EOF hal_misc.c
