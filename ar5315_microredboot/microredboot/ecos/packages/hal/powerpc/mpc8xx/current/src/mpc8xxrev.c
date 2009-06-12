//==========================================================================
//
//        mpc8xxrev.c
//
//        MPC8xx Revision Identification
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
// Author(s):     jskov
// Contributors:  jskov
// Date:          2000-02-23
// Description:   Print MPC8xx revision numbers
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
    cyg_uint16 rev_num;

    CYG_TEST_INIT();

    asm volatile ("mfspr %0, 287;": "=r" (pvr));
    diag_printf("Processor PVR:         0x%08x\n", pvr);

    asm volatile ("mfspr %0, 638;": "=r" (part_mask));
    diag_printf("Processor IMMR[16:31]: 0x%04x\n", (part_mask & 0xffff));

    HAL_READ_UINT16(CYGARC_REG_REV_NUM, rev_num);
    diag_printf("Processor REV_NUM:     0x%04x\n", rev_num);


    diag_printf("\nSee http://www.motorola.com/SPS/RISC/netcomm/ for erratas.\n");
    diag_printf("Look for MPC8xx Versions and Masks in the Publications Library\n");

    CYG_TEST_PASS_FINISH("mpc8xx revision dump");
}
// EOF mpc8xxrev.c
