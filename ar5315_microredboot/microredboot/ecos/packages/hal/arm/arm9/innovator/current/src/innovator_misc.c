//==========================================================================
//
//      innovator_misc.c
//
//      HAL misc board support code for ARM9/INNOVATOR
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
// Author(s):    Patrick Doyle <wpd@delcomsys.com>
// Contributors: Patrick Doyle <wpd@delcomsys.com>
// Date:         2002-12-17
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//==========================================================================

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
#include <cyg/hal/innovator.h>          // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#include <string.h>			// memset

// -------------------------------------------------------------------------
// MMU initialization:
// 
// These structures are laid down in memory to define the translation
// table.
// 

// ARM Translation Table Base Bit Masks
#define ARM_TRANSLATION_TABLE_MASK               0xFFFFC000

// ARM Domain Access Control Bit Masks
#define ARM_ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << (domain_num)*2)
#define ARM_ACCESS_TYPE_CLIENT(domain_num)       (0x1 << (domain_num)*2)
#define ARM_ACCESS_TYPE_MANAGER(domain_num)      (0x3 << (domain_num)*2)

struct ARM_MMU_FIRST_LEVEL_FAULT {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID 0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    int id : 2;
    int imp : 2;
    int domain : 4;
    int sbz : 1;
    int base_address : 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
    int id : 2;
    int b : 1;
    int c : 1;
    int imp : 1;
    int domain : 4;
    int sbz0 : 1;
    int ap : 2;
    int sbz1 : 8;
    int base_address : 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID 0x3

#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
   (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE 0x4000

#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base,              \
                        cacheable, bufferable, perm)                      \
    CYG_MACRO_START                                                       \
        register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
                                                                          \
        desc.word = 0;                                                    \
        desc.section.id = ARM_MMU_FIRST_LEVEL_SECTION_ID;                 \
        desc.section.imp = 1;                                             \
        desc.section.domain = 0;                                          \
        desc.section.c = (cacheable);                                     \
        desc.section.b = (bufferable);                                    \
        desc.section.ap = (perm);                                         \
        desc.section.base_address = (actual_base);                        \
        *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) \
                            = desc.word;                                  \
    CYG_MACRO_END

#define X_ARM_MMU_SECTION(abase,vbase,size,cache,buff,access)      \
    { int i; int j = abase; int k = vbase;                         \
      for (i = size; i > 0 ; i--,j++,k++)                          \
      {                                                            \
        ARM_MMU_SECTION(ttb_base, j, k, cache, buff, access);      \
      }                                                            \
    }

union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARM_MMU_FIRST_LEVEL_RESERVED reserved;
};

#define ARM_UNCACHEABLE                         0
#define ARM_CACHEABLE                           1
#define ARM_UNBUFFERABLE                        0
#define ARM_BUFFERABLE                          1

#define ARM_ACCESS_PERM_NONE_NONE               0
#define ARM_ACCESS_PERM_RO_NONE                 0
#define ARM_ACCESS_PERM_RO_RO                   0
#define ARM_ACCESS_PERM_RW_NONE                 1
#define ARM_ACCESS_PERM_RW_RO                   2
#define ARM_ACCESS_PERM_RW_RW                   3

void
hal_mmu_init(void)
{
    unsigned long ttb_base = INNOVATOR_SDRAM_PHYS_BASE + 0x4000;
    unsigned long i;

    // Set the TTB register
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    // Set the Domain Access Control Register
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
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

    // First clear all TT entries - ie Set them to Faulting
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    //               Actual  Virtual  Size   Attributes                                                    Function
    //		     Base     Base     MB      cached?           buffered?        access permissions
    //             xxx00000  xxx00000
    X_ARM_MMU_SECTION(0x000,  0x100,     4,  ARM_CACHEABLE,   ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); // FLASH CS0
    X_ARM_MMU_SECTION(0x100,  0x000,    32,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); // SDRAM
    X_ARM_MMU_SECTION(0x080,  0x080,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); // FPGA
    X_ARM_MMU_SECTION(0x200,  0x200,     1,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); // Internal SRAM
    X_ARM_MMU_SECTION(0xE00,  0xE00,   512,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); // Internal Peripherals
}

