//==========================================================================
//
//      devs/pcmcia/cerfpda/cerfpda_pcmcia.c
//
//      PCMCIA support (Card Services)
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
// Date:         2000-07-06
// Purpose:      PCMCIA support
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/io_pcmcia.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_intr.h>           // HAL interrupt macros
#include <cyg/hal/drv_api.h>

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>   // Configuration header
#include <cyg/kernel/kapi.h>
#endif
#include <cyg/hal/hal_if.h>

#include <cyg/io/pcmcia.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_sa11x0.h>  // Board definitions
#include <cyg/hal/cerfpda.h>

#ifdef CYGPKG_KERNEL
static cyg_interrupt cf_detect_interrupt;
static cyg_handle_t  cf_detect_interrupt_handle;
static cyg_interrupt cf_irq_interrupt;
static cyg_handle_t  cf_irq_interrupt_handle;

// This ISR is called when a CompactFlash board is inserted
static int
cf_detect_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_interrupt_mask(SA1110_CF_DETECT);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR handles the board insertion
static void
cf_detect_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;
    struct cf_slot *slot = (struct cf_slot *)data;
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        slot->state = CF_SLOT_STATE_Inserted;
    } else {
        slot->state = CF_SLOT_STATE_Removed;  // Implies powered up, etc.
    }
    cyg_interrupt_acknowledge(SA1110_CF_DETECT);
    cyg_interrupt_unmask(SA1110_CF_DETECT);
}

// This ISR is called when the card interrupt occurs
static int
cf_irq_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(SA1110_CF_IRQ);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR handles the card interrupt
static void
cf_irq_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    struct cf_slot *slot = (struct cf_slot *)data;
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    cyg_bool was_ctrlc_int;
#endif

    // Clear interrupt [edge indication]
    cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
    was_ctrlc_int = HAL_CTRLC_CHECK(vector, data);
    if (!was_ctrlc_int) // Fall through and run normal code
#endif
    // Process interrupt
    (slot->irq_handler.handler)(vector, count, slot->irq_handler.param);
    // Allow interrupts to happen again
    cyg_drv_interrupt_unmask(SA1110_CF_IRQ);
}
#endif

//
// Fill in the details of a PCMCIA slot and initialize the device
//
void
cf_hwr_init(struct cf_slot *slot)
{
    static int int_init = 0;
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;
    if (!int_init) {
        int_init = 1;
#ifdef CYGPKG_KERNEL
        // Set up interrupts
        cyg_drv_interrupt_create(SA1110_CF_DETECT,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t) slot,  //  Data item passed to interrupt handler
                                 (cyg_ISR_t *)cf_detect_isr,
                                 (cyg_DSR_t *)cf_detect_dsr,
                                 &cf_detect_interrupt_handle,
                                 &cf_detect_interrupt);
        cyg_drv_interrupt_attach(cf_detect_interrupt_handle);
        cyg_drv_interrupt_configure(SA1110_CF_DETECT, true, true);  // Detect either edge
        cyg_drv_interrupt_acknowledge(SA1110_CF_DETECT);
        cyg_drv_interrupt_unmask(SA1110_CF_DETECT);
        cyg_drv_interrupt_create(SA1110_CF_IRQ,
                                 99,                     // Priority - what goes here?
                                 (cyg_addrword_t) slot,  //  Data item passed to interrupt handler
                                 (cyg_ISR_t *)cf_irq_isr,
                                 (cyg_DSR_t *)cf_irq_dsr,
                                 &cf_irq_interrupt_handle,
                                 &cf_irq_interrupt);
        cyg_drv_interrupt_attach(cf_irq_interrupt_handle);
        cyg_drv_interrupt_unmask(SA1110_CF_IRQ);
#endif
        cyg_drv_interrupt_configure(SA1110_CF_IRQ, false, false);  // Falling edge
        cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
    }
    slot->attr = (unsigned char *)0x38000000;
    slot->attr_length = 0x200;
    slot->io = (unsigned char *)0x30000000;
    slot->io_length = 0x04000000;
    slot->mem = (unsigned char *)0x3C000000;
    slot->mem_length = 0x04000000;
    slot->int_num = SA1110_CF_IRQ;
