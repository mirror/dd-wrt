//==========================================================================
//
//      aeb_misc.c
//
//      HAL misc board support code for ARM AEB-1
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

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>             // calling interface
#include <cyg/hal/hal_misc.h>           // helper functions
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#include <cyg/hal/drv_api.h>            // HAL ISR support
#endif

/*------------------------------------------------------------------------*/
// On-board timer
/*------------------------------------------------------------------------*/

// Timer registers
#define CYG_DEVICE_TIMER0       ((volatile cyg_uint8  *)0xFFFF1800)
#define CYG_DEVICE_TIMER1       ((volatile cyg_uint8  *)0xFFFF1804)
#define CYG_DEVICE_TIMER2       ((volatile cyg_uint8  *)0xFFFF1808)
#define CYG_DEVICE_TIMER_CTL    ((volatile cyg_uint8  *)0xFFFF180C)
#define CYG_DEVICE_IOCR         ((volatile cyg_uint32 *)0xFFFFA410)
#define CYG_DEVICE_CPM_PCSR     ((volatile cyg_uint32 *)0xFFFFAC04)
#define CYG_DEVICE_CPM_CT0CCR   ((volatile cyg_uint32 *)0xFFFFAC18)
#define CYG_DEVICE_CPM_CT1CCR   ((volatile cyg_uint32 *)0xFFFFAC1C)
#define CYG_DEVICE_CPM_CT2CCR   ((volatile cyg_uint32 *)0xFFFFAC20)

#define IOCR_CT0G               (0x3<<9)
#define IOCR_CT0G_EXTERNAL      (0x0<<9)
#define IOCR_CT0G_PWM0          (0x1<<9)
#define IOCR_CT0G_LOW           (0x2<<9)
#define IOCR_CT0G_HIGH          (0x3<<9)

#define IOCR_CT1G               (0x3<<11)
#define IOCR_CT1G_EXTERNAL      (0x0<<11)
#define IOCR_CT1G_PWM0          (0x1<<11)
#define IOCR_CT1G_LOW           (0x2<<11)
#define IOCR_CT1G_HIGH          (0x3<<11)

#define IOCR_CT2G               (0x3<<13)
#define IOCR_CT2G_EXTERNAL      (0x0<<13)
#define IOCR_CT2G_PWM0          (0x1<<13)
#define IOCR_CT2G_LOW           (0x2<<13)
#define IOCR_CT2G_HIGH          (0x3<<13)

#define TIMER_CTL_TYPE          (0x1<<0)
#define TIMER_CTL_TYPE_BIN      (0x0<<0)
#define TIMER_CTL_TYPE_BCD      (0x1<<0)
#define TIMER_CTL_MODE          (0x7<<1)
#define TIMER_CTL_MODE_IOTC     (0x0<<1) // Interrupt on terminal count
#define TIMER_CTL_MODE_HROS     (0x1<<1) // Hardware retriggerable one-shot
#define TIMER_CTL_MODE_RG       (0x2<<1) // Rate generator
#define TIMER_CTL_MODE_SWG      (0x3<<1) // Square-wave generator
#define TIMER_CTL_MODE_STS      (0x4<<1) // Software triggered strobe
#define TIMER_CTL_MODE_HTS      (0x5<<1) // Hardware triggered strobe
#define TIMER_CTL_RW            (0x3<<4)
#define TIMER_CTL_RW_LATCH      (0x0<<4) // Counter latch
#define TIMER_CTL_RW_LSB        (0x1<<4)
#define TIMER_CTL_RW_MSB        (0x2<<4)
#define TIMER_CTL_RW_BOTH       (0x3<<4)
#define TIMER_CTL_SC            (0x3<<6)
#define TIMER_CTL_SC_CTR0       (0x0<<6)
#define TIMER_CTL_SC_CTR1       (0x1<<6)
#define TIMER_CTL_SC_CTR2       (0x2<<6)
#define TIMER_CTL_SC_RBC        (0x3<<6)

// Interrupt controller registers
#define CYG_DEVICE_ICTL_ICR0    ((volatile cyg_uint32 *)0xFFFFA800)
#define CYG_DEVICE_ICTL_ICR1    ((volatile cyg_uint32 *)0xFFFFA804)
#define CYG_DEVICE_ICTL_ICLR    ((volatile cyg_uint32 *)0xFFFFA808)
#define CYG_DEVICE_ICTL_IRQER   ((volatile cyg_uint32 *)0xFFFFA80C)
#define CYG_DEVICE_ICTL_FIQER   ((volatile cyg_uint32 *)0xFFFFA810)
#define CYG_DEVICE_ICTL_IRQSR   ((volatile cyg_uint32 *)0xFFFFA814)
#define CYG_DEVICE_ICTL_FIQSR   ((volatile cyg_uint32 *)0xFFFFA818)
#define CYG_DEVICE_ICTL_IPR     ((volatile cyg_uint32 *)0xFFFFA81C)

