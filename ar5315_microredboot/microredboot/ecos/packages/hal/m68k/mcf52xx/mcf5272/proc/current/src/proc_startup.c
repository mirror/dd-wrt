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

#include <cyg/infra/cyg_type.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_startup.h>
#include <cyg/hal/hal_memmap.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

/*****************************************************************************
proc_reset --  Processor-specific reset vector initialization routine

     This routine must be called with interrupts disabled.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void proc_reset(void)
{
    int i;

    //   Set up the mapping of our internal registers.  The LSB indicates that
    // the registers are valid.

    mcf5272_wr_mbar((CYG_WORD32)(MCF5272_MBAR | 1));

    //   Initialize the vector base register in the interrupt controller.

    MCF5272_SIM->intc.ipvr = HAL_PROG_INT_VEC_BASE;

    //   Initialize the  interrupt  control  register  and  the  icr  priority
    // mirror.  Disable all interrupts by setting all priorities to zero.

    for (i=0; i < 4; i++)
    {
        MCF5272_SIM->intc.icr[i] = hal_icr_pri_mirror[i] = 0x88888888;
    }

    //   Enable/disable the data transfter acknowledge output pin.

    MCF5272_SIM->gpio.pbcnt = ((MCF5272_SIM->gpio.pbcnt &
                                ~(MCF5272_GPIO_PBCNT_TA_MSK)) |
                               ((HAL_MCF5272_ENABLE_DATA_TA) ?
                                (MCF5272_GPIO_PBCNT_TA_EN) :
                                (MCF5272_GPIO_PBCNT_TA_DE)));

    //   Do any platform-specific reset initialization.

    plf_reset();
}

