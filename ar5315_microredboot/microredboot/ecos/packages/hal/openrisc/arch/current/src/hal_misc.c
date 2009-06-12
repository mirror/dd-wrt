//==========================================================================
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
// Author(s):    nickg
// Contributors: nickg, jlarmour
// Date:         1999-01-21
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

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_if.h>             // hal_ctrlc_isr()

#include CYGHWR_MEMORY_LAYOUT_H

/*------------------------------------------------------------------------*/
/* If required, define a variable to store the clock period.              */

#ifdef CYGHWR_HAL_CLOCK_PERIOD_DEFINED

CYG_WORD32 cyg_hal_clock_period;

#endif

/*------------------------------------------------------------------------*/
/* First level C exception handler.                                       */

externC void __handle_exception (void);

externC HAL_SavedRegisters *_hal_registers;

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
externC void* volatile __mem_fault_handler;
#endif

externC cyg_uint8 cyg_hal_mips_process_fpe( HAL_SavedRegisters *regs );

externC cyg_uint32 cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    // If we caught an exception inside the stubs, see if we were expecting it
    // and if so jump to the saved address
    if (__mem_fault_handler) {
        regs->pc = (CYG_HAL_OPENRISC_REG)(signed long)__mem_fault_handler;
        return 0; // Caught an exception inside stubs        
    }

    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = regs;
    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && defined(CYGPKG_HAL_EXCEPTIONS)

    // We should decode the vector and pass a more appropriate
    // value as the second argument. For now we simply pass a
    // pointer to the saved registers. We should also divert
    // breakpoint and other debug vectors into the debug stubs.
    
    cyg_hal_deliver_exception( regs->vector, (CYG_ADDRWORD)regs );

#else
    
    CYG_FAIL("Exception!!!");
    
#endif    
    return 0;
}

/*------------------------------------------------------------------------*/
/* default ISR                                                            */

externC cyg_uint32
hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    return 0;
}

/*------------------------------------------------------------------------*/
// Come here if interrupt triggered, but no apparent cause
void hal_spurious_IRQ(HAL_SavedRegisters *regs) CYGBLD_ATTRIB_WEAK;
void
hal_spurious_IRQ(HAL_SavedRegisters *regs)
{
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
    cyg_hal_exception_handler(regs);
#else
    CYG_FAIL("Spurious interrupt!!");
#endif    
}
/*------------------------------------------------------------------------*/

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors(void)
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
    
} // cyg_hal_invoke_constructors()

/*------------------------------------------------------------------------*/
/* Determine the index of the ls bit of the supplied mask.                */

cyg_uint32 hal_lsbit_index(cyg_uint32 mask)
{
    cyg_uint32 n = mask;

    static const signed char tab[64] =
    { -1, 0, 1, 12, 2, 6, 0, 13, 3, 0, 7, 0, 0, 0, 0, 14, 10,
      4, 0, 0, 8, 0, 0, 25, 0, 0, 0, 0, 0, 21, 27 , 15, 31, 11,
      5, 0, 0, 0, 0, 0, 9, 0, 0, 24, 0, 0 , 20, 26, 30, 0, 0, 0,
      0, 23, 0, 19, 29, 0, 22, 18, 28, 17, 16, 0
    };

    n &= ~(n-1UL);
    n = (n<<16)-n;
    n = (n<<6)+n;
    n = (n<<4)+n;

    return tab[n>>26];
}

/*------------------------------------------------------------------------*/
/* Determine the index of the ms bit of the supplied mask.                */

cyg_uint32 hal_msbit_index(cyg_uint32 mask)
{
    cyg_uint32 x = mask;    
    cyg_uint32 w;

    /* Phase 1: make word with all ones from that one to the right */
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;

    /* Phase 2: calculate number of "1" bits in the word        */
    w = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w = w + (w >> 4);
    w = (w & 0x000F000F) + ((w >> 8) & 0x000F000F);
    return (cyg_uint32)((w + (w >> 16)) & 0xFF) - 1;

}

/*------------------------------------------------------------------------*/
/* Delay for some number of useconds.                                     */
void 
hal_delay_us(int us)
{
    cyg_uint32 val1, val2;
    int diff;
    long usticks;
    long ticks;

    // Calculate the number of counter register ticks per microsecond.
    
    usticks = (CYGNUM_HAL_RTC_PERIOD * CYGNUM_HAL_RTC_DENOMINATOR) / 1000000;

    // Make sure that the value is not zero.
    if( usticks == 0 ) usticks = 1;
    
    while( us > 0 )
    {
        int us1 = us;

        // Wait in bursts of less than 10000us to avoid any overflow
        // problems in the multiply.
        if( us1 > 10000 )
            us1 = 10000;

        us -= us1;

        ticks = us1 * usticks;

        HAL_CLOCK_READ(&val1);
        while (ticks > 0) {
            do {
                HAL_CLOCK_READ(&val2);
            } while (val1 == val2);
            diff = val2 - val1;
            if (diff < 0) diff += CYGNUM_HAL_RTC_PERIOD;
            ticks -= diff;
            val1 = val2;
        }
    }
}

/*------------------------------------------------------------------------*/

void hal_arch_program_new_stack(void *_func)
{
    externC void hal_program_new_stack( void *func, CYG_ADDRESS addr);
    hal_program_new_stack( (void *)_func,
                   (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS)) & ~7 );
}

/*------------------------------------------------------------------------*/
/* Idle thread action                                                     */

#include <cyg/infra/diag.h>

void hal_idle_thread_action( cyg_uint32 count )
{
}

/*------------------------------------------------------------------------*/
/* End of hal_misc.c                                                      */
