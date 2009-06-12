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
// Date:         2002-01-09
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

#ifdef CYGHWR_HAL_SH_CACHE_MODE_WRITE_BACK
# define CACHE_MODE HAL_UCACHE_WRITEBACK_MODE
#else
# define CACHE_MODE HAL_UCACHE_WRITETHRU_MODE
#endif

externC void
cyg_var_enable_caches(void)
{
    // If relying on a ROM monitor do not invalidate the caches as the
    // ROM monitor may have (non-synced) state in the caches.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)
    // Initialize cache.
    HAL_UCACHE_INVALIDATE_ALL();    

#ifdef HAL_UCACHE_WRITE_MODE
    // Set cache modes
    HAL_UCACHE_WRITE_MODE(CACHE_MODE);
#endif
#endif
#ifdef CYGHWR_HAL_SH_CACHE_ENABLE
    // Enable cache.
    HAL_UCACHE_ENABLE();
#endif
}

//---------------------------------------------------------------------------
// Interrupt function support

static void
hal_interrupt_set_vectors(void)
{
#if (CYGARC_SH_MOD_INTC == 1)
    // variant specific
    HAL_WRITE_UINT16(CYGARC_REG_VCRDMA0,
                     CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_DMAC0_TE);
    HAL_WRITE_UINT16(CYGARC_REG_VCRDMA1,
                     CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_DMAC1_TE);
    HAL_WRITE_UINT16(CYGARC_REG_VCRWDT, 
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_WDT_ITI)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_REF_CMI));

    HAL_WRITE_UINT16(CYGARC_REG_VCRA,
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_EDMAC_EINT)<<8);
    
    HAL_WRITE_UINT16(CYGARC_REG_VCRC,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_FRT_ICI)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_FRT_OCI));
    HAL_WRITE_UINT16(CYGARC_REG_VCRD,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_FRT_OVI)<<8));
    HAL_WRITE_UINT16(CYGARC_REG_VCRE,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU0_TGI0A)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU0_TGI0B));
    HAL_WRITE_UINT16(CYGARC_REG_VCRF,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU0_TGI0C)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU0_TGI0D));
    HAL_WRITE_UINT16(CYGARC_REG_VCRG,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU0_TGI0V)<<8));
    HAL_WRITE_UINT16(CYGARC_REG_VCRH,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU1_TGI1A)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU1_TGI1B));
    HAL_WRITE_UINT16(CYGARC_REG_VCRI,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU1_TGI1V)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU1_TGI1U));
    HAL_WRITE_UINT16(CYGARC_REG_VCRJ,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU2_TGI2A)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU2_TGI2B));
    HAL_WRITE_UINT16(CYGARC_REG_VCRK,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU2_TGI2V)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_TPU2_TGI2U));
    HAL_WRITE_UINT16(CYGARC_REG_VCRL,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF1_ERI1)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF1_RXI1));
    HAL_WRITE_UINT16(CYGARC_REG_VCRM,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF1_BRI1)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF1_TXI1));
    HAL_WRITE_UINT16(CYGARC_REG_VCRN,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF2_ERI2)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF2_RXI2));
    HAL_WRITE_UINT16(CYGARC_REG_VCRO,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF2_BRI2)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SCIF2_TXI2));
    HAL_WRITE_UINT16(CYGARC_REG_VCRP,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO0_RERI0)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO0_TERI0));
    HAL_WRITE_UINT16(CYGARC_REG_VCRQ,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO0_RDFI0)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO0_TDEI0));
    HAL_WRITE_UINT16(CYGARC_REG_VCRR,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO1_RERI1)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO1_TERI1));
    HAL_WRITE_UINT16(CYGARC_REG_VCRS,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO1_RDFI1)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO1_TDEI1));
    HAL_WRITE_UINT16(CYGARC_REG_VCRT,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO2_RERI2)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO2_TERI2));
    HAL_WRITE_UINT16(CYGARC_REG_VCRU,
                     ((CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO2_RDFI2)<<8) |
                     (CYGNUM_HAL_INTERRUPT_HW_EXC_BASE+CYGNUM_HAL_INTERRUPT_SIO2_TDEI2));

#elif (CYGARC_SH_MOD_INTC == 2)
    /* Hardwired vectors */
#else
# error "No priority handling for INTC variant"
#endif
}

externC cyg_uint8 cyg_hal_ILVL_table[];
externC cyg_uint8 cyg_hal_IMASK_table[];

