//==========================================================================
//
//      plf_mk_defs.c
//
//      HAL (platform) "make defs" program
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
// Contributors: gthomas, jskov
// Date:         2000-05-11
// Purpose:      Platform dependent definition generator
// Description:  This file contains code that can be compiled by the target
//               compiler and used to generate machine specific definitions
//               suitable for use in assembly code.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header
#include <cyg/hal/hal_cache.h>          // HAL header
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/instrmnt.h>
#endif

/*
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * This technique was first used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 */

#define DEFINE(sym, val) \
        asm volatile("\n\t.equ\t" #sym ",%0" : : "i" (val))

int
main(void)
{
    // Some other exception related definitions
    DEFINE(CYGNUM_HAL_ISR_COUNT, CYGNUM_HAL_ISR_COUNT);
    DEFINE(CYGNUM_HAL_VSR_COUNT, CYGNUM_HAL_VSR_COUNT);
    DEFINE(CYGNUM_HAL_VECTOR_INTERRUPT, CYGNUM_HAL_VECTOR_INTERRUPT);
    DEFINE(CYGNUM_HAL_VECTOR_BREAKPOINT, CYGNUM_HAL_VECTOR_BREAKPOINT);

    // Interrupt details
    DEFINE(CYGNUM_HAL_INTERRUPT_INTC_V320USC_base, CYGNUM_HAL_INTERRUPT_INTC_V320USC_base);
    DEFINE(CYGNUM_HAL_INTERRUPT_INTC_PCI_base, CYGNUM_HAL_INTERRUPT_INTC_PCI_base);
    DEFINE(CYGNUM_HAL_INTERRUPT_INTC_IO_base, CYGNUM_HAL_INTERRUPT_INTC_IO_base);
    DEFINE(CYGARC_REG_INT_STAT, CYGARC_REG_INT_STAT);
    DEFINE(CYGARC_REG_PCI_STAT, CYGARC_REG_PCI_STAT);
    DEFINE(CYGARC_REG_IO_STAT, CYGARC_REG_IO_STAT);
    DEFINE(CYGARC_REG_PCI_MASK, CYGARC_REG_PCI_MASK);
    DEFINE(CYGARC_REG_IO_MASK, CYGARC_REG_IO_MASK);
    DEFINE(CYGARC_REG_INT_CFG0, CYGARC_REG_INT_CFG0);
    DEFINE(CYGARC_REG_INT_CFG3, CYGARC_REG_INT_CFG3);
#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)
    DEFINE(CYGNUM_HAL_INTERRUPT_CHAINING, CYGNUM_HAL_INTERRUPT_CHAINING);
#endif

    return 0;
}

//--------------------------------------------------------------------------
// EOF plf_mk_defs.c
