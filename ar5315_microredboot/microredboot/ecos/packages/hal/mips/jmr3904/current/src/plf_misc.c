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

extern void patch_dbg_syscalls(void * vector);

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

#if !defined(CYGHWR_HAL_MIPS_TX39_JMR3904_ENABLE_TOE)
    
    HAL_TX39_DEBUG_TOE_DISABLE();

#else

    HAL_TX39_DEBUG_TOE_ENABLE();

#endif

#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)   && \
    defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
    {
        patch_dbg_syscalls( (void *)(&hal_vsr_table[0]) );
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
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon)
    // The following code should be at the very start of this function so
    // that it can access the RA register before it is saved and reused.
    register CYG_WORD32 ra;
    asm volatile ( "move %0,$31;" : "=r" (ra) );

        {
            typedef void install_bpt_fn(void *epc);
            CYG_WORD32 pc;
            HAL_SavedRegisters *sreg = (HAL_SavedRegisters *)regs;
            install_bpt_fn *ibp = (install_bpt_fn *)hal_vsr_table[35];

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

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

#define DIAG_BASE       0xfffff300
#define DIAG_SLCR       (DIAG_BASE+0x00)
#define DIAG_SLSR       (DIAG_BASE+0x04)
#define DIAG_SLDICR     (DIAG_BASE+0x08)
#define DIAG_SLDISR     (DIAG_BASE+0x0C)
#define DIAG_SFCR       (DIAG_BASE+0x10)
#define DIAG_SBRG       (DIAG_BASE+0x14)
#define DIAG_TFIFO      (DIAG_BASE+0x20)
#define DIAG_RFIFO      (DIAG_BASE+0x30)

#define BRG_T0          0x0000
#define BRG_T2          0x0100
#define BRG_T4          0x0200
#define BRG_T5          0x0300

struct Hal_SavedRegisters *hal_saved_interrupt_state;

void hal_ctrlc_isr_init(void)
{
    CYG_WORD16 dicr;
    
    HAL_READ_UINT16( DIAG_SLDICR , dicr );
    dicr = 0x0001;
    HAL_WRITE_UINT16( DIAG_SLDICR , dicr );

    HAL_INTERRUPT_SET_LEVEL( CYGHWR_HAL_GDB_PORT_VECTOR, 1 );
    HAL_INTERRUPT_UNMASK( CYGHWR_HAL_GDB_PORT_VECTOR );
}

cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    char c;
    CYG_WORD16 disr;

    HAL_INTERRUPT_ACKNOWLEDGE( CYGHWR_HAL_GDB_PORT_VECTOR );
    
    HAL_READ_UINT16( DIAG_SLDISR , disr );

    if( disr & 0x0001 )
    {

        disr = disr & ~0x0001;
    
        HAL_READ_UINT8( DIAG_RFIFO, c );
    
        HAL_WRITE_UINT16( DIAG_SLDISR , disr );
        
        if( cyg_hal_is_break( &c , 1 ) )
            cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );
    }

    return 2;
}

#endif

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