static void
hal_interrupt_update_level(int vector)
{
    cyg_uint16 iprX;
    int level;

    level = cyg_hal_IMASK_table[vector] ? cyg_hal_ILVL_table[vector] : 0;

    switch( (vector) ) {
#if (CYGARC_SH_MOD_INTC == 1)
        /* IPRA */
    case CYGNUM_HAL_INTERRUPT_DMAC0_TE:
    case CYGNUM_HAL_INTERRUPT_DMAC1_TE:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_DMAC_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_DMAC_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_WDT_ITI:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_WDT_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_WDT_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;

        /* This vector can not be configured */
    case CYGNUM_HAL_INTERRUPT_REF_CMI:
        break;

        /* IPRB */
    case CYGNUM_HAL_INTERRUPT_EDMAC_EINT:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_EDMAC_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_EDMAC_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_FRT_ICI:
    case CYGNUM_HAL_INTERRUPT_FRT_OCI:
    case CYGNUM_HAL_INTERRUPT_FRT_OVI:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_FRT_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_FRT_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;

#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
        /* IPRC */
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_IRQ0_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_IRQ0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_IRQ1_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_IRQ1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_IRQ2_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_IRQ2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_IRQ3_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_IRQ3_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
#else
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL3:
        /* Cannot change levels */
        break;                                                           
#endif
        /* IPRD */
    case CYGNUM_HAL_INTERRUPT_TPU0_TGI0A:
    case CYGNUM_HAL_INTERRUPT_TPU0_TGI0B:
    case CYGNUM_HAL_INTERRUPT_TPU0_TGI0C:
    case CYGNUM_HAL_INTERRUPT_TPU0_TGI0D:
    case CYGNUM_HAL_INTERRUPT_TPU0_TGI0V:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_TPU0_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_TPU0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_TPU1_TGI1A:
    case CYGNUM_HAL_INTERRUPT_TPU1_TGI1B:
    case CYGNUM_HAL_INTERRUPT_TPU1_TGI1V:
    case CYGNUM_HAL_INTERRUPT_TPU1_TGI1U:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_TPU1_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_TPU1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_TPU2_TGI2A:
    case CYGNUM_HAL_INTERRUPT_TPU2_TGI2B:
    case CYGNUM_HAL_INTERRUPT_TPU2_TGI2V:
    case CYGNUM_HAL_INTERRUPT_TPU2_TGI2U:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_TPU2_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_TPU2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SCIF1_ERI1:
    case CYGNUM_HAL_INTERRUPT_SCIF1_RXI1:
    case CYGNUM_HAL_INTERRUPT_SCIF1_BRI1:
    case CYGNUM_HAL_INTERRUPT_SCIF1_TXI1:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_SCIF1_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_SCIF1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;

        /* IPRE */
    case CYGNUM_HAL_INTERRUPT_SCIF2_ERI2:
    case CYGNUM_HAL_INTERRUPT_SCIF2_RXI2:
    case CYGNUM_HAL_INTERRUPT_SCIF2_BRI2:
    case CYGNUM_HAL_INTERRUPT_SCIF2_TXI2:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_SCIF2_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_SCIF2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SIO0_RERI0:
    case CYGNUM_HAL_INTERRUPT_SIO0_TERI0:
    case CYGNUM_HAL_INTERRUPT_SIO0_RDFI0:
    case CYGNUM_HAL_INTERRUPT_SIO0_TDEI0:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_SIO0_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_SIO0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SIO1_RERI1:
    case CYGNUM_HAL_INTERRUPT_SIO1_TERI1:
    case CYGNUM_HAL_INTERRUPT_SIO1_RDFI1:
    case CYGNUM_HAL_INTERRUPT_SIO1_TDEI1:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_SIO1_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_SIO1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SIO2_RERI2:
    case CYGNUM_HAL_INTERRUPT_SIO2_TERI2:
    case CYGNUM_HAL_INTERRUPT_SIO2_RDFI2:
    case CYGNUM_HAL_INTERRUPT_SIO2_TDEI2:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_SIO2_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_SIO2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
#elif (CYGARC_SH_MOD_INTC == 2)

        /* IPRA */
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_IRQ0_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_IRQ0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_IRQ1_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_IRQ1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_IRQ2_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_IRQ2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);
        iprX &= ~CYGARC_REG_IPRA_IRQ3_MASK;
        iprX |= (level)*CYGARC_REG_IPRA_IRQ3_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);
        break;

        /* IPRB */
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ4:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_IRQ4_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_IRQ4_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ5:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_IRQ5_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_IRQ5_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ6:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_IRQ6_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_IRQ6_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ7:
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);
        iprX &= ~CYGARC_REG_IPRB_IRQ7_MASK;
        iprX |= (level)*CYGARC_REG_IPRB_IRQ7_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);
        break;

        /* IPRB */
    case CYGNUM_HAL_INTERRUPT_DMAC0_DEI0:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_DMAC0_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_DMAC0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_DMAC1_DEI1:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_DMAC1_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_DMAC1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_DMAC2_DEI2:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_DMAC2_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_DMAC2_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_DMAC3_DEI3:
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);
        iprX &= ~CYGARC_REG_IPRC_DMAC3_MASK;
        iprX |= (level)*CYGARC_REG_IPRC_DMAC3_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);
        break;

        /* IPRD */
    case CYGNUM_HAL_INTERRUPT_MTU0_TGI0A:
    case CYGNUM_HAL_INTERRUPT_MTU0_TGI0B:
    case CYGNUM_HAL_INTERRUPT_MTU0_TGI0C:
    case CYGNUM_HAL_INTERRUPT_MTU0_TGI0D:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_MTU0A_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_MTU0A_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU0_TGI0V:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_MTU0B_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_MTU0B_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU1_TGI1A:
    case CYGNUM_HAL_INTERRUPT_MTU1_TGI1B:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_MTU1A_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_MTU1A_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU1_TGI1V:
    case CYGNUM_HAL_INTERRUPT_MTU1_TGI1U:
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);
        iprX &= ~CYGARC_REG_IPRD_MTU1B_MASK;
        iprX |= (level)*CYGARC_REG_IPRD_MTU1B_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);
        break;

        /* IPRE */
    case CYGNUM_HAL_INTERRUPT_MTU2_TGI2A:
    case CYGNUM_HAL_INTERRUPT_MTU2_TGI2B:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_MTU2A_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_MTU2A_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU2_TGI2V:
    case CYGNUM_HAL_INTERRUPT_MTU2_TGI2U:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_MTU2B_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_MTU2B_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU3_TGI3A:
    case CYGNUM_HAL_INTERRUPT_MTU3_TGI3B:
    case CYGNUM_HAL_INTERRUPT_MTU3_TGI3C:
    case CYGNUM_HAL_INTERRUPT_MTU3_TGI3D:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_MTU3A_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_MTU3A_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU3_TGI3V:
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);
        iprX &= ~CYGARC_REG_IPRE_MTU3B_MASK;
        iprX |= (level)*CYGARC_REG_IPRE_MTU3B_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);
        break;

        /* IPRF */
    case CYGNUM_HAL_INTERRUPT_MTU4_TGI4A:
    case CYGNUM_HAL_INTERRUPT_MTU4_TGI4B:
    case CYGNUM_HAL_INTERRUPT_MTU4_TGI4C:
    case CYGNUM_HAL_INTERRUPT_MTU4_TGI4D:
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);
        iprX &= ~CYGARC_REG_IPRF_MTU4A_MASK;
        iprX |= (level)*CYGARC_REG_IPRF_MTU4A_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_MTU4_TGI4V:
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);
        iprX &= ~CYGARC_REG_IPRF_MTU4B_MASK;
        iprX |= (level)*CYGARC_REG_IPRF_MTU4B_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SCI0_ERI0:
    case CYGNUM_HAL_INTERRUPT_SCI0_RXI0:
    case CYGNUM_HAL_INTERRUPT_SCI0_TXI0:
    case CYGNUM_HAL_INTERRUPT_SCI0_TEI0:
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);
        iprX &= ~CYGARC_REG_IPRF_SCI0_MASK;
        iprX |= (level)*CYGARC_REG_IPRF_SCI0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_SCI1_ERI1:
    case CYGNUM_HAL_INTERRUPT_SCI1_RXI1:
    case CYGNUM_HAL_INTERRUPT_SCI1_TXI1:
    case CYGNUM_HAL_INTERRUPT_SCI1_TEI1:
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);
        iprX &= ~CYGARC_REG_IPRF_SCI1_MASK;
        iprX |= (level)*CYGARC_REG_IPRF_SCI1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);
        break;

        /* IPRG */
    case CYGNUM_HAL_INTERRUPT_AD_ADI0:
    case CYGNUM_HAL_INTERRUPT_AD_ADI1:
        HAL_READ_UINT16(CYGARC_REG_IPRG, iprX);
        iprX &= ~CYGARC_REG_IPRG_AD_MASK;
        iprX |= (level)*CYGARC_REG_IPRG_AD_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRG, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_DTC_SWDTCE:
        HAL_READ_UINT16(CYGARC_REG_IPRG, iprX);
        iprX &= ~CYGARC_REG_IPRG_DTC_MASK;
        iprX |= (level)*CYGARC_REG_IPRG_DTC_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRG, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_CMT0_CMI0:
        HAL_READ_UINT16(CYGARC_REG_IPRG, iprX);
        iprX &= ~CYGARC_REG_IPRG_CMT0_MASK;
        iprX |= (level)*CYGARC_REG_IPRG_CMT0_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRG, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_CMT1_CMI1:
        HAL_READ_UINT16(CYGARC_REG_IPRG, iprX);
        iprX &= ~CYGARC_REG_IPRG_CMT1_MASK;
        iprX |= (level)*CYGARC_REG_IPRG_CMT1_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRG, iprX);
        break;

        /* IPRH */
    case CYGNUM_HAL_INTERRUPT_WDT_ITI:
    case CYGNUM_HAL_INTERRUPT_BSC_CMI:
        HAL_READ_UINT16(CYGARC_REG_IPRH, iprX);
        iprX &= ~CYGARC_REG_IPRH_WDT_MASK;
        iprX |= (level)*CYGARC_REG_IPRH_WDT_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRH, iprX);
        break;
    case CYGNUM_HAL_INTERRUPT_IO_OEI:
        HAL_READ_UINT16(CYGARC_REG_IPRH, iprX);
        iprX &= ~CYGARC_REG_IPRH_IO_MASK;
        iprX |= (level)*CYGARC_REG_IPRH_IO_PRI1;
        HAL_WRITE_UINT16(CYGARC_REG_IPRH, iprX);
        break;


