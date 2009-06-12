//==========================================================================
//
//      nano_misc.c
//
//      HAL misc board support code for StrongARM SA1110/nanoEngine
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
// Contributors: hmt
//               Travis C. Furrer <furrer@mit.edu>
// Date:         2001-02-12
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

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_sa11x0.h>         // Hardware definitions
#include <cyg/hal/nano.h>               // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

// All the MM table layout is here:
#include <cyg/hal/hal_mm.h>

#ifdef CYGPKG_IO_PCI
cyg_uint32 cyg_pci_window_real_base = 0;
#endif

#include <string.h> // memset

void
hal_mmu_init(void)
{
    unsigned long ttb_base = SA11X0_RAM_BANK0_BASE + 0x4000;
    unsigned long i;

#ifdef CYG_HAL_STARTUP_ROM
    // SDRAM Memory Sizing:
    //
    // The board can have 4 memory configurations:
    //
    //  * One 8Mb device on SDCS0 (with gaps <= A11 N/C in 8Mb devices)
    //  * Two 8Mb devices, SDCS0,SDCS1 (with gaps) making 16Mb
    //  * One 16Mb device on SDCS0
    //  * Two 16Mb devices on SDCS0,SDCS1 making 32Mb
    //
    // Gaps: the SDRAM setup and wiring are the same for both 8Mb and 16Mb
    // devices.  A11 of the SDRAM's addressing is not connected when 8Mb
    // devices are installed; the 8Mb device occupies 16Mb of space.  A11
    // on the SDRAM is the most significant bit below the two bank-select
    // bits, there are 4 banks each occupying 4Mb, so therefore in "real
    // money" it has the value 2Mb.
    //
    // SDCS0 maps to the 128Mb area 0xC00 Mb (0xC00000000)
    // SDCS1 maps to the 128Mb area 0xC80 Mb (0xC80000000)
    //
    // So therefore, if there are two 8Mb devices installed, we have memory
    // in these ranges:
    //
    //    0xC0000000...0xC01FFFFF
    //    0xC0400000...0xC05FFFFF
    //    0xC0800000...0xC09FFFFF
    //    0xC0C00000...0xC0DFFFFF
    //
    //    0xC8000000...0xC81FFFFF
    //    0xC8400000...0xC85FFFFF
    //    0xC8800000...0xC89FFFFF
    //    0xC8C00000...0xC8DFFFFF
    //
    // This function is currently executing on a stack in real memory
    // addresses, somewhere in that initial range 0xC0000000...0xC01FFFFF,
    // so we can probe other related addresses to discover how much memory
    // we really have, and then set up the Memory Map accordingly.

    typedef volatile cyg_uint32 *mptr; // for memory access
    typedef cyg_uint32 aptr;        // for arithmetic
    volatile int testmem[4] = {0};  // on my stack, ergo in base memory

    // Pointers to test memory, and into the possible gap:
    mptr basep =  &testmem[0];
    mptr basep2 = &testmem[1];
    mptr gapp =  (mptr)((2 * SZ_1M) | (aptr)basep);
        
    // And just look at any pair of words to discover whether there is
    // a 2nd device there.
    mptr dev2p = (mptr)((0xC80u * SZ_1M) | (aptr)basep);
    mptr dev2p2 = (mptr)(4 + (aptr)dev2p);

    // This is a pointer to where hal_dram_size is in physical memory,
    // since memory mapping is not yet set up:
    int *p_hdsize = (int *)((aptr)(&hal_dram_size) | (0xC00u *SZ_1M));
    int *p_hdtype = (int *)((aptr)(&hal_dram_type) | (0xC00u *SZ_1M));

    *p_hdsize = 0; // Initialize it to a bogus value here.
    *p_hdtype = 0x0108; // Initialize it to a bogus value here.
    
    // Equivalent of asserts for our assumptions, but too early to use
    // CYG_ASSERT( ..., "basep not in bank0 of device 0, lower half" );
    if ( 0xC0000000u >= (aptr)basep ) {
        *p_hdtype |= 1 << 16; // flag an error code
        goto breakout;
    }
    if ( 0xC0200000u < (aptr)basep ) {
        *p_hdtype |= 2 << 16;
        goto breakout;
    }
    
    // First confirm that the technique works for memory that's
    // definitely there!
    *basep  = 0x55667711;
    *basep2 = 0xCC33AA44;
    if ( 0x55667711 != *basep ||
         0xCC33AA44 != *basep2 ) {
        *p_hdtype |= 10 << 16;
        goto breakout;
    }
    // Now test writing to the gap...
    *gapp = 0xDD2288BB;
    if ( 0xCC33AA44 != *basep2 ) { // Should not be corrupted whatever
        *p_hdtype |= 20 << 16;
        goto breakout;
    }
    if ( 0xDD2288BB != *basep &&
         0x55667711 != *basep ) { // Should be one of those
        *p_hdtype |= 30 << 16;
        goto breakout;
    }

    if (0xDD2288BB == *basep)
        *p_hdtype = 8; // Lower byte is SDRAM size in Mb.
    else {
        // it could be 16Mb or 32Mb:
        basep2 = (mptr)((16 * SZ_1M) | (aptr)basep);
        *basep = 0xFF11AA00;
        *basep2 = 0x33557799;
        // (intersperse some other activity)
        testmem[2] += testmem[3];
        if ( 0xFF11AA00 != *basep &&
             0x33557799 != *basep ) { // Should be one of those
            *p_hdtype |= 40 << 16;
            goto breakout;
        }
        *p_hdtype = (0xFF11AA00 == *basep) ? 32 : 16;
    }

    // Now test whether dev2p stores data:
    *dev2p  = 0x11224488;
    *dev2p2 = 0x77BBDDEE;
    // (intersperse some other activity)
    testmem[2] += testmem[3];

    *p_hdtype |= ((( 0x11224488 == *dev2p ) && (0x77BBDDEE == *dev2p2 ))
                  ? 0x200 : 0x100); // Next byte is devcount.
    
 breakout:
    // NB: *p_hdtype is carefully crafted to make a system with an error
    // detected above default to a single 8Mb device in all the ensuing
    // setup.

    // So now we should know:
    *p_hdsize = (0xff & ((*p_hdtype) >> 8)) * (0xff & *p_hdtype) * SZ_1M;
    
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
    X_ARM_MMU_SECTION(0x000,  0x500,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Boot flash ROMspace */

#endif // CYG_HAL_STARTUP_ROM - ROM start only
// We can do the mapping of the other stuff (NOT RAM) in RAM startup OK.

    X_ARM_MMU_SECTION(0x180,  0x180,    16,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Ethernet Adaptor */
    X_ARM_MMU_SECTION(0x400,  0x400,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* External IO Mux */
    X_ARM_MMU_SECTION(0x480,  0x480,   128,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* External IO NonMux */

#ifdef CYG_HAL_STARTUP_ROM // ROM start only

    X_ARM_MMU_SECTION(0x800,  0x800, 0x400,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* StrongARM(R) Registers */

//    X_ARM_MMU_SECTION(0xC00,      0,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0,... */
//    X_ARM_MMU_SECTION(0xC00,  0xC00,    32,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0,... */

    if ( (*p_hdtype & 32) ) { // Then they are 32Mb devices - KISS:
        X_ARM_MMU_SECTION(0xC00,      0,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        X_ARM_MMU_SECTION(0xC00,  0xC00,    32,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        if ( 0x200 & *p_hdtype ) { // Got the 2nd device?
            X_ARM_MMU_SECTION(0xC80,     32,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            X_ARM_MMU_SECTION(0xC80,  0xC80,    32,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
        }
    }
    else if ( (*p_hdtype & 16) ) { // Then they are 16Mb devices - KISS:
        X_ARM_MMU_SECTION(0xC00,      0,    16,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        X_ARM_MMU_SECTION(0xC00,  0xC00,    16,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        if ( 0x200 & *p_hdtype ) { // Got the 2nd device?
            X_ARM_MMU_SECTION(0xC80,     16,    16,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            X_ARM_MMU_SECTION(0xC80,  0xC80,    16,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
        }
    }
    else { // Then they are 8Mb devices, complicated:
        X_ARM_MMU_SECTION(0xC00,      0,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        X_ARM_MMU_SECTION(0xC04,      2,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        X_ARM_MMU_SECTION(0xC08,      4,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        X_ARM_MMU_SECTION(0xC0C,      6,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        // Next slot is 16Mb of space, with 2Mb gaps per 4Mb.
        X_ARM_MMU_SECTION(0xC00,  0xC00,    16,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 0 */
        if ( 0x200 & *p_hdtype ) { // Got the 2nd device?
            X_ARM_MMU_SECTION(0xC80,      8,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            X_ARM_MMU_SECTION(0xC84,     10,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            X_ARM_MMU_SECTION(0xC88,     12,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            X_ARM_MMU_SECTION(0xC8C,     14,     2,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
            // Next slot is also 16Mb of space, with 2Mb gaps per 4Mb.
            X_ARM_MMU_SECTION(0xC80,  0xC80,    16,  ARM_UNCACHEABLE, ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* DRAM Bank 1 */
        }
    }
#endif // CYG_HAL_STARTUP_ROM - ROM start only

#ifdef CYGPKG_IO_PCI
    /*
     * Actual Base = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE
     * Virtual Base = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE
     * Size = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE
     * Memory accessible from PCI space. Overrides part of the above mapping.
     */
    for (i = CYGHWR_HAL_ARM_NANO_PCI_MEM_MAP_BASE >> 20;
         i < ((CYGHWR_HAL_ARM_NANO_PCI_MEM_MAP_BASE+CYGHWR_HAL_ARM_NANO_PCI_MEM_MAP_SIZE) >> 20); 
         i++) {
#ifndef CYG_HAL_STARTUP_ROM
        // RAM start - common code below must go via uncached pointer
        int *p_hdsize = (int *)(((cyg_uint32)&hal_dram_size) | (0xC00u *SZ_1M));
#endif // not CYG_HAL_STARTUP_ROM - RAM start only
        // Find the actual real address as above if already mapped:
        cyg_uint32 phys = hal_virt_to_phys_address( ((cyg_uint32)i) << 20 );
        int j = phys >> 20;
        if ( ! ( 0xc00 < j && j < 0xe00 ) ) {
            // Not in physical SDRAM so yet mapped - so steal some from the main area.
            int k = (*p_hdsize) >> 20; // Top MegaByte
            k--;
            phys = hal_virt_to_phys_address( ((cyg_uint32)k) << 20 );
            j = phys >> 20;
            CYG_ASSERT( 0xc00 < j && j < 0xe00, "Top Mb physical address not in SDRAM" );
            (*p_hdsize) = (k << 20); // We just stole 1Mb.
            *(ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, k)) = 0; // smash the old entry
        }
        CYG_ASSERT( 0xc00 < j && j < 0xe00, "PCI physical address not in SDRAM" );
        ARM_MMU_SECTION(ttb_base, j, i,
                        ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }
#endif

#ifdef CYG_HAL_STARTUP_ROM
    X_ARM_MMU_SECTION(0xE00,  0xE00,   128,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* Zeros (Cache Clean) Bank */
#endif // CYG_HAL_STARTUP_ROM - ROM start only

    // All done, phew!
}

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
// RAM startup only - rewrite relevent bits depending on config
#ifndef CYG_HAL_STARTUP_ROM
    HAL_DCACHE_SYNC();            // Force data out
    hal_mmu_init();               // This works on real addresses only
    HAL_DCACHE_INVALIDATE_ALL();  // Flush TLBs: make new mmu state effective
#endif // ! CYG_HAL_STARTUP_ROM - RAM start only
#ifdef CYGPKG_IO_PCI
    cyg_pci_window_real_base =
        hal_virt_to_phys_address( CYGHWR_HAL_ARM_NANO_PCI_MEM_MAP_BASE );
#endif
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void nano_program_new_stack(void *func)
{
    register CYG_ADDRESS stack_ptr asm("sp");
    register CYG_ADDRESS old_stack asm("r4");
    register code_fun *new_func asm("r0");
    old_stack = stack_ptr;
    stack_ptr = CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS);
    new_func = (code_fun*)func;
    new_func();
    stack_ptr = old_stack;
    return;
}

//
// Memory layout
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
    else
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_SECTION_heap1 + CYGMEM_SECTION_heap1_SIZE) ) {
        // hal_dram_size excludes the PCI window on this platform.
        if ( regionend > (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size )
            // Only report if the heap shrank; if it abuts RAMtop, the
            // previous test will have caught it already.  If RAM enlarged,
            // but the heap did not abut RAMtop then there is likely
            // something in the way, so don't trample it.
            regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
#endif
    return regionend;
}


// ------------------------------------------------------------------------
// EOF nano_misc.c
