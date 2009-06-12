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
// Date:         2000-02-11
// Purpose:      PowerPC variant interrupt handlers
// Description:  This file contains code to handle interrupt related issues
//               on the PowerPC variant.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_type.h>

extern void hal_platform_IRQ_init(void);

//
// Sadly, the IBM PPC40x family of devices are only related by number
// and not always by functionality.  In particular, the 403 has a
// completely different interrupt controller than the 405.  For now
// at least, these differences are controlled by CDL within this file.
//

#if defined(CYGHWR_HAL_POWERPC_PPC4XX_403)

static cyg_uint32 exier_mask[] = {
    0x00000000, // Unused
    0x00000000, // Unused
    0x80000000, // CYGNUM_HAL_INTERRUPT_CRITICAL         2
    0x08000000, // CYGNUM_HAL_INTERRUPT_SERIAL_RCV       3
    0x04000000, // CYGNUM_HAL_INTERRUPT_SERIAL_XMT       4
    0x02000000, // CYGNUM_HAL_INTERRUPT_JTAG_RCV         5
    0x01000000, // CYGNUM_HAL_INTERRUPT_JTAG_XMT         6
    0x00800000, // CYGNUM_HAL_INTERRUPT_DMA0             7
    0x00400000, // CYGNUM_HAL_INTERRUPT_DMA1             8
    0x00200000, // CYGNUM_HAL_INTERRUPT_DMA2             9
    0x00100000, // CYGNUM_HAL_INTERRUPT_DMA3            10
    0x00000010, // CYGNUM_HAL_INTERRUPT_EXT0            11
    0x00000008, // CYGNUM_HAL_INTERRUPT_EXT1            12
    0x00000004, // CYGNUM_HAL_INTERRUPT_EXT2            13
    0x00000002, // CYGNUM_HAL_INTERRUPT_EXT3            14
    0x00000001, // CYGNUM_HAL_INTERRUPT_EXT4            15
};

// This table inverts bit number to signal number
cyg_uint32 EXISR_TAB[] = {
    CYGNUM_HAL_INTERRUPT_CRITICAL,     // 0x80000000
    0x00000000,                        // 0x40000000
    0x00000000,                        // 0x20000000
    0x00000000,                        // 0x10000000
    CYGNUM_HAL_INTERRUPT_SERIAL_RCV,   // 0x08000000
    CYGNUM_HAL_INTERRUPT_SERIAL_XMT,   // 0x04000000
    CYGNUM_HAL_INTERRUPT_JTAG_RCV,     // 0x02000000
    CYGNUM_HAL_INTERRUPT_JTAG_XMT,     // 0x01000000
    CYGNUM_HAL_INTERRUPT_DMA0,         // 0x00800000
    CYGNUM_HAL_INTERRUPT_DMA1,         // 0x00400000
    CYGNUM_HAL_INTERRUPT_DMA2,         // 0x00200000
    CYGNUM_HAL_INTERRUPT_DMA3,         // 0x00100000
    0x00000000,                        // 0x00080000
    0x00000000,                        // 0x00040000
    0x00000000,                        // 0x00020000
    0x00000000,                        // 0x00010000
    0x00000000,                        // 0x00008000
    0x00000000,                        // 0x00004000
    0x00000000,                        // 0x00002000
    0x00000000,                        // 0x00001000
    0x00000000,                        // 0x00000800
    0x00000000,                        // 0x00000400
    0x00000000,                        // 0x00000200
    0x00000000,                        // 0x00000100
    0x00000000,                        // 0x00000080
    0x00000000,                        // 0x00000040
    0x00000000,                        // 0x00000020
    CYGNUM_HAL_INTERRUPT_EXT0,         // 0x00000010
    CYGNUM_HAL_INTERRUPT_EXT1,         // 0x00000008
    CYGNUM_HAL_INTERRUPT_EXT2,         // 0x00000004
    CYGNUM_HAL_INTERRUPT_EXT3,         // 0x00000002
    CYGNUM_HAL_INTERRUPT_EXT4          // 0x00000001
};

cyg_uint32 _hold_tcr = 0;  // Shadow of hardware register

