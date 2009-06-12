//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Author(s):    nickg
// Contributors: nickg, jlarmour, jskov
// Date:         2000-05-15
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED

#include <cyg/hal/hal_if.h>             // Calling interface definitions
#include <cyg/hal/hal_misc.h>           // Helper functions

//--------------------------------------------------------------------------

#define CYGARC_REG_SUPERIO_IREG     0xb4000398
#define CYGARC_REG_SUPERIO_DREG     0xb4000399

#define CYGARC_REG_SUPERIO_FER      0x00 // function enable
#define CYGARC_REG_SUPERIO_FAR      0x01 // function address
#define CYGARC_REG_SUPERIO_PTR      0x02 // power and test
#define CYGARC_REG_SUPERIO_CLK      0x51 // clock controller


#define CYGARC_REG_SUPERIO_FER_PAR  0x01
#define CYGARC_REG_SUPERIO_FER_SCC1 0x02
#define CYGARC_REG_SUPERIO_FER_SCC2 0x04

#define CYGARC_REG_SUPERIO_CLK_14M  0x00 // 14MHz source
#define CYGARC_REG_SUPERIO_CLK_CME  0x04 // clock multiplier enabled

#define CYGARC_REG_SUPERIO_PTR_PPEXT 0x80 // extended




static void write_super_io(cyg_uint8 offset, cyg_uint8 data)
{
    HAL_WRITE_UINT8(CYGARC_REG_SUPERIO_IREG, offset);
    HAL_WRITE_UINT8(CYGARC_REG_SUPERIO_DREG, data);
    HAL_WRITE_UINT8(CYGARC_REG_SUPERIO_DREG, data);
}

void hal_platform_init(void)
{
    hal_if_init();

    // Configure the SuperIO chip
    write_super_io(CYGARC_REG_SUPERIO_FER, 
                   CYGARC_REG_SUPERIO_FER_SCC1|CYGARC_REG_SUPERIO_FER_SCC2);
    write_super_io(CYGARC_REG_SUPERIO_FAR, 0x10);
    write_super_io(CYGARC_REG_SUPERIO_CLK, 
                   CYGARC_REG_SUPERIO_CLK_14M|CYGARC_REG_SUPERIO_CLK_CME);

    // Set up VSC320 interrupt controller. INT1 must merge INT2 and
    // INT3 according to the platform specification.
    HAL_WRITE_UINT32(CYGARC_REG_INT_CFG1,
                     CYGARC_REG_INT_CFG_INT2|CYGARC_REG_INT_CFG_INT3);

    // Unmask vectors which are entry points for interrupt controllers
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_V320USC_INT0);
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_V320USC_INT1);
    HAL_INTERRUPT_UNMASK(CYGNUM_HAL_INTERRUPT_IO);
}

//--------------------------------------------------------------------------
// End of plf_misc.c
