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

#include <cyg/hal/plf_z8530.h>

/*------------------------------------------------------------------------*/

#ifdef CYGSEM_HAL_MIPS_VR4300_VRC437X_DIAG_ACKS_INT_0

static cyg_uint32 hal_int0_count = 0;

static cyg_uint32 hal_int0_isr( cyg_uint32 vector, cyg_uint32 data )
{
    hal_int0_count++;
    HAL_INTERRUPT_ACKNOWLEDGE( CYGNUM_HAL_INTERRUPT_VRC437X );
    return 0;
}

#endif

/*------------------------------------------------------------------------*/


void hal_platform_init(void)
{
#ifdef CYGSEM_HAL_MIPS_VR4300_VRC437X_DIAG_ACKS_INT_0
    HAL_INTERRUPT_ATTACH( CYGNUM_HAL_INTERRUPT_VRC437X, hal_int0_isr, 0, 0 );
#endif
    
#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)
    // Set up eCos/ROM interfaces
    hal_if_init();
#endif    

#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)      && \
    (defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)    || \
     defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs))

    {
        extern CYG_ADDRESS hal_virtual_vector_table[32];
        void patch_dbg_syscalls( void * );
        patch_dbg_syscalls( (void *)(&hal_virtual_vector_table[0]) );
    }
#endif

}
/*------------------------------------------------------------------------*/
// One-time PCI initialization.

void cyg_hal_plf_pci_init(void)
{
    cyg_uint8  next_bus;

    // Configure PCI bus.
    next_bus = 1;
    cyg_pci_configure_bus(0, &next_bus);

}
    

/*------------------------------------------------------------------------*/
/* Functions to support the detection and execution of a user provoked    */
/* program break. These are usually called from interrupt routines.       */

#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT) && \
    defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

cyg_bool cyg_hal_is_break(char *buf, int size)
{
    while( size )
        if( buf[--size] == 0x03 ) return true;

    return false;
}

void cyg_hal_user_break( CYG_ADDRWORD *regs )
{
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs)
    // The following code should be at the very start of this function so
    // that it can access the RA register before it is saved and reused.
    register CYG_WORD32 ra;
    asm volatile ( "move %0,$31;" : "=r" (ra) );

        {
            extern CYG_ADDRESS hal_virtual_vector_table[64];        
            typedef void install_bpt_fn(void *epc);
            CYG_WORD32 pc;
            HAL_SavedRegisters *sreg = (HAL_SavedRegisters *)regs;
            install_bpt_fn *ibp = (install_bpt_fn *)hal_virtual_vector_table[35];

            if( regs == NULL ) pc = ra;
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

#endif

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if !defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT) && \
    defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

#if CYGHWR_HAL_MIPS_VR4300_VRC437X_GDB_PORT == 0
#define DUART_CHAN      DUART_A
#else
#define DUART_CHAN      DUART_B
#endif

struct Hal_SavedRegisters *hal_saved_interrupt_state;

void hal_ctrlc_isr_init(void)
{
    HAL_DUART_WRITE_CR( DUART_CHAN, 1, 0x10 );
    HAL_DUART_WRITE_CR( DUART_CHAN, 9, 0x0a );    
    HAL_INTERRUPT_SET_LEVEL( CYGHWR_HAL_GDB_PORT_VECTOR, 0 );
    HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR );
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{

    char c;
    cyg_uint8 rr0;

    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR ); 
    HAL_DUART_READ_CR(DUART_CHAN, 0, rr0 );

    // The following return value prevents a spurious interrupt report.
    // That is what should happen, but for some reason we get the odd extra
    // character when running in the test farm. 
    
    if ( (rr0 & 0x01) == 0 ) return 2;

    HAL_DUART_READ_RR( DUART_CHAN, c );

    if( cyg_hal_is_break( &c , 1 ) )
        cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );

    return 2;
}

#endif

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