#define ICTL_ICR0_CH0           (0x3<<0)
#define ICTL_ICR0_CH0_HL        (0x1<<0)
#define ICTL_ICR0_CH0_HL_AL     (0x0<<0)  // Active low
#define ICTL_ICR0_CH0_HL_AH     (0x1<<0)  // Active high
#define ICTL_ICR0_CH0_EL        (0x2<<0)
#define ICTL_ICR0_CH0_EL_LT     (0x0<<0)  // Level triggered
#define ICTL_ICR0_CH0_EL_ET     (0x2<<0)  // Edge triggered
#define ICTL_ICR0_CH1           (0x3<<2)
#define ICTL_ICR0_CH1_HL        (0x1<<2)
#define ICTL_ICR0_CH1_HL_AL     (0x0<<2)  // Active low
#define ICTL_ICR0_CH1_HL_AH     (0x1<<2)  // Active high
#define ICTL_ICR0_CH1_EL        (0x2<<2)
#define ICTL_ICR0_CH1_EL_LT     (0x0<<2)  // Level triggered
#define ICTL_ICR0_CH1_EL_ET     (0x2<<2)  // Edge triggered
#define ICTL_ICR0_CH2           (0x3<<4)
#define ICTL_ICR0_CH2_HL        (0x1<<4)
#define ICTL_ICR0_CH2_HL_AL     (0x0<<4)  // Active low
#define ICTL_ICR0_CH2_HL_AH     (0x1<<4)  // Active high
#define ICTL_ICR0_CH2_EL        (0x2<<4)
#define ICTL_ICR0_CH2_EL_LT     (0x0<<4)  // Level triggered
#define ICTL_ICR0_CH2_EL_ET     (0x2<<4)  // Edge triggered
#define ICTL_ICR0_CH3           (0x3<<6)
#define ICTL_ICR0_CH3_HL        (0x1<<6)
#define ICTL_ICR0_CH3_HL_AL     (0x0<<6)  // Active low
#define ICTL_ICR0_CH3_HL_AH     (0x1<<6)  // Active high
#define ICTL_ICR0_CH3_EL        (0x2<<6)
#define ICTL_ICR0_CH3_EL_LT     (0x0<<6)  // Level triggered
#define ICTL_ICR0_CH3_EL_ET     (0x2<<6)  // Edge triggered
#define ICTL_ICR0_CH4           (0x3<<8)
#define ICTL_ICR0_CH4_HL        (0x1<<8)
#define ICTL_ICR0_CH4_HL_AL     (0x0<<8)  // Active low
#define ICTL_ICR0_CH4_HL_AH     (0x1<<8)  // Active high
#define ICTL_ICR0_CH4_EL        (0x2<<8)
#define ICTL_ICR0_CH4_EL_LT     (0x0<<8)  // Level triggered
#define ICTL_ICR0_CH4_EL_ET     (0x2<<8)  // Edge triggered
#define ICTL_ICR0_CH5           (0x3<<10)
#define ICTL_ICR0_CH5_HL        (0x1<<10)
#define ICTL_ICR0_CH5_HL_AL     (0x0<<10)  // Active low
#define ICTL_ICR0_CH5_HL_AH     (0x1<<10)  // Active high
#define ICTL_ICR0_CH5_EL        (0x2<<10)
#define ICTL_ICR0_CH5_EL_LT     (0x0<<10)  // Level triggered
#define ICTL_ICR0_CH5_EL_ET     (0x2<<10)  // Edge triggered
#define ICTL_ICR1_CH6           (0x1<<0)
#define ICTL_ICR1_CH6_HL        (0x1<<0)
#define ICTL_ICR1_CH6_HL_AL     (0x0<<0)  // Active low
#define ICTL_ICR1_CH6_HL_AH     (0x1<<0)  // Active high
#define ICTL_ICR1_CH7           (0x1<<1)
#define ICTL_ICR1_CH7_HL        (0x1<<1)
#define ICTL_ICR1_CH7_HL_AL     (0x0<<1)  // Active low
#define ICTL_ICR1_CH7_HL_AH     (0x1<<1)  // Active high
#define ICTL_ICR1_CH8           (0x1<<2)
#define ICTL_ICR1_CH8_HL        (0x1<<2)
#define ICTL_ICR1_CH8_HL_AL     (0x0<<2)  // Active low
#define ICTL_ICR1_CH8_HL_AH     (0x1<<2)  // Active high

// Clock control registers
#define PCSR_CT0CS              (1<<3)
#define PCSR_CT1CS              (1<<4)
#define PCSR_CT2CS              (1<<5)
#define CT_X16                  16

