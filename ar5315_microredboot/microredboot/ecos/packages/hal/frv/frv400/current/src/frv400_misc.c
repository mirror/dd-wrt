//==========================================================================
//
//      frv400_misc.c
//
//      HAL misc board support code for Fujitsu MB93091 ( FR-V 400)
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
#include <cyg/hal/frv400.h>             // Hardware definitions
#include <cyg/hal/hal_if.h>             // calling interface API

#include <pkgconf/io_pci.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    _period = period;
    // Set timer #1 to run in terminal count mode for period
    HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_SEL1|_FRV400_TCTR_RLOHI|_FRV400_TCTR_MODE0);
    HAL_WRITE_UINT8(_FRV400_TCSR1, period & 0xFF);
    HAL_WRITE_UINT8(_FRV400_TCSR1, period >> 8);
    // Configure interrupt
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_TIMER1, 1, 1);  // Interrupt when TOUT1 is high
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    cyg_int16 offset;
    cyg_uint8 _val;

    // Latch & read counter from timer #1
    HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_LATCH|_FRV400_TCTR_RLOHI|_FRV400_TCTR_SEL1);
    HAL_READ_UINT8(_FRV400_TCSR1, _val);
    offset = _val;
    HAL_READ_UINT8(_FRV400_TCSR1, _val);
    offset |= _val << 8;    // This will be the number of clocks beyond 0
    period += offset;
    // Reinitialize with adjusted count
    // Set timer #1 to run in terminal count mode for period
    HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_SEL1|_FRV400_TCTR_RLOHI|_FRV400_TCTR_MODE0);
    HAL_WRITE_UINT8(_FRV400_TCSR1, period & 0xFF);
    HAL_WRITE_UINT8(_FRV400_TCSR1, period >> 8);
}

// Read the current value of the clock, returning the number of hardware "ticks"
// that have occurred (i.e. how far away the current value is from the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_int16 offset;
    cyg_uint8 _val;

    // Latch & read counter from timer #1
    HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_LATCH|_FRV400_TCTR_RLOHI|_FRV400_TCTR_SEL1);
    HAL_READ_UINT8(_FRV400_TCSR1, _val);
    offset = _val;
    HAL_READ_UINT8(_FRV400_TCSR1, _val);
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
        HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_SEL2|_FRV400_TCTR_RLOHI|_FRV400_TCTR_MODE0);
        HAL_WRITE_UINT8(_FRV400_TCSR2, _MIN_DELAY & 0xFF);
        HAL_WRITE_UINT8(_FRV400_TCSR2, _MIN_DELAY >> 8);
        timeout = 100000;
        // Wait for TOUT to indicate terminal count reached
        do {
            HAL_WRITE_UINT8(_FRV400_TCTR, _FRV400_TCTR_RB|_FRV400_TCTR_RB_NCOUNT|_FRV400_TCTR_RB_CTR2);
            HAL_READ_UINT8(_FRV400_TCSR2, stat);
            if (--timeout == 0) break;
        } while ((stat & _FRV400_TCxSR_TOUT) == 0);
    }
}

//
// Early stage hardware initialization
//   Some initialization has already been done before we get here.  For now
// just set up the interrupt environment.

long _system_clock;  // Calculated clock frequency

