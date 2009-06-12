//==========================================================================
//
//      aaed2000_misc.c
//
//      HAL misc board support code for ARM9/AAED2000
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
// Contributors: hmt, Travis C. Furrer <furrer@mit.edu>, jskov
// Date:         2000-05-21
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
#include <cyg/hal/aaed2000.h>           // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

#include <string.h> // memset


// -------------------------------------------------------------------------
// MMU initialization:
// 
// These structures are laid down in memory to define the translation
// table.
// 

/*
 * ARM Translation Table Base Bit Masks */
#define ARM_TRANSLATION_TABLE_MASK               0xFFFFC000

/*
 * ARM Domain Access Control Bit Masks
 */
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
    unsigned long ttb_base = AAED2000_SDRAM_PHYS_BASE + 0x4000;
    unsigned long i;

    /*
     * Set the TTB register
     */
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

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
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(i) /*:*/);

    /*
     * First clear all TT entries - ie Set them to Faulting
     */
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    /*               Actual  Virtual  Size   Attributes                                                    Function  */
    /*		     Base     Base     MB      cached?           buffered?        access permissions                 */
    /*             xxx00000  xxx00000                                                                                */
    X_ARM_MMU_SECTION(0x000,  0x600,    32,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Boot flash ROMspace CS0 */
    X_ARM_MMU_SECTION(0x100,  0x100,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Ethernet */
    X_ARM_MMU_SECTION(0x300,  0x300,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* AAED2000 board registers */
    X_ARM_MMU_SECTION(0x400,  0x400,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA slot - I/O */
    X_ARM_MMU_SECTION(0x440,  0x440,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA slot - stat*/
    X_ARM_MMU_SECTION(0x480,  0x480,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA slot - attribute */
    X_ARM_MMU_SECTION(0x4C0,  0x4C0,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* PCMCIA slot - common */
    X_ARM_MMU_SECTION(0x500,  0x500,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CF slot - I/O */
    X_ARM_MMU_SECTION(0x540,  0x540,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CF slot - stat*/
    X_ARM_MMU_SECTION(0x580,  0x580,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CF slot - attribute */
    X_ARM_MMU_SECTION(0x5C0,  0x5C0,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* CF slot - common */
    X_ARM_MMU_SECTION(0x800,  0x800,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* AAEC2000 registers */
    // DRAM is non-contiguous, laid out in weird and wonderful ways...
    X_ARM_MMU_SECTION(0xF00,  0x000,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xF10,  0x004,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xF40,  0x008,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xF50,  0x00C,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xF80,  0x010,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xF90,  0x014,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xFC0,  0x018,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    X_ARM_MMU_SECTION(0xFD0,  0x01C,     4,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW); /* SDRAM */
    // Map in DRAM raw as well
    X_ARM_MMU_SECTION(0xF00,  0xF00,   256,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW); /* Raw SDRAM */
}

//
// Platform specific initialization
//
void
plf_hardware_init(void)
{
    HAL_WRITE_UINT8(AAEC_PCDR, 0x22);
    HAL_WRITE_UINT8(AAEC_PCCDR, 0);
    HAL_WRITE_UINT8(AAEC_PBDDR, 0x83);
    HAL_WRITE_UINT8(AAEC_PINMUX, 
                    AAEC_PINMUX_UART3CON | AAEC_PINMUX_PD0CON | AAEC_PINMUX_PE0CON);

    // FIXME - all platform interrupt sources should be configured here
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_TS,  0, 0 );  // Low pulse
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_ETH, 0, 1 );  // High pulse
}

//
// Support for platform specific I/O channels
//

externC void lcd_comm_init(void);

void
plf_if_init(void) 
{
    aaed2000_KeyboardInit();
#ifdef CYGSEM_AAED2000_LCD_COMM
    // Initialize I/O channel
    lcd_comm_init();
#endif
}

// -------------------------------------------------------------------------
void hal_clock_initialize(cyg_uint32 period)
{
    // Use timer1 for the kernel clock
    HAL_WRITE_UINT32(AAEC_TMR_T1LOAD, period);
    HAL_WRITE_UINT32(AAEC_TMR_T1CONTROL, 
                     AAEC_TMR_TxCONTROL_ENABLE
                     | AAEC_TMR_TxCONTROL_MODE_PERIODIC
                     | AAEC_TMR_TxCONTROL_508KHZ);

    // Unmask timer 0 interrupt
    HAL_INTERRUPT_CONFIGURE( CYGNUM_HAL_INTERRUPT_RTC, 1, 1 );
    HAL_INTERRUPT_UNMASK( CYGNUM_HAL_INTERRUPT_RTC );
}

// This routine is called during a clock interrupt.
void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    // Clear pending interrupt bit
    HAL_INTERRUPT_ACKNOWLEDGE(vector);
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

// Note: The "contract" for this function is that the value is the number
// of hardware clocks that have happened since the last interrupt (i.e.
// when it was reset).

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 ctr;

    HAL_READ_UINT32(AAEC_TMR_T1VALUE, ctr);
    ctr = CYGNUM_HAL_RTC_PERIOD - ctr;
    *pvalue = ctr;
}

//
// Delay for some number of micro-seconds
//   Use timer #3 which runs at [fixed] 7.3728 MHz
//   Since this is only a 16 bit counter, it may be necessary
//   to run a loop to achieve sufficiently large delay values.
//
// Note: The 7.3728MHz value does not seem to work in practice
// It seems to be off by about a factor of 2.
//
void hal_delay_us(cyg_int32 usecs)
{
    static struct _tmr_vals {
        int us_val, tmr_val;
    } tmr_vals[] = {
        { 2*1000, 7372 },
        {  2*100,  737 },
        {   2*10,   74 },
        {    2*1,    7 },
        {      0,    0 }
    };
    struct _tmr_vals *vals = tmr_vals;
    cyg_uint32 state;
    
    while (vals->tmr_val) {
        while (usecs >= vals->us_val) {
            // disable timer #3
            HAL_WRITE_UINT32(AAEC_TMR_T3CONTROL, 0);
            HAL_WRITE_UINT32(AAEC_TMR_T3EOI, 0);
            // configure for tmr_val
            HAL_WRITE_UINT32(AAEC_TMR_T3LOAD, vals->tmr_val);
            // enable
            HAL_WRITE_UINT32(AAEC_TMR_T3CONTROL, 
                             AAEC_TMR_TxCONTROL_ENABLE | AAEC_TMR_TxCONTROL_MODE_FREE);
            // wait for overflow
            do {
                HAL_READ_UINT32(AAEC_INT_RSR, state);
            } while ((state & (1<<AAEC_INTS_T3OI)) == 0);
            usecs -= vals->us_val;
        }
        vals++;
    }
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int hal_IRQ_handler(void)
{
    int irq = CYGNUM_HAL_INTERRUPT_NONE;
    int vec;
    cyg_uint32 sr;

    HAL_READ_UINT32(AAEC_INT_SR, sr);
    for (vec = 0; vec <= CYGNUM_HAL_INTERRUPT_BMIINTR; vec++) {
        if (sr & (1<<vec)) {
            irq = vec;
            break;
        }
    }

    return irq;
}

//
// Interrupt control
//

struct {
    int   gpio_int;   // GPIO (F) interrupt source
    cyg_haladdress eoi;        // Acknowledge location
} AAED2000_INTMAP[] = {
    { 0, 0}, // CYGNUM_HAL_INTERRUPT_TS            CYGNUM_HAL_INTERRUPT_GPIO0FIQ
    {-1, AAEC_CSC_BLEOI}, // CYGNUM_HAL_INTERRUPT_BLINT         1
    {-1, AAEC_CSC_TEOI},  // CYGNUM_HAL_INTERRUPT_WEINT         2
    {-1, AAEC_CSC_MCEOI}, // CYGNUM_HAL_INTERRUPT_MCINT         3
    {-1, AAEC_COD_CDEOI}, // CYGNUM_HAL_INTERRUPT_CSINT         4
    { 1, 0}, // CYGNUM_HAL_INTERRUPT_ETH           CYGNUM_HAL_INTERRUPT_GPIO1INTR
    { 2, 0}, // CYGNUM_HAL_INTERRUPT_PCMCIA_CD2    CYGNUM_HAL_INTERRUPT_GPIO2INTR
    { 3, 0}, // CYGNUM_HAL_INTERRUPT_PCMCIA_CD1    CYGNUM_HAL_INTERRUPT_GPIO3INTR
    {-1, AAEC_TMR_T1EOI}, // CYGNUM_HAL_INTERRUPT_TC1OI         8
    {-1, AAEC_TMR_T2EOI}, // CYGNUM_HAL_INTERRUPT_TC2OI         9
    {-1, AAEC_RTC_RTCEOI},// CYGNUM_HAL_INTERRUPT_RTCMI        10
    {-1, AAEC_CSC_TEOI},  // CYGNUM_HAL_INTERRUPT_TINTR        11
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_UART1INTR    12
    {-1, AAEC_UART2_UMS2EOI}, // CYGNUM_HAL_INTERRUPT_UART2INTR    13
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_LCDINTR      14
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_SSEOTI       15
    {-1, AAEC_UART2_UMS3EOI}, // CYGNUM_HAL_INTERRUPT_UART3INTR    16
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_SCIINTR      17
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_AACINTR      18
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_MMCINTR      19
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_USBINTR      20
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_DMAINTR      21
    {-1, AAEC_TMR_T3EOI}, // CYGNUM_HAL_INTERRUPT_TC3OI        22
    { 4, 0}, // CYGNUM_HAL_INTERRUPT_SCI_VCCEN    CYGNUM_HAL_INTERRUPT_GPIO4INTR
    { 5, 0}, // CYGNUM_HAL_INTERRUPT_SCI_DETECT   CYGNUM_HAL_INTERRUPT_GPIO5INTR
    { 6, 0}, // CYGNUM_HAL_INTERRUPT_PCMCIA_RDY1  CYGNUM_HAL_INTERRUPT_GPIO6INTR
    { 7, 0}, // CYGNUM_HAL_INTERRUPT_PCMCIA_RDY2  CYGNUM_HAL_INTERRUPT_GPIO7INTR
    {-1, 0}, // CYGNUM_HAL_INTERRUPT_BMIINTR      27
};

void hal_interrupt_mask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    if (vector <= CYGNUM_HAL_INTERRUPT_BMIINTR) {
        HAL_WRITE_UINT32(AAEC_INT_ENC, (1 << vector));
    }
}

void hal_interrupt_unmask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    if (vector <= CYGNUM_HAL_INTERRUPT_BMIINTR) {
        HAL_WRITE_UINT32(AAEC_INT_ENS, (1 << vector));
    }
}

void hal_interrupt_acknowledge(int vector)
{
    cyg_haladdress eoi;
    int gpio;
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    if (vector <= CYGNUM_HAL_INTERRUPT_BMIINTR) {
        // Must be cleared at the source
        if ((eoi = AAED2000_INTMAP[vector].eoi) != 0) {
            HAL_WRITE_UINT32(eoi, 0);  // Any write clears interrupt
        } else if ((gpio = AAED2000_INTMAP[vector].gpio_int) >= 0) {
            // GPIO interrupts require special care
            HAL_WRITE_UINT32(AAEC_GPIO_FEOI, (1<<gpio));
        }
    }
}

void hal_interrupt_configure(int vector, int level, int up)
{
    int gpio;
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
    if (vector <= CYGNUM_HAL_INTERRUPT_BMIINTR) {
        if ((gpio = AAED2000_INTMAP[vector].gpio_int) >= 0) {
            // Only GPIO interrupts can be configured
            int mask = (1<<gpio);
            cyg_uint32 cur;
            // Set type (level or edge)
            HAL_READ_UINT32(AAEC_GPIO_INT_TYPE1, cur);
            if (level) {
                // Level driven
                cur &= ~mask;
            } else {
                // Edge driven
                cur |= mask;
            }
            HAL_WRITE_UINT32(AAEC_GPIO_INT_TYPE1, cur);
            // Set level (high/rising or low/falling)
            HAL_READ_UINT32(AAEC_GPIO_INT_TYPE2, cur);
            if (up) {
                // Trigger on high/rising
                cur |= mask;
            } else {
                // Trigger on low/falling
                cur &= ~mask;
            }
            HAL_WRITE_UINT32(AAEC_GPIO_INT_TYPE2, cur);
            // Enable as interrupt
            HAL_READ_UINT32(AAEC_GPIO_INTEN, cur);
            cur |= mask;
            HAL_WRITE_UINT32(AAEC_GPIO_INTEN, cur);
        }
    }
}

void hal_interrupt_set_level(int vector, int level)
{
}

cyg_uint32
hal_virt_to_phys_address(cyg_uint32 virt)
{
    cyg_uint32 phys = 0xFFFFFFFF, dram_page;
    static cyg_uint32 _dram_map[] = {
        0xF0000000, 0xF1000000, 0xF4000000, 0xF5000000,
        0xF8000000, 0xF9000000, 0xFC000000, 0xFD000000
    };

    // Hard-wired, rather than walk the tables
    switch ((virt & 0xF0000000) >> 28) {
    case 0x0: // DRAM
        if ((virt & 0x0E000000) == 0) {
            dram_page = _dram_map[((virt & 0x01C00000) >> 22)];
            phys = dram_page | virt;
        } else {
            phys = 0xFFFFFFFF;
        }
        break;
    case 0x6: // FLASH
        phys = (virt & 0x0FFFFFFF);
        break;
    case 0x1:
    case 0x2:
    case 0x7:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
        // Not mapped
        phys = 0xFFFFFFFF;
        break;
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x8:
    case 0xF:
        // Mapped 1-1
        phys = virt;
        break;
    }
    return phys;
}
