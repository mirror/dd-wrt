//==========================================================================
//
//      var_misc.c
//
//      HAL implementation miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
// Contributors: jskov, gthomas
// Date:         2000-02-04
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/quicc/ppc8xx.h>
#include <cyg/infra/cyg_type.h>         // types
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_mem.h>            // some of the functions defined here

//--------------------------------------------------------------------------
void
hal_variant_init(void)
{
    // Disable serialization
    {
        cyg_uint32 ictrl;
        CYGARC_MFSPR (ICTRL, ictrl);
        ictrl |= ICTRL_NOSERSHOW;
        CYGARC_MTSPR (ICTRL, ictrl);
    }

#ifndef CYGSEM_HAL_USE_ROM_MONITOR
    // Reset CPM
    _mpc8xx_reset_cpm();
#endif
}

//--------------------------------------------------------------------------
// Variant specific idle thread action.
bool
hal_variant_idle_thread_action( cyg_uint32 count )
{
#if 0
    cyg_uint32 *psivec  = (cyg_uint32*)CYGARC_REG_IMM_SIVEC ;
    cyg_uint32 *psimask = (cyg_uint32*)CYGARC_REG_IMM_SIMASK;
    cyg_uint32 *psipend = (cyg_uint32*)CYGARC_REG_IMM_SIPEND;
    cyg_uint16 *ptbscr =  (cyg_uint16*)CYGARC_REG_IMM_TBSCR;

    diag_printf( "TBSCR %04x, vec %d: sivec %08x, simask %08x, sipend %08x\n",
                 (cyg_uint32)(*ptbscr), (*psivec)>>26, *psivec,
                 *psimask, *psipend );
#endif

    // Let architecture idle thread action run
    return true;
}

//---------------------------------------------------------------------------
// Use MMU resources to map memory regions.  
// Takes and returns an int used to ID the MMU resource to use. This ID
// is increased as resources are used and should be used for subsequent
// invocations.
//
// The MPC8xx CPUs do not have BATs. Fortunately we don't currently
// use the MMU, so we can simulate BATs by using the TLBs.

#if defined(CYGHWR_HAL_POWERPC_MPC8XX_860) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_860T) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_852T) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_855T) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_862T) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_866T) || \
    defined(CYGHWR_HAL_POWERPC_MPC8XX_862P)
#define NUM_TLBS 32
#elif defined(CYGHWR_HAL_POWERPC_MPC8XX_823) || defined(CYGHWR_HAL_POWERPC_MPC8XX_850)
#define NUM_TLBS 8
#else
#error Missing TLB information for this platform
#endif

int
cyg_hal_map_memory (int id,CYG_ADDRESS virt, CYG_ADDRESS phys, 
                    cyg_int32 size, cyg_uint8 flags)
{
    cyg_uint32 epn, rpn, twc, ctr = 0;

    epn = (virt & MI_EPN_EPNMASK) | MI_EPN_EV;
    rpn = ((phys & MI_RPN_RPNMASK) 
           | MI_RPN_PPRWRW | MI_RPN_LPS | MI_RPN_SH | MI_RPN_V);
    if (flags & CYGARC_MEMDESC_CI) 
        rpn |= MI_RPN_CI;

    twc = MI_TWC_PS8MB | MI_TWC_V;
    if (flags & CYGARC_MEMDESC_GUARDED) 
        twc |= MI_TWC_G;

    // Ignore attempts to use more than max_tlbs.
    while (id < NUM_TLBS && size > 0) {
        ctr = id << MI_CTR_INDX_SHIFT;
        
        // Instruction TLB.
        CYGARC_MTSPR (MI_TWC, twc);
        CYGARC_MTSPR (MI_CTR, ctr);
        CYGARC_MTSPR (MI_EPN, epn);
        CYGARC_MTSPR (MI_RPN, rpn);

        // Data TLB.
        {
            cyg_uint32 drpn;

            // Need to mark data page as changed or an exception
            // will be generated on first write to the page.
            drpn = rpn | MD_RPN_CHANGED;

            CYGARC_MTSPR (MD_TWC, twc);
            CYGARC_MTSPR (MD_CTR, ctr);
            CYGARC_MTSPR (MD_EPN, epn);
            CYGARC_MTSPR (MD_RPN, drpn);
        }

        // Move to next 8MB block.
        size -= 8*1024*1024;
        epn  += 8*1024*1024;
        rpn  += 8*1024*1024;
        id++;
    }

    // Make caches default disabled when MMU is disabled.
    CYGARC_MTSPR (MI_CTR, ctr | CYGARC_REG_MI_CTR_CIDEF);
    CYGARC_MTSPR (MD_CTR, ctr | CYGARC_REG_MD_CTR_CIDEF);

    return id;
}