void hal_hardware_init(void)
{
    cyg_uint32 clk;

    // Set up interrupt controller
    HAL_WRITE_UINT16(_FRV400_IRC_MASK, 0xFFFE);  // All masked
    HAL_WRITE_UINT16(_FRV400_IRC_RC, 0xFFFE);    // All cleared
    HAL_WRITE_UINT16(_FRV400_IRC_IRL, 0x10);     // Clear IRL (interrupt request latch)    

    // Onboard FPGA interrupts
    HAL_WRITE_UINT16(_FRV400_FPGA_CONTROL, _FRV400_FPGA_CONTROL_IRQ);  // Enable IRQ registers
    HAL_WRITE_UINT16(_FRV400_FPGA_IRQ_MASK,      // Set up for LAN, PCI INTx
                     0x7FFE & 
                     ~(_FRV400_FPGA_IRQ_LAN |
                       _FRV400_FPGA_IRQ_INTA |
                       _FRV400_FPGA_IRQ_INTB |
                       _FRV400_FPGA_IRQ_INTC |
                       _FRV400_FPGA_IRQ_INTD)
        );
    HAL_WRITE_UINT16(_FRV400_FPGA_IRQ_LEVELS,    // Set up for LAN, PCI INTx
                     0x7FFE & 
                     ~(_FRV400_FPGA_IRQ_LAN |
                       _FRV400_FPGA_IRQ_INTA |
                       _FRV400_FPGA_IRQ_INTB |
                       _FRV400_FPGA_IRQ_INTC |
                       _FRV400_FPGA_IRQ_INTD)
        );
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_LAN, 1, 0);  // Level, low

    // Set up system clock
    HAL_READ_UINT32(_FRV400_MB_CLKSW, clk);
    _system_clock = (((clk&0xFF) * 125 * 2) / 240) * 1000000;

    // Set scalers to achieve 1us resolution in timer
    HAL_WRITE_UINT8(_FRV400_TPRV, _system_clock / (1000*1000));
    HAL_WRITE_UINT8(_FRV400_TCKSL0, 0x80);
    HAL_WRITE_UINT8(_FRV400_TCKSL1, 0x80);
    HAL_WRITE_UINT8(_FRV400_TCKSL2, 0x80);

    hal_if_init();

    // Initialize real-time clock (for delays, etc, even if kernel doesn't use it)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    _frv400_pci_init();
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_LAN:
        HAL_READ_UINT16(_FRV400_FPGA_IRQ_MASK, _mask);
        _mask |= _FRV400_FPGA_IRQ_LAN;
        HAL_WRITE_UINT16(_FRV400_FPGA_IRQ_MASK, _mask);
        break;
    }
    HAL_READ_UINT16(_FRV400_IRC_MASK, _mask);
    _mask |= (1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRV400_IRC_MASK, _mask);
}

void hal_interrupt_unmask(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_LAN:
        HAL_READ_UINT16(_FRV400_FPGA_IRQ_MASK, _mask);
        _mask &= ~_FRV400_FPGA_IRQ_LAN;
        HAL_WRITE_UINT16(_FRV400_FPGA_IRQ_MASK, _mask);
        break;
    }
    HAL_READ_UINT16(_FRV400_IRC_MASK, _mask);
    _mask &= ~(1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRV400_IRC_MASK, _mask);
}

