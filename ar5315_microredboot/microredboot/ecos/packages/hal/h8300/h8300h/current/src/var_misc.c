//==========================================================================
//
//      var_misc.c
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
// Author(s):    ysato
// Contributors: ysato
// Date:         2002-03-01
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/var_arch.h>
#include <cyg/hal/var_intr.h>
#include <cyg/hal/hal_io.h>

/*------------------------------------------------------------------------*/
/* Variant specific initialization routine.                               */

void hal_variant_init(void)
{
}

struct int_regs {
    CYG_BYTE *ier;
    CYG_BYTE *isr;
    CYG_BYTE mask;
    CYG_BYTE status;
};

#define REGS_DEF(ier,isr,mask,status) \
        {(CYG_BYTE *)ier,(CYG_BYTE *)isr,mask,status}

const struct int_regs interrupt_registers[]= {
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x01,0x01),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x02,0x02),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x04,0x04),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x08,0x08),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x10,0x10),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x20,0x20),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x40,0x40),
    REGS_DEF(CYGARC_IER,CYGARC_ISR,0x80,0x80),
    REGS_DEF(CYGARC_TCSR,CYGARC_TCSR,0x20,0x80),
    REGS_DEF(CYGARC_RTMCSR,CYGARC_RTMCSR,0x40,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_ADCSR,CYGARC_ADCSR,0x40,0x80),
    REGS_DEF(CYGARC_TISRA,CYGARC_TISRA,0x10,0x01),
    REGS_DEF(CYGARC_TISRB,CYGARC_TISRB,0x10,0x01),
    REGS_DEF(CYGARC_TISRC,CYGARC_TISRC,0x10,0x01),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TISRA,CYGARC_TISRA,0x20,0x02),
    REGS_DEF(CYGARC_TISRB,CYGARC_TISRB,0x20,0x02),
    REGS_DEF(CYGARC_TISRC,CYGARC_TISRC,0x20,0x02),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TISRA,CYGARC_TISRA,0x40,0x04),
    REGS_DEF(CYGARC_TISRB,CYGARC_TISRB,0x40,0x04),
    REGS_DEF(CYGARC_TISRC,CYGARC_TISRC,0x40,0x04),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x40,0x40),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x80,0x80),
    REGS_DEF(CYGARC_8TCR1,CYGARC_8TCSR1,0xC0,0xC0),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x20,0x20),
    REGS_DEF(CYGARC_8TCR2,CYGARC_8TCSR2,0x40,0x40),
    REGS_DEF(CYGARC_8TCR2,CYGARC_8TCSR2,0x80,0x80),
    REGS_DEF(CYGARC_8TCR3,CYGARC_8TCSR3,0xC0,0xC0),
    REGS_DEF(CYGARC_8TCR2,CYGARC_8TCSR2,0x20,0x20),
    REGS_DEF(CYGARC_DTCR0A,CYGARC_DTCR0A,0x08,0x80),
    REGS_DEF(CYGARC_DTCR0B,CYGARC_DTCR0B,0x08,0x80),
    REGS_DEF(CYGARC_DTCR1A,CYGARC_DTCR1A,0x08,0x80),
    REGS_DEF(CYGARC_DTCR1B,CYGARC_DTCR1B,0x08,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_SCR0,CYGARC_SSR0,0x40,0x30),
    REGS_DEF(CYGARC_SCR0,CYGARC_SSR0,0x40,0x40),
    REGS_DEF(CYGARC_SCR0,CYGARC_SSR0,0x80,0x80),
    REGS_DEF(CYGARC_SCR0,CYGARC_SSR0,0x04,0x04),
    REGS_DEF(CYGARC_SCR1,CYGARC_SSR1,0x40,0x30),
    REGS_DEF(CYGARC_SCR1,CYGARC_SSR1,0x40,0x40),
    REGS_DEF(CYGARC_SCR1,CYGARC_SSR1,0x80,0x80),
    REGS_DEF(CYGARC_SCR1,CYGARC_SSR1,0x04,0x04),
    REGS_DEF(CYGARC_SCR2,CYGARC_SSR2,0x40,0x30),
    REGS_DEF(CYGARC_SCR2,CYGARC_SSR2,0x40,0x40),
    REGS_DEF(CYGARC_SCR2,CYGARC_SSR2,0x80,0x80),
    REGS_DEF(CYGARC_SCR2,CYGARC_SSR2,0x04,0x04)
};

