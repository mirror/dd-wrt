//=============================================================================
//
//      hal_aux.c
//
//      HAL auxiliary objects and code; per platform
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:hmt, gthomas
// Date:        1999-06-08
// Purpose:     HAL aux objects: startup tables.
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_pci.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_mem.h>            // HAL memory definitions
#include <cyg/hal/ppc_regs.h>           // Platform registers
#include <cyg/hal/hal_if.h>             // hal_if_init
#include <cyg/hal/hal_intr.h>           // interrupt definitions
#include <cyg/hal/hal_cache.h>
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/io/pci.h>
#include <cyg/hal/hal_io.h>             // I/O macros
#include CYGHWR_MEMORY_LAYOUT_H

// Functions defined in this module
void _csb281_fs6377_init(int mode);
static void _csb281_i2c_init(void);

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. The regions defined below are the minimum requirements.
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    // Mapping for the Cogent CSB281 development boards
    CYGARC_MEMDESC_NOCACHE( 0x70000000, 0x10000000 ), // FLASH region, LCD, PS/2
    CYGARC_MEMDESC_NOCACHE( 0xf0000000, 0x10000000 ), // PCI space, LEDS, control
    CYGARC_MEMDESC_CACHE(   CYGMEM_REGION_ram, CYGMEM_REGION_ram_SIZE ), // Main memory
// Main memory, mapped non-cacheable for PCI use
    CYGARC_MEMDESC_NOCACHE_PA(CYGMEM_SECTION_pci_window, 
                              CYGARC_PHYSICAL_ADDRESS(CYGMEM_SECTION_pci_window), 
                              CYGMEM_SECTION_pci_window_SIZE),

    CYGARC_MEMDESC_TABLE_END
};

//--------------------------------------------------------------------------
// Platform init code.
void
hal_platform_init(void)
{
    cyg_uint32 bcsr, gcr, frr, eicr, iack;
    int vec;

    // Initialize I/O interfaces
    hal_if_init();
    // Reset interrupt controller/state
    HAL_READ_UINT32LE(_CSB281_EPIC_GCR, gcr);
    HAL_READ_UINT32LE(_CSB281_EPIC_FRR, frr);
    HAL_WRITE_UINT32LE(_CSB281_EPIC_GCR, gcr | _CSB281_EPIC_GCR_R);
    do {
        HAL_READ_UINT32LE(_CSB281_EPIC_GCR, gcr);
    } while ((gcr & _CSB281_EPIC_GCR_R) != 0);
    HAL_WRITE_UINT32LE(_CSB281_EPIC_GCR, gcr | _CSB281_EPIC_GCR_M);
    HAL_READ_UINT32LE(_CSB281_EPIC_EICR, eicr);  // Force direct interrupts
    eicr &= ~_CSB281_EPIC_EICR_SIE;
    HAL_WRITE_UINT32LE(_CSB281_EPIC_EICR, eicr);
    for (vec = CYGNUM_HAL_INTERRUPT_IRQ0; vec <= CYGNUM_HAL_ISR_MAX; vec++) {
        HAL_INTERRUPT_CONFIGURE(vec, 0, 0);  // Default to low-edge
        HAL_INTERRUPT_SET_LEVEL(vec, 0x0F);  // Priority
    }
    vec = (frr & 0x0FFF0000) >> 16;  // Number of interrupt sources
    while (vec-- > 0) {
        HAL_READ_UINT32LE(_CSB281_EPIC_IACK, iack);  
        HAL_WRITE_UINT32LE(_CSB281_EPIC_EOI, 0);
    }
    HAL_WRITE_UINT32LE(_CSB281_EPIC_PCTPR, 1); // Enables interrupts
#ifndef CYGSEM_HAL_USE_ROM_MONITOR
    // Reset peripherals
    HAL_READ_UINT32(_CSB281_BCSR, bcsr);
    HAL_WRITE_UINT32(_CSB281_BCSR, _zero_bit(bcsr, _CSB281_BCSR_PRESET));
    HAL_WRITE_UINT32(_CSB281_BCSR, _one_bit(bcsr, _CSB281_BCSR_PRESET));
    _csb281_i2c_init();
    _csb281_fs6377_init(0);
#endif
#ifdef CYGSEM_CSB281_LCD_COMM
    lcd_comm_init();
#endif
    _csb281_pci_init();
}

//--------------------------------------------------------------------------
// Interrupt support