void hal_interrupt_acknowledge(int vector)
{
    cyg_uint16 _mask;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_LAN:
        HAL_WRITE_UINT16(_FRV400_FPGA_IRQ_REQUEST,      // Clear LAN interrupt
                         0x7FFE & ~_FRV400_FPGA_IRQ_LAN);
        break;
    }
    _mask = (1<<(vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1));
    HAL_WRITE_UINT16(_FRV400_IRC_RC, _mask);
    HAL_WRITE_UINT16(_FRV400_IRC_IRL, 0x10);  // Clears IRL latch
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
        HAL_READ_UINT16(_FRV400_IRC_IRR5, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFFC) | (_trig<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_TIMER1:
        HAL_READ_UINT16(_FRV400_IRC_IRR5, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFF3) | (_trig<<2);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_TIMER2:
        HAL_READ_UINT16(_FRV400_IRC_IRR5, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR5, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFFCF) | (_trig<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA0:
        HAL_READ_UINT16(_FRV400_IRC_IRR4, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xFCFF) | (_trig<<8);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA1:
        HAL_READ_UINT16(_FRV400_IRC_IRR4, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xF3FF) | (_trig<<10);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA2:
        HAL_READ_UINT16(_FRV400_IRC_IRR4, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0xCFFF) | (_trig<<12);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_DMA3:
        HAL_READ_UINT16(_FRV400_IRC_IRR4, _irr);
        _irr = (_irr & 0x0FFF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<12);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR4, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM0, _tmr);
        _tmr = (_tmr & 0x3FFF) | (_trig<<14);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM0, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_UART0:
        HAL_READ_UINT16(_FRV400_IRC_IRR6, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR6, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM1, _tmr);
        _tmr = (_tmr & 0xFCFF) | (_trig<<8);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_UART1:
        HAL_READ_UINT16(_FRV400_IRC_IRR6, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR6, _irr);
        HAL_READ_UINT16(_FRV400_IRC_ITM1, _tmr);
        _tmr = (_tmr & 0xF3FF) | (_trig<<10);
        HAL_WRITE_UINT16(_FRV400_IRC_ITM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT0:
        HAL_READ_UINT16(_FRV400_IRC_IRR3, _irr);
        _irr = (_irr & 0xFFF0) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRV400_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFFC) | (_trig<<0);
        HAL_WRITE_UINT16(_FRV400_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT1:
        HAL_READ_UINT16(_FRV400_IRC_IRR3, _irr);
        _irr = (_irr & 0xFF0F) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRV400_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFF3) | (_trig<<2);
        HAL_WRITE_UINT16(_FRV400_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT2:
        HAL_READ_UINT16(_FRV400_IRC_IRR3, _irr);
        _irr = (_irr & 0xF0FF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<8);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRV400_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFFCF) | (_trig<<4);
        HAL_WRITE_UINT16(_FRV400_IRC_TM1, _tmr);
        break;
    case  CYGNUM_HAL_INTERRUPT_EXT3:
        HAL_READ_UINT16(_FRV400_IRC_IRR3, _irr);
        _irr = (_irr & 0x0FFF) | ((vector-CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1+1)<<12);
        HAL_WRITE_UINT16(_FRV400_IRC_IRR3, _irr);
        HAL_READ_UINT16(_FRV400_IRC_TM1, _tmr);
        _tmr = (_tmr & 0xFF3F) | (_trig<<6);
        HAL_WRITE_UINT16(_FRV400_IRC_TM1, _tmr);
        break;
    default:
        ; // Nothing to do
    };
}

void hal_interrupt_set_level(int vector, int level)
{
//    UNIMPLEMENTED(__FUNCTION__);
}

// PCI support

externC void
_frv400_pci_init(void)
{
    static int _init = 0;
    cyg_uint8 next_bus;
    cyg_uint32 cmd_state;

    if (_init) return;
    _init = 1;

    // Enable controller - most of the basic configuration
    // was set up at boot time in "platform.inc"

    // Setup for bus mastering
    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                            CYG_PCI_CFG_COMMAND, cmd_state);
    if ((cmd_state & CYG_PCI_CFG_COMMAND_MEMORY) == 0) {
        HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                 CYG_PCI_CFG_COMMAND,
                                 CYG_PCI_CFG_COMMAND_MEMORY |
                                 CYG_PCI_CFG_COMMAND_MASTER |
                                 CYG_PCI_CFG_COMMAND_PARITY |
                                 CYG_PCI_CFG_COMMAND_SERR);

        // Setup latency timer field
        HAL_PCI_CFG_WRITE_UINT8(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                CYG_PCI_CFG_LATENCY_TIMER, 32);

        // Configure PCI bus.
        next_bus = 1;
        cyg_pci_configure_bus(0, &next_bus);
    }

}

