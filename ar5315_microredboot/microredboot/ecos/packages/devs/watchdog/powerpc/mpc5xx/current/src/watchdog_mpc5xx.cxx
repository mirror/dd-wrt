//==========================================================================
//
//      devs/watchdog/powerpc/mpc5xx/watchdog_mpc5xx.cxx
//
//      Watchdog implementation for MPC5XX
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
// Author(s):    Bob Koninckx
// Contributors: Bob Koninckx
// Date:         2003-05-18
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the mpc5xx watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/watchdog.h>           // configuration for this package

#include <cyg/infra/cyg_trac.h>         // tracing macros

#include <cyg/hal/hal_io.h>             // IO register access
#include <cyg/hal/hal_arch.h>           // Register definitions

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// MPC5xx SYPCR register bit definitions
#define MPC5XX_SYPCR_SWTC 0xffff0000
#define MPC5XX_SYPCR_SWP  0x00000001
#define MPC5XX_SYPCR_SWRI 0x00000002
#define MPC5XX_SYPCR_SWE  0x00000004
#define MPC5XX_SYPCR_SWF  0x00000008

// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();

    cyg_uint32 sypcr;
	HAL_READ_UINT32(CYGARC_REG_IMM_SYPCR, sypcr);
    
    resolution = (sypcr & MPC5XX_SYPCR_SWTC) >> 16;
	if(sypcr & MPC5XX_SYPCR_SWP)
	  resolution *= 2048;

	// Now we have it in ticks, convert to nanoseconds
	// This holds for a system clock of 40 Mhz (25 nanosecond ticks) which is normal
	// for the MPC5xx
	resolution *= 25;
        
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.
// On powerpc, the watchdog is enabled by default. If the watchdog package
// is present, board setup does not disable it, so, nothing special to be
// done here.
void
Cyg_Watchdog::start(void)
{
    CYG_REPORT_FUNCTION();

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    cyg_uint16 swsr;
	swsr = 0x556c;
	HAL_WRITE_UINT16(CYGARC_REG_IMM_SWSR, swsr);
	swsr = 0xaa39;
	HAL_WRITE_UINT16(CYGARC_REG_IMM_SWSR, swsr);
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// EOF watchdog_mpc5xx.cxx