#undef _TIMERS_TESTING
#ifdef _TIMERS_TESTING
static void aeb_setup_timer1(cyg_uint32 period);
#endif

static cyg_uint32 _period;

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
static cyg_interrupt abort_interrupt;
static cyg_handle_t  abort_interrupt_handle;

// This ISR is called only for the Abort button interrupt
static int
aeb_abort_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_hal_user_break((CYG_ADDRWORD*)regs);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT0);
    return 0;  // No need to run DSR
}
#endif

void hal_clock_initialize(cyg_uint32 period)
{
    cyg_uint32 iocr;

    // Set counter GATE input low (0) to halt counter while it's being setup
    HAL_READ_UINT32(CYG_DEVICE_IOCR, iocr);
    iocr = (iocr & ~IOCR_CT0G) | IOCR_CT0G_LOW;
    HAL_WRITE_UINT32(CYG_DEVICE_IOCR, iocr);

    // Scale timer0 clock
    HAL_WRITE_UINT32(CYG_DEVICE_CPM_CT0CCR, CT_X16);

    // Initialize counter, mode 2 = rate generator
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER_CTL, 
                    TIMER_CTL_TYPE_BIN|
                    TIMER_CTL_MODE_RG|
                    TIMER_CTL_RW_BOTH|
                    TIMER_CTL_SC_CTR0);
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER0, (period & 0xFF));        // LSB
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER0, ((period >> 8) & 0xFF)); // MSB
    // Enable timer
    iocr = (iocr & ~IOCR_CT0G) | IOCR_CT0G_HIGH;
    HAL_WRITE_UINT32(CYG_DEVICE_IOCR, iocr);
    _period = period;
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,
                             99,           // Priority
                             0,            // Data item passed to interrupt handler
                             aeb_abort_isr,
                             0,
                             &abort_interrupt_handle,
                             &abort_interrupt);
    cyg_drv_interrupt_attach(abort_interrupt_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif
#ifdef _TIMERS_TESTING
    aeb_setup_timer1(period/10);
#endif
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    _period = period;
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 value;
    cyg_uint8 reg;
    do {
        HAL_WRITE_UINT8(CYG_DEVICE_TIMER_CTL,
                        TIMER_CTL_RW_LATCH|TIMER_CTL_SC_CTR0);
        HAL_READ_UINT8(CYG_DEVICE_TIMER0, reg); // LSB
        value = reg;
        HAL_READ_UINT8(CYG_DEVICE_TIMER0, reg); // MSB
        value |= (reg << 8);
    } while (value <= 2);  // Hardware malfunction?
    *pvalue = _period - (value & 0xFFFF);   // Note: counter is only 16 bits
                                            // and decreases
}

void hal_hardware_init(void)
{
    // Any hardware/platform initialization that needs to be done.
    // Set all unknowns as edge triggered
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_ICR0,
                     ICTL_ICR0_CH0_HL_AL|ICTL_ICR0_CH0_EL_ET|
                     ICTL_ICR0_CH1_HL_AL|ICTL_ICR0_CH1_EL_ET|
                     ICTL_ICR0_CH2_HL_AL|ICTL_ICR0_CH2_EL_ET|
                     ICTL_ICR0_CH3_HL_AL|ICTL_ICR0_CH3_EL_ET|
                     ICTL_ICR0_CH4_HL_AL|ICTL_ICR0_CH4_EL_ET|
                     ICTL_ICR0_CH5_HL_AL|ICTL_ICR0_CH5_EL_ET);
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_ICR1,
                     ICTL_ICR1_CH6_HL_AL|
                     ICTL_ICR1_CH7_HL_AL|
                     ICTL_ICR1_CH8_HL_AL);
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_ICLR, 0xFFFF);  // CLear all interrupts
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_IRQER, 0x0000);  // All disabled
    // Clear and initialize cache
    HAL_UCACHE_INVALIDATE_ALL();
    HAL_UCACHE_ENABLE();

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
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IRQSR, irq_status);
    for (vector = 0;  vector < 16;  vector++) {
        if (irq_status & (1<<vector)) return vector;
    }
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IRQER, mask);
    old_mask = mask;
    mask &= ~(1<<vector);
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_IRQER, mask);
}

#if 0
void hal_interrupt_status(void)
{
    int irq_status, irq_enable, ipr_value, timer_value;
    cyg_uint8 reg;
    HAL_READ_UINT8(CYG_DEVICE_TIMER0, reg); // LSB
    timer_value = reg;
    HAL_READ_UINT8(CYG_DEVICE_TIMER0, reg); // MSB
    timer_value |= (reg << 8);
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IRQSR, irq_status);
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IRQER, irq_enable);
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IPR, ipr_value);
    diag_printf("Interrupt: IRQ: %x.%x.%x, Timer: %x\n", irq_status,
                irq_enable, ipr_value, timer_value);
}
#endif

