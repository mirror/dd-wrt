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
// Author(s):    nickg
// Contributors: nickg, jlarmour, dmoseley, msalter
// Date:         2000-06-06
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // architectural definitions

#include <cyg/hal/hal_intr.h>           // Interrupt handling

#include <cyg/hal/hal_cache.h>          // Cache handling

#include <cyg/hal/hal_if.h>

/*------------------------------------------------------------------------*/

#if defined(CYGPKG_CYGMON)
#include CYGHWR_MEMORY_LAYOUT_H
extern unsigned long cygmon_memsize;
#endif

extern void __txchar(unsigned ch);

void hal_platform_init(void)
{
    // Set up eCos/ROM interfaces
    hal_if_init();
}


/*------------------------------------------------------------------------*/
/* Delay for some number of useconds.                                     */
void 
hal_delay_us(cyg_uint32 us)
{
    us /= 3;
    while (us-- > 0);
}

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void hal_ceb_reset(void)
{
}

void hal_plf_write_prog_halfword(unsigned long addr, unsigned short val)
{
  unsigned short pch = addr >> 16;
  unsigned short pcl = addr & 0xffff;
  
  while(cyg_hal_plf_read_sr_tbr() & 1);	// while TXD_FULL
  cyg_hal_plf_write_tbr(pch);
  cyg_hal_plf_write_sr_tbr(0x03);	// addr_high short

  while(cyg_hal_plf_read_sr_tbr() & 1);	// while TXD_FULL
  cyg_hal_plf_write_tbr(pcl);
  cyg_hal_plf_write_sr_tbr(0x05);	// addr_low short

  while(cyg_hal_plf_read_sr_tbr() & 1);	// while TXD_FULL
  cyg_hal_plf_write_tbr(val);
  cyg_hal_plf_write_sr_tbr(0x07);	// program data value

  while(cyg_hal_plf_read_sr_tbr() & 1);	// while TXD_FULL
}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
