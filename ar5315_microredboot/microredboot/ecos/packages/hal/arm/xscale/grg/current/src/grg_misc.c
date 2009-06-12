//==========================================================================
//
//      grg_misc.c
//
//      HAL misc board support code for Intel XScale GRG
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
// Date:         2003-02-05
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>     // base types
#include <cyg/infra/cyg_trac.h>     // tracing macros
#include <cyg/infra/cyg_ass.h>      // assertion macros

#include <cyg/hal/hal_io.h>         // IO macros
#include <cyg/hal/hal_arch.h>       // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>       // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_ixp425.h>     // Hardware definitions
#include <cyg/hal/grg.h>            // Platform specifics

#include <cyg/infra/diag.h>             // diag_printf

//
// Platform specific initialization
//

void
plf_hardware_init(void)
{
    // POWER_FAIL IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_PWR_FAIL_IRQ_N);   
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_PWR_FAIL, 1, 0);

    // DSL IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_DSL_IRQ_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_DSL, 1, 0);

    // SLIC_A IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_SLIC_A_IRQ_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_SLIC_A, 1, 0);

    // SLIC_B IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_SLIC_B_IRQ_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_SLIC_B, 1, 0);

    // DSP IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_DSP_IRQ_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_DSP, 1, 0);

    // IDE IRQ
    HAL_GPIO_OUTPUT_DISABLE(GPIO_IDE_IRQ_N);
    HAL_INTERRUPT_CONFIGURE(CYGNUM_HAL_INTERRUPT_IDE, 1, 0);

    // IO RESET_N  (DSP/SLICs)
    HAL_GPIO_OUTPUT_SET(GPIO_IO_RESET_N);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_IO_RESET_N);
    
    // SPI_CS1_N
    HAL_GPIO_OUTPUT_SET(GPIO_SPI_CS1_N);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_SPI_CS1_N);   // Eth PHY

    // SPI_CS0_N
    HAL_GPIO_OUTPUT_SET(GPIO_SPI_CS0_N);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_SPI_CS0_N);   // SLICs

    // SPI_SCK
    HAL_GPIO_OUTPUT_CLEAR(GPIO_SPI_SCK);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_SPI_SCK);

    // SPI_SDI
    HAL_GPIO_OUTPUT_CLEAR(GPIO_SPI_SDI);
    HAL_GPIO_OUTPUT_ENABLE(GPIO_SPI_SDI);

    // SPI_SDI
    HAL_GPIO_OUTPUT_DISABLE(GPIO_SPI_SDI);

#ifdef CYGPKG_IO_PCI
    extern void hal_plf_pci_init(void);
    hal_plf_pci_init();
#endif
}

// ------------------------------------------------------------------------
// EOF grg_misc.c
