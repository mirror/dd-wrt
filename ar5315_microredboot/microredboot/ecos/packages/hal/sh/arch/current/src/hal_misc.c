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
// Copyright (C) 2003 Nick Garnett 
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
// Author(s):    jskov
// Contributors: jskov, jlarmour, nickg
// Date:         1999-04-03
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_cache.h>          // HAL cache
#include <cyg/hal/hal_intr.h>           // HAL interrupts/exceptions

#include <cyg/hal/sh_regs.h>            // timer registers

//---------------------------------------------------------------------------
// Functions used during initialization.

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
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
externC void* volatile __mem_fault_handler;
#endif

void
cyg_hal_exception_handler(HAL_SavedRegisters *regs)
{
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
    if (__mem_fault_handler && 
        ((regs->event >= _CYGNUM_HAL_VECTOR_FIRST_MEM_FAULT) &&
         (regs->event <= _CYGNUM_HAL_VECTOR_LAST_MEM_FAULT))) {
        regs->pc = (unsigned long)__mem_fault_handler;
        return; // Caught an exception inside stubs        
    }

    // Set the pointer to the registers of the current exception
    // context. At entry the GDB stub will expand the
    // HAL_SavedRegisters structure into a (bigger) register array.
    _hal_registers = regs;

    __handle_exception();

#elif defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT) && \
      defined(CYGPKG_HAL_EXCEPTIONS)

    cyg_hal_deliver_exception( regs->event, (CYG_ADDRWORD)regs );

#else

    CYG_FAIL("Exception!!!");
    
#endif    
    
    return;
}

//---------------------------------------------------------------------------
// Default ISR
externC cyg_uint32
hal_arch_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    return 0;
}

//--------------------------------------------------------------------------
// Determine the index of the ls bit of the supplied mask.

cyg_uint32
hal_lsbit_index(cyg_uint32 mask)
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

//--------------------------------------------------------------------------
// Determine the index of the ms bit of the supplied mask.

cyg_uint32
hal_msbit_index(cyg_uint32 mask)
{
    cyg_uint32 x = mask;    
    cyg_uint32 w;

    // Phase 1: make word with all ones from that one to the right
    x |= x >> 16;
    x |= x >> 8;
    x |= x >> 4;
    x |= x >> 2;
    x |= x >> 1;

    // Phase 2: calculate number of "1" bits in the word
    w = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
    w = w + (w >> 4);
    w = (w & 0x000F000F) + ((w >> 8) & 0x000F000F);
    return (cyg_uint32)((w + (w >> 16)) & 0xFF) - 1;

}

//---------------------------------------------------------------------------
// Idle thread action

void
hal_idle_thread_action( cyg_uint32 count )
{
}

#ifdef CYGHWR_SH_RTC_TIMER_IS_TMU
//---------------------------------------------------------------------------
// Low-level delay (in microseconds)

void
hal_delay_us(int usecs)
{
    unsigned char _tstr;  // Current clock control
    volatile unsigned char *tstr = (volatile unsigned char *)CYGARC_REG_TSTR;
    volatile unsigned long *tcnt = (volatile unsigned long *)CYGARC_REG_TCNT1;
    volatile unsigned long *tcor = (volatile unsigned long *)CYGARC_REG_TCOR1;
    volatile unsigned short *tcr = (volatile unsigned short *)CYGARC_REG_TCR1;
    unsigned long clocks_per_us = (((CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/1000000) +
                                    (CYGHWR_HAL_SH_TMU_PRESCALE_0-1))               /
                                   CYGHWR_HAL_SH_TMU_PRESCALE_0);
    volatile int diff, diff2;
    volatile cyg_uint32 val1, val2;

    *tcr = (( 4==CYGHWR_HAL_SH_TMU_PRESCALE_0) ? CYGARC_REG_TCR_TPSC_4 :
            (16==CYGHWR_HAL_SH_TMU_PRESCALE_0) ? CYGARC_REG_TCR_TPSC_16:
            (64==CYGHWR_HAL_SH_TMU_PRESCALE_0) ? CYGARC_REG_TCR_TPSC_64:
                                                 CYGARC_REG_TCR_TPSC_256);

    *tcnt = 0x0FFFFFFF;
    *tcor = 0x0FFFFFFF;
    
    _tstr = *tstr;
    *tstr |= CYGARC_REG_TSTR_STR1;  // Enable channel 1
    while (usecs-- > 0) {
        diff = 0;
        val1 = *tcnt;
        while (diff < clocks_per_us) {
            while ((val2 = *tcnt) == val1) ;
            diff2 = val1 - val2;
            if (diff2 < 0) diff2 += *tcor;
            diff += diff2;
            val1 = val2;
        }
    }
    *tstr = _tstr;                  // Restore timer to previous state
}
#endif

//---------------------------------------------------------------------------
// End of hal_misc.c
