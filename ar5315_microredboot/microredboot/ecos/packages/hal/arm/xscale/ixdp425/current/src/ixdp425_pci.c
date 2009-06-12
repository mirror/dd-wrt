//==========================================================================
//
//      ixdp425_pci.c
//
//      HAL PCI board support code for Intel XScale IXDP425
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
// Author(s):    msalter
// Contributors: msalter
// Date:         2002-12-11
// Purpose:      HAL PCI board support
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
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>

#ifdef CYGPKG_IO_PCI
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

#define IXP425_PCI_MAX_DEV      4
#define IXP425_PCI_IRQ_LINES    4

// PCI pin mappings
#define PCI_CLK_GPIO     14  // CLK0
#define PCI_RESET_GPIO   13
#define PCI_INTA_GPIO    11
#define PCI_INTB_GPIO    10
#define PCI_INTC_GPIO    9
#define PCI_INTD_GPIO    8

#define INTA    CYGNUM_HAL_INTERRUPT_GPIO11
#define INTB    CYGNUM_HAL_INTERRUPT_GPIO10
#define INTC    CYGNUM_HAL_INTERRUPT_GPIO9
#define INTD    CYGNUM_HAL_INTERRUPT_GPIO8

static const int pci_irq_table[IXP425_PCI_MAX_DEV][IXP425_PCI_IRQ_LINES] = {
    {INTA, INTB, INTC, INTD},
    {INTB, INTC, INTD, INTA},
    {INTC, INTD, INTA, INTB},
    {INTD, INTA, INTB, INTC}
};
    
void
cyg_hal_plf_pci_translate_interrupt(cyg_uint32 bus, cyg_uint32 devfn,
				    CYG_ADDRWORD *vec, cyg_bool *valid)
{
    cyg_uint8 pin;
    cyg_uint32 slot = 3 - (CYG_PCI_DEV_GET_DEV(devfn) - 18);
    
    HAL_PCI_CFG_READ_UINT8(bus, devfn, CYG_PCI_CFG_INT_PIN, pin);
    *vec = -1;
    *valid = false;

    if (slot < IXP425_PCI_MAX_DEV && pin <= IXP425_PCI_IRQ_LINES) {
	*vec = pci_irq_table[slot][pin-1];
        *valid = true;
    }
}


#define HAL_PCI_CLOCK_ENABLE() \
    *IXP425_GPCLKR |= GPCLKR_CLK0_ENABLE;  // GPIO(0) used for PCI clock

#define HAL_PCI_CLOCK_DISABLE() \
    *IXP425_GPCLKR &= ~GPCLKR_CLK0_ENABLE;  // GPIO(0) used for PCI clock

#define HAL_PCI_CLOCK_CONFIG() \
    *IXP425_GPCLKR |= GPCLKR_CLK0_PCLK2;

#define HAL_PCI_RESET_ASSERT() \
    HAL_GPIO_OUTPUT_CLEAR(PCI_RESET_GPIO);

#define HAL_PCI_RESET_DEASSERT() \
    HAL_GPIO_OUTPUT_SET(PCI_RESET_GPIO);

void
hal_plf_pci_init(void)
{
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
    HAL_PCI_RESET_ASSERT();
    HAL_PCI_CLOCK_DISABLE();

    // Set GPIO line direction
    HAL_GPIO_OUTPUT_ENABLE(PCI_CLK_GPIO);
    HAL_GPIO_OUTPUT_ENABLE(PCI_RESET_GPIO);
    HAL_GPIO_OUTPUT_DISABLE(PCI_INTA_GPIO);
    HAL_GPIO_OUTPUT_DISABLE(PCI_INTB_GPIO);
    HAL_GPIO_OUTPUT_DISABLE(PCI_INTC_GPIO);
    HAL_GPIO_OUTPUT_DISABLE(PCI_INTD_GPIO);

    // configure PCI interrupt lines for active low irq
    HAL_INTERRUPT_CONFIGURE(INTA, 1, 0);
    HAL_INTERRUPT_CONFIGURE(INTB, 1, 0);
    HAL_INTERRUPT_CONFIGURE(INTC, 1, 0);
    HAL_INTERRUPT_CONFIGURE(INTD, 1, 0);

    // wait 1ms to satisfy "minimum reset assertion time" of the PCI spec.
    HAL_DELAY_US(1000);
    HAL_PCI_CLOCK_CONFIG();
    HAL_PCI_CLOCK_ENABLE();

    // wait 100us to satisfy "minimum reset assertion time from clock stable" 
    // requirement of the PCI spec.
    HAL_DELAY_US(100);
    HAL_PCI_RESET_DEASSERT();
#endif
}

#endif // CYGPKG_IO_PCI