externC void
hal_variant_IRQ_init(void)
{
    cyg_uint32 iocr;

    // Ensure all interrupts masked (disabled) & cleared
    CYGARC_MTDCR(DCR_EXIER, 0);
    CYGARC_MTDCR(DCR_EXISR, 0xFFFFFFFF);

    // Configure all external interrupts to be level/low
    CYGARC_MFDCR(DCR_IOCR, iocr);
    iocr &= ~0xFFC00000;
    CYGARC_MTDCR(DCR_IOCR, iocr);

    // Disable timers
    CYGARC_MTSPR(SPR_TCR, 0);

    // Let the platform do any overrides
    hal_platform_IRQ_init();
}

externC void 
hal_ppc40x_interrupt_mask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MFDCR(DCR_EXIER, exier);
        exier &= ~exier_mask[vector];
        CYGARC_MTDCR(DCR_EXIER, exier);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
        break;
    }
}

externC void 
hal_ppc40x_interrupt_unmask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MFDCR(DCR_EXIER, exier);
        exier |= exier_mask[vector];
        CYGARC_MTDCR(DCR_EXIER, exier);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
        break;
    }
}

externC void 
hal_ppc40x_interrupt_acknowledge(int vector)
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_EXT0:
    case CYGNUM_HAL_INTERRUPT_EXT1:
    case CYGNUM_HAL_INTERRUPT_EXT2:
    case CYGNUM_HAL_INTERRUPT_EXT3:
    case CYGNUM_HAL_INTERRUPT_EXT4:
        CYGARC_MTDCR(DCR_EXISR, exier_mask[vector]);
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_PIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_FIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_WIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_CRITICAL:
    case CYGNUM_HAL_INTERRUPT_SERIAL_RCV:
    case CYGNUM_HAL_INTERRUPT_SERIAL_XMT:
    case CYGNUM_HAL_INTERRUPT_JTAG_RCV:
    case CYGNUM_HAL_INTERRUPT_JTAG_XMT:
    case CYGNUM_HAL_INTERRUPT_DMA0:
    case CYGNUM_HAL_INTERRUPT_DMA1:
    case CYGNUM_HAL_INTERRUPT_DMA2:
    case CYGNUM_HAL_INTERRUPT_DMA3:
    default:
        break;
    }
}

// Note: These functions are only [well] defined for "external" interrupts
// which can be controlled via the EXIER register.
externC void 
hal_ppc40x_interrupt_configure(int vector, int level, int dir)
{
    cyg_uint32 mask, new_state, iocr;

    if ((vector >= CYGNUM_HAL_INTERRUPT_EXT0) &&
        (vector <= CYGNUM_HAL_INTERRUPT_EXT4)) {
        mask = 0x03 << (30 - ((vector - CYGNUM_HAL_INTERRUPT_EXT0)*2));
        new_state = (dir & 0x01);  // Up/Down    
        if (level == 0) {
            // Edge triggered
            new_state = 0x02;
        }
        new_state <<= (30 - ((vector - CYGNUM_HAL_INTERRUPT_EXT0)*2));
        CYGARC_MFDCR(DCR_IOCR, iocr);
        iocr = (iocr & ~mask) | new_state;
        CYGARC_MTDCR(DCR_IOCR, iocr);
    }
}

externC void 
hal_ppc40x_interrupt_set_level(int vector, int level)
{
}
#endif // CYGHWR_HAL_POWERPC_PPC4XX_403

#if defined(CYGHWR_HAL_POWERPC_PPC4XX_405) || defined(CYGHWR_HAL_POWERPC_PPC4XX_405GP)

cyg_uint32 _hold_tcr = 0;  // Shadow of hardware register

externC void
hal_variant_IRQ_init(void)
{
#ifndef HAL_PLF_INTERRUPT_INIT
    // Ensure all interrupts masked (disabled) & cleared
    CYGARC_MTDCR(DCR_UIC0_ER, 0);
    CYGARC_MTDCR(DCR_UIC0_CR, 0);
    CYGARC_MTDCR(DCR_UIC0_PR, 0xFFFFE000);
    CYGARC_MTDCR(DCR_UIC0_TR, 0);
    CYGARC_MTDCR(DCR_UIC0_VCR, 0);  // Makes vector identification easy
    CYGARC_MTDCR(DCR_UIC0_SR, 0xFFFFFFFF);
#else
    HAL_PLF_INTERRUPT_INIT();
#endif

    // Disable timers
    CYGARC_MTSPR(SPR_TCR, 0);

    // Let the platform do any overrides
    hal_platform_IRQ_init();
}