CYG_ADDRWORD _pvrs[] = {
    _CSB281_EPIC_IVPR0,     // CYGNUM_HAL_INTERRUPT_IRQ0   0x02
    _CSB281_EPIC_IVPR1,     // CYGNUM_HAL_INTERRUPT_IRQ1   0x03
    _CSB281_EPIC_IVPR2,     // CYGNUM_HAL_INTERRUPT_IRQ2   0x04
    _CSB281_EPIC_IVPR3,     // CYGNUM_HAL_INTERRUPT_IRQ3   0x05
    _CSB281_EPIC_IVPR4,     // CYGNUM_HAL_INTERRUPT_IRQ4   0x06
    _CSB281_EPIC_UART0VPR,  // CYGNUM_HAL_INTERRUPT_UART0  0x07
    _CSB281_EPIC_UART1VPR,  // CYGNUM_HAL_INTERRUPT_UART1  0x08
    _CSB281_EPIC_GTVPR0,    // CYGNUM_HAL_INTERRUPT_TIMER0 0x09
    _CSB281_EPIC_GTVPR1,    // CYGNUM_HAL_INTERRUPT_TIMER1 0x0A
    _CSB281_EPIC_GTVPR2,    // CYGNUM_HAL_INTERRUPT_TIMER2 0x0B
    _CSB281_EPIC_GTVPR3,    // CYGNUM_HAL_INTERRUPT_TIMER3 0x0C
    _CSB281_EPIC_I2CVPR,    // CYGNUM_HAL_INTERRUPT_I2C    0x0D
    _CSB281_EPIC_DMA0VPR,   // CYGNUM_HAL_INTERRUPT_DMA0   0x0E
    _CSB281_EPIC_DMA1VPR,   // CYGNUM_HAL_INTERRUPT_DMA1   0x0F
    _CSB281_EPIC_MSGVPR,    // CYGNUM_HAL_INTERRUPT_MSG    0x10
};

void 
hal_interrupt_mask(int vector)
{
    cyg_uint32 pvr;

    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    if (vector < CYGNUM_HAL_INTERRUPT_IRQ0) {
        // Can't do much with non-external interrupts
        return;
    }
    HAL_READ_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
    pvr |= _CSB281_EPIC_PVR_M;
    HAL_WRITE_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
//    diag_printf("%s(%d)\n", __FUNCTION__, vector);
}

void 
hal_interrupt_unmask(int vector)
{
    cyg_uint32 pvr;

    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    if (vector < CYGNUM_HAL_INTERRUPT_IRQ0) {
        // Can't do much with non-external interrupts
        return;
    }
    HAL_READ_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
    pvr &= ~_CSB281_EPIC_PVR_M;
    HAL_WRITE_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
//    diag_printf("%s(%d)\n", __FUNCTION__, vector);
}

void 
hal_interrupt_acknowledge(int vector)
{
}

void 
hal_interrupt_configure(int vector, int level, int up)
{
    cyg_uint32 pvr;

    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    if (vector < CYGNUM_HAL_INTERRUPT_IRQ0) {
        // Can't do much with non-external interrupts
        return;
    }
//    diag_printf("%s(%d, %d, %d)\n", __FUNCTION__, vector, level, up);
    HAL_READ_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
    pvr &= _CSB281_EPIC_PVR_M;  // Preserve mask
    pvr |= vector;
    if (level) {
        pvr |= _CSB281_EPIC_PVR_S;
    } else {
        pvr &= ~_CSB281_EPIC_PVR_S;
    }
    if (up) {
        pvr |= _CSB281_EPIC_PVR_P;
    } else {
        pvr &= ~_CSB281_EPIC_PVR_P;
    }
    HAL_WRITE_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
}