#else
# error "No hal_interrupt_update_level handling for INTC type"
#endif

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
    switch( (vector) ) {
    case CYGNUM_HAL_INTERRUPT_NMI:
        /* fall through */
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL_MAX:
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        /* fall through */
#if (CYGARC_SH_MOD_INTC == 1)
    case CYGNUM_HAL_INTERRUPT_DMAC0_TE ... CYGNUM_HAL_ISR_MAX:
#elif (CYGARC_SH_MOD_INTC == 2)
    case CYGNUM_HAL_INTERRUPT_DMAC0_DEI0 ... CYGNUM_HAL_ISR_MAX:
#else
# error "Cannot unmask for INTC"
#endif
        cyg_hal_IMASK_table[vector] = 0;
        hal_interrupt_update_level(vector);
        break;
    default:
        CYG_FAIL("Unknown interrupt vector");
        break;
    }
}

void
hal_interrupt_unmask(int vector)
{
    switch( (vector) ) {
    case CYGNUM_HAL_INTERRUPT_NMI:
        /* fall through */
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL_MAX:
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        /* fall through */
#if (CYGARC_SH_MOD_INTC == 1)
    case CYGNUM_HAL_INTERRUPT_DMAC0_TE ... CYGNUM_HAL_ISR_MAX:
#elif (CYGARC_SH_MOD_INTC == 2)
    case CYGNUM_HAL_INTERRUPT_DMAC0_DEI0 ... CYGNUM_HAL_ISR_MAX:
#else
# error "Cannot unmask for INTC"
#endif
        cyg_hal_IMASK_table[vector] = 1;
        hal_interrupt_update_level(vector);
        break;
    default:
        CYG_FAIL("Unknown interrupt vector");
        break;
    }
}

