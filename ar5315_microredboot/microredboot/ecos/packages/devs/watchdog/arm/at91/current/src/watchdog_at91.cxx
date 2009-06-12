//==========================================================================
//
//      devs/watchdog/arm/at91/watchdog_at91.cxx
//
//      Watchdog implementation for ARM AT91 CPU
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
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
// Author(s):    tkoeller
// Contributors: tkoeller, nickg
// Date:         2002-05-05
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the ATMEL AT91 watchdog timer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/infra.h>
#include <pkgconf/kernel.h>
#include <pkgconf/watchdog.h>
#include <pkgconf/devs_watchdog_arm_at91.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/io/watchdog.hxx>

#if !defined(CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT)
#include <cyg/hal/hal_platform_ints.h>
#include <cyg/kernel/intr.hxx>
#endif

//==========================================================================

#define MCLK_FREQUENCY_KHZ  (CYGNUM_HAL_ARM_AT91_CLOCK_SPEED/1000)
#define MAX_TICKS     	    0x0000ffff
#define BASE_TICKS    	    (MCLK_FREQUENCY_KHZ * CYGNUM_DEVS_WATCHDOG_ARM_AT91_DESIRED_TIMEOUT_MS)

#if defined(CYGHWR_HAL_ARM_AT91_R40008) || \
    defined(CYGHWR_HAL_ARM_AT91_R40807)

#if   BASE_TICKS / 8 <= MAX_TICKS
#define DIVIDER 0
#define DIV_FACTOR 8
#elif BASE_TICKS / 32 <= MAX_TICKS
#define DIVIDER 1
#define DIV_FACTOR 32
#elif BASE_TICKS / 128 <= MAX_TICKS
#define DIVIDER 2
#define DIV_FACTOR 128
#elif BASE_TICKS / 1024 <= MAX_TICKS
#define DIVIDER 3
#define DIV_FACTOR 1024
#else
#error Desired resolution beyond hardware capabilities
#endif

#elif defined(CYGHWR_HAL_ARM_AT91_M55800A)

#if   BASE_TICKS / 32 <= MAX_TICKS
#define DIVIDER 0
#define DIV_FACTOR 32
#elif BASE_TICKS / 128 <= MAX_TICKS
#define DIVIDER 1
#define DIV_FACTOR 128
#elif BASE_TICKS / 1024 <= MAX_TICKS
#define DIVIDER 2
#define DIV_FACTOR 1024
#elif BASE_TICKS / 4096 <= MAX_TICKS
#define DIVIDER 3
#define DIV_FACTOR 4096
#else
#error Desired resolution beyond hardware capabilities
#endif
#elif defined(CYGHWR_HAL_ARM_AT91_JTST)
#if   BASE_TICKS / 32 <= MAX_TICKS
#define DIVIDER 0
#define DIV_FACTOR 32
#elif BASE_TICKS / 128 <= MAX_TICKS
#define DIVIDER 1
#define DIV_FACTOR 128
#elif BASE_TICKS / 1024 <= MAX_TICKS
#define DIVIDER 2
#define DIV_FACTOR 1024
#elif BASE_TICKS / 2046 <= MAX_TICKS
#define DIVIDER 3
#define DIV_FACTOR 2046
#else
#error Desired resolution beyond hardware capabilities
#endif

#endif

#define TICKS 	    ((BASE_TICKS / DIV_FACTOR) | 0xfff)
#define RESOLUTION  ((cyg_uint64) (TICKS * DIV_FACTOR ) * 1000000 / MCLK_FREQUENCY_KHZ)

//==========================================================================

#if defined(CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT)

#define OMRVAL	(AT91_WD_OMR_OKEY | AT91_WD_OMR_RSTEN | AT91_WD_OMR_WDEN)

void
Cyg_Watchdog::init_hw(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();
  resolution = RESOLUTION;
  CYG_REPORT_RETURN();
}

#else /* defined(CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT) */

//==========================================================================

#define OMRVAL	(AT91_WD_OMR_OKEY | AT91_WD_OMR_IRQEN | AT91_WD_OMR_WDEN)
#define INT_PRIO    7

//==========================================================================

static Cyg_Watchdog *wd;

//==========================================================================

static cyg_uint32
isr(cyg_vector vector, CYG_ADDRWORD data)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARG2XV(vector, data);

  wd->trigger();
  Cyg_Interrupt::acknowledge_interrupt(CYGNUM_HAL_INTERRUPT_WATCHDOG);
  CYG_REPORT_RETVAL(Cyg_Interrupt::HANDLED);
  return Cyg_Interrupt::HANDLED;
}

//==========================================================================

static Cyg_Interrupt wdint(
    CYGNUM_HAL_INTERRUPT_WATCHDOG,
    INT_PRIO,
    0,
    isr,
    NULL
  );

//==========================================================================

void
Cyg_Watchdog::init_hw(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();

  wd = this;
  resolution = RESOLUTION;
  wdint.configure_interrupt(CYGNUM_HAL_INTERRUPT_WATCHDOG, false, true);
  wdint.attach();
  wdint.acknowledge_interrupt(CYGNUM_HAL_INTERRUPT_WATCHDOG);
  wdint.unmask_interrupt(CYGNUM_HAL_INTERRUPT_WATCHDOG);
  CYG_REPORT_RETURN();
}

#endif	/* defined(CYGSEM_WATCHDOG_RESETS_ON_TIMEOUT) */

//==========================================================================
/*
 * Reset watchdog timer. This needs to be called regularly to prevent
 * the watchdog from firing.
 */

void
Cyg_Watchdog::reset(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();

  /* Write magic code to reset the watchdog. */
  HAL_WRITE_UINT32(AT91_WD + AT91_WD_CR, AT91_WD_CR_RSTKEY);
  CYG_REPORT_RETURN();
}

//==========================================================================
/*
 * Start watchdog to generate a hardware reset
 * or interrupt when expiring.
 */

void
Cyg_Watchdog::start(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();
  
  HAL_WRITE_UINT32(AT91_WD + AT91_WD_OMR, AT91_WD_OMR_OKEY);
  HAL_WRITE_UINT32(
    AT91_WD + AT91_WD_CMR,
    AT91_WD_CMR_CKEY | ((TICKS >> 10) & AT91_WD_CMR_HPCV) | DIVIDER
  );
  HAL_WRITE_UINT32(AT91_WD + AT91_WD_CR, AT91_WD_CR_RSTKEY);
  HAL_WRITE_UINT32(AT91_WD + AT91_WD_OMR, OMRVAL);
  CYG_REPORT_RETURN();
}

//==========================================================================
// End of watchdog_at91.cxx