#ifdef CYG_HAL_STARTUP_ROM
    // Disable CF bus & power (idle/off)
    cerfpda_BCR(SA1110_BCR_CF_POWER |
                SA1110_BCR_CF_RESET |
                SA1110_BCR_CF_BUS,
                SA1110_BCR_CF_POWER_OFF |
                SA1110_BCR_CF_RESET_DISABLE |
                SA1110_BCR_CF_BUS_OFF);
#endif
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        if ((_cerfpda_BCR & SA1110_BCR_CF_POWER) == SA1110_BCR_CF_POWER_ON) {
            // Assume that the ROM environment has turned the bus on
            slot->state = CF_SLOT_STATE_Ready;
        } else {
            slot->state = CF_SLOT_STATE_Inserted;
        }
    } else {
        slot->state = CF_SLOT_STATE_Empty;
    }
}

void
cf_hwr_poll(struct cf_slot *slot)
{
    unsigned long new_state = *SA11X0_GPIO_PIN_LEVEL;
    if ((new_state & SA1110_GPIO_CF_DETECT) == SA1110_GPIO_CF_PRESENT) {
        slot->state = CF_SLOT_STATE_Inserted;
    } else {
        slot->state = CF_SLOT_STATE_Empty;
    }
}

static void
do_delay(int ticks)
{
#ifdef CYGPKG_KERNEL
    cyg_thread_delay(ticks);
#else
    CYGACC_CALL_IF_DELAY_US(10000*ticks);
#endif
}

//
// Transition the card/slot to a new state
// note: currently only supports transitions to Ready, Empty
//
bool
cf_hwr_change_state(struct cf_slot *slot, int new_state)
{    
    int i, ptr, len;
    unsigned char buf[256];

    if (new_state == CF_SLOT_STATE_Ready) {
        if (slot->state == CF_SLOT_STATE_Inserted) {
            cerfpda_BCR(SA1110_BCR_CF_POWER |
                        SA1110_BCR_CF_RESET |
                        SA1110_BCR_CF_BUS,
                        SA1110_BCR_CF_POWER_ON |
                        SA1110_BCR_CF_RESET_DISABLE |
                        SA1110_BCR_CF_BUS_OFF);
            do_delay(30);  // At least 300 ms
            slot->state = CF_SLOT_STATE_Powered;
            cerfpda_BCR(SA1110_BCR_CF_POWER |
                        SA1110_BCR_CF_RESET |
                        SA1110_BCR_CF_BUS,
                        SA1110_BCR_CF_POWER_ON |
                        SA1110_BCR_CF_RESET_ENABLE |
                        SA1110_BCR_CF_BUS_ON);
            do_delay(1);  // At least 10 us
            slot->state = CF_SLOT_STATE_Reset;
            cerfpda_BCR(SA1110_BCR_CF_POWER |
                        SA1110_BCR_CF_RESET |
                        SA1110_BCR_CF_BUS,
                        SA1110_BCR_CF_POWER_ON |
                        SA1110_BCR_CF_RESET_DISABLE |
                        SA1110_BCR_CF_BUS_ON);
            do_delay(5); // At least 20 ms
            // Wait until the card is ready to talk
            for (i = 0;  i < 10;  i++) {
                ptr = 0;
                if (cf_get_CIS(slot, CF_CISTPL_VERS_1, buf, &len, &ptr)) {
                    slot->state = CF_SLOT_STATE_Ready;
                    break;
                }
                do_delay(10); 
            }
        }
    }
}

//
// Acknowledge interrupt (used in non-kernel environments)
//
void
cf_hwr_clear_interrupt(struct cf_slot *slot)
{
    // Clear interrupt [edge indication]
    cyg_drv_interrupt_acknowledge(SA1110_CF_IRQ);
}
