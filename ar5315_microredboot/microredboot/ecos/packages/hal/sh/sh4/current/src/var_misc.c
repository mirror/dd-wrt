//==========================================================================
//
//      var_misc.c
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

//---------------------------------------------------------------------------
// Initial cache enabling

#ifdef CYGHWR_HAL_SH_CACHE_MODE_P0_WRITE_BACK
# define CACHE_MODE_P0 0
#else
# define CACHE_MODE_P0 CYGARC_REG_CCR_WT
#endif

#ifdef CYGHWR_HAL_SH_CACHE_MODE_P1_WRITE_BACK
# define CACHE_MODE_P1 CYGARC_REG_CCR_CB
#else
# define CACHE_MODE_P1 0
#endif

externC void
cyg_var_enable_caches(void)
{
    // If relying on a ROM monitor do not invalidate the caches as the
    // ROM monitor may have (non-synced) state in the caches.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)
    // Initialize cache.
    HAL_DCACHE_INVALIDATE_ALL();    
    HAL_ICACHE_INVALIDATE_ALL();    

    // Set cache modes
    HAL_DCACHE_WRITE_MODE_SH(CACHE_MODE_P0|CACHE_MODE_P1);
#endif
#ifdef CYGHWR_HAL_SH_CACHE_ENABLE
    // Enable cache.
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_ENABLE();
#endif
}

//---------------------------------------------------------------------------
void
hal_variant_init(void)
{
#ifdef CYGHWR_HAL_SH_FPU
    __externC void __set_fpscr(cyg_uint32 __fpscr);
    // Call __set_fpscr to allow GCC (in libgcc) to update its internal
    // state for the floating point mode.
    __set_fpscr(CYG_FPSCR);
#endif
}

//---------------------------------------------------------------------------

externC cyg_uint8 cyg_hal_ILVL_table[];
externC cyg_uint8 cyg_hal_IMASK_table[];

static void
hal_interrupt_update_level(int vector)
{
    cyg_uint16 iprX;                                                     
    int level;

    level = cyg_hal_IMASK_table[vector] ? cyg_hal_ILVL_table[vector] : 0;

    switch( (vector) ) {
    /* IPRA */
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_TMU0_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_TMU0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_TMU1_TUNI1:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_TMU1_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_TMU1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_TMU2_TUNI2:
    case CYGNUM_HAL_INTERRUPT_TMU2_TICPI2:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_TMU2_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_TMU2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_RTC_ATI:
    case CYGNUM_HAL_INTERRUPT_RTC_PRI:
    case CYGNUM_HAL_INTERRUPT_RTC_CUI:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_RTC_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_RTC_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;

    /* IPRB */
    case CYGNUM_HAL_INTERRUPT_SCI_ERI:
    case CYGNUM_HAL_INTERRUPT_SCI_RXI:
    case CYGNUM_HAL_INTERRUPT_SCI_TXI:
    case CYGNUM_HAL_INTERRUPT_SCI_TEI:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_SCI_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_SCI_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_WDT_ITI:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_WDT_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_WDT_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_REF_RCMI:
    case CYGNUM_HAL_INTERRUPT_REF_ROVI:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_REF_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_REF_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;

    /* IPRC */
    case CYGNUM_HAL_INTERRUPT_HUDI:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_HUDI_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_HUDI_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_GPIO:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_GPIO_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_GPIO_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_DMAC_DMTE0:
    case CYGNUM_HAL_INTERRUPT_DMAC_DMTE1:
    case CYGNUM_HAL_INTERRUPT_DMAC_DMTE2:
    case CYGNUM_HAL_INTERRUPT_DMAC_DMTE3:
    case CYGNUM_HAL_INTERRUPT_DMAC_DMAE:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_DMAC_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_DMAC_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SCIF_ERI:
    case CYGNUM_HAL_INTERRUPT_SCIF_RXI:
    case CYGNUM_HAL_INTERRUPT_SCIF_BRI:
    case CYGNUM_HAL_INTERRUPT_SCIF_TXI:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_SCIF_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_SCIF_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
#if (CYGARC_SH_MOD_INTC == 2)
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
    case CYGNUM_HAL_INTERRUPT_IRL0:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_IRL0_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_IRL0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRL1:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_IRL1_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_IRL1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRL2:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_IRL2_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_IRL2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRL3:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_IRL3_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_IRL3_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
#endif
    case CYGNUM_HAL_INTERRUPT_PCISERR:
    {
        cyg_uint32 intpri;
        HAL_READ_UINT32(CYGARC_REG_INTPRI00, intpri);
        intpri &= ~CYGARC_REG_INTPRI00_PCISERR_MASK;
        intpri |= (level)*CYGARC_REG_INTPRI00_PCISERR_PRI1;
        HAL_WRITE_UINT32(CYGARC_REG_INTPRI00, intpri);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_PCIDMA3 ... CYGNUM_HAL_INTERRUPT_PCIERR:
    {
        cyg_uint32 intpri;
        HAL_READ_UINT32(CYGARC_REG_INTPRI00, intpri);
        intpri &= ~CYGARC_REG_INTPRI00_PCIERR_MASK;
        intpri |= (level)*CYGARC_REG_INTPRI00_PCIERR_PRI1;
        HAL_WRITE_UINT32(CYGARC_REG_INTPRI00, intpri);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_TUNI3:
    {
        cyg_uint32 intpri;
        HAL_READ_UINT32(CYGARC_REG_INTPRI00, intpri);
        intpri &= ~CYGARC_REG_INTPRI00_TUNI3_MASK;
        intpri |= (level)*CYGARC_REG_INTPRI00_TUNI3_PRI1;
        HAL_WRITE_UINT32(CYGARC_REG_INTPRI00, intpri);
        break;
    }
    case CYGNUM_HAL_INTERRUPT_TUNI4:
    {
        cyg_uint32 intpri;
        HAL_READ_UINT32(CYGARC_REG_INTPRI00, intpri);
        intpri &= ~CYGARC_REG_INTPRI00_TUNI4_MASK;
        intpri |= (level)*CYGARC_REG_INTPRI00_TUNI4_PRI1;
        HAL_WRITE_UINT32(CYGARC_REG_INTPRI00, intpri);
        break;
    }
#endif // CYGARC_SH_MOD_INTC
    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_5C0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_5E0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_6E0:
        /* Do nothing for these reserved vectors. */
        break;

    // Platform extensions
    CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vector, level)

    default:
        CYG_FAIL("Unknown interrupt vector");                             
        break;
    }
}

void
hal_interrupt_set_level(int vector, int level)
{
    CYG_ASSERT((0 <= (level) && 15 >= (level)), "Illegal level");
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= (vector)
                && CYGNUM_HAL_ISR_MAX >= (vector)), "Illegal vector");

    cyg_hal_ILVL_table[vector] = level;

    hal_interrupt_update_level(vector);
}