void hal_interrupt_unmask(int vector)
{
    cyg_uint32 mask, old_mask;
    HAL_READ_UINT32(CYG_DEVICE_ICTL_IRQER, mask);
    old_mask = mask;
    mask |= (1<<vector);
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_IRQER, mask);
}

void hal_interrupt_acknowledge(int vector)
{
    HAL_WRITE_UINT32(CYG_DEVICE_ICTL_ICLR, (1<<vector));
}

void hal_interrupt_configure(int vector, int level, int up)
{
//    diag_printf("%s(%d,%d,%d)\n", __PRETTY_FUNCTION__, vector, level, up);
}

void hal_interrupt_set_level(int vector, int level)
{
//    diag_printf("%s(%d,%d)\n", __PRETTY_FUNCTION__, vector, level);
}

void hal_show_IRQ(int vector, int data, int handler)
{
    //    diag_printf("IRQ - vector: %x, data: %x, handler: %x\n", vector,
    //                  data, handler);
}

#ifdef _TIMERS_TESTING
#include <cyg/hal/drv_api.h>            // HAL ISR support
static cyg_interrupt timer1_interrupt;
static cyg_handle_t  timer1_interrupt_handle;
static cyg_uint32    timer1_count;

// This ISR is called only for the high speed timer under test
static int
aeb_timer1_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_TIMER1);
    timer1_count++;
    return 0;  // No need to run DSR
}

static void
aeb_setup_timer1(cyg_uint32 period)
{
    cyg_uint32 iocr;

    // Set counter GATE input low (0) to halt counter while it's being setup
    HAL_READ_UINT32(CYG_DEVICE_IOCR, iocr);
    iocr = (iocr & ~IOCR_CT1G) | IOCR_CT1G_LOW;
    HAL_WRITE_UINT32(CYG_DEVICE_IOCR, iocr);

    // Scale timer0 clock
    HAL_WRITE_UINT32(CYG_DEVICE_CPM_CT1CCR, CT_X16);

    // Initialize counter, mode 2 = rate generator
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER_CTL, 
                    TIMER_CTL_TYPE_BIN|
                    TIMER_CTL_MODE_RG|
                    TIMER_CTL_RW_BOTH|
                    TIMER_CTL_SC_CTR1);
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER1, (period & 0xFF));        // LSB
    HAL_WRITE_UINT8(CYG_DEVICE_TIMER1, ((period >> 8) & 0xFF)); // MSB
    // Enable timer
    iocr = (iocr & ~IOCR_CT1G) | IOCR_CT1G_HIGH;
    HAL_WRITE_UINT32(CYG_DEVICE_IOCR, iocr);
    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_TIMER1,
                             99,             // Priority
                             0,              // Data item passed to interrupt handler
                             aeb_timer1_isr,
                             0,
                             &timer1_interrupt_handle,
                             &timer1_interrupt);
    cyg_drv_interrupt_attach(timer1_interrupt_handle);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_TIMER1);
}
#endif

//-----------------------------------------------------------------------------
// Reset board (definitions from watchdog file aeb1.cxx)

// Register definitions
#define CYGARC_REG_WATCHDOG_BASE        0xFFFFAC00
#define CYGARC_REG_WATCHDOG_WDCTLR      (CYGARC_REG_WATCHDOG_BASE+0x30)
#define CYGARC_REG_WATCHDOG_WDCNTR      (CYGARC_REG_WATCHDOG_BASE+0x34)

// Control register bits
#define CYGARC_REG_WATCHDOG_WDCTLR_EN        0x01 // enable
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_NMF   0x00 // non-maskable fiq
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_ER    0x04 // external reset
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_SR    0x06 // system reset
#define CYGARC_REG_WATCHDOG_WDCTLR_FRZ       0x08 // lock enable bit
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_MASK  0x70 // time out period

#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_17    0x00 // 2^17
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_17_P  5242880 // = 5.2ms

#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_25    0x40 // 2^25
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_25_P  1342177300 // = 1.3421773s

void
hal_aeb_reset(void)
{
    // Clear the watchdog counter.
    HAL_WRITE_UINT32(CYGARC_REG_WATCHDOG_WDCNTR, 0);

    // Enable the watchdog with the smallest timeout.
    HAL_WRITE_UINT8(CYGARC_REG_WATCHDOG_WDCTLR, 
                    (CYGARC_REG_WATCHDOG_WDCTLR_TOP_17
                     | CYGARC_REG_WATCHDOG_WDCTLR_FRZ
                     | CYGARC_REG_WATCHDOG_WDCTLR_RSP_SR
                     | CYGARC_REG_WATCHDOG_WDCTLR_EN));

    // Wait for it...
    for(;;);
}

/*------------------------------------------------------------------------*/
// EOF hal_misc.c
