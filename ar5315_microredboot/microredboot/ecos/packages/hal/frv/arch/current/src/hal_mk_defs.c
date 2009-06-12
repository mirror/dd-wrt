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
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
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
// Date:         2001-09-07
// Purpose:      FUJISTU architecture dependent definition generator
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
        asm volatile("\n.equ\t" #sym " %0" : : "i" (val))

int
main(void)
{
#ifdef CYGPKG_KERNEL
    DEFINE(RAISE_INTR, CYG_INSTRUMENT_CLASS_INTR|CYG_INSTRUMENT_EVENT_INTR_RAISE);
#endif
#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)
    DEFINE(CYGNUM_CALL_IF_TABLE_SIZE, CYGNUM_CALL_IF_TABLE_SIZE);
#endif
    DEFINE(CYGNUM_HAL_INTERRUPT_NONE, CYGNUM_HAL_INTERRUPT_NONE);
    DEFINE(CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE, CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE);
    DEFINE(CYGNUM_HAL_ISR_COUNT, CYGNUM_HAL_ISR_COUNT);
    DEFINE(CYGNUM_HAL_VECTOR_SYSCALL, CYGNUM_HAL_VECTOR_SYSCALL);
    DEFINE(CYGNUM_HAL_VECTOR_BREAKPOINT, CYGNUM_HAL_VECTOR_BREAKPOINT);
    DEFINE(CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1, CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_1);
    DEFINE(CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15, CYGNUM_HAL_VECTOR_EXTERNAL_INTERRUPT_LEVEL_15);

    // Register state
    DEFINE(_TS_GPR0, offsetof(HAL_SavedRegisters, gpr[0]));
    DEFINE(_TS_GPR1, offsetof(HAL_SavedRegisters, gpr[1]));
    DEFINE(_TS_SP, offsetof(HAL_SavedRegisters, gpr[1]));
    DEFINE(_TS_GPR2, offsetof(HAL_SavedRegisters, gpr[2]));
    DEFINE(_TS_GPR3, offsetof(HAL_SavedRegisters, gpr[3]));
    DEFINE(_TS_GPR4, offsetof(HAL_SavedRegisters, gpr[4]));
    DEFINE(_TS_GPR5, offsetof(HAL_SavedRegisters, gpr[5]));
    DEFINE(_TS_GPR6, offsetof(HAL_SavedRegisters, gpr[6]));
    DEFINE(_TS_GPR7, offsetof(HAL_SavedRegisters, gpr[7]));
    DEFINE(_TS_GPR8, offsetof(HAL_SavedRegisters, gpr[8]));
    DEFINE(_TS_GPR9, offsetof(HAL_SavedRegisters, gpr[9]));
    DEFINE(_TS_GPR10, offsetof(HAL_SavedRegisters, gpr[10]));
    DEFINE(_TS_GPR11, offsetof(HAL_SavedRegisters, gpr[11]));
    DEFINE(_TS_GPR12, offsetof(HAL_SavedRegisters, gpr[12]));
    DEFINE(_TS_GPR13, offsetof(HAL_SavedRegisters, gpr[13]));
    DEFINE(_TS_GPR14, offsetof(HAL_SavedRegisters, gpr[14]));
    DEFINE(_TS_GPR15, offsetof(HAL_SavedRegisters, gpr[15]));
    DEFINE(_TS_GPR16, offsetof(HAL_SavedRegisters, gpr[16]));
    DEFINE(_TS_GPR17, offsetof(HAL_SavedRegisters, gpr[17]));
    DEFINE(_TS_GPR18, offsetof(HAL_SavedRegisters, gpr[18]));
    DEFINE(_TS_GPR19, offsetof(HAL_SavedRegisters, gpr[19]));
    DEFINE(_TS_GPR20, offsetof(HAL_SavedRegisters, gpr[20]));
    DEFINE(_TS_GPR21, offsetof(HAL_SavedRegisters, gpr[21]));
    DEFINE(_TS_GPR22, offsetof(HAL_SavedRegisters, gpr[22]));
    DEFINE(_TS_GPR23, offsetof(HAL_SavedRegisters, gpr[23]));
    DEFINE(_TS_GPR24, offsetof(HAL_SavedRegisters, gpr[24]));
    DEFINE(_TS_GPR25, offsetof(HAL_SavedRegisters, gpr[25]));
    DEFINE(_TS_GPR26, offsetof(HAL_SavedRegisters, gpr[26]));
    DEFINE(_TS_GPR27, offsetof(HAL_SavedRegisters, gpr[27]));
    DEFINE(_TS_GPR28, offsetof(HAL_SavedRegisters, gpr[28]));
    DEFINE(_TS_GPR29, offsetof(HAL_SavedRegisters, gpr[29]));
    DEFINE(_TS_GPR30, offsetof(HAL_SavedRegisters, gpr[30]));
    DEFINE(_TS_GPR31, offsetof(HAL_SavedRegisters, gpr[31]));
#if _NGPR != 32
    DEFINE(_TS_GPR32, offsetof(HAL_SavedRegisters, gpr[32]));
    DEFINE(_TS_GPR33, offsetof(HAL_SavedRegisters, gpr[33]));
    DEFINE(_TS_GPR34, offsetof(HAL_SavedRegisters, gpr[34]));
    DEFINE(_TS_GPR35, offsetof(HAL_SavedRegisters, gpr[35]));
    DEFINE(_TS_GPR36, offsetof(HAL_SavedRegisters, gpr[36]));
    DEFINE(_TS_GPR37, offsetof(HAL_SavedRegisters, gpr[37]));
    DEFINE(_TS_GPR38, offsetof(HAL_SavedRegisters, gpr[38]));
    DEFINE(_TS_GPR39, offsetof(HAL_SavedRegisters, gpr[39]));
    DEFINE(_TS_GPR40, offsetof(HAL_SavedRegisters, gpr[40]));
    DEFINE(_TS_GPR41, offsetof(HAL_SavedRegisters, gpr[41]));
    DEFINE(_TS_GPR42, offsetof(HAL_SavedRegisters, gpr[42]));
    DEFINE(_TS_GPR43, offsetof(HAL_SavedRegisters, gpr[43]));
    DEFINE(_TS_GPR44, offsetof(HAL_SavedRegisters, gpr[44]));
    DEFINE(_TS_GPR45, offsetof(HAL_SavedRegisters, gpr[45]));
    DEFINE(_TS_GPR46, offsetof(HAL_SavedRegisters, gpr[46]));
    DEFINE(_TS_GPR47, offsetof(HAL_SavedRegisters, gpr[47]));
    DEFINE(_TS_GPR48, offsetof(HAL_SavedRegisters, gpr[48]));
    DEFINE(_TS_GPR49, offsetof(HAL_SavedRegisters, gpr[49]));
    DEFINE(_TS_GPR50, offsetof(HAL_SavedRegisters, gpr[50]));
    DEFINE(_TS_GPR51, offsetof(HAL_SavedRegisters, gpr[51]));
    DEFINE(_TS_GPR52, offsetof(HAL_SavedRegisters, gpr[52]));
    DEFINE(_TS_GPR53, offsetof(HAL_SavedRegisters, gpr[53]));
    DEFINE(_TS_GPR54, offsetof(HAL_SavedRegisters, gpr[54]));
    DEFINE(_TS_GPR55, offsetof(HAL_SavedRegisters, gpr[55]));
    DEFINE(_TS_GPR56, offsetof(HAL_SavedRegisters, gpr[56]));
    DEFINE(_TS_GPR57, offsetof(HAL_SavedRegisters, gpr[57]));
    DEFINE(_TS_GPR58, offsetof(HAL_SavedRegisters, gpr[58]));
    DEFINE(_TS_GPR59, offsetof(HAL_SavedRegisters, gpr[59]));
    DEFINE(_TS_GPR60, offsetof(HAL_SavedRegisters, gpr[60]));
    DEFINE(_TS_GPR61, offsetof(HAL_SavedRegisters, gpr[61]));
    DEFINE(_TS_GPR62, offsetof(HAL_SavedRegisters, gpr[62]));
    DEFINE(_TS_GPR63, offsetof(HAL_SavedRegisters, gpr[63]));
#endif
    DEFINE(_TS_PC,   offsetof(HAL_SavedRegisters, pc));
    DEFINE(_TS_PSR,  offsetof(HAL_SavedRegisters, psr));
    DEFINE(_TS_LR,   offsetof(HAL_SavedRegisters, lr));
    DEFINE(_TS_CCR,  offsetof(HAL_SavedRegisters, ccr));
    DEFINE(_TS_LCR,  offsetof(HAL_SavedRegisters, lcr));
    DEFINE(_TS_CCCR, offsetof(HAL_SavedRegisters, cccr));
    DEFINE(_TS_LR,   offsetof(HAL_SavedRegisters, lr));
    DEFINE(_TS_VECTOR, offsetof(HAL_SavedRegisters, vector));
#define _roundup(n,s) ((((n)+(s-1))/s)*s)
    DEFINE(_TS_size, _roundup(sizeof(HAL_SavedRegisters),8));

    DEFINE(_NGPR, _NGPR);
    DEFINE(_NFPR, _NFPR);

    DEFINE(_PSR_ET, _PSR_ET);
    DEFINE(_PSR_S, _PSR_S);
    DEFINE(_PSR_PS, _PSR_PS);
    DEFINE(_PSR_CM, _PSR_CM);
    DEFINE(_PSR_PIVL_MASK, _PSR_PIVL_MASK);
    DEFINE(_PSR_PIVL_SHIFT, _PSR_PIVL_SHIFT);

    DEFINE(_HSR0_ICE, _HSR0_ICE);
    DEFINE(_HSR0_DCE, _HSR0_DCE);
    DEFINE(_HSR0_IMMU, _HSR0_IMMU);
    DEFINE(_HSR0_DMMU, _HSR0_DMMU);

    return 0;
}


/*------------------------------------------------------------------------*/
// EOF hal_mk_defs.c
