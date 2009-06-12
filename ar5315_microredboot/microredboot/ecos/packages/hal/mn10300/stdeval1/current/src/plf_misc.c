//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
    // Note that the hardware seems to come up with the
    // caches containing random data. Hence they must be
    // invalidated before being enabled.
    
    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

#if defined(CYGPKG_KERNEL)                      && \
    defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)   && \
    defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
    {
        extern CYG_ADDRESS hal_virtual_vector_table[32];
        extern void patch_dbg_syscalls(void * vector);
        patch_dbg_syscalls( (void *)(&hal_virtual_vector_table[0]) );
    }
#endif    
}

/*------------------------------------------------------------------------*/
/* Functions to support the detection and execution of a user provoked    */
/* program break. These are usually called from interrupt routines.       */

cyg_bool cyg_hal_is_break(char *buf, int size)
{
    while( size )
        if( buf[--size] == 0x03 ) return true;

    return false;
}

void cyg_hal_user_break( CYG_ADDRWORD *regs )
{

#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)

        {
            extern CYG_ADDRESS hal_virtual_vector_table[64];        
            typedef void install_bpt_fn(void *pc);
            CYG_WORD32 retpc = ((CYG_WORD32 *)(&regs))[-1];
            CYG_WORD32 pc;
            HAL_SavedRegisters *sreg = (HAL_SavedRegisters *)regs;
            install_bpt_fn *ibp = (install_bpt_fn *)hal_virtual_vector_table[35];

            if( regs == NULL ) pc = retpc;
            else pc = sreg->pc;

            if( ibp != NULL ) ibp((void *)pc);
        }
    
#elif defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    {
        extern void breakpoint(void);
        breakpoint();
    }
    
#else

    HAL_BREAKPOINT(breakinst);

#endif
}

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

struct Hal_SavedRegisters *hal_saved_interrupt_state;

static void hal_ctrlc_isr_init(void)
{
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{

//    char c;

    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR ); 

    return 2;
}

#endif

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
