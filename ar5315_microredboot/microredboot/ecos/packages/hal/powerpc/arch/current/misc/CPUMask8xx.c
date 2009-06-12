//==========================================================================
//
//        CPUMask8xx.c
//
//        Print PowerPC 8xx CPU Mask revision
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
// Date:          1999-06-24
// Description:   Prints the CPU Part and Mask numbers.
//        See http://www.mot.com/SPS/RISC/netcomm/aesop/mpc8XX/860/860revs.htm
//####DESCRIPTIONEND####

#include <cyg/infra/diag.h>
#include <cyg/hal/ppc_regs.h>
#include <cyg/hal/hal_io.h>

#define DPRAM_BASE    0x2000
#define CPM_MISC_BASE (DPRAM_BASE + 0x1cb0)

externC void
cyg_start( void )
{
    cyg_uint32 immr, part_no, mcrev_no;

    asm volatile ("mfspr %0, %1;": "=r" (immr): "I" (CYGARC_REG_IMMR));

    part_no = (immr & (CYGARC_REG_IMMR_PARTNUM|CYGARC_REG_IMMR_MASKNUM));
    
    HAL_READ_UINT16((immr&CYGARC_REG_IMMR_BASEMASK)+CPM_MISC_BASE, mcrev_no);

    diag_printf("MPC8xx IMMR[16:31]/Part# %04x, "
                "Microcode Rev No# %04x\n", part_no, mcrev_no);
}   
// EOF CPUMask8xx.c
