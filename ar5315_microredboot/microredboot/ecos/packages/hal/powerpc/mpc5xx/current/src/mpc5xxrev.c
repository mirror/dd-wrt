//==========================================================================
//
//        mpc5xxrev.c
//
//        MPC5xx Revision Identification
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
// Author(s):     Bob Koninckx
// Contributors:  Bob Koninckx
// Date:          2001-12-11
// Description:   Print MPC5xx revision numbers
//####DESCRIPTIONEND####

#include <cyg/infra/testcase.h>
#include <stdio.h>
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_io.h>
#include <cyg/infra/diag.h>

int
main( void )
{
    cyg_uint32 pvr, part_mask;

    CYG_TEST_INIT();

    asm volatile ("mfspr %0, 287;": "=r" (pvr));
    diag_printf("Processor PVR:         0x%08x\n", pvr);

    asm volatile ("mfspr %0, 638;": "=r" (part_mask));
    part_mask = (part_mask >> 16);
    diag_printf("Processor IMMR[00:15]: 0x%04x\n", (part_mask & 0xffff));

    diag_printf("\nSee http://www.motorola.com/SPS/RISC/netcomm/ for erratas.\n");
    diag_printf("Look for MPC5xx Versions and Masks in the Publications Library\n");

    CYG_TEST_PASS_FINISH("mpc5xx revision dump");
}
// EOF mpc5xxrev.c