//----------------------------------------------------------------------------
// Platform specific initialization

void
plf_hardware_init(void)
{
#ifdef LATER
    // Disable PLD interrupts
    HAL_WRITE_UINT32(INNOVATOR_INT_MASK_CLEAR, 
                     INNOVATOR_INT_SOURCE_P0 | INNOVATOR_INT_SOURCE_P1 |
                     INNOVATOR_INT_SOURCE_P2 | INNOVATOR_INT_SOURCE_P3 |
                     INNOVATOR_INT_SOURCE_P4 | INNOVATOR_INT_SOURCE_P5);
    // Make PLD0 generate IRQ
    HAL_WRITE_UINT32(INNOVATOR_INT_PRIORITY_0, 0);
#endif
    cyg_uint8 tmp;
    // This should be protected by some sort of #ifdef to test to see if
    // the ethernet has been enabled or not.  I'll add that later.
    HAL_READ_UINT8(0x0800000B, tmp);
    HAL_WRITE_UINT8(0x0800000B, tmp & ~1);
    HAL_DELAY_US(750);

}

// -------------------------------------------------------------------------
void
hal_clock_initialize(cyg_uint32 period)
{
#ifdef LATER
    cyg_uint32 cr;

    HAL_WRITE_UINT32(INNOVATOR_TIMER0_CR, 0);
    HAL_WRITE_UINT32(INNOVATOR_TIMER0_PRE, CYGNUM_HAL_ARM_INNOVATOR_TIMER_PRESCALE - 1);
    HAL_WRITE_UINT32(INNOVATOR_TIMER0_LIMIT, period);
    cr = INNOVATOR_TIMER_CR_MODE_HEARBEAT | INNOVATOR_TIMER_CR_IE;
    HAL_WRITE_UINT32(INNOVATOR_TIMER0_CR, cr);
    HAL_WRITE_UINT32(INNOVATOR_TIMER0_CR, cr | INNOVATOR_TIMER_CR_S);

    // Unmask timer 0 interrupt
    HAL_INTERRUPT_CONFIGURE( CYGNUM_HAL_INTERRUPT_RTC, 1, 1 );
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_RTC );
#endif
}

// This routine is called during a clock interrupt.
void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
#ifdef LATER
    cyg_uint32 cr;

    // Clear pending interrupt bit
    HAL_READ_UINT32(INNOVATOR_TIMER0_CR, cr);
    cr |= INNOVATOR_TIMER_CR_CI;
    HAL_WRITE_UINT32(INNOVATOR_TIMER0_CR, cr);
#endif
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

void
hal_clock_read(cyg_uint32 *pvalue)
{
#ifdef LATER
    cyg_uint32 ctr;

    HAL_READ_UINT32(INNOVATOR_TIMER0_READ, ctr);
    *pvalue = ctr;
#endif
}

