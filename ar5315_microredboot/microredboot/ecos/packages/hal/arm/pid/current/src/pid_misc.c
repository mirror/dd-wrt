//==========================================================================
//
//      pid_misc.c
//
//      HAL misc board support code for ARM PID7
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
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_if.h>             // calling interface

/*------------------------------------------------------------------------*/
// On-board timer
/*------------------------------------------------------------------------*/

// Timer registers
#define CYG_DEVICE_TIMER1_BASE   0x0A800020
#define CYG_DEVICE_TIMER1_LOAD \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER1_BASE + 0x00))
    // Load value, read/write
#define CYG_DEVICE_TIMER1_CURRENT \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER1_BASE + 0x04))
    // Current value, read
#define CYG_DEVICE_TIMER1_CONTROL \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER1_BASE + 0x08))
    // Control register, read/write
#define CYG_DEVICE_TIMER1_CLEAR \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER1_BASE + 0x0C))
    // Clears interrrupt, write only

#define CYG_DEVICE_TIMER_BASE   0x0A800020
#define CYG_DEVICE_TIMER_LOAD \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x00))
    // Load value, read/write
#define CYG_DEVICE_TIMER_CURRENT \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x04))
    // Current value, read
#define CYG_DEVICE_TIMER_CONTROL \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x08))
    // Control register, read/write
#define CYG_DEVICE_TIMER_CLEAR \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x0C))
    // Clears interrrupt, write only

// Clock/timer control register
#define CTL_ENABLE      0x80            // Bit   7: 1 - counter enabled
#define CTL_DISABLE     0x00            //          0 - counter disabled
#define CTL_FREERUN     0x00            // Bit   6: 0 - free running counter
#define CTL_PERIODIC    0x40            //          1 - periodic timer mode
#define CTL_SCALE_1     0x00            // Bits 32: 00 - Scale clock by 1
#define CTL_SCALE_16    0x04            //          01 - Scale by 16
#define CTL_SCALE_256   0x08            //          10 - Scale by 256
                                        //               12.8us/tick

// Interrupt controller registers
#define CYG_DEVICE_ICTL_BASE    0x0A000000
#define CYG_DEVICE_IRQ_Status \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + 0x00))
    // Current status, read only
#define CYG_DEVICE_IRQ_Enable \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + 0x08))
    // Enable status, read only
#define CYG_DEVICE_IRQ_EnableSet \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + 0x08))
    // Enable (1's only), write only
#define CYG_DEVICE_IRQ_EnableClear \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + 0x0C))
    // Disable (1's only), write only

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, period);
    //diag_printf("psr = %x\n", psr());
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CONTROL, CTL_DISABLE);    // Turn off
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_LOAD, period);
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CONTROL,
                     CTL_ENABLE | CTL_PERIODIC | CTL_SCALE_16);
    _period = period;
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    //diag_init();  diag_printf("%s\n", __PRETTY_FUNCTION__);
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER_CLEAR, 0);
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;
//    diag_init();  diag_printf("%s\n", __PRETTY_FUNCTION__);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CURRENT, value);
    value &= 0xFFFF;
    *pvalue = _period - (value & 0xFFFF);   // Note: counter is only 16 bits
                                            //       and decreases
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 value;
    cyg_uint64 ticks = ((usecs*CYGNUM_HAL_RTC_PERIOD*CYGNUM_HAL_RTC_DENOMINATOR)/1000000);

    HAL_WRITE_UINT32(CYG_DEVICE_TIMER1_CONTROL, CTL_DISABLE);    // Turn off
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER1_LOAD, ticks);
    HAL_WRITE_UINT32(CYG_DEVICE_TIMER1_CONTROL,
                     CTL_ENABLE | CTL_FREERUN | CTL_SCALE_16);

    // Wait for timer to underflow
    do {
        HAL_READ_UINT32(CYG_DEVICE_TIMER_CURRENT, value);
        value &= 0xFFFF;
    } while (value < 0x7fff);

    HAL_WRITE_UINT32(CYG_DEVICE_TIMER1_CONTROL, CTL_DISABLE);    // Turn off
}

// -------------------------------------------------------------------------

void hal_hardware_init(void)
{
    // Any hardware/platform initialization that needs to be done.
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableClear, 0xFFFF); // Clear all
                                                         // interrupt sources
    // Set up eCos/ROM interfaces
    hal_if_init();
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    // Do hardware-level IRQ handling
    int irq_status, vector;
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Status, irq_status);
    //diag_init();  diag_printf("%s, status: %x\n", __PRETTY_FUNCTION__, irq_status); 
    for (vector = 1;  vector < 16;  vector++) {
        if (irq_status & (1<<vector)) return vector;
    }
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, vector);
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableClear, 1<<vector);
}

#if 0
void hal_interrupt_status(void)
{
    int irq_status, irq_enable, timer_status, timer_value, timer_load;
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Status, irq_status);
    HAL_READ_UINT32(CYG_DEVICE_IRQ_Enable, irq_enable);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_LOAD, timer_load);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CURRENT, timer_value);
    HAL_READ_UINT32(CYG_DEVICE_TIMER_CONTROL, timer_status);    
    diag_printf("Interrupt: IRQ: %x.%x, TIMER: %x.%x.%x, psr: %x\n",
                irq_status, irq_enable, timer_status, timer_value,
                timer_load, psr());
}
#endif

void hal_interrupt_unmask(int vector)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, vector);
    HAL_WRITE_UINT32(CYG_DEVICE_IRQ_EnableSet, 1<<vector);
}

void hal_interrupt_acknowledge(int vector)
{
    //diag_init();  diag_printf("%s(%d)\n", __PRETTY_FUNCTION__, vector);
}

void hal_interrupt_configure(int vector, int level, int up)
{
    //diag_init();  diag_printf("%s(%d,%d,%d)\n", __PRETTY_FUNCTION__, vector, level, up);
}

void hal_interrupt_set_level(int vector, int level)
{
    //diag_init();  diag_printf("%s(%d,%d)\n", __PRETTY_FUNCTION__, vector, level);
}

void hal_show_IRQ(int vector, int data, int handler)
{
    //    diag_printf("IRQ - vector: %x, data: %x, handler: %x\n", vector, data, handler);
}
 
/*---------------------------------------------------------------------------*/
/* End of hal_misc.c */
