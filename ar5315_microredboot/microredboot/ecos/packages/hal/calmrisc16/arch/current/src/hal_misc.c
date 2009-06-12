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

#include <cyg/hal/hal_cache.h>          // Cache handling

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

externC cyg_uint32 cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
    int vec = regs->vector;

    // adjust PC after 'break' insns
    if (vec == CYGNUM_HAL_VECTOR_IRQ) {
	if (__read_prog_uint16((void *)(regs->spc_irq - 2)) == HAL_BREAKINST)
	    regs->spc_irq -= 2;
    }

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    // If we caught an exception inside the stubs, see if we were expecting it
    // and if so jump to the saved address
    if (__mem_fault_handler) {
	switch (vec) {
	  case CYGNUM_HAL_VECTOR_SWI:
	    regs->a[6] = (CYG_ADDRWORD)__mem_fault_handler;
	    break;
	  case CYGNUM_HAL_VECTOR_FIQ:
	    regs->spc_fiq = (CYG_ADDRWORD)__mem_fault_handler;
	    break;
	  default:
	    regs->spc_irq = (CYG_ADDRWORD)__mem_fault_handler;
	    break;
	}
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
    
    cyg_hal_deliver_exception( vec, (CYG_ADDRWORD)regs );

#else
    
    CYG_FAIL("Exception!!!");
    
#endif    
    return 0;
}

/*------------------------------------------------------------------------*/
/* default ISR                                                            */

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
externC cyg_uint32 hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
#if defined(CYGDBG_HAL_CALM32_DEBUG_GDB_CTRLC_SUPPORT) &&      \
    defined(CYGHWR_HAL_GDB_PORT_VECTOR) &&              \
    defined(HAL_CTRLC_ISR)

#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN    
    if( vector == CYGHWR_HAL_GDB_PORT_VECTOR )
#endif        
    {
        cyg_uint32 result = HAL_CTRLC_ISR( vector, data );
        if( result != 0 ) return result;
    }
    
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
#if defined(HAL_DIAG_IRQ_CHECK)
    {
        cyg_uint32 ret;
        /* let ROM monitor handle unexpected interrupts */
        HAL_DIAG_IRQ_CHECK(vector, ret);
        if (ret<=0)
            return ret;
    }
#endif // def HAL_DIAG_IRQ_CHECK
#endif // def CYGSEM_HAL_USE_ROM_MONITOR_CygMon
#endif
    
    CYG_TRACE1(true, "Interrupt: %d", vector);
    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}

#else // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

externC cyg_uint32 hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
#if defined(CYGDBG_HAL_CALM16_DEBUG_GDB_CTRLC_SUPPORT) &&      \
    defined(CYGHWR_HAL_GDB_PORT_VECTOR) &&              \
    defined(HAL_CTRLC_ISR)

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
#if defined(HAL_DIAG_IRQ_CHECK)
    {
        cyg_uint32 ret;
        /* let ROM monitor handle unexpected interrupts */
        HAL_DIAG_IRQ_CHECK(vector, ret);
        if (ret<=0)
            return ret;
    }
#endif // def HAL_DIAG_IRQ_CHECK
#endif // def CYGSEM_HAL_USE_ROM_MONITOR_CygMon
#endif

    return 0;
}

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

/*------------------------------------------------------------------------*/
/* data copy and bss zero functions                                       */

typedef void (CYG_SYM_ADDRESS)(void);

// All these must use this type of address to stop them being given relocations
// relative to $gp (i.e. assuming they would be in .sdata)
extern CYG_SYM_ADDRESS __ram_data_start;
extern CYG_SYM_ADDRESS __ram_data_end;
extern CYG_SYM_ADDRESS __rom_data_start;    

#ifdef CYG_HAL_STARTUP_ROM      
void hal_copy_data(void)
{
    short *p = (short *)&__ram_data_start;
    short *q = (short *)&__rom_data_start;
    short x;
    
    while( p != (short *)&__ram_data_end ) {
	asm volatile( "ldc %0,@%1\n" : "=r"(x) : "r"(q) );
        *p++ = x;
	q++;
    }
}
#endif

extern CYG_SYM_ADDRESS __bss_start;
extern CYG_SYM_ADDRESS __bss_end;

void hal_zero_bss(void)
{
    short *p = (short *)&__bss_start;

    while( p != (short *)&__bss_end )
        *p++ = 0;   
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
/* Idle thread action                                                     */

#include <cyg/infra/diag.h>

void hal_idle_thread_action( cyg_uint32 count )
{
}

/*------------------------------------------------------------------------*/
/* End of hal_misc.c                                                      */
