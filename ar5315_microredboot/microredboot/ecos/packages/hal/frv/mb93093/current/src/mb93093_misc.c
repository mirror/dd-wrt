//==========================================================================
//
//      mb93093_misc.c
//
//      HAL misc board support code for Fujitsu MB93093 (FR-V 403)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// Date:         2001-09-07
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
#include <cyg/infra/diag.h>             // diag_printf() and friends

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/mb93093.h>            // Hardware definitions
#include <cyg/hal/hal_if.h>             // calling interface API

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    _period = period;
    // Set timer #1 to run in terminal count mode for period
    HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_SEL1|_FRVGEN_TCTR_RLOHI|_FRVGEN_TCTR_MODE0);
    HAL_WRITE_UINT8(_FRVGEN_TCSR1, period & 0xFF);
    HAL_WRITE_UINT8(_FRVGEN_TCSR1, period >> 8);
    // Configure interrupt
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_TIMER1, 1, 1);  // Interrupt when TOUT1 is high
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    cyg_int16 offset;
    cyg_uint8 _val;

    // Latch & read counter from timer #1
    HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_LATCH|_FRVGEN_TCTR_RLOHI|_FRVGEN_TCTR_SEL1);
    HAL_READ_UINT8(_FRVGEN_TCSR1, _val);
    offset = _val;
    HAL_READ_UINT8(_FRVGEN_TCSR1, _val);
    offset |= _val << 8;    // This will be the number of clocks beyond 0
    period += offset;
    // Reinitialize with adjusted count
    // Set timer #1 to run in terminal count mode for period
    HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_SEL1|_FRVGEN_TCTR_RLOHI|_FRVGEN_TCTR_MODE0);
    HAL_WRITE_UINT8(_FRVGEN_TCSR1, period & 0xFF);
    HAL_WRITE_UINT8(_FRVGEN_TCSR1, period >> 8);
}

// Read the current value of the clock, returning the number of hardware "ticks"
// that have occurred (i.e. how far away the current value is from the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_int16 offset;
    cyg_uint8 _val;

    // Latch & read counter from timer #1
    HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_LATCH|_FRVGEN_TCTR_RLOHI|_FRVGEN_TCTR_SEL1);
    HAL_READ_UINT8(_FRVGEN_TCSR1, _val);
    offset = _val;
    HAL_READ_UINT8(_FRVGEN_TCSR1, _val);
    offset |= _val << 8;

    // 'offset' is the current timer value
    *pvalue = _period - offset;
}

// Delay for some number of useconds.
// Assumptions:
//   Use timer #2
//   Min granularity is 10us
#define _MIN_DELAY 10

void hal_delay_us(int us)
{
    cyg_uint8 stat;
    int timeout;

    while (us >= _MIN_DELAY) {
	us -= _MIN_DELAY;
        // Set timer #2 to run in terminal count mode for _MIN_DELAY us
        HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_SEL2|_FRVGEN_TCTR_RLOHI|_FRVGEN_TCTR_MODE0);
        HAL_WRITE_UINT8(_FRVGEN_TCSR2, _MIN_DELAY & 0xFF);
        HAL_WRITE_UINT8(_FRVGEN_TCSR2, _MIN_DELAY >> 8);
        timeout = 100000;
        // Wait for TOUT to indicate terminal count reached
        do {
            HAL_WRITE_UINT8(_FRVGEN_TCTR, _FRVGEN_TCTR_RB|_FRVGEN_TCTR_RB_NCOUNT|_FRVGEN_TCTR_RB_CTR2);
            HAL_READ_UINT8(_FRVGEN_TCSR2, stat);
            if (--timeout == 0) break;
        } while ((stat & _FRVGEN_TCxSR_TOUT) == 0);
    }
}

//
// Early stage hardware initialization
//   Some initialization has already been done before we get here.  For now
// just set up the interrupt environment and the LEDs


long _system_clock;  // Calculated clock frequency

void hal_hardware_init(void)
{
    cyg_uint32 clk, clkc, psr;
    char *model;

    // Set up interrupt controller
    HAL_WRITE_UINT16(_FRVGEN_IRC_MASK, 0xFFFE);  // All masked
    HAL_WRITE_UINT16(_FRVGEN_IRC_RC, 0xFFFE);    // All cleared
    HAL_WRITE_UINT16(_FRVGEN_IRC_IRL, 0x10);     // Clear IRL (interrupt request latch)    

#if 0
    // Onboard FPGA interrupts
    HAL_WRITE_UINT16(_FRVGEN_FPGA_CONTROL, _FRVGEN_FPGA_CONTROL_IRQ);  // Enable IRQ registers
    HAL_WRITE_UINT16(_FRVGEN_FPGA_IRQ_MASK,      // Set up for LAN, PCI INTx
                     0x7FFE & 
                     ~(_FRVGEN_FPGA_IRQ_LAN |
                       _FRVGEN_FPGA_IRQ_INTA |
                       _FRVGEN_FPGA_IRQ_INTB |
                       _FRVGEN_FPGA_IRQ_INTC |
                       _FRVGEN_FPGA_IRQ_INTD)
        );

    HAL_WRITE_UINT16(_FRVGEN_FPGA_IRQ_LEVELS,    // Set up for LAN, PCI INTx
                     0x7FFE & 
                     ~(_FRVGEN_FPGA_IRQ_LAN |
                       _FRVGEN_FPGA_IRQ_INTA |
                       _FRVGEN_FPGA_IRQ_INTB |
                       _FRVGEN_FPGA_IRQ_INTC |
                       _FRVGEN_FPGA_IRQ_INTD)
        );

#endif
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_LAN, 1, 1);  // Level, High

    //    _system_clock = 65625000;
    _system_clock = 66000000;

    // Set scalers to achieve 1us resolution in timer
    HAL_WRITE_UINT8(_FRVGEN_TPRV, _system_clock / (1000*1000));
    HAL_WRITE_UINT8(_FRVGEN_TCKSL0, 0x80);
    HAL_WRITE_UINT8(_FRVGEN_TCKSL1, 0x80);
    HAL_WRITE_UINT8(_FRVGEN_TCKSL2, 0x80);

    // Make sure UART clock prescaler is at power-on reset setting.
    HAL_WRITE_UINT32(_FRVGEN_UCPSR, 0);
    HAL_WRITE_UINT32(_FRVGEN_UCPVR, 0);

    hal_if_init();

    // Initialize real-time clock (for delays, etc, even if kernel doesn't use it)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
