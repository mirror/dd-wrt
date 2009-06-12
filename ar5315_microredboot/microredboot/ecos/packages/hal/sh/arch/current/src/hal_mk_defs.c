//==========================================================================
//
//      hal_mk_defs.c
//
//      HAL (architecture) "make defs" program
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
// Date:         2000-02-21
// Purpose:      SH architecture dependent definition generator
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
    // setjmp/longjmp buffer
    DEFINE(CYGARC_JMPBUF_SP, offsetof(hal_jmp_buf_t, sp));
    DEFINE(CYGARC_JMPBUF_PR, offsetof(hal_jmp_buf_t, pr));
    DEFINE(CYGARC_JMPBUF_R8, offsetof(hal_jmp_buf_t, r8));
    DEFINE(CYGARC_JMPBUF_R9, offsetof(hal_jmp_buf_t, r9));
    DEFINE(CYGARC_JMPBUF_R10, offsetof(hal_jmp_buf_t, r10));
    DEFINE(CYGARC_JMPBUF_R11, offsetof(hal_jmp_buf_t, r11));
    DEFINE(CYGARC_JMPBUF_R12, offsetof(hal_jmp_buf_t, r12));
    DEFINE(CYGARC_JMPBUF_R13, offsetof(hal_jmp_buf_t, r13));
    DEFINE(CYGARC_JMPBUF_R14, offsetof(hal_jmp_buf_t, r14));

    // Exception/interrupt/context save buffer
    DEFINE(CYGARC_SHREG_REGS, offsetof(HAL_SavedRegisters, r[0]));
    DEFINE(CYGARC_SHREG_MACH, offsetof(HAL_SavedRegisters, mach));
    DEFINE(CYGARC_SHREG_MACL, offsetof(HAL_SavedRegisters, macl));
    DEFINE(CYGARC_SHREG_SP, offsetof(HAL_SavedRegisters, r[15]));
    DEFINE(CYGARC_SHREG_PR, offsetof(HAL_SavedRegisters, pr));
    DEFINE(CYGARC_SHREG_SR, offsetof(HAL_SavedRegisters, sr));
    DEFINE(CYGARC_SHREG_PC, offsetof(HAL_SavedRegisters, pc));
    DEFINE(CYGARC_SH_CONTEXT_SIZE, offsetof(HAL_SavedRegisters, context_size));

    // Below only saved on exceptions/interrupts
    DEFINE(CYGARC_SHREG_VBR, offsetof(HAL_SavedRegisters, vbr));
    DEFINE(CYGARC_SHREG_GBR, offsetof(HAL_SavedRegisters, gbr));
    DEFINE(CYGARC_SHREG_EVENT, offsetof(HAL_SavedRegisters, event));
    DEFINE(CYGARC_SH_EXCEPTION_SIZE, sizeof(HAL_SavedRegisters));

    DEFINE(CYGARC_SH_EXCEPTION_DECREMENT, sizeof(HAL_SavedRegisters));

    // Some other exception related definitions
    DEFINE(CYGNUM_HAL_ISR_COUNT, CYGNUM_HAL_ISR_COUNT);
    DEFINE(CYGNUM_HAL_VECTOR_INTERRUPT, CYGNUM_HAL_VECTOR_INTERRUPT);
    DEFINE(CYGNUM_HAL_VSR_MAX, CYGNUM_HAL_VSR_MAX);
    DEFINE(CYGNUM_HAL_VSR_COUNT, CYGNUM_HAL_VSR_COUNT);
    DEFINE(CYGNUM_HAL_VSR_EXCEPTION_COUNT, CYGNUM_HAL_VSR_EXCEPTION_COUNT);

    return 0;
}

//--------------------------------------------------------------------------
// EOF hal_mk_defs.c