void
hal_interrupt_mask(int vector)                                    
{
#ifdef CYGPKG_HAL_SH_7751
    if (vector >= CYGNUM_HAL_INTERRUPT_PCISERR && vector <= CYGNUM_HAL_INTERRUPT_TUNI4) {
        HAL_WRITE_UINT32(CYGARC_REG_INTMSK00, 1 << (vector - CYGNUM_HAL_INTERRUPT_PCISERR));
        return;
    }
#endif

    switch( vector ) {
    case CYGNUM_HAL_INTERRUPT_NMI:
        break;
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        // fall through
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0 ... CYGNUM_HAL_ISR_MAX:
        cyg_hal_IMASK_table[vector] = 0;
        hal_interrupt_update_level(vector);
        break;
    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:
        /* Do nothing for these reserved vectors. */
        break;
    default:
        CYG_FAIL("Unknown interrupt vector");
        break;
    }
}

void
hal_interrupt_unmask(int vector)                                  
{
#ifdef CYGPKG_HAL_SH_7751
    if (vector >= CYGNUM_HAL_INTERRUPT_PCISERR && vector <= CYGNUM_HAL_INTERRUPT_TUNI4) {
        HAL_WRITE_UINT32(CYGARC_REG_INTMSKCLR00, 1 << (vector - CYGNUM_HAL_INTERRUPT_PCISERR));
        return;
    }
#endif

    switch( (vector) ) {
    case CYGNUM_HAL_INTERRUPT_NMI:
        break;
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        // fall through
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0 ... CYGNUM_HAL_ISR_MAX: 
        cyg_hal_IMASK_table[vector] = 1;
        hal_interrupt_update_level(vector);
        break;
    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:
        /* Do nothing for these reserved vectors. */
        break;
    default:
        CYG_FAIL("Unknown interrupt vector");
        break;
    }
}

void
hal_interrupt_acknowledge(int vector)
{
    CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vector);
}

void 
hal_interrupt_configure(int vector, int level, int up)
{
    CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vector, level, up);
}
