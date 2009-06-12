//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Date:         2002-02-17
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types

#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/var_arch.h>

/*------------------------------------------------------------------------*/

void hal_platform_init(void)
{
    hal_if_init();
}

void h8300h_reset(void)
{
    __asm__ ("jmp @@0\n\t");
}

/*------------------------------------------------------------------------*/
/* Control C ISR support                                                  */

#if defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

struct Hal_SavedRegisters *hal_saved_interrupt_state;

#endif

/*------------------------------------------------------------------------*/
/* clock support                                                            */

void hal_clock_initialize(cyg_uint32 period)
{
    CYG_BYTE prescale;
#if CYGNUM_HAL_H8300_RTC_PRESCALE == 8
    prescale = 0x01;
#else
#if CYGNUM_HAL_H8300_RTC_PRESCALE == 64
    prescale = 0x02;
#else
#if CYGNUM_HAL_H8300_RTC_PRESCALE == 8192
    prescale = 0x03;
#else
#error illigal RTC prescale setting
#endif
#endif
#endif
    HAL_WRITE_UINT8(CYGARC_TCORA3,period);
    HAL_WRITE_UINT8(CYGARC_8TCNT3,0x00);
    HAL_WRITE_UINT8(CYGARC_8TCR3,0x48 | prescale);
    HAL_WRITE_UINT8(CYGARC_8TCSR3,0x00);
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    HAL_WRITE_UINT8(CYGARC_8TCR3,0x00);
    HAL_WRITE_UINT8(CYGARC_8TCSR3,0x00);
    hal_clock_initialize(period);
}

void hal_clock_read(cyg_uint32 *pvalue)
{
    CYG_BYTE val;
    HAL_READ_UINT8(CYGARC_8TCNT3,val);
    *pvalue = val;
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
