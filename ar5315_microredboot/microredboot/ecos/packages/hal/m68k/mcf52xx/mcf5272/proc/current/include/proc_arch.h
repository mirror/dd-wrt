#ifndef CYGONCE_HAL_PROC_ARCH_H
#define CYGONCE_HAL_PROC_ARCH_H
//=============================================================================
//
//      proc_arch.h
//
//      Processor variant specific abstractions
//
//=============================================================================
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
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/mcf5272_sim.h>
//#include <cyg/hal/hal_memmap.h>

// Declare the global pointer to the SIM registers.
// Everyone should use the MCF5272_SIM macro so it can be easily changed.
externC volatile mcf5272_sim_t * const mcf5272_sim_p;
#define MCF5272_SIM mcf5272_sim_p
//#define MCF5272_SIM ((volatile mcf5272_sim_t *) MCF5272_MBAR)

/* ************************************************************************ */
/* These routines write to  the special purpose  registers in the  ColdFire */
/* core.  Since these registers are write-only in the supervisor model,  no */
/* corresponding read routines exist.                                       */

externC void mcf5272_wr_mbar(CYG_WORD32);

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_PROC_ARCH_H

