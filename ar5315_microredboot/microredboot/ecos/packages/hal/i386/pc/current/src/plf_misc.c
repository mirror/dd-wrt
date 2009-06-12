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
// Contributors: nickg, jlarmour, pjo
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

#include <cyg/hal/plf_misc.h>

#include <cyg/hal/hal_io.h>

#ifdef  CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
#include <cyg/hal/hal_if.h>
#endif

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#include <cyg/kernel/ktypes.h>
#include <cyg/kernel/kapi.h>
#endif /* CYGPKG_KERNEL */

/*------------------------------------------------------------------------*/

extern void patch_dbg_syscalls(void * vector);

extern void hal_pcmb_init(void);

#ifdef CYGPKG_HAL_SMP_SUPPORT
__externC void cyg_hal_smp_init(void);
#endif

//----------------------------------------------------------------------------
// ISR tables

volatile CYG_ADDRESS    hal_interrupt_handlers[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRWORD   hal_interrupt_data[CYGNUM_HAL_ISR_COUNT];
volatile CYG_ADDRESS    hal_interrupt_objects[CYGNUM_HAL_ISR_COUNT];

//-----------------------------------------------------------------------------
// IDT interrupt gate initialization

externC void cyg_hal_pc_set_idt_entry(CYG_ADDRESS routine,short *idtEntry)
{  
   
   idtEntry[0]=routine & 0xFFFF;
   idtEntry[1]=8;
   idtEntry[2]=0x8E00;
   idtEntry[3]=routine >> 16;
}

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
    int vector;

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();

    // Call motherboard init function
    hal_pcmb_init();
    
    // ISR table setup: plant the default ISR in all interrupt handlers
    // and the default interrupt VSR in the equivalent VSR table slots.
    for (vector = CYGNUM_HAL_ISR_MIN; vector <= CYGNUM_HAL_ISR_MAX; vector++)        
    {
        cyg_uint32 index;
        HAL_TRANSLATE_VECTOR( vector, index );
        hal_interrupt_handlers[index] = (CYG_ADDRESS) HAL_DEFAULT_ISR;
        HAL_VSR_SET( vector, &__default_interrupt_vsr, NULL );
    }
    
#if !defined(CYG_HAL_STARTUP_RAM)
    for (vector = CYGNUM_HAL_EXCEPTION_MIN;
         vector <= CYGNUM_HAL_EXCEPTION_MAX;
         vector++)        
    {
#if defined(CYGHWR_HAL_I386_FPU_SWITCH_LAZY)
        // If we are doing lazy FPU switching, the FPU switch VSR has
        // already been installed, so avoid overwriting it.
        if( vector != CYGNUM_HAL_VECTOR_NO_DEVICE )
#endif
        {
            HAL_VSR_SET( vector, &__default_exception_vsr, NULL );
        }
    }
#endif

#ifdef CYGPKG_REDBOOT

    // Start the timer device running if we are in a RedBoot
    // configuration. 
    
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
    
#endif    
    
    hal_if_init();

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT) || \
    defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
    {
        void hal_ctrlc_isr_init(void);
        hal_ctrlc_isr_init();
    }

#endif
        
#ifdef CYGPKG_HAL_SMP_SUPPORT

    cyg_hal_smp_init();
    
#endif
    
}

/*------------------------------------------------------------------------*/

void hal_pc_reset(void)
{
    /* Use Intel's IDT triple-fault trick. */
    asm("movl $badIdt, %eax\n"
        "lidt (%eax)\n"
        "int $3\n"
        "hlt\n"

        ".align 4\n"

        "badIdt:\n"
        ".word		0\n"
        ".long		0\n"
        ) ;
}

/*------------------------------------------------------------------------*/

void 
hal_delay_us(int us)
{
    while( us > 0 )
    {
        cyg_uint32 us1 = us;
        cyg_int32 ticks;
        cyg_uint32 cval1, cval2;

        // Wait in bursts of 1s to avoid overflow problems with the
        // multiply by 1000 below.
        
        if( us1 > 1000000 )
            us1 = 1000000;

        us -= us1;
        
        // The PC clock ticks at 838ns per tick. So we convert the us
        // value we were given to clock ticks and wait for that many
        // to pass.

        ticks = (us1 * 1000UL) / 838UL;

        HAL_CLOCK_READ( &cval1 );

        // We just loop, waiting for clock ticks to happen,
        // and subtracting them from ticks when they do.
        
        while( ticks > 0 )
        {
            cyg_int32 diff;
            HAL_CLOCK_READ( &cval2 );

            diff = cval2 - cval1;

            // Cope with counter wrap-around.
            if( diff < 0 )
                diff += CYGNUM_HAL_RTC_PERIOD;

            ticks -= diff;
            cval1 = cval2;

        }
    }
}


/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
