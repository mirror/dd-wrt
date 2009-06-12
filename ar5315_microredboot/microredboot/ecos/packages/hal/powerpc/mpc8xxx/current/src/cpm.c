//==========================================================================
//
//      cpm.c
//
//      PowerPC QUICC2 support functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):    Gary Thomas 
// Contributors: 
// Date:         2003-03-04
// Purpose:      Common support for the QUICC2/CPM
// Description:  
//               
// Usage:
// Notes:        
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <string.h>           // memset

// eCos headers decribing PowerQUICC2:
#include <cyg/hal/mpc8xxx.h>

// Information about DPRAM usage
// This lets the CPM/DPRAM information be shared by all environments
//
static struct dpram_info {
    unsigned int offset, len;
} CPM_DPRAM[] = {
    { DPRAM_BD_OFFSET, 0x1000 },
    { 0xB000, 0x1000 },
    { 0, 0}
};
static int *nextBd = (int *)(CYGHWR_HAL_VSR_TABLE + 0x1F0);
static struct dpram_info **info = (struct dpram_info **)(CYGHWR_HAL_VSR_TABLE + 0x1F4);

/*
 * Reset the communications processor
 */

void
_mpc8xxx_reset_cpm(void)
{
    static int init_done = 0;

    if (init_done) return;
    init_done++;

    IMM->cpm_cpcr = CPCR_RST | CPCR_FLG;
    while (IMM->cpm_cpcr & CPCR_FLG)
        CYG_EMPTY_STATEMENT;

    *nextBd = CPM_DPRAM[0].offset;
    *info = CPM_DPRAM;
    // Set up SMCx offsets
    IMM->pram.standard.smc1 = DPRAM_SMC1_OFFSET;
    IMM->pram.standard.smc2 = DPRAM_SMC2_OFFSET;
}

//
// Allocate a chunk of memory in the shared CPM memory, typically
// used for buffer descriptors, etc.  The length will be aligned
// to a multiple of 8 bytes.  This is somewhat complicated on the 
// QUICC2/CPM since there are multiple regions of shared DPRAM
// which are legal to use, of varying size and spread all over.
//
unsigned int
_mpc8xxx_allocBd(int len)
{
    struct dpram_info *ip = *info;
    unsigned int bd;

    bd = *nextBd;
    len = (len + 7) & ~7;  // Multiple of 8 bytes
    *nextBd += len;
    if (*nextBd >= (ip->offset+ip->len)) {
        ip++;
        *nextBd = ip->offset;
        *info = ip;
    }
    return bd;
}

// EOF cpm.c
