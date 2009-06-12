//==========================================================================
//
//      ebsa285_misc.c
//
//      HAL misc board support code for StrongARM EBSA285-1
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
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ebsa285.h>        // Hardware definitions

#include <cyg/infra/diag.h>             // diag_printf

#include <string.h> // memset

/*
 * Toggle LED for debugging purposes.
 */
/*
 * EBSA-285 Soft I/O Register
 */
#define EBSA_285_SOFT_IO_REGISTER           ((cyg_uint32 *)0x40012000)

/*
 * EBSA-285 Soft I/O Register Bit Field definitions
 */
#define EBSA_285_SOFT_IO_TOGGLE             0x80
#define EBSA_285_SOFT_IO_RED_LED            0x04
#define EBSA_285_SOFT_IO_GREEN_LED          0x02
#define EBSA_285_SOFT_IO_AMBER_LED          0x01
#define EBSA_285_SOFT_IO_J9_9_10_MASK       0x40
#define EBSA_285_SOFT_IO_J9_11_12_MASK      0x20
#define EBSA_285_SOFT_IO_J9_13_14_MASK      0x10
#define EBSA_285_SOFT_IO_SWITCH_L_MASK      0x0F

static void
hal_bsp_mmu_init(int sdram_size);

// Some initialization has already been done before we get here.
//
// Set up the interrupt environment.
// Set up the MMU so that we can use caches.
// Enable caches.
// - All done!


void hal_hardware_init(void)
{
    // Disable all interrupt sources:
    *SA110_IRQCONT_IRQENABLECLEAR = 0xffffffff;
    *SA110_IRQCONT_FIQENABLECLEAR = 0xffffffff; // including FIQ
    // Disable the timers
    *SA110_TIMER1_CONTROL = 0;
    *SA110_TIMER2_CONTROL = 0;
    *SA110_TIMER3_CONTROL = 0;
    *SA110_TIMER4_CONTROL = 0;

    *SA110_TIMER1_CLEAR = 0;            // Clear any pending interrupt
    *SA110_TIMER2_CLEAR = 0;            // (Data: don't care)
    *SA110_TIMER3_CLEAR = 0;
    *SA110_TIMER4_CLEAR = 0;

    // Let the timer run at a default rate (for delays)
    hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);

    // Set up MMU so that we can use caches
    hal_bsp_mmu_init( hal_dram_size );

    // Enable caches
    HAL_DCACHE_ENABLE();
    HAL_ICACHE_ENABLE();

    // Set up eCos/ROM interfaces
    hal_if_init();
}

// -------------------------------------------------------------------------
// MMU initialization:

static void
hal_bsp_mmu_init(int sdram_size)
{
    unsigned long ttb_base = ((unsigned long)0x4000); // could be external
    unsigned long i;

    *EBSA_285_SOFT_IO_REGISTER = ~EBSA_285_SOFT_IO_RED_LED; // Red LED on


// For if we assign the ttb base dynamically:
//    if ((ttb_base & ARM_TRANSLATION_TABLE_MASK) != ttb_base) {
//        // we cannot do this:
//        while ( 1 ) {
//            *EBSA_285_SOFT_IO_REGISTER = 0; // All LEDs on
//            for ( i = 100000; i > 0 ; i++ ) ;
//            *EBSA_285_SOFT_IO_REGISTER = 7; // All LEDs off
//            for ( i = 100000; i > 0 ; i++ ) ;
//#ifdef CYG_HAL_STARTUP_RAM
//            return; // Do not bother looping forever...
//#endif
//        }
//    }

    /*
     * Set the TTB register
     */
    asm volatile ("mcr  p15,0,%0,c2,c0,0" 
                  :
                  : "r"(ttb_base)
                /*:*/
        );
    /*
     * Set the Domain Access Control Register
     */
    i = ARM_ACCESS_TYPE_MANAGER(0)    | 
        ARM_ACCESS_TYPE_NO_ACCESS(1)  |
        ARM_ACCESS_TYPE_NO_ACCESS(2)  |
        ARM_ACCESS_TYPE_NO_ACCESS(3)  |
        ARM_ACCESS_TYPE_NO_ACCESS(4)  |
        ARM_ACCESS_TYPE_NO_ACCESS(5)  |
        ARM_ACCESS_TYPE_NO_ACCESS(6)  |
        ARM_ACCESS_TYPE_NO_ACCESS(7)  |
        ARM_ACCESS_TYPE_NO_ACCESS(8)  |
        ARM_ACCESS_TYPE_NO_ACCESS(9)  |
        ARM_ACCESS_TYPE_NO_ACCESS(10) |
        ARM_ACCESS_TYPE_NO_ACCESS(11) |
        ARM_ACCESS_TYPE_NO_ACCESS(12) |
        ARM_ACCESS_TYPE_NO_ACCESS(13) |
        ARM_ACCESS_TYPE_NO_ACCESS(14) |
        ARM_ACCESS_TYPE_NO_ACCESS(15);

    asm volatile ("mcr  p15,0,%0,c3,c0,0" 
                  :
                  : "r"(i)
                /*:*/
        );

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*
     * We only do direct mapping for the EBSA board. That is, all
     * virt_addr == phys_addr.
     */

    /*
     * Actual Base = 0x000(00000)
     * Virtual Base = 0x000(00000)
     * Size = Max SDRAM
     * SDRAM
     */
    for (i = 0x000; i < (sdram_size >> 20); i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_BUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

#ifdef CYGPKG_IO_PCI
    /*
     * Actual Base = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE
     * Virtual Base = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE
     * Size = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE
     * Memory accessible from PCI space. Overrides part of the above mapping.
     */
    for (i = CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE >> 20;
         i < ((CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_BASE+CYGHWR_HAL_ARM_EBSA285_PCI_MEM_MAP_SIZE) >> 20); 
         i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }
#endif

    /*
     * Actual Base = 0x400(00000)
     * Virtual Base = 0x400(00000)
     * Size = 1M
     * 21285 Registers
     *
     * Actual Base = 0x400(10000)
     * Virtual Base = 0x400(10000)
     * Size = 1M
     * Soft I/O port and XBus IO
     */
    ARM_MMU_SECTION(ttb_base, 0x400, 0x400,
		    ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
		    ARM_ACCESS_PERM_RW_RW);

    /*
     * Actual Base = 0x410(00000) - 0x413(FFFFF)
     * Virtual Base = 0x410(00000) - 0x413(FFFFF)
     * Size = 4M
     * FLASH ROM
     */
    for (i = 0x410; i <= 0x413; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base = 0x420(00000)
     * Virtual Base = 0x420(00000)
     * Size = 1M
     * 21285 CSR Space
     */
    ARM_MMU_SECTION(ttb_base, 0x420, 0x420, 
                    ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                    ARM_ACCESS_PERM_RW_RW);

    /*
     * Actual Base = 0x500(00000)-0x50F(FFFFF)
     * Virtual Base = 0x500(00000)-0x50F(FFFFF)
     * Size = 16M
     * Zeros (Cache Clean) Bank
     */
    for (i = 0x500; i <= 0x50F; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_CACHEABLE, ARM_BUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base = 0x780(00000)-0x78F(FFFFF)
     * Virtual Base = 0x780(00000)-0x78F(FFFFF)
     * Size = 16M
     * Outbound Write Flush
     */
    for (i = 0x780; i <= 0x78F; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base = 0x790(00000)-0x7C0(FFFFF)
     * Virtual Base = 0x790(00000)-0x7C0(FFFFF)
     * Size = 65M
     * PCI IACK/Config/IO Space
     */
    for (i = 0x790; i <= 0x7C0; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_UNBUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    /*
     * Actual Base = 0x800(00000) - 0xFFF(FFFFF)
     * Virtual Base = 0x800(00000) - 0xFFF(FFFFF)
     * Size = 2G
     * PCI Memory Space
     */
    for (i = 0x800; i <= 0xFFF; i++) {
        ARM_MMU_SECTION(ttb_base, i, i,
                        ARM_UNCACHEABLE, ARM_BUFFERABLE,
                        ARM_ACCESS_PERM_RW_RW);
    }

    *EBSA_285_SOFT_IO_REGISTER = ~EBSA_285_SOFT_IO_AMBER_LED; // AmberLED on

}

/*------------------------------------------------------------------------*/

//
// Memory layout
//

externC cyg_uint8 *
hal_arm_mem_real_region_top( cyg_uint8 *regionend )
{
    CYG_ASSERT( hal_dram_size > 0, "Didn't detect DRAM size!" );
    CYG_ASSERT( hal_dram_size <=  256<<20,
                "More than 256MB reported - that can't be right" );

    // is it the "normal" end of the DRAM region? If so, it should be
    // replaced by the real size
    if ( regionend ==
         ((cyg_uint8 *)CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE) ) {
        regionend = (cyg_uint8 *)CYGMEM_REGION_ram + hal_dram_size;
    }
    return regionend;
} // hal_arm_mem_real_region_top()



// -------------------------------------------------------------------------
static cyg_uint32 _period;

void hal_clock_initialize(cyg_uint32 period)
{
    _period = period;

    *SA110_TIMER3_CONTROL = 0;          // Disable while we are setting up

    *SA110_TIMER3_LOAD = period;        // Reload value

    *SA110_TIMER3_CLEAR = 0;            // Clear any pending interrupt
                                        // (Data: don't care)

    *SA110_TIMER3_CONTROL = 0x000000cc; // Enable, Periodic (auto-reload),
                   // External clock (3.68MHz on irq_in_l[2] for Timer 3)

    *SA110_TIMER3_CLEAR = 0;            // Clear any pending interrupt again

    // That's all.
}

// This routine is called during a clock interrupt.

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    *SA110_TIMER3_CLEAR = period; // Clear any pending interrupt (Data: don't care)
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

void hal_clock_read(cyg_uint32 *pvalue)
{
    *pvalue = (cyg_uint32)(_period) - *SA110_TIMER3_VALUE;
}

//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    int diff, diff2;
    cyg_uint32 val1, val2;

    while (usecs-- > 0) {
        diff = 0;
        val1 = *SA110_TIMER3_VALUE;
        while (diff < 3) {
            while ((val2 = *SA110_TIMER3_VALUE) == val1) ;
            if (*SA110_TIMER3_LOAD) {
                // A kernel is running, the counter may get reset as we watch
                diff2 = val2 - val1;
                if (diff2 < 0) diff2 += *SA110_TIMER3_LOAD;
                diff += diff2;
            } else {
                diff += val2 - val1;
            }
        }
    }
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
    int sources;
    int index;

#if 0 // test FIQ and print alert if active - really for debugging
    sources = *SA110_IRQCONT_FIQSTATUS;
    if ( 0 != sources )
        diag_printf( "FIQ source active!!! - fiqstatus %08x irqstatus %08x\n",
                     sources, *SA110_IRQCONT_IRQSTATUS );
    else
#endif // Scan FIQ sources also

    sources = *SA110_IRQCONT_IRQSTATUS;

// if we come to support FIQ properly...
//    if ( 0 == sources )
//        sources = *SA110_IRQCONT_FIQSTATUS;

    // Nothing wrong with scanning them in the order provided...
    // and it'll make the serial device steal fewer cycles.
    // So, knowing this is an ARM:
    if ( sources & 0xff )
        index = 0;
    else if ( sources & 0xff00 )
        index = 8;
    else if ( sources & 0xff0000 )
        index = 16;
    else // if ( sources & 0xff000000 )
        index = 24;

    do {
        if ( (1 << index) & sources )
            return index;
        index++;
    } while ( index & 7 );
    
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    *SA110_IRQCONT_IRQENABLECLEAR = 1 << vector;
}

void hal_interrupt_unmask(int vector)
{
    *SA110_IRQCONT_IRQENABLESET = 1 << vector;
}

void hal_interrupt_acknowledge(int vector)
{
    // Nothing to do here.
}

void hal_interrupt_configure(int vector, int level, int up)
{
    // No interrupts are configurable on this hardware
}

void hal_interrupt_set_level(int vector, int level)
{
    // No interrupts are configurable on this hardware
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void ebsa285_program_new_stack(void *func)
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

/*------------------------------------------------------------------------*/
// EOF ebsa285_misc.c