externC void 
_frv400_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid)
{
    cyg_uint8 req;                                                            
    cyg_uint8 dev = CYG_PCI_DEV_GET_DEV(devfn);

    if (dev == CYG_PCI_MIN_DEV) {
        // On board LAN
        *vec = CYGNUM_HAL_INTERRUPT_LAN;
        *valid = true;
    } else {
        HAL_PCI_CFG_READ_UINT8(bus, devfn, CYG_PCI_CFG_INT_PIN, req);         
        if (0 != req) {                                                           
            CYG_ADDRWORD __translation[4] = {                                       
                CYGNUM_HAL_INTERRUPT_PCIINTC,   /* INTC# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTB,   /* INTB# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTA,   /* INTA# */                         
                CYGNUM_HAL_INTERRUPT_PCIINTD};  /* INTD# */                         
                                                                                
            /* The PCI lines from the different slots are wired like this  */       
            /* on the PCI backplane:                                       */       
            /*                pin6A     pin7B    pin7A   pin8B             */       
            /* I/O Slot 1     INTA#     INTB#    INTC#   INTD#             */       
            /* I/O Slot 2     INTD#     INTA#    INTB#   INTC#             */       
            /* I/O Slot 3     INTC#     INTD#    INTA#   INTB#             */       
            /*                                                             */       
            /* (From PCI Development Backplane, 3.2.2 Interrupts)          */       
            /*                                                             */       
            /* Devsel signals are wired to, resulting in device IDs:       */       
            /* I/O Slot 1     AD30 / dev 19      [(8+1)&3 = 1]             */       
            /* I/O Slot 2     AD29 / dev 18      [(7+1)&3 = 0]             */       
            /* I/O Slot 3     AD28 / dev 17      [(6+1)&3 = 3]             */       
                                                                                
            *vec = __translation[((req+dev)&3)];        
            *valid = true;                                                         
        } else {                                                                    
            /* Device will not generate interrupt requests. */                      
            *valid = false;                                                        
        }                                                                           
        diag_printf("Int - dev: %d, req: %d, vector: %d\n", dev, req, *vec);
    }
}

// PCI configuration space access
#define _EXT_ENABLE 0x80000000  // Could be 0x80000000

static __inline__ cyg_uint32
_cfg_addr(int bus, int devfn, int offset)
{
    return _EXT_ENABLE | (bus << 22) | (devfn << 8) | (offset << 0);
}

externC cyg_uint8 
_frv400_pci_cfg_read_uint8(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint8 cfg_val = (cyg_uint8)0xFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + ((offset << 1) ^ 0x03);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x03);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x03);
    }
    HAL_READ_UINT8(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint8)0xFF;
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint16 
_frv400_pci_cfg_read_uint16(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint16 cfg_val = (cyg_uint16)0xFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + ((offset << 1) ^ 0x02);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x02);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x02);
    }
    HAL_READ_UINT16(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint16)0xFFFF;
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint32 
_frv400_pci_cfg_read_uint32(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr, status;
    cyg_uint32 cfg_val = (cyg_uint32)0xFFFFFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + (offset << 1);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA;
    }
    HAL_READ_UINT32(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint32)0xFFFFFFFF;
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC void
_frv400_pci_cfg_write_uint8(int bus, int devfn, int offset, cyg_uint8 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + ((offset << 1) ^ 0x03);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x03);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x03);
    }
    HAL_WRITE_UINT8(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
}

externC void
_frv400_pci_cfg_write_uint16(int bus, int devfn, int offset, cyg_uint16 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + ((offset << 1) ^ 0x02);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset ^ 0x02);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA + ((offset & 0x03) ^ 0x02);
    }
    HAL_WRITE_UINT16(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
}

externC void
_frv400_pci_cfg_write_uint32(int bus, int devfn, int offset, cyg_uint32 cfg_val)
{
    cyg_uint32 cfg_addr, addr, status;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    if ((bus == 0) && (CYG_PCI_DEV_GET_DEV(devfn) == 0)) {
        // PCI bridge
        addr = _FRV400_PCI_CONFIG + (offset << 1);
    } else {
        cfg_addr = _cfg_addr(bus, devfn, offset);
        HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, cfg_addr);
        addr = _FRV400_PCI_CONFIG_DATA;
    }
    HAL_WRITE_UINT32(addr, cfg_val);
    HAL_READ_UINT16(_FRV400_PCI_STAT_CMD, status);
    if (status & _FRV400_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_FRV400_PCI_STAT_CMD, status & _FRV400_PCI_STAT_ERROR_MASK);
    }
    HAL_WRITE_UINT32(_FRV400_PCI_CONFIG_ADDR, 0);
}

/*------------------------------------------------------------------------*/
// EOF frv400_misc.c