//
// Delay for some number of micro-seconds
//
void
hal_delay_us(cyg_int32 usecs)
{
#ifdef LATER
    // Use timer 2 
    cyg_uint32 cr;
    // Divide by 1000000 in two steps to preserve precision.
    cyg_uint32 wait_clocks = ((CYGNUM_HAL_ARM_INNOVATOR_PERIPHERAL_CLOCK/100000)*usecs)/10;

    HAL_WRITE_UINT32(INNOVATOR_TIMER1_CR, 0);
    HAL_WRITE_UINT32(INNOVATOR_TIMER1_PRE, 0);
    HAL_WRITE_UINT32(INNOVATOR_TIMER1_LIMIT, wait_clocks);
    cr = INNOVATOR_TIMER_CR_MODE_ONE_SHOT|INNOVATOR_TIMER_CR_CI;
    HAL_WRITE_UINT32(INNOVATOR_TIMER1_CR, cr);
    HAL_WRITE_UINT32(INNOVATOR_TIMER1_CR, cr | INNOVATOR_TIMER_CR_S);

    // wait for start bit to clear
    do {
        HAL_READ_UINT32(INNOVATOR_TIMER1_CR, cr);
    } while ((INNOVATOR_TIMER_CR_S & cr) != 0);

    //clear interrupt flag
    HAL_WRITE_UINT32(INNOVATOR_TIMER1_CR, 0);
#else
#if 0
    volatile unsigned long long x;
    volatile unsigned long long loop_count = usecs / 10 + 1;

    for (x = 0; x < loop_count; x++) ;
#else
    volatile cyg_uint32 *CNTL_TIMER = (volatile cyg_uint32 *)(0xFFFEC500 + 0x00);
    volatile cyg_uint32 *LOAD_TIM   = (volatile cyg_uint32 *)(0xFFFEC500 + 0x04);
    volatile cyg_uint32 *READ_TIM   = (volatile cyg_uint32 *)(0xFFFEC500 + 0x08);
    cyg_uint32 timer_val, prev_val;
    int too_long = 0;

    if (usecs <= 0) {
      return;
    } else if (usecs > 357913941) {
      /* Clamp at MAX_INT32 / 6 */
      usecs = 357913941;
    }
    /* Enable the clock and halt the timer */
    HAL_WRITE_UINT32(CNTL_TIMER, 0x00000020);

    /* Load the timer */
    HAL_WRITE_UINT32(LOAD_TIM, 6 * usecs);

    /* Start the timer */
    HAL_READ_UINT32(READ_TIM, prev_val);
    HAL_WRITE_UINT32(CNTL_TIMER, 0x00000021);

    /* Wait for it to load (but not too long) */
    do {
      HAL_READ_UINT32(READ_TIM, timer_val);
      if (++too_long >= 100) {
        break;
      }
    } while (timer_val == prev_val);

    /* Wait for it to count down to zero */
    do {
      HAL_READ_UINT32(READ_TIM, timer_val);
    } while (timer_val > 0);
#endif
#endif
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int
hal_IRQ_handler(void)
{
#ifdef LATER
    int vec;
    cyg_uint32 isr;

    HAL_READ_UINT32(INNOVATOR_INT_REQUEST_STATUS, isr);
    for (vec = CYGNUM_HAL_INTERRUPT_PLD_0;
         vec <= CYGNUM_HAL_INTERRUPT_FAST_COMMS; vec++) {
        if (isr & (1<<vec)) {
            return vec;
        }
    }
#endif
    return CYGNUM_HAL_INTERRUPT_NONE;
}

//----------------------------------------------------------------------------
// Interrupt control
//
void
hal_interrupt_mask(int vector)
{
#ifdef LATER
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    HAL_WRITE_UINT32(INNOVATOR_INT_MASK_CLEAR, 1<<vector);
#endif
}

void
hal_interrupt_unmask(int vector)
{
#ifdef LATER
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    HAL_WRITE_UINT32(INNOVATOR_INT_MASK_SET, 1<<vector);
#endif
}

void
hal_interrupt_acknowledge(int vector)
{
#ifdef LATER
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
#endif

}

void
hal_interrupt_configure(int vector, int level, int up)
{
#ifdef LATER
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    CYG_ASSERT(level || up, "Cannot do falling edge");
#endif
}

void
hal_interrupt_set_level(int vector, int level)
{
#ifdef LATER
    cyg_uint32 reg;
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    CYG_ASSERT(level <= 63 && level >= 0, "Invalid level");

    HAL_READ_UINT32(INNOVATOR_INT_PRIORITY_0+4*vector, reg);
    reg &= ~INNOVATOR_INT_PRIORITY_LVL_mask;
    reg |= (level & INNOVATOR_INT_PRIORITY_LVL_mask);
    HAL_WRITE_UINT32(INNOVATOR_INT_PRIORITY_0+4*vector, reg);
#endif
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void innovator_program_new_stack(void *func)
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