void
hal_interrupt_acknowledge(int vector)
{
#if (CYGARC_SH_MOD_INTC == 1)
    if ( (vector) >= CYGNUM_HAL_INTERRUPT_IRQ_IRQ0
         && (vector) <= CYGNUM_HAL_INTERRUPT_IRQ_IRQ3) {

        cyg_uint8 irqcsr;

        HAL_READ_UINT8(CYGARC_REG_IRQCSR, irqcsr);
        switch ( vector ) {
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:
            irqcsr &= ~CYGARC_REG_IRQCSR_IRQ0F;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:
            irqcsr &= ~CYGARC_REG_IRQCSR_IRQ1F;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:
            irqcsr &= ~CYGARC_REG_IRQCSR_IRQ2F;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:
            irqcsr &= ~CYGARC_REG_IRQCSR_IRQ3F;
            break;
#endif
        default:
            CYG_FAIL("Unhandled interrupt vector");
        }
        HAL_WRITE_UINT8(CYGARC_REG_IRQCSR, irqcsr);
    }
#elif (CYGARC_SH_MOD_INTC == 2)
    if ( (vector) >= CYGNUM_HAL_INTERRUPT_IRQ_IRQ0
         && (vector) <= CYGNUM_HAL_INTERRUPT_IRQ_IRQ7) {
        cyg_uint16 isr;
        cyg_uint16 mask = ~(1<<(7-(vector - CYGNUM_HAL_INTERRUPT_IRQ_IRQ0)));
        
        HAL_READ_UINT16(CYGARC_REG_ISR, isr);
        isr &= mask;
        HAL_WRITE_UINT16(CYGARC_REG_ISR, isr);
    }
#else
# error "No hal_interrupt_acknowledge code for this INTC"
#endif

    CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vector);
}

