/*==========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
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
// Author(s):    nickg, gthomas
// Contributors: nickg, gthomas, jlarmour
// Date:         2001-03-21
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_v85x.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header

/*------------------------------------------------------------------------*/
/* First level C exception handler.                                       */

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;

void
exception_handler(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
//    diag_printf("Exception! Frame: %x\n", regs);
//    show_regs(regs);
    static int gdb_active;
    if (gdb_active == 0) {
        gdb_active = 1;
        _hal_registers = regs;
        __handle_exception();
        gdb_active = 0;
    }

#elif defined(CYGPKG_KERNEL_EXCEPTIONS)

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    cyg_hal_deliver_exception( regs->vector, (CYG_ADDRWORD)regs );

#else

    CYG_FAIL("Exception!!!");
    
#endif    
    
    return;
}

/*------------------------------------------------------------------------*/
/* C++ support - run initial constructors                                 */

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors (void)
{

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];
    
    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--) {
        (*p) ();
    }
#endif
}

/*------------------------------------------------------------------------*/
/* default ISR                                                            */

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
externC cyg_uint32
hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    CYG_TRACE1(true, "Interrupt: %d", vector);

    diag_printf("Spurious Interrupt!!! - vector: %d, data: %x\n", vector, 
                data);
    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}
#else
externC cyg_uint32
hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    CYG_TRACE1(true, "Interrupt: %d", vector);

#ifndef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#ifdef CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT
#ifdef CYGDBG_HAL_CTRLC_ISR
    // then see if it is an incoming character interrupt and break
    // into the stub ROM if the char is a ^C.
    if ( CYGDBG_HAL_CTRLC_ISR( vector, data ) )
        return 1; // interrupt handled
#endif
#endif
#endif

    diag_printf("Spurious Interrupt!!! - vector: %d, data: %x\n", vector, 
                data);
    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}
#endif

/*------------------------------------------------------------------------*/
/* Idle thread action                                                     */

void
hal_idle_thread_action( cyg_uint32 count )
{
    // power saving instruction
    asm("halt");
}

/*-------------------------------------------------------------------------*/
/* Misc functions                                                          */

cyg_uint32
hal_lsbit_index(cyg_uint32 mask)
{
    int i;
    for (i = 0;  i < 32;  i++) {
      if (mask & (1<<i)) return ((cyg_uint32)i);
    }
    return ((cyg_uint32)-1);
}

cyg_uint32
hal_msbit_index(cyg_uint32 mask)
{
    int i;
    for (i = 31;  i >= 0;  i--) {
      if (mask & (1<<i)) return ((cyg_uint32)i);
    }
    return ((cyg_uint32)-1);
}

void
show_regs(HAL_SavedRegisters *regs)
{
    int i;
    diag_printf("Regs\n");
    for (i = 0;  i < 32;  i++) {
        if ((i % 8) == 0) {
            diag_printf("R%2d: ", i);
        }
        diag_printf("0x%08x ", regs->d[i]);
        if ((i % 8) == 7) {
            diag_printf("\n");
        }
    }
    diag_printf("PC = %x, PSW = %x\n", regs->pc, regs->psw);
}

/*------------------------------------------------------------------------*/
// EOF hal_misc.c
