/*==========================================================================
//
//      hal_mk_defs.c
//
//      HAL (architecture) "make defs" program
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-02-20
// Purpose:      ARM architecture dependent definition generator
// Description:  This file contains code that can be compiled by the target
//               compiler and used to generate machine specific definitions
//               suitable for use in assembly code.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# include <cyg/kernel/instrmnt.h>
#endif
#include <cyg/hal/hal_if.h>

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
    DEFINE(armreg_r0, offsetof(HAL_SavedRegisters, d[HAL_THREAD_CONTEXT_R0]));
    DEFINE(armreg_r4, offsetof(HAL_SavedRegisters, d[HAL_THREAD_CONTEXT_R4]));
    DEFINE(armreg_r8, offsetof(HAL_SavedRegisters, d[HAL_THREAD_CONTEXT_R8]));
    DEFINE(armreg_r9, offsetof(HAL_SavedRegisters, d[HAL_THREAD_CONTEXT_R9]));
    DEFINE(armreg_r10, offsetof(HAL_SavedRegisters, d[HAL_THREAD_CONTEXT_R10]));
    DEFINE(armreg_sp, offsetof(HAL_SavedRegisters, sp));
    DEFINE(armreg_fp, offsetof(HAL_SavedRegisters, fp));
    DEFINE(armreg_ip, offsetof(HAL_SavedRegisters, ip));
    DEFINE(armreg_lr, offsetof(HAL_SavedRegisters, lr));
    DEFINE(armreg_pc, offsetof(HAL_SavedRegisters, pc));
    DEFINE(armreg_cpsr, offsetof(HAL_SavedRegisters, cpsr));
    DEFINE(armreg_vector, offsetof(HAL_SavedRegisters, vector));
    DEFINE(armreg_svclr, offsetof(HAL_SavedRegisters, svc_lr));
    DEFINE(armreg_svcsp, offsetof(HAL_SavedRegisters, svc_sp));
    DEFINE(ARMREG_SIZE, sizeof(HAL_SavedRegisters));
    DEFINE(CYGNUM_HAL_ISR_COUNT, CYGNUM_HAL_ISR_COUNT);
    DEFINE(CYGNUM_HAL_VSR_COUNT, CYGNUM_HAL_VSR_COUNT);
    DEFINE(CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION,
           CYGNUM_HAL_EXCEPTION_ILLEGAL_INSTRUCTION);
    DEFINE(CYGNUM_HAL_EXCEPTION_INTERRUPT, CYGNUM_HAL_EXCEPTION_INTERRUPT);
    DEFINE(CYGNUM_HAL_EXCEPTION_CODE_ACCESS,
           CYGNUM_HAL_EXCEPTION_CODE_ACCESS);
    DEFINE(CYGNUM_HAL_EXCEPTION_DATA_ACCESS,
           CYGNUM_HAL_EXCEPTION_DATA_ACCESS);
    DEFINE(CYGNUM_HAL_VECTOR_IRQ, CYGNUM_HAL_VECTOR_IRQ);
#ifdef CYGPKG_KERNEL
    DEFINE(RAISE_INTR, CYG_INSTRUMENT_CLASS_INTR|CYG_INSTRUMENT_EVENT_INTR_RAISE);
#endif
    DEFINE(CPSR_IRQ_DISABLE, CPSR_IRQ_DISABLE);
    DEFINE(CPSR_FIQ_DISABLE, CPSR_FIQ_DISABLE);
    DEFINE(CPSR_THUMB_ENABLE, CPSR_THUMB_ENABLE);
    DEFINE(CPSR_USER_MODE, CPSR_USER_MODE);
    DEFINE(CPSR_IRQ_MODE, CPSR_IRQ_MODE);
    DEFINE(CPSR_FIQ_MODE, CPSR_FIQ_MODE);
    DEFINE(CPSR_SUPERVISOR_MODE, CPSR_SUPERVISOR_MODE);
    DEFINE(CPSR_UNDEF_MODE, CPSR_UNDEF_MODE);
    DEFINE(CPSR_MODE_BITS, CPSR_MODE_BITS);
    DEFINE(CPSR_INITIAL, CPSR_INITIAL);
    DEFINE(CPSR_THREAD_INITIAL, CPSR_THREAD_INITIAL);
#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)
    DEFINE(CYGNUM_CALL_IF_TABLE_SIZE, CYGNUM_CALL_IF_TABLE_SIZE);
#endif
    DEFINE(CYGNUM_HAL_INTERRUPT_NONE, CYGNUM_HAL_INTERRUPT_NONE);

    return 0;
}


/*------------------------------------------------------------------------*/
// EOF hal_mk_defs.c