void
hal_interrupt_configure(int vector, int level, int up)
{
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
#if (CYGARC_SH_MOD_INTC == 1)
    if ( (vector) >= CYGNUM_HAL_INTERRUPT_IRQ_IRQ0
         && (vector) <= CYGNUM_HAL_INTERRUPT_IRQ_IRQ3) {

        cyg_uint8 irqcsr, ss, mask;
        ss = 0;
        mask = CYGARC_REG_IRQCSR_IRQ_mask;
        if (level) {
            ss = CYGARC_REG_IRQCSR_IRQ_LOWLEVEL;
            CYG_ASSERT(!(up), "Cannot trigger on high level!");
        } else {
            if (up) 
                ss = CYGARC_REG_IRQCSR_IRQ_RISING;
            else
                ss = CYGARC_REG_IRQCSR_IRQ_FALLING;
        }

        switch( (vector) ) {
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:
            ss <<= CYGARC_REG_IRQCSR_IRQ3_shift;
            mask <<= CYGARC_REG_IRQCSR_IRQ3_shift;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:
            ss <<= CYGARC_REG_IRQCSR_IRQ2_shift;
            mask <<= CYGARC_REG_IRQCSR_IRQ2_shift;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:
            ss <<= CYGARC_REG_IRQCSR_IRQ1_shift;
            mask <<= CYGARC_REG_IRQCSR_IRQ1_shift;
            break;
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:
            ss <<= CYGARC_REG_IRQCSR_IRQ0_shift;
            mask <<= CYGARC_REG_IRQCSR_IRQ0_shift;
            break;
        default:
            CYG_FAIL("Unhandled interrupt vector");
        }

        HAL_READ_UINT8(CYGARC_REG_IRQCSR, irqcsr);
        irqcsr &= ~mask;
        irqcsr |= ss;
        HAL_WRITE_UINT8(CYGARC_REG_IRQCSR, irqcsr);
    }
#endif
#endif

    CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vector, level, up);
}

//---------------------------------------------------------------------------
void
hal_variant_init(void)
{
    HAL_CLOCK_VAR_INITIALIZE(CYGNUM_HAL_RTC_PERIOD);

    hal_interrupt_set_vectors();
}

//---------------------------------------------------------------------------
// Low-level delay (in microseconds)

#ifdef CYGARC_SH_MOD_FRT
void
hal_delay_us(int usecs)
{
    cyg_uint16 val1, val2;
    cyg_int32 diff, clocks, wrap;

    /* Read compare A register */
    HAL_WRITE_UINT8(CYGARC_REG_TOCR, CYGARC_REG_TOCR_OLVLA);
    HAL_READ_UINT16(CYGARC_REG_OCR, val1);
    wrap = (cyg_int32)val1;

    clocks = CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/CYGHWR_HAL_SH_RTC_PRESCALE/1000*usecs/1000;

    HAL_READ_UINT16(CYGARC_REG_FRC, val1);
    while (clocks > 0) {
        HAL_READ_UINT16(CYGARC_REG_FRC, val2);

        diff = (cyg_int32)val1 - (cyg_int32)val2;
        if (diff > 0)
            clocks += (diff-wrap);
        else
            clocks += diff;

        val1 = val2;
    }
}
#elif defined(CYGARC_SH_MOD_CMT)
void
hal_delay_us(int usecs)
{
    cyg_uint16 val1, val2;
    cyg_int32 diff, clocks, wrap;

    /* Read compare 0 register */
    HAL_READ_UINT16(CYGARC_REG_CMCOR0, val1);
    wrap = (cyg_int32)val1;

    clocks = CYGHWR_HAL_SH_ONCHIP_PERIPHERAL_SPEED/CYGHWR_HAL_SH_RTC_PRESCALE/1000*usecs/1000;

    HAL_READ_UINT16(CYGARC_REG_CMCNT0, val1);
    while (clocks > 0) {
        HAL_READ_UINT16(CYGARC_REG_CMCNT0, val2);

        diff = (cyg_int32)val1 - (cyg_int32)val2;
        if (diff > 0)
            clocks += (diff-wrap);
        else
            clocks += diff;

        val1 = val2;
    }
}
#else
# error "No hal_delay_us implementation"
#endif


//---------------------------------------------------------------------------
// End of hal_misc.c
