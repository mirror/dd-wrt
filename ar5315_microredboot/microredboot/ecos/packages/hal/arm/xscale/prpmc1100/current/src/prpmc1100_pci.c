//==========================================================================
//
//      prpmc1100_pci.c
//
//      HAL PCI board support code for Intel XScale PrPMC1100
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Date:         2003-03-27
// Purpose:      HAL PCI board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/
#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#ifdef CYGPKG_IO_PCI

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>


#define IXP425_PCI_MAX_DEV      4
#define IXP425_PCI_IRQ_LINES    4

// PCI pin mappings
#define PCI_CLK_GPIO     14  // CLK0
#define PCI_RESET_GPIO   12

#define PCI_INTA_GPIO    11
#define PCI_INTB_GPIO    10
#define PCI_INTC_GPIO     9
#define PCI_INTD_GPIO     8

#define INTA    CYGNUM_HAL_INTERRUPT_GPIO11
#define INTB    CYGNUM_HAL_INTERRUPT_GPIO10
#define INTC    CYGNUM_HAL_INTERRUPT_GPIO9
#define INTD    CYGNUM_HAL_INTERRUPT_GPIO8

void
cyg_hal_plf_pci_translate_interrupt(cyg_uint32 bus, cyg_uint32 devfn,
				    CYG_ADDRWORD *vec, cyg_bool *valid)
{
    *vec = INTA;
    *valid = true;
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
    cyg_uint8  next_bus;
    int  is_host = (*IXP425_PCI_CSR & PCI_CSR_HOST);

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

    if (is_host) {
	// PCI interrupt inputs
	HAL_GPIO_OUTPUT_DISABLE(PCI_INTA_GPIO);
	HAL_GPIO_OUTPUT_DISABLE(PCI_INTB_GPIO);
	HAL_GPIO_OUTPUT_DISABLE(PCI_INTC_GPIO);
	HAL_GPIO_OUTPUT_DISABLE(PCI_INTD_GPIO);

	// configure PCI interrupt lines for active low irq
	HAL_INTERRUPT_CONFIGURE(INTA, 1, 0);
	HAL_INTERRUPT_CONFIGURE(INTB, 1, 0);
	HAL_INTERRUPT_CONFIGURE(INTC, 1, 0);
	HAL_INTERRUPT_CONFIGURE(INTD, 1, 0);

    } else {
	// PCI interrupt outputs
	HAL_GPIO_OUTPUT_SET(PCI_INTA_GPIO);
	HAL_GPIO_OUTPUT_ENABLE(PCI_INTA_GPIO);
	HAL_GPIO_OUTPUT_SET(PCI_INTB_GPIO);
	HAL_GPIO_OUTPUT_ENABLE(PCI_INTB_GPIO);
	HAL_GPIO_OUTPUT_SET(PCI_INTC_GPIO);
	HAL_GPIO_OUTPUT_ENABLE(PCI_INTC_GPIO);
	HAL_GPIO_OUTPUT_SET(PCI_INTD_GPIO);
	HAL_GPIO_OUTPUT_ENABLE(PCI_INTD_GPIO);
    }

    HAL_DELAY_US(100);
#endif

    cyg_hal_plf_pci_init();

    if (is_host) {
	int delay = 200;  // Wait up to 20 seconds for EREADY
	while (delay-- > 0)
	    if (*PRPMC_CTL_REG & PRPMC_CTL_EREADY)
		break;
	    else
		hal_delay_us(100000);

	next_bus = 1;
	cyg_pci_configure_bus(0, &next_bus);
    } else
	*PRPMC_CTL_REG = PRPMC_CTL_RESETOUT | PRPMC_CTL_INTN_GPIO | PRPMC_CTL_EREADY;
}

#endif // CYGPKG_IO_PCI