void 
hal_interrupt_set_level(int vector, int level)
{
    cyg_uint32 pvr;

    CYG_ASSERT( vector <= CYGNUM_HAL_ISR_MAX, "Invalid vector");
    CYG_ASSERT( vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    if (vector < CYGNUM_HAL_INTERRUPT_IRQ0) {
        // Can't do much with non-external interrupts
        return;
    }
    HAL_READ_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
    pvr &= ~(_CSB281_EPIC_PVR_PRIO_MASK<<_CSB281_EPIC_PVR_PRIO_SHIFT);
    pvr |= (level<<_CSB281_EPIC_PVR_PRIO_SHIFT);
    HAL_WRITE_UINT32LE(_pvrs[vector-CYGNUM_HAL_INTERRUPT_IRQ0], pvr);
}


//--------------------------------------------------------------------------
// PCI support

externC void
_csb281_pci_init(void)
{
    static int _init = 0;
    cyg_uint8 next_bus;
    cyg_uint32 cmd_state;

    if (_init) return;
    _init = 1;

    // Initialize PCI support
    cyg_pci_init();

    // Setup for bus mastering
    HAL_PCI_CFG_READ_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                            CYG_PCI_CFG_COMMAND, cmd_state);
    if ((cmd_state & CYG_PCI_CFG_COMMAND_MEMORY) == 0) {
        // Force PCI-side window to 0
        HAL_PCI_CFG_WRITE_UINT32(0, CYG_PCI_DEV_MAKE_DEVFN(0,0),
                                 CYG_PCI_CFG_BAR_0, 0x01);
        // Enable bus mastering from host
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

    if (0){
        cyg_uint8 devfn;
        cyg_pci_device_id devid;
        cyg_pci_device dev_info;
        int i;

        devid = CYG_PCI_DEV_MAKE_ID(next_bus-1, 0) | CYG_PCI_NULL_DEVFN;
        while (cyg_pci_find_next(devid, &devid)) {
            devfn = CYG_PCI_DEV_GET_DEVFN(devid);
            cyg_pci_get_device_info(devid, &dev_info);

            diag_printf("\n");
            diag_printf("Bus:        %d\n", CYG_PCI_DEV_GET_BUS(devid));
            diag_printf("PCI Device: %d\n", CYG_PCI_DEV_GET_DEV(devfn));
            diag_printf("PCI Func  : %d\n", CYG_PCI_DEV_GET_FN(devfn));
            diag_printf("Vendor Id : 0x%08X\n", dev_info.vendor);
            diag_printf("Device Id : 0x%08X\n", dev_info.device);
            for (i = 0; i < dev_info.num_bars; i++) {
                diag_printf("  BAR[%d]    0x%08x /", i, dev_info.base_address[i]);
                diag_printf(" probed size 0x%08x / CPU addr 0x%08x\n",
                            dev_info.base_size[i], dev_info.base_map[i]);
            }
        }
    }

    // Configure interrupts (high level)?
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_PCI0, 1, 1);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_PCI1, 1, 1);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_LAN, 1, 1);
}

externC void 
_csb281_pci_translate_interrupt(int bus, int devfn, int *vec, int *valid)
{
    cyg_uint8 dev = CYG_PCI_DEV_GET_DEV(devfn);

    // Purely slot based
    if (dev >= CYG_PCI_MIN_DEV) {
        CYG_ADDRWORD __translation[] = {                                       
            CYGNUM_HAL_INTERRUPT_PCI0,
            CYGNUM_HAL_INTERRUPT_PCI1,
            CYGNUM_HAL_INTERRUPT_LAN
        };
        *vec = __translation[dev-CYG_PCI_MIN_DEV];
        *valid = true;
    } else {
        *valid = false;
    }
#if 0
    diag_printf("Int - dev: %d, vector: %d [%s]\n", 
                dev, *vec, *valid ? "OK" : "BAD");
#endif
}

// PCI configuration space access
#define _EXT_ENABLE                     0x80000000

//
// Prepare for a config cycle on the PCI bus
//
static __inline__ cyg_uint32
_cfg_sel(int bus, int devfn, int offset)
{
    cyg_uint32 cfg_addr, addr;
    cyg_uint32 bcsr;

    HAL_READ_UINT32(_CSB281_BCSR, bcsr);
    bcsr = (bcsr & ~0x07) | (1<<(CYG_PCI_DEV_GET_DEV(devfn)-CYG_PCI_MIN_DEV));
    HAL_WRITE_UINT32(_CSB281_BCSR, bcsr);
    cfg_addr = _EXT_ENABLE | 
        (bus << 16) | 
        (CYG_PCI_DEV_GET_DEV(devfn) << 11) | 
        (CYG_PCI_DEV_GET_FN(devfn) << 8) | 
        ((offset & 0xFF) << 0);
    HAL_WRITE_UINT32LE(_CSB281_PCI_CONFIG_ADDR, cfg_addr);
    addr = _CSB281_PCI_CONFIG_DATA + (offset & 0x03);
    return addr;
}

