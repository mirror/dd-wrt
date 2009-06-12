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
var_init_cache_acr --  Initialize the cache and access control registers

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
static void var_init_cache_acr(void)
{

    //   Invalidate and disable the cache and ACRs.

    mcf52xx_wr_cacr((CYG_WORD32)0x01000000);
    mcf52xx_wr_acr0((CYG_WORD32)0);
    mcf52xx_wr_acr1((CYG_WORD32)0);

    //   Call a routine to set  up  the  cache  and  ACRs  for  this  specific
    // platform.

    plf_init_cache_acr();

}

/*****************************************************************************
var_reset --  Variant-specific reset vector initialization routine

     This routine must be called with interrupts disabled.

INPUT:

OUTPUT:

RETURN VALUE:

     None

*****************************************************************************/
void var_reset(void)
{

    //   Initialize the processor's vector base register.

    mcf52xx_wr_vbr((CYG_WORD32)__ramvec_start);

    //   Initialize the cache and access control registers.

    var_init_cache_acr();

    //   Do any processor-specific reset initialization.

    proc_reset();
}


