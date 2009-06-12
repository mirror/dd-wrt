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
// Date:         2002-12-31
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
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x01,0x01),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x02,0x02),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x04,0x04),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x08,0x08),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x10,0x10),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x20,0x20),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x40,0x40),
    REGS_DEF(CYGARC_IERL,CYGARC_ISRL,0x80,0x80),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x01,0x01),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x02,0x02),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x04,0x04),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x08,0x08),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x10,0x10),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x20,0x20),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x40,0x40),
    REGS_DEF(CYGARC_IERH,CYGARC_ISRH,0x80,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TCSR,CYGARC_TCSR,0x20,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_REFCR,CYGARC_REFCR,0x40,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_ADCSR,CYGARC_ADCSR,0x40,0x80),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TIER0,CYGARC_TSR0,0x01,0x01),
    REGS_DEF(CYGARC_TIER0,CYGARC_TSR0,0x02,0x02),
    REGS_DEF(CYGARC_TIER0,CYGARC_TSR0,0x04,0x04),
    REGS_DEF(CYGARC_TIER0,CYGARC_TSR0,0x08,0x08),
    REGS_DEF(CYGARC_TIER0,CYGARC_TSR0,0x10,0x10),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TIER1,CYGARC_TSR1,0x01,0x01),
    REGS_DEF(CYGARC_TIER1,CYGARC_TSR1,0x02,0x02),
    REGS_DEF(CYGARC_TIER1,CYGARC_TSR1,0x10,0x10),
    REGS_DEF(CYGARC_TIER1,CYGARC_TSR1,0x20,0x20),
    REGS_DEF(CYGARC_TIER2,CYGARC_TSR2,0x01,0x01),
    REGS_DEF(CYGARC_TIER2,CYGARC_TSR2,0x02,0x02),
    REGS_DEF(CYGARC_TIER2,CYGARC_TSR2,0x10,0x10),
    REGS_DEF(CYGARC_TIER2,CYGARC_TSR2,0x20,0x20),
    REGS_DEF(CYGARC_TIER3,CYGARC_TSR3,0x01,0x01),
    REGS_DEF(CYGARC_TIER3,CYGARC_TSR3,0x02,0x02),
    REGS_DEF(CYGARC_TIER3,CYGARC_TSR3,0x04,0x04),
    REGS_DEF(CYGARC_TIER3,CYGARC_TSR3,0x08,0x08),
    REGS_DEF(CYGARC_TIER3,CYGARC_TSR3,0x10,0x10),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_TIER4,CYGARC_TSR4,0x01,0x01),
    REGS_DEF(CYGARC_TIER4,CYGARC_TSR4,0x02,0x02),
    REGS_DEF(CYGARC_TIER4,CYGARC_TSR4,0x10,0x10),
    REGS_DEF(CYGARC_TIER4,CYGARC_TSR4,0x20,0x20),
    REGS_DEF(CYGARC_TIER5,CYGARC_TSR5,0x01,0x01),
    REGS_DEF(CYGARC_TIER5,CYGARC_TSR5,0x02,0x02),
    REGS_DEF(CYGARC_TIER5,CYGARC_TSR5,0x10,0x10),
    REGS_DEF(CYGARC_TIER5,CYGARC_TSR5,0x20,0x20),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x40,0x40),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x80,0x80),
    REGS_DEF(CYGARC_8TCR0,CYGARC_8TCSR0,0x20,0x20),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_8TCR1,CYGARC_8TCSR1,0x40,0x40),
    REGS_DEF(CYGARC_8TCR1,CYGARC_8TCSR1,0x80,0x80),
    REGS_DEF(CYGARC_8TCR1,CYGARC_8TCSR1,0x20,0x20),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(CYGARC_DMABCRL,CYGARC_DMABCRL,0x01,0x10),
    REGS_DEF(CYGARC_DMABCRL,CYGARC_DMABCRL,0x02,0x20),
    REGS_DEF(CYGARC_DMABCRL,CYGARC_DMABCRL,0x04,0x40),
    REGS_DEF(CYGARC_DMABCRL,CYGARC_DMABCRL,0x08,0x80),
    REGS_DEF(CYGARC_EDMDR0L,CYGARC_EDMDR0L,0x80,0x40),
    REGS_DEF(CYGARC_EDMDR1L,CYGARC_EDMDR1L,0x80,0x40),
    REGS_DEF(CYGARC_EDMDR2L,CYGARC_EDMDR2L,0x80,0x40),
    REGS_DEF(CYGARC_EDMDR3L,CYGARC_EDMDR3L,0x80,0x40),
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
    REGS_DEF(CYGARC_SCR2,CYGARC_SSR2,0x04,0x04),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
    REGS_DEF(NULL,NULL,0,0),
};

void
hal_interrupt_mask(int vector)					  
{
    CYG_BYTE ier;
    const struct int_regs *regs=&interrupt_registers[vector-CYGNUM_HAL_INTERRUPT_EXTERNAL_0];
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
    const struct int_regs *regs=&interrupt_registers[vector-CYGNUM_HAL_INTERRUPT_EXTERNAL_0];
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
    const struct int_regs *regs=&interrupt_registers[vector-CYGNUM_HAL_INTERRUPT_EXTERNAL_0];
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

void
hal_interrupt_set_level(int vector,int level)
{
    CYG_WORD *ipr;
    CYG_WORD ipr_mask;
    int offset;
    if ((vector < CYGNUM_HAL_INTERRUPT_EXTERNAL_0) || 
        (vector > CYGNUM_HAL_INTERRUPT_EXTERNAL_15)) {
        return;
    } else {
	vector -= 16;
	ipr = (CYG_WORD *)CYGARC_IPRA + vector/4;
	offset = (3 - (vector & 3)) * 4;
        ipr_mask = 0x0f << offset;
	*ipr &= ~ipr_mask;
	*ipr |= (level & 0x07) << offset;
    }
}

void
hal_interrupt_configure(int vector,int level,int up)
{
    CYG_WORD iscr,mask;
    CYG_WORD *iscr_ptr;
    int conf=1;
    if (level) conf = 0;
    if (up) conf = 2;
    CYG_ASSERT(!(up && level), "Cannot trigger on high level!"); 
    if (vector >= CYGNUM_HAL_INTERRUPT_EXTERNAL_0 &&
        vector <= CYGNUM_HAL_INTERRUPT_EXTERNAL_15) {
	iscr_ptr = (vector <= CYGNUM_HAL_INTERRUPT_EXTERNAL_7)?CYGARC_ISCRL:CYGARC_ISCRH;
	mask = 3 << ((vector - CYGNUM_HAL_INTERRUPT_EXTERNAL_0) & 7) * 2;
        HAL_READ_UINT16(iscr_ptr,iscr);
	iscr &= ~mask;
	iscr |= conf << ((vector - CYGNUM_HAL_INTERRUPT_EXTERNAL_0) & 7) * 2;
        HAL_WRITE_UINT16(iscr_ptr,iscr);
    } else {
        CYG_FAIL("Unhandled interrupt vector");
    }
}

/*------------------------------------------------------------------------*/
/* End of var_misc.c                                                      */