externC cyg_uint8 
_csb281_pci_cfg_read_uint8(int bus, int devfn, int offset)
{
    cyg_uint32 addr;
    cyg_uint8 cfg_val = (cyg_uint8)0xFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);
    HAL_READ_UINT8LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint8)0xFF;
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint16 
_csb281_pci_cfg_read_uint16(int bus, int devfn, int offset)
{
    cyg_uint32 addr;
    cyg_uint16 cfg_val = (cyg_uint16)0xFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);    
    HAL_READ_UINT16LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16LE(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint16)0xFFFF;
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC cyg_uint32 
_csb281_pci_cfg_read_uint32(int bus, int devfn, int offset)
{
    cyg_uint32 addr;
    cyg_uint32 cfg_val = (cyg_uint32)0xFFFFFFFF;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x) = ", __FUNCTION__, bus, devfn, offset);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);
    HAL_READ_UINT32LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        cfg_val = (cyg_uint32)0xFFFFFFFF;
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%x\n", cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
    return cfg_val;
}

externC void
_csb281_pci_cfg_write_uint8(int bus, int devfn, int offset, cyg_uint8 cfg_val)
{
    cyg_uint32 addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);
    HAL_WRITE_UINT8LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
}

externC void
_csb281_pci_cfg_write_uint16(int bus, int devfn, int offset, cyg_uint16 cfg_val)
{
    cyg_uint32 addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);
    HAL_WRITE_UINT16LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
}

externC void
_csb281_pci_cfg_write_uint32(int bus, int devfn, int offset, cyg_uint32 cfg_val)
{
    cyg_uint32 addr;

#ifdef CYGPKG_IO_PCI_DEBUG
    diag_printf("%s(bus=%x, devfn=%x, offset=%x, val=%x)\n", __FUNCTION__, bus, devfn, offset, cfg_val);
#endif // CYGPKG_IO_PCI_DEBUG
    addr = _cfg_sel(bus, devfn, offset);
    HAL_WRITE_UINT32LE(addr, cfg_val);
#if 0
    HAL_READ_UINT16(_CSB281_PCI_STAT_CMD, status);
    if (status & _CSB281_PCI_STAT_ERROR_MASK) {
        // Cycle failed - clean up and get out
        HAL_WRITE_UINT16(_CSB281_PCI_STAT_CMD, status & _CSB281_PCI_STAT_ERROR_MASK);
    }
#endif
    HAL_WRITE_UINT32(_CSB281_PCI_CONFIG_ADDR, 0);
}

//--------------------------------------------------------------------------
// I2C support
static void
_csb281_i2c_init(void)
{
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_ALL);   // SDA=1, SCL=1
}

static void
_csb281_i2c_delay(void)
{
    int ctr;

    for (ctr = 0;  ctr < 100*10;  ctr++);
}

// Issue start sequence which is SDA(1->0) with SCL(1)
static void
_csb281_i2c_start(void)
{
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SDA);   // SDA=1, SCL=?
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=1, SCL=1
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SDA);   // SDA=0, SCL=1
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=0, SCL=0
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SDA);   // SDA=1, SCL=1
    _csb281_i2c_delay();
}

// Issue stop sequence which is SDA(0->1) with SCL(1)
static void
_csb281_i2c_stop(void)
{
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SDA);   // SDA=0, SCL=?
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=1, SCL=1
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SDA);   // SDA=1, SCL=1
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=0, SCL=0
    _csb281_i2c_delay();
}

// Send an 8-bit value, MSB first, SCL(1->0) clocks the data
static int
_csb281_i2c_put(unsigned char val)
{
    int bit, csr;

    for (bit = 7;  bit >= 0;  bit--) {
        if ((val & (1 << bit))) {
            HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SDA);   // SDA=1, SCL=?
        } else {
            HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SDA);   // SDA=0, SCL=?
        }
        _csb281_i2c_delay();
        HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=?, SCL=1
        _csb281_i2c_delay();
        HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=?, SCL=0
    }
    // Now wait for ACK
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SDA);   // SDA=1, SCL=0
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=1, SCL=1
    _csb281_i2c_delay();
    HAL_READ_UINT32(_CSB281_2WCSR, csr);  // Read current state
    if ((csr & _CSB281_2WCSR_GET_SDA)) {
        // No ACK!
        return -1;
    }
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=?, SCL=0
    _csb281_i2c_delay();
    return 0;
}

