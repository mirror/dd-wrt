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

#include <cyg/infra/cyg_type.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_startup.h>
#include <cyg/hal/hal_memmap.h>
#include <cyg/hal/hal_arch.h>

/*****************************************************************************
plf_init_cache_acr -- Initialize the cache and access control registers

     The var_init_cache_acr routine already invalidated the cache and ACRs.
This routine only needs to enable the ACRs that it will use.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void plf_init_cache_acr(void)
{

    // Enable the instruction cache with the following options:
    // Enable CPUSHL invalidation.
    // No freeze.
    // Invalidate all cache lines (flush).
    // No external arbiter control.
    // Disable non-cacheable instruction bursting.
    // Default memory is cacheable.
    // Enable buffered writes.
    // Read and write access permitted by default.
    // Instruction fetch size is cache line.

    mcf52xx_wr_cacr((CYG_WORD32)0x81000102);

    //   Leave the access control registers disabled by default.

}

/*****************************************************************************
plf_reset -- Platform-specific reset vector initialization routine

     This routine must be called with interrupts disabled.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void plf_reset(void)
{

}