// Initialize MMU to a sane (NOP) state.
//
// Initialize TLBs with 0, Valid bits unset.
void
cyg_hal_clear_MMU (void)
{
    cyg_uint32 ctr = 0;
    int id;

    CYGARC_MTSPR (M_CASID, 0);

    for (id = 0; id < NUM_TLBS; id++) {
        ctr = id << MI_CTR_INDX_SHIFT;
        
        // Instruction TLBs.
        CYGARC_MTSPR (MI_TWC, 0);
        CYGARC_MTSPR (MI_CTR, ctr);
        CYGARC_MTSPR (MI_EPN, 0);
        CYGARC_MTSPR (MI_RPN, 0);
        // Data TLBs.
        CYGARC_MTSPR (MD_TWC, 0);
        CYGARC_MTSPR (MD_CTR, ctr);
        CYGARC_MTSPR (MD_EPN, 0);
        CYGARC_MTSPR (MD_RPN, 0);
    }

    // Make caches default disabled when MMU is disabled.
    CYGARC_MTSPR (MI_CTR, ctr | CYGARC_REG_MI_CTR_CIDEF);
    CYGARC_MTSPR (MD_CTR, ctr | CYGARC_REG_MD_CTR_CIDEF);
}

#ifdef CYGPKG_PROFILE_GPROF
//--------------------------------------------------------------------------
//
// Profiling support - uses a separate high-speed timer
//

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/quicc/ppc8xx.h>
#include <cyg/profile/profile.h>

// Can't rely on Cyg_Interrupt class being defined.
#define Cyg_InterruptHANDLED 1

#define PIT_IRQ_LEVEL 4
#define PIT_IRQ CYGNUM_HAL_INTERRUPT_SIU_LVL4
#define ID_PIT       34512


// Periodic timer ISR.
static cyg_uint32 
isr_pit(CYG_ADDRWORD vector, CYG_ADDRWORD data, HAL_SavedRegisters *regs)
{

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_SIU_PIT);
    __profile_hit(regs->pc);

    return Cyg_InterruptHANDLED;
}

void
hal_enable_profile_timer(int resolution)
{
    // Run periodic timer interrupt for profile 
    cyg_uint16 piscr;
    int period = resolution / 100;

    // Attach pit arbiter.
    HAL_INTERRUPT_ATTACH (PIT_IRQ,
                          &hal_arbitration_isr_pit, ID_PIT, 0);
    HAL_INTERRUPT_UNMASK (PIT_IRQ);

    // Attach pit isr.
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_SIU_PIT, &isr_pit,
                          ID_PIT, 0);
    HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_PIT, PIT_IRQ_LEVEL);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_PIT);


    // Set period.
    HAL_WRITE_UINT32 (CYGARC_REG_IMM_PITC, 
                      (2*period) << CYGARC_REG_IMM_PITC_COUNT_SHIFT);

    // Enable.
    HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
    piscr |= CYGARC_REG_IMM_PISCR_PTE;
    HAL_WRITE_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
}
#endif

//--------------------------------------------------------------------------
// End of var_misc.c
