//===========================================================================
//
//      hal_misc.c
//
//      HAL miscellaneous functions
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: proven, pjo
// Date:        1999-02-20
// Purpose:     HAL miscellaneous functions
// Description: This file contains miscellaneous functions provided by the
//              HAL.
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>      // diag_printf

#include <cyg/hal/hal_arch.h>

#include <cyg/hal/hal_if.h>

//---------------------------------------------------------------------------

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

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

//---------------------------------------------------------------------------
// First level C exception handler.

externC void __handle_exception (void);
externC HAL_SavedRegisters *_hal_registers;

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
externC void* volatile __mem_fault_handler;
#endif

void
cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    // If we caught an exception inside the stubs, see if we were expecting it
    // and if so jump to the saved address
    if (__mem_fault_handler) {
        regs->pc = (CYG_ADDRWORD)__mem_fault_handler;
        return; // Caught an exception inside stubs        
    }

    _hal_registers = regs;
    __handle_exception();
    
#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && defined(CYGPKG_HAL_EXCEPTIONS)
    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.

    cyg_hal_deliver_exception( regs->vector>>8, (CYG_ADDRWORD)regs );

#else
    CYG_FAIL("Exception!!!");
#endif    
    return;
}

/*------------------------------------------------------------------------*/
/* default architecture ISR                                               */
/* The real default ISR is in hal/common/.../src/hal_misc.c               */

externC cyg_uint32 hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    // For some reason I seem to be geting spurious interrupts on
    // interrupt 0x27(39). This is the parallel port. This is masked
    // in the PIC so the exact reason for these is a mystery. They do
    // appear to be real interrupts since they always appear just
    // after a clear of the interrupt flag, and the stack shows a
    // proper interrupt context being pushed by the hardware. This may
    // be some feature of the Celeron CPU I am using. 
    
    if( vector == 0x27 )
        return 2;

    diag_printf("hal_arch_default_isr: %d (data: %d)\n", vector,data);
    CYG_FAIL("Spurious Interrupt!!!");    
    
    return 0;
}

//---------------------------------------------------------------------------
// End of hal_misc.c