void
hal_interrupt_mask(int vector)					  
{
    CYG_BYTE ier;
    const struct int_regs *regs;
    if( vector < 12 )
        return;
    regs=&interrupt_registers[vector-12];
    if (vector == CYGNUM_HAL_INTERRUPT_WDT) {
        HAL_READ_UINT8(CYGARC_TCSR,ier);
	ier &= ~0x20;
	HAL_WRITE_UINT16(CYGARC_TCSR,0xa500 | ier);
    } else {
        if ((vector >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0) && regs->ier) {
            HAL_READ_UINT8(regs->ier,ier);
	    ier &= ~(regs->mask);
            HAL_WRITE_UINT8(regs->ier,ier);
	} else {
            CYG_FAIL("Unknown interrupt vector");                             
	}
    }
}

void
hal_interrupt_unmask(int vector)					  
{
    CYG_BYTE ier;
    const struct int_regs *regs;
    if( vector < 12 )
        return;
    regs=&interrupt_registers[vector-12];
    if (vector == CYGNUM_HAL_INTERRUPT_WDT) {
        HAL_READ_UINT8(CYGARC_TCSR,ier);
	ier |= 0x20;
	HAL_WRITE_UINT16(CYGARC_TCSR,0xa500 | ier);
    } else {
        if ((vector >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0) && regs->ier) {
            HAL_READ_UINT8(regs->ier,ier);
	    ier |= regs->mask;
            HAL_WRITE_UINT8(regs->ier,ier);
	} else {
            CYG_FAIL("Unknown interrupt vector");                             
	}
    }
}

void
hal_interrupt_acknowledge(int vector)					  
{
    CYG_BYTE isr;
    const struct int_regs *regs;
    if( vector < 12 )
        return;
    regs=&interrupt_registers[vector-12];    
    if (vector >= CYGNUM_HAL_INTERRUPT_DEND0A &&
        vector <= CYGNUM_HAL_INTERRUPT_DEND1B)
        return;
    if (vector == CYGNUM_HAL_INTERRUPT_WDT) {
        HAL_READ_UINT8(CYGARC_TCSR,isr);
	isr &= ~0x80;
	HAL_WRITE_UINT16(CYGARC_TCSR,0xa500 | isr);
    } else {
        if ((vector >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0) && regs->isr) {
            HAL_READ_UINT8(regs->isr,isr);
	    isr &= ~(regs->status);
            HAL_WRITE_UINT8(regs->isr,isr);
	} else {
            CYG_FAIL("Unknown interrupt vector");                             
	}
    }
}

const char priority_table[]={
   7, 6, 5, 5, 4, 4,-1,-1,
   3, 3,-1, 3, 2, 2, 2, 2,
   1, 1, 1, 1, 0, 0, 0, 0,
  15,15,15,15,14,14,14,14,
  13,13,13,13,-1,-1,-1,-1,
  11,11,11,11,10,10,10,10,
   9,9,9,9
};

CYG_BYTE cyg_hal_level_table[64];

void
hal_interrupt_set_level(int vector,int level)
{
    CYG_BYTE *ipr;
    CYG_BYTE ipr_mask;
    int priority;
    if( vector < 12 )
        return;
    priority = priority_table[vector-12];
    ipr = (CYG_BYTE *)CYGARC_IPRA + ((priority & 0xf8) >> 3);
    if (priority>=0) {
        ipr_mask = 1 << (priority & 0x07);
	if (level == 0) {
	    *ipr &= ~ipr_mask;
	    cyg_hal_level_table[vector] = 0x00;
	} else {
	    *ipr |= ipr_mask;
	    cyg_hal_level_table[vector] = 0x80;
	}
    } else {
        CYG_FAIL("Unknown interrupt vector");                             
    }
}

void
hal_interrupt_configure(int vector,int level,int up)
{
    cyg_uint8 iscr,mask;
    if (vector >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0 &&
        vector <= CYGNUM_HAL_INTERRUPT_EXTERNAL_7) {
	mask = 1 << (vector - CYGNUM_HAL_INTERRUPT_EXTERNAL_0);
        HAL_READ_UINT8(CYGARC_ISCR,iscr);
        if (level) {
	    iscr &= ~mask;
	}
        if (up) {
	    iscr |= mask;
	}
        CYG_ASSERT(!(up && level), "Cannot trigger on high level!"); 
        HAL_WRITE_UINT8(CYGARC_ISCR,iscr);
    }
}

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
