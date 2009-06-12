//==========================================================================
//
//      devs/watchdog/sh/sh3/watchdog_sh3.cxx
//
//      Watchdog implementation for Hitachi SH CPUs
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
// Contributors: jskov
// Date:         1999-09-01
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the Hitachi SH watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package
#include <pkgconf/kernel.h>             // kernel config

#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/kernel/instrmnt.h>        // instrumentation

#include <cyg/hal/hal_io.h>             // IO register access
#include <cyg/hal/sh_regs.h>            // watchdog register definitions

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();
    
    // No hardware init needed.

    resolution          = CYGARC_REG_WTCSR_PERIOD;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start()
{
    CYG_REPORT_FUNCTION();

    // Init the watchdog timer (note: 8 bit reads, 16 bit writes)
    cyg_uint16 csr;
    // First disable without changing other bits.
    HAL_READ_UINT8(CYGARC_REG_WTCSR, csr);
    csr |= CYGARC_REG_WTCSR_WRITE;
    csr &= ~CYGARC_REG_WTCSR_TME;
    HAL_WRITE_UINT16(CYGARC_REG_WTCSR, csr);
    // Then set control bits and clear counter.
    csr = (CYGARC_REG_WTCSR_WRITE
           |CYGARC_REG_WTCSR_WT_IT
           |CYGARC_REG_WTCSR_CKSx_SETTING);
    HAL_WRITE_UINT16(CYGARC_REG_WTCSR, csr);
    HAL_WRITE_UINT16(CYGARC_REG_WTCNT, CYGARC_REG_WTCNT_WRITE);
    // Finally enable timer.
    csr |= CYGARC_REG_WTCSR_TME;
    HAL_WRITE_UINT16(CYGARC_REG_WTCSR, csr);
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    HAL_WRITE_UINT16(CYGARC_REG_WTCNT, CYGARC_REG_WTCNT_WRITE);
    
    CYG_REPORT_RETURN();
}

#if 0
// -------------------------------------------------------------------------
// Trigger the watchdog as if the timer had expired.

void
Cyg_Watchdog::action_reset(void)
{
    CYG_REPORT_FUNCTION();
    
    start();

    HAL_WRITE_UINT16(CYGARC_REG_WTCNT, CYGARC_REG_WTCNT_WRITE|0xfe);

    CYG_REPORT_RETURN();
}
#endif

// -------------------------------------------------------------------------
// EOF watchdog_sh3.cxx
