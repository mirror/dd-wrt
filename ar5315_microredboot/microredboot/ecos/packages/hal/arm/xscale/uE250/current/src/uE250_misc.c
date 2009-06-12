//=============================================================================
//
//      uE250_misc.c
//
//      Miscellaneous platform support for NMI uE250
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_pxa2x0.h>
// FIXME
#include <cyg/hal/uE250.h>              // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_mm.h>

#include <string.h> // memset

externC void initialize_plx_bridge(void);

void
hal_mmu_init(void)
{
    // Set up the translation tables at offset 0x4000
    unsigned long ttb_base = PXA2X0_RAM_BANK0_BASE + 0x4000;
    unsigned long i;

    /*
     * Set the TTB register
     */
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_DACR_DEFAULT;
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*               Actual  Virtual  Size   Attributes                                                    Function  */
    /*		     Base     Base     MB      cached?           buffered?        access permissions                 */
    /*             xxx00000  xxx00000                                                                                */
#define _CACHED   ARM_CACHEABLE
#define _UNCACHED ARM_UNCACHEABLE
#define _BUF      ARM_BUFFERABLE
#define _NOBUF    ARM_UNBUFFERABLE
#define _RWRW     ARM_ACCESS_PERM_RW_RW
    X_ARM_MMU_SECTION(0x000,  0x500,    32,  _CACHED,     _BUF, _RWRW); /* Boot flash ROMspace */
    X_ARM_MMU_SECTION(0x150,  0x150,     4,  _UNCACHED, _NOBUF, _RWRW); /* PCI Config space CS5 */
    X_ARM_MMU_SECTION(0x040,  0x340,    64,  _UNCACHED, _NOBUF, _RWRW); /* PCI Control regs CS1 */
    X_ARM_MMU_SECTION(0xA00,  0x000,    64,  _CACHED,     _BUF, _RWRW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xA00,  0xC00,    64,  _UNCACHED,   _BUF, _RWRW); /* DRAM Bank 0 */
    X_ARM_MMU_SECTION(0xE00,  0xE00,   128,  _CACHED,     _BUF, _RWRW); /* Zeros (Cache Clean) Bank */
    
    X_ARM_MMU_SECTION(0x0c0,  0x0c0,    64,  _UNCACHED, _NOBUF, _RWRW); /* PCI Mem space CS3 */
    X_ARM_MMU_SECTION(0x100,  0x100,    64,  _UNCACHED, _NOBUF, _RWRW); /* PCI Mem space CS4 */
    X_ARM_MMU_SECTION(0x140,  0x140,    16,  _UNCACHED, _NOBUF, _RWRW); /* PCI Mem space CS5 */
    X_ARM_MMU_SECTION(0x160,  0x160,    32,  _UNCACHED, _NOBUF, _RWRW); /* PCI I/O space CS5 */
    X_ARM_MMU_SECTION(0x400,  0x400,    64,  _UNCACHED, _NOBUF, _RWRW); /* Peripheral Registers */
    X_ARM_MMU_SECTION(0x440,  0x440,    64,  _UNCACHED, _NOBUF, _RWRW); /* LCD Registers */
    X_ARM_MMU_SECTION(0x480,  0x480,    64,  _UNCACHED, _NOBUF, _RWRW); /* Memory Ctl Registers */
    X_ARM_MMU_SECTION(0x900,  0x900,    64,  _UNCACHED, _NOBUF, _RWRW); /* PCI I/O Space */
    }

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{

    *PXA2X0_GPCR0 = 0x00400000;
    // PXA250_GPSR0 = 0x00200000;

    // RAM startup only - rewrite relevent bits depending on config
#ifndef CYG_HAL_STARTUP_ROM
    HAL_DCACHE_SYNC();            // Force data out
    HAL_DCACHE_INVALIDATE_ALL();  // Flush TLBs: make new mmu state effective
#endif // ! CYG_HAL_STARTUP_ROM - RAM start only

    hal_if_init();

    cyg_hal_plf_pci_init();

    initialize_plx_bridge();
}

//
// Memory layout - runtime variations of all kinds.
//
externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  256<<20,
                "More than 256MB reported - that can't be right" );
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
        // hal_dram_size excludes the PCI window on this platform.
        if ( regionend > (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size )
            regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
#endif
    return regionend;
}

// ------------------------------------------------------------------------
// Extended, platform-specific, interrupt handling

// FIXME - still needs work to support interrupts from PXA bridge

int  
_uE250_extended_irq(void)
{
    cyg_uint32 stat = PCICTL_STATUS_REG;
    int irq = 0;

    if (stat & 0x1F0) {
        PCICTL_INT_RESET = 0xFF;  // Clear all pending interrupts
    }
    if (stat & 0x00F) {
        // PCI interrupt
        for (irq = 0;  irq < 4; irq++) {
            if ((stat & (1 << irq)) != 0) {
                break;
            }
        }
        irq += _uPCI_BASE_INTERRUPT;
    }
    HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_GPIO1);
    PCICTL_INT_EDGE = 0xFF;   // Generate interrupts
    return irq;
}

void 
_uE250_extended_int_mask(int vector)
{
    if (vector <= CYGNUM_HAL_INTERRUPT_PCI_INTD) {
        PCICTL_IRQ_MASK &= ~(1<<(vector-CYGNUM_HAL_INTERRUPT_PCI_INTA));
    }
}

void 
_uE250_extended_int_unmask(int vector)
{
    if (vector <= CYGNUM_HAL_INTERRUPT_PCI_INTD) {
        PCICTL_IRQ_MASK |= (1<<(vector-CYGNUM_HAL_INTERRUPT_PCI_INTA));
    }
}

void 
_uE250_extended_int_acknowledge(int vector)
{
}

void 
_uE250_extended_int_configure(int vector, int level, int up)
{
}

void 
_uE250_extended_int_set_level(int vector, int level)
{
}


// ------------------------------------------------------------------------
// EOF uE250_misc.c
