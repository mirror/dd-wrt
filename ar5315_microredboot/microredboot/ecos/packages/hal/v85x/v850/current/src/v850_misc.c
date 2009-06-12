//==========================================================================
//
//      v850_misc.c
//
//      HAL CPU variant miscellaneous functions
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
// Date:         2000-03-15
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/v850_common.h>

externC void cyg_hal_platform_hardware_init(void);

//
// Interrupt management functions
//

static volatile unsigned char *interrupt_control_registers[] = {
    CYG_HAL_V85X_INTERRUPT_CONTROL_REGISTERS   // Defined in <plf_intr.h>
};

#define INT_CONTROL_PENDING   0x80
#define INT_CONTROL_DISABLE   0x40
#define INT_CONTROL_LEVEL(n)  n

#define INT_CONTROL_DEFAULT INT_CONTROL_DISABLE|INT_CONTROL_LEVEL(7)

//
// Mask, i.e. disable, interrupt #vector from occurring
//
void 
hal_interrupt_mask(int vector)
{
    volatile unsigned char *ctl;
    CYG_ASSERT(vector >= CYGNUM_HAL_ISR_MIN, "invalid interrupt vector [< MIN]");
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX, "invalid interrupt vector [> MAX]");
    ctl = interrupt_control_registers[vector-CYGNUM_HAL_ISR_MIN];
    CYG_ASSERT((void *)ctl != 0, "invalid interrupt vector [not defined]");
    if (ctl ) {
        *ctl |= INT_CONTROL_DISABLE;
    }
}

//
// Unmask, i.e. enable, interrupt #vector
//
void 
hal_interrupt_unmask(int vector)
{
    volatile unsigned char *ctl;
    CYG_ASSERT(vector >= CYGNUM_HAL_ISR_MIN, "invalid interrupt vector [< MIN]");
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX, "invalid interrupt vector [> MAX]");
    ctl = interrupt_control_registers[vector-CYGNUM_HAL_ISR_MIN];
    CYG_ASSERT((void *)ctl != 0, "invalid interrupt vector [not defined]");
    if (ctl ) {
        *ctl &= ~INT_CONTROL_DISABLE;
    }
}

//
// Acknowledge, i.e. clear, interrupt #vector
//
void 
hal_interrupt_acknowledge(int vector)
{
    volatile unsigned char *ctl;
    CYG_ASSERT(vector >= CYGNUM_HAL_ISR_MIN, "invalid interrupt vector [< MIN]");
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX, "invalid interrupt vector [> MAX]");
    ctl = interrupt_control_registers[vector-CYGNUM_HAL_ISR_MIN];
    CYG_ASSERT((void *)ctl != 0, "invalid interrupt vector [not defined]");
    if (ctl ) {
        *ctl &= ~INT_CONTROL_PENDING;
    }
}

static void
init_interrupts(void)
{
    int i;
    volatile unsigned char *ctl;
    for (i = CYGNUM_HAL_ISR_MIN;  i <= CYGNUM_HAL_ISR_MAX;  i++) {
        ctl = interrupt_control_registers[i-CYGNUM_HAL_ISR_MIN];
        if (ctl) {
            *ctl = INT_CONTROL_DEFAULT;
        }
    }
}

//
// Initialize the hardware.  This may involve platform specific code.
//
void
cyg_hal_hardware_init(void)
{
    init_interrupts();
    cyg_hal_platform_hardware_init();
}

/*------------------------------------------------------------------------*/
/* End of v850_misc.c                                                      */