#if 0
    case CYGNUM_HAL_INTERRUPT_LAN:
        HAL_READ_UINT16(_FRVGEN_FPGA_IRQ_MASK, _mask);
        _mask |= _FRVGEN_FPGA_IRQ_LAN;
        HAL_WRITE_UINT16(_FRVGEN_FPGA_IRQ_MASK, _mask);
        break;
#endif
    }
    HAL_READ_UINT16(_FRVGEN_IRC_MASK, _mask);
    _mask |= (1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRVGEN_IRC_MASK, _mask);
}

void hal_interrupt_unmask(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_LAN:
#if 0
        HAL_READ_UINT16(_FRVGEN_FPGA_IRQ_MASK, _mask);
        _mask &= ~_FRVGEN_FPGA_IRQ_LAN;
        HAL_WRITE_UINT16(_FRVGEN_FPGA_IRQ_MASK, _mask);
        break;
#endif
    }
    HAL_READ_UINT16(_FRVGEN_IRC_MASK, _mask);
    _mask &= ~(1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRVGEN_IRC_MASK, _mask);
}

void hal_interrupt_acknowledge(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_LAN:
#if 0
        HAL_WRITE_UINT16(_FRVGEN_FPGA_IRQ_REQUEST,      // Clear LAN interrupt
                         0x7FFE & ~_FRVGEN_FPGA_IRQ_LAN);
#endif
        break;
    }
    _mask = (1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRVGEN_IRC_RC, _mask);
    HAL_WRITE_UINT16(_FRVGEN_IRC_IRL, 0x10);  // Clears IRL latch
}

//
// Configure an interrupt
//  level - boolean (0=> edge, 1=>level)
//  up - edge: (0=>falling edge, 1=>rising edge)
//       level: (0=>low, 1=>high)
//
void hal_interrupt_configure(int vector, int level, int up)
{
    cyg_uint16 _irr, _tmr, _trig;

    if (level) {
        if (up) {
            _trig = 0;     // level, high
        } else {
            _trig = 1;     // level, low
        }
    } else {
        if (up) {
            _trig = 2;     // edge, rising
        } else {
            _trig = 3;     // edge, falling
        }
    }
    switch (vector) {
    case  CYGNUM_HAL_INTERRUPT_TIMER0:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR5, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFFC) | (_trig<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_TIMER1:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR5, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFF3) | (_trig<<2);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_TIMER2:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR5, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFCF) | (_trig<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA0:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR4, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFCFF) | (_trig<<8);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA1:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR4, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xF3FF) | (_trig<<10);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA2:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR4, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xCFFF) | (_trig<<12);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA3:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR4, _irr);
        _irr = (_irr & 0x0FFF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<12);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0x3FFF) | (_trig<<14);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_UART0:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR6, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR6, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM1, _tmr);
        _tmr = (_tmr & 0xFCFF) | (_trig<<8);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_UART1:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR6, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR6, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_ITM1, _tmr);
        _tmr = (_tmr & 0xF3FF) | (_trig<<10);
        HAL_WRITE_UINT16(_FRVGEN_IRC_ITM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT0:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR3, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFFC) | (_trig<<0);
        HAL_WRITE_UINT16(_FRVGEN_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT1:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR3, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFF3) | (_trig<<2);
        HAL_WRITE_UINT16(_FRVGEN_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT2:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR3, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFCF) | (_trig<<4);
        HAL_WRITE_UINT16(_FRVGEN_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT7:
        HAL_READ_UINT16(_FRVGEN_IRC_IRR2, _irr);
        _irr = (_irr & 0x0FFF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<12);
        HAL_WRITE_UINT16(_FRVGEN_IRC_IRR2, _irr);
        HAL_READ_UINT16(_FRVGEN_IRC_TM1, _tmr);
        _tmr = (_tmr & 0x3FFF) | (_trig<<14);
        HAL_WRITE_UINT16(_FRVGEN_IRC_TM1, _tmr);
        break;
    default:
        ; // Nothing to do
    };
}

void hal_interrupt_set_level(int vector, int level)
{
//    UNIMPLEMENTED(__FUNCTION__);
}

/*------------------------------------------------------------------------*/
// EOF mb93093_misc.c