externC void 
hal_ppc40x_interrupt_mask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_first...CYGNUM_HAL_INTERRUPT_last:
#ifndef HAL_PLF_INTERRUPT_MASK
        CYGARC_MFDCR(DCR_UIC0_ER, exier);
        exier &= ~(1<<(31-(vector-CYGNUM_HAL_INTERRUPT_405_BASE)));
        CYGARC_MTDCR(DCR_UIC0_ER, exier);
#else
        HAL_PLF_INTERRUPT_MASK(vector);
#endif
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr &= ~TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
        break;
    }
}

externC void 
hal_ppc40x_interrupt_unmask(int vector)
{
    cyg_uint32 exier, tcr;

    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_first...CYGNUM_HAL_INTERRUPT_last:
#ifndef HAL_PLF_INTERRUPT_UNMASK
        CYGARC_MFDCR(DCR_UIC0_ER, exier);
        exier |= (1<<(31-(vector-CYGNUM_HAL_INTERRUPT_405_BASE)));
        CYGARC_MTDCR(DCR_UIC0_ER, exier);
#else
        HAL_PLF_INTERRUPT_UNMASK(vector);
#endif
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_PIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_FIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MFSPR(SPR_TCR, tcr);
        tcr = _hold_tcr;
        tcr |= TCR_WIE;
        CYGARC_MTSPR(SPR_TCR, tcr);
        _hold_tcr = tcr;
        break;
    default:
        break;
    }
}

externC void 
hal_ppc40x_interrupt_acknowledge(int vector)
{
    switch (vector) {
    case CYGNUM_HAL_INTERRUPT_first...CYGNUM_HAL_INTERRUPT_last:
#ifndef HAL_PLF_INTERRUPT_ACKNOWLEDGE
        CYGARC_MTDCR(DCR_UIC0_SR, (1<<(31-(vector-CYGNUM_HAL_INTERRUPT_405_BASE))));
#else
        HAL_PLF_INTERRUPT_ACKNOWLEDGE(vector);
#endif
        break;
    case CYGNUM_HAL_INTERRUPT_VAR_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_PIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_FIXED_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_FIS);  // clear & acknowledge interrupt
        break;
    case CYGNUM_HAL_INTERRUPT_WATCHDOG_TIMER:
        CYGARC_MTSPR(SPR_TSR, TSR_WIS);  // clear & acknowledge interrupt
        break;
    default:
        break;
    }
}

// Note: These functions are only [well] defined for "external" interrupts
externC void 
hal_ppc40x_interrupt_configure(int vector, int level, int dir)
{
#ifndef HAL_PLF_INTERRUPT_CONFIGURE
    cyg_uint32 mask, new_state, iocr;

    if ((vector >= CYGNUM_HAL_INTERRUPT_IRQ0) &&
        (vector <= CYGNUM_HAL_INTERRUPT_IRQ6)) {
        mask = (1<<(31-(vector-CYGNUM_HAL_INTERRUPT_405_BASE)));
        // Set polarity
        if (dir) {
            // High true
            new_state = mask;
        } else {
            // Low true
            new_state = 0;
        }
        CYGARC_MFDCR(DCR_UIC0_PR, iocr);
        iocr = (iocr & ~mask) | new_state;
        CYGARC_MTDCR(DCR_UIC0_PR, iocr);
        // Set edge/level
        if (level == 0) {
            // Edge triggered
            new_state = mask;
        } else {
            // Level triggered
            new_state = 0;
        }
        CYGARC_MFDCR(DCR_UIC0_TR, iocr);
        iocr = (iocr & ~mask) | new_state;
        CYGARC_MTDCR(DCR_UIC0_TR, iocr);
    }
#else
    HAL_PLF_INTERRUPT_CONFIGURE(vector, level, dir);
#endif
}

externC void 
hal_ppc40x_interrupt_set_level(int vector, int level)
{
#ifndef HAL_PLF_INTERRUPT_SET_LEVEL
    // Nothing to do for UIC
#else
    HAL_PLF_INTERRUPT_SET_LEVEL(vector, level);
#endif
}
#endif // CYGHWR_HAL_POWERPC_PPC4XX_405

// -------------------------------------------------------------------------
// EOF var_intr.c