static unsigned char
_csb281_i2c_get(void)
{
    unsigned char val = 0;
    int bit, csr;

    for (bit = 7;  bit >= 0;  bit--) {
        HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=?, SCL=1
        _csb281_i2c_delay();
        HAL_READ_UINT32(_CSB281_2WCSR, csr);  // Read current state
        if ((csr & _CSB281_2WCSR_GET_SDA)) {
            val |= (1 << bit);
        }
        HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=?, SCL=0
        _csb281_i2c_delay();
    }
    // Need extra transition (for ACK time slot)
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_SET_SCL);   // SDA=?, SCL=0
    _csb281_i2c_delay();
    HAL_WRITE_UINT32(_CSB281_2WCSR, _CSB281_2WCSR_CLR_SCL);   // SDA=?, SCL=0
    _csb281_i2c_delay();
    return val;
}

int
_csb281_i2c_write_reg(int addr, int reg, unsigned char val)
{
    _csb281_i2c_start();
    if (_csb281_i2c_put(addr << 1) < 0) {
        return -1;
    }
    if (_csb281_i2c_put(reg) < 0) {
        return -1;
    }
    if (_csb281_i2c_put(val) < 0) {
        return -1;
    }
    _csb281_i2c_stop();
    return 0;
}

int
_csb281_i2c_read_reg(int addr, int reg)
{
    unsigned char val;
    _csb281_i2c_start();
    if (_csb281_i2c_put(addr << 1) < 0) {
        return -1;
    }
    if (_csb281_i2c_put(reg) < 0) {
        return -1;
    }
    _csb281_i2c_start();
    if (_csb281_i2c_put((addr << 1) | 0x01) < 0) {
        return -1;
    }
    val = _csb281_i2c_get();
    _csb281_i2c_stop();
    return val;
}

//--------------------------------------------------------------------------
// FS6377 Clock generator support

static unsigned char _fs6377_init_data[] = {
    0x28, 0xEF, 0x53, 0x03, 0x4B, 0x80, 0x32, 0x80,
    0x94, 0x32, 0x80, 0xD4, 0x56, 0xF6, 0xF6, 0xE0
};

// Setup for CRT mode 640x480 @75Hz
static unsigned char _fs6377_init_data_CRT[] = {
    0x10, 0x3b, 0x49, 0x03, 0x4B, 0x80, 0x32, 0x80,
    0x94, 0x32, 0x80, 0xD4, 0x66, 0xF6, 0xF6, 0xE0 
};

void
_csb281_fs6377_init(int mode)
{
    int reg;
    unsigned char *data;

    if (mode) {
        data = _fs6377_init_data_CRT;
    } else {
        data = _fs6377_init_data;
    }
    for (reg = 0;  reg < 16;  reg++) {
        if (_csb281_i2c_write_reg(_CSB281_FS6377_DEV, reg, *data++) < 0) {
            diag_printf("** Can't write FS6377 register %d\n", reg);
            return;
        }
    }
}

//--------------------------------------------------------------------------
// Blink the value 'val' into the LEDs
//   LED0 - clock
//   LED1 - value

static void
_set_leds(int led0, int led1)
{
    cyg_uint32 bcsr;

    HAL_READ_UINT32(_CSB281_BCSR, bcsr);
    bcsr = _one_bit(bcsr, (_CSB281_BCSR_LED0 | _CSB281_BCSR_LED1));
    if (led0) bcsr = _zero_bit(bcsr, _CSB281_BCSR_LED0);
    if (led1) bcsr = _zero_bit(bcsr, _CSB281_BCSR_LED1);
    HAL_WRITE_UINT32(_CSB281_BCSR, bcsr);
}

static void
_led_delay(int len)
{
    int ctr, limit;
    int cache_state;

    HAL_ICACHE_IS_ENABLED(cache_state);
    limit = cache_state ? 0x100000 : 0x40000;
    while (len--) {
        for (ctr = 0;  ctr < limit;  ctr++);
    }
}

void
_csb281_led(int val)
{
    int bit, ctr;

    for (ctr = 0;  ctr < 8;  ctr++) {
        _set_leds(0,0);
        _led_delay(1);
        _set_leds(0,1);
        _led_delay(1);
    }
    _set_leds(0,0);
    _led_delay(16);
    for (bit = 7;  bit >= 0;  bit--) {
        _set_leds(1, val & (1<<bit));
        _led_delay(8);
        _set_leds(0, 0);
        _led_delay(8);
    }
}

// EOF hal_aux.c
