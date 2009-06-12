//==========================================================================
//
//      var_intr.c
//
//      PowerPC variant interrupt handlers
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:         2000-02-11
// Purpose:      PowerPC variant interrupt handlers
// Description:  This file contains code to handle interrupt related issues
//               on the PowerPC variant.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_arbiter.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Since the interrupt sources do not have fixed vectors on the 860
// SIU, some arbitration is required.

// More than one interrupt source can be programmed to use the same
// vector, so all sources on the same vector have to be queried to
// find the one raising the interrupt. This functionality has not been
// implemented, but the arbiter functions for each of the SIU
// interrupt sources can be called in sequence without change.



// Timebase interrupt can be caused by match on either reference A
// or B.  
// Note: If only one interrupt source is assigned per vector, and only
// reference interrupt A or B is used, this ISR is not
// necessary. Attach the timerbase reference A or B ISR directly to
// the LVLx vector instead.
externC cyg_uint32
hal_arbitration_isr_tb (CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isr_ret;
    cyg_uint16 tbscr;

    HAL_READ_UINT16 (CYGARC_REG_IMM_TBSCR, tbscr);
    if (tbscr & CYGARC_REG_IMM_TBSCR_REFA) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SIU_TB_A);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    if (tbscr & CYGARC_REG_IMM_TBSCR_REFB) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SIU_TB_B);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    return 0;
}

// Periodic interrupt.
// Note: If only one interrupt source is assigned per vector, this ISR
// is not necessary. Attach the periodic interrupt ISR directly to the
// LVLx vector instead.
externC cyg_uint32
hal_arbitration_isr_pit (CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isr_ret;
    cyg_uint16 piscr;

    HAL_READ_UINT16 (CYGARC_REG_IMM_PISCR, piscr);
    if (piscr & CYGARC_REG_IMM_PISCR_PS) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SIU_PIT);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    return 0;
}

// Real time clock interrupts can be caused by the alarm or
// once-per-second.
// Note: If only one interrupt source is assigned per vector, and only
// the alarm or once-per-second interrupt is used, this ISR is not
// necessary. Attach the alarm or once-per-second ISR directly to the
// LVLx vector instead.
externC cyg_uint32
hal_arbitration_isr_rtc (CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isr_ret;
    cyg_uint16 rtcsc;

    HAL_READ_UINT16 (CYGARC_REG_IMM_RTCSC, rtcsc);
    if (rtcsc & CYGARC_REG_IMM_RTCSC_SEC) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SIU_RTC_SEC);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    if (rtcsc & CYGARC_REG_IMM_RTCSC_ALR) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_SIU_RTC_ALR);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    return 0;
}

// Communication Processor Module interrupt can be caused by any of
// the CPM sources.
externC cyg_uint32
hal_arbitration_isr_cpm (CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 isr_ret;
    cyg_uint16 civr;

    HAL_WRITE_UINT16 (CYGARC_REG_IMM_CIVR, CYGARC_REG_IMM_CIVR_IACK);
    HAL_READ_UINT16 (CYGARC_REG_IMM_CIVR, civr);
    civr >>= CYGARC_REG_IMM_CIVR_VECTOR_SHIFT;
    if (civr) {
        isr_ret = hal_call_isr (CYGNUM_HAL_INTERRUPT_CPM_LAST - civr);
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
        if (isr_ret & CYG_ISR_HANDLED)
#endif
            return isr_ret;
    }

    return 0;
}

externC void
hal_variant_IRQ_init(void)
{
#ifdef CYGSEM_HAL_POWERPC_MPC860_CPM_ENABLE
    // Attach first-level CPM arbiter to the configured SIU level and
    // enable CPM interrupts.
#define ID_CPM 0xDEAD
#define CYGPRI_SIU_LVL (CYGNUM_HAL_INTERRUPT_SIU_LVL0 \
                        +CYGHWR_HAL_POWERPC_MPC860_CPM_LVL*2)

    HAL_INTERRUPT_ATTACH (CYGPRI_SIU_LVL, &hal_arbitration_isr_cpm, ID_CPM, 0);
    HAL_INTERRUPT_UNMASK (CYGPRI_SIU_LVL);
    HAL_INTERRUPT_SET_LEVEL (CYGNUM_HAL_INTERRUPT_SIU_CPM, 
                             CYGHWR_HAL_POWERPC_MPC860_CPM_LVL);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_SIU_CPM);
#endif
}

// -------------------------------------------------------------------------
// EOF var_intr.c
