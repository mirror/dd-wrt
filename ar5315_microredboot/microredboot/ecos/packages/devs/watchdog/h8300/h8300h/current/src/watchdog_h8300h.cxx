//==========================================================================
//
//      devs/watchdog/h8300/h83000/watchdog_h8300.cxx
//
//      Watchdog implementation for Hitachi H8/300H CPUs
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
// Author(s):    yoshinori sato
// Contributors: yoshinori sato
// Date:         2002-04-29
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the Hitachi H8/300H watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package

#include <cyg/infra/cyg_trac.h>         // tracing macros

#include <cyg/hal/hal_io.h>             // IO register access
#include <cyg/hal/mod_regs_wdt.h>       // watchdog register definitions

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();
    
    // No hardware init needed.

    resolution          = CYGARC_WDT_PERIOD;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start()
{
    CYG_REPORT_FUNCTION();

    //Stop WDT
    HAL_WRITE_UINT16(CYGARC_TCSR,CYGARC_TCSR_MAGIC);
    //Clear WDT Count
    HAL_WRITE_UINT16(CYGARC_TCSR,CYGARC_TCNT_MAGIC);
    //Start WDT
    HAL_WRITE_UINT16(CYGARC_TCSR,CYGARC_TCSR_MAGIC|
                                 CYGARC_WDT_WT|CYGARC_WDT_TME|CYGARC_WDT_CKS);
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    HAL_WRITE_UINT16(CYGARC_TCSR,CYGARC_TCNT_MAGIC);
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// EOF watchdog_h8300.cxx
