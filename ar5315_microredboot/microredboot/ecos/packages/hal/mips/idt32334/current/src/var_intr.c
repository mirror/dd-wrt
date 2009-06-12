//==========================================================================
//
//      var_intr.c
//
//      IDT3233x variant interrupt handlers
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
// Author(s):    tmichals
// Contributors: tmichals
// Date:         2002-09-20
// Purpose:      IDT 3233x variant interrupt handlers
// Description:  This file contains code to handle interrupt related issues
//               on the idt 3233x variant.
//
//####DESCRIPTIONEND####
//
//==========================================================================
#include <pkgconf/hal.h>
#include <pkgconf/system.h>

#include CYGBLD_HAL_PLATFORM_H
#include CYGHWR_MEMORY_LAYOUT_H

#include <cyg/infra/cyg_type.h>         // Base types

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_stub.h>           // Stub macros
#include <cyg/hal/hal_if.h>             // calling interface API
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>

/* This is the Reference board configuration */							
#include <cyg/hal/idt79rc233x.h>

#include <cyg/infra/diag.h>             // diag_printf
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
							 
							 
typedef cyg_uint32 cyg_ISR(cyg_uint32 vector, CYG_ADDRWORD data);

extern void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );

static inline cyg_uint32
hal_call_isr (cyg_uint32 vector)
{
    cyg_ISR *isr;
    CYG_ADDRWORD data;
    cyg_uint32 isr_ret;

    isr = (cyg_ISR*) hal_interrupt_handlers[vector];
    data = hal_interrupt_data[vector];

    isr_ret = (*isr) (vector, data);

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    if (isr_ret & CYG_ISR_CALL_DSR) {
        cyg_interrupt_post_dsr (hal_interrupt_objects[vector]);
    }
#endif

    return isr_ret & ~CYG_ISR_CALL_DSR;
}
							 
/* This is only for expansion interrupts */
								 
void cyg_hal_interrupt_mask ( cyg_uint32 vector )
{
    cyg_uint32 reg, _old;

    /* done to void race conditions */
    HAL_DISABLE_INTERRUPTS(_old);

    switch (vector)
        {
        case CYGNUM_HAL_INTERRUPT_SIO_0:
            HAL_WRITE_UINT32(INTR_COM0_REG, 0);
            HAL_READ_UINT32(INTR_MASK_REG, reg);
            HAL_WRITE_UINT32(INTR_MASK_REG, (reg & (~(1 << SERIAL_PORT0_GROUP))));
            break;

        case CYGNUM_HAL_INTERRUPT_SIO_1:
            HAL_WRITE_UINT32(INTR_COM1_REG, 0);
            HAL_READ_UINT32(INTR_MASK_REG, reg);
            HAL_WRITE_UINT32(INTR_MASK_REG, (reg & (~(1 << SERIAL_PORT1_GROUP))));
            break;

        default:
            HAL_INTERRUPT_MASK_CPU(vector);
        }

    HAL_RESTORE_INTERRUPTS(_old);
    return;
}

void cyg_hal_interrupt_unmask( cyg_uint32 vector)
{
    cyg_uint32 reg, _old;
    
    /* done to void race conditions */
    HAL_DISABLE_INTERRUPTS(_old);
    
    switch (vector)
        {
        case CYGNUM_HAL_INTERRUPT_SIO_0:
            HAL_WRITE_UINT32(INTR_COM0_REG, 1);
            HAL_READ_UINT32(INTR_MASK_REG, reg);
            HAL_WRITE_UINT32(INTR_MASK_REG, (reg | ((1 << SERIAL_PORT0_GROUP))));
            break;

        case CYGNUM_HAL_INTERRUPT_SIO_1:
            HAL_WRITE_UINT32(INTR_COM1_REG, 1);
            HAL_READ_UINT32(INTR_MASK_REG, reg);
            HAL_WRITE_UINT32(INTR_MASK_REG, (reg | ((1 << SERIAL_PORT1_GROUP))));
            break;

        default:
            HAL_INTERRUPT_UNMASK_CPU(vector);
	}

    HAL_RESTORE_INTERRUPTS(_old);
    return;
}

void cyg_hal_interrupt_acknowledge (cyg_uint32 vector)
{
    cyg_uint32 reg, _old;

    switch (vector)
	{
        case CYGNUM_HAL_INTERRUPT_SIO_0:
            HAL_WRITE_UINT32(INTR_CLEAR_COM0, 1);
            HAL_WRITE_UINT32(INTR_CLEAR_REG, (reg | ((1 << SERIAL_PORT0_GROUP))));
            break;

        case CYGNUM_HAL_INTERRUPT_SIO_1:
            HAL_WRITE_UINT32(INTR_CLEAR_COM1, 1);
            HAL_WRITE_UINT32(INTR_CLEAR_REG, (reg | ((1 << SERIAL_PORT1_GROUP))));
            break;

        default:
            HAL_INTERRUPT_ACKNOWLEDGE_CPU(vector);
	}
}

externC cyg_uint32
hal_extended_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isrRet;
    cyg_uint32 pendingIsr;
    cyg_uint32 isrNum;

    
    HAL_READ_UINT32 (INTR_STATUS_PTR, pendingIsr);

    // Although we could check 32-bits of the register, according
    // to the IDT32334 docs, only bits 1 to 14 are actually used.
    for (isrNum=1; isrNum <=14; isrNum++)
        if ( (1 << isrNum) & pendingIsr)
            break;

    if (pendingIsr) {
        isrRet = hal_call_isr (CYGNUM_LAST_IDT_INTERRUPT + isrNum);

        if (isrRet & CYG_ISR_HANDLED)
            return isrRet;
    }

    return 0;
}

externC void
hal_variant_IRQ_init(void)
{
    HAL_INTERRUPT_MASK (CYGNUM_EXPANSION);

    /* clear the mask register */
    HAL_WRITE_UINT32(INTR_MASK_REG, 0); /* all expansion interrupts are masked */

    HAL_INTERRUPT_ATTACH (CYGNUM_EXPANSION, &hal_extended_isr, 0, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_EXPANSION);
}
								 
externC void
hal_IRQ_init(void)
{
    int i;

    /* yes this is a quick fix; arch.inc should be changed, but this is
     * good enough for now */
    for (i=0; i <= CYGNUM_HAL_INTERRUPT_5;++i )
		HAL_INTERRUPT_MASK (i);

    hal_variant_IRQ_init();
}

// EOF var_intr.c
