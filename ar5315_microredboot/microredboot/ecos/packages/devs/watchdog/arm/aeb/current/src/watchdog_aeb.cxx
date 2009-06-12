//==========================================================================
//
//      devs/watchdog/arm/aeb/watchdog_aeb.cxx
//
//      Watchdog implementation for ARM AEB1 board (SHARP LH77790 CPU)
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
//               with the SHARP LH77790 watchdog timer.
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

#include <cyg/io/watchdog.hxx>          // watchdog API

// -------------------------------------------------------------------------
// Register definitions
#define CYGARC_REG_WATCHDOG_BASE        0xFFFFAC00
#define CYGARC_REG_WATCHDOG_WDCTLR      (CYGARC_REG_WATCHDOG_BASE+0x30)
#define CYGARC_REG_WATCHDOG_WDCNTR      (CYGARC_REG_WATCHDOG_BASE+0x34)

// Control register bits
#define CYGARC_REG_WATCHDOG_WDCTLR_EN        0x01 // enable
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_NMF   0x00 // non-maskable fiq
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_ER    0x04 // external reset
#define CYGARC_REG_WATCHDOG_WDCTLR_RSP_SR    0x06 // system reset
#define CYGARC_REG_WATCHDOG_WDCTLR_FRZ       0x08 // lock enable bit
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_MASK  0x70 // time out period

#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_17    0x00 // 2^17
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_17_P  5242880 // = 5.2ms

#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_25    0x40 // 2^25
#define CYGARC_REG_WATCHDOG_WDCTLR_TOP_25_P  1342177300 // = 1.3421773s


// -------------------------------------------------------------------------
// Constructor

void
Cyg_Watchdog::init_hw(void)
{
    CYG_REPORT_FUNCTION();
    
    // No HW init.

    resolution          = CYGARC_REG_WATCHDOG_WDCTLR_TOP_25_P;

    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Start the watchdog running.

void
Cyg_Watchdog::start()
{
    CYG_REPORT_FUNCTION();

    // Clear the watchdog counter.
    HAL_WRITE_UINT32(CYGARC_REG_WATCHDOG_WDCNTR, 0);

    // Enable the watchdog (and lock/FRZ it).
    HAL_WRITE_UINT8(CYGARC_REG_WATCHDOG_WDCTLR, 
                    (CYGARC_REG_WATCHDOG_WDCTLR_TOP_25
                     | CYGARC_REG_WATCHDOG_WDCTLR_FRZ
                     | CYGARC_REG_WATCHDOG_WDCTLR_RSP_SR
                     | CYGARC_REG_WATCHDOG_WDCTLR_EN));
    
    CYG_REPORT_RETURN();
}

// -------------------------------------------------------------------------
// Reset watchdog timer. This needs to be called regularly to prevent
// the watchdog firing.

void
Cyg_Watchdog::reset()
{    
    CYG_REPORT_FUNCTION();

    HAL_WRITE_UINT32(CYGARC_REG_WATCHDOG_WDCNTR, 0);
    
    CYG_REPORT_RETURN();
}

#if 0
// -------------------------------------------------------------------------
// Action which will do a board reset. Application can register this
// action to get a board reset on watchdog timeout.

void
Cyg_Watchdog::reset_action(void)
{
    CYG_REPORT_FUNCTION();
    
    // Clear the watchdog counter.
    HAL_WRITE_UINT32(CYGARC_REG_WATCHDOG_WDCNTR, 0);

    // Enable the watchdog with the smallest timeout.
    HAL_WRITE_UINT8(CYGARC_REG_WATCHDOG_WDCTLR, 
                    (CYGARC_REG_WATCHDOG_WDCTLR_TOP_17
                     | CYGARC_REG_WATCHDOG_WDCTLR_FRZ
                     | CYGARC_REG_WATCHDOG_WDCTLR_RSP_SR
                     | CYGARC_REG_WATCHDOG_WDCTLR_EN));

    CYG_REPORT_RETURN();
}
#endif

// -------------------------------------------------------------------------
// EOF watchdog_aeb.cxx
