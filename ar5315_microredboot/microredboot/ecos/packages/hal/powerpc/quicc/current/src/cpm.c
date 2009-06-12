//==========================================================================
//
//      cpm.c
//
//      PowerPC QUICC support functions
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
// Purpose:      Common support for the QUICC/CPM
// Description:  
//               
// Usage:
// Notes:        
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_powerpc_quicc.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <string.h>           // memset

// eCos headers decribing PowerQUICC:
#include <cyg/hal/quicc/ppc8xx.h>

// Information about DPRAM usage
// This lets the CPM/DPRAM information be shared by all environments
//
static short *nextBd = (short *)(CYGHWR_HAL_VSR_TABLE + 0x1F0);

/*
 * Reset the communications processor
 */

void
_mpc8xx_reset_cpm(void)
{
    EPPC *eppc = eppc_base();
    static int init_done = 0;

    if (init_done) return;
    init_done++;

    eppc->cp_cr = QUICC_CPM_CR_RESET | QUICC_CPM_CR_BUSY;
    memset(eppc->pram, 0, sizeof(eppc->pram));
    while (eppc->cp_cr & QUICC_CPM_CR_BUSY)
        CYG_EMPTY_STATEMENT;

    *nextBd = QUICC_BD_BASE;
}

//
// Allocate a chunk of memory in the shared CPM memory, typically
// used for buffer descriptors, etc.  The length will be aligned
// to a multiple of 8 bytes.
//
unsigned short
_mpc8xx_allocBd(int len)
{
    unsigned short bd;

    bd = *nextBd;
    if ((bd < QUICC_BD_BASE) || (bd > QUICC_BD_END)) {
        // Most likely not set up - make a guess :-(
        bd = *nextBd = QUICC_BD_BASE+0x400;
    }
    CYG_ASSERT((len & 0x7) == 0, "BD length must be multiple of 8 bytes");
    len = (len + 7) & ~7;  // Multiple of 8 bytes
    *nextBd += len;
    CYG_ASSERT(*nextBd < QUICC_BD_END, "Out of buffer descriptors!");
    if (*nextBd >= QUICC_BD_END) {
        *nextBd = QUICC_BD_BASE;
    }
    return bd;
}

#define BRG_MAX      4
#define BRG_UNAVAIL -1
#define BRG_FREE    -2
static unsigned long *brg[BRG_MAX];  // Available generators
static int alloc[BRG_MAX];           // Which port is assigned where
                                     // -1 indicates unavailable
                                     // -2 indicates free
                                     // xx indicates port assignment

static void
_mpc8xx_mark_brg(int port, int brgnum)
{
    if (brgnum >= BRG_MAX) {
        return;  // Invalid selection
    }
    if (alloc[brgnum] == BRG_FREE) {
        // Allocation unknown
        alloc[brgnum] = port;
    }
}

unsigned long *
_mpc8xx_allocate_brg(int port)
{
    EPPC *eppc = eppc_base();
    static int init = 0;
    int brgnum;

    if (!init) {
        // Set up available pool
#if defined(CYGHWR_HAL_POWERPC_MPC8XX_852T)
        // The 852T variant only has BRG3/BRG4
        alloc[0] = BRG_UNAVAIL;
        alloc[1] = BRG_UNAVAIL;
#else
        brg[0] = (unsigned long *)&eppc->brgc1;  alloc[0] = BRG_FREE;
        brg[1] = (unsigned long *)&eppc->brgc2;  alloc[1] = BRG_FREE;
#endif
        brg[2] = (unsigned long *)&eppc->brgc3;  alloc[2] = BRG_FREE;
        brg[3] = (unsigned long *)&eppc->brgc4;  alloc[3] = BRG_FREE;
#if !defined(CYGSEM_HAL_ROM_MONITOR)
        // Figure out how hardware was set by previous ROM monitor
#if CYGNUM_HAL_QUICC_SMC1 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SMC1, (eppc->si_simode >> 12) & 0x07);
#endif
#if CYGNUM_HAL_QUICC_SMC2 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SMC2, (eppc->si_simode >> 28) & 0x07);
#endif
#if CYGNUM_HAL_QUICC_SCC1 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SCC1, (eppc->si_sicr >> 0) & 0x07);
#endif
#if CYGNUM_HAL_QUICC_SCC2 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SCC2, (eppc->si_sicr >> 8) & 0x07);
#endif
#if CYGNUM_HAL_QUICC_SCC3 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SCC3, (eppc->si_sicr >> 16) & 0x07);
#endif
#if CYGNUM_HAL_QUICC_SCC4 > 0
        _mpc8xx_mark_brg(QUICC_CPM_SCC4, (eppc->si_sicr >> 24) & 0x07);
#endif
#endif
        init = 1;
    }
    // Find a free generator (or if port has already been assigned)
    for (brgnum = 0;  brgnum < BRG_MAX;  brgnum++) {
        if (alloc[brgnum] >= 0) {
            // See if it is for this port
            if (alloc[brgnum] == port) {
                // It is - just reuse it (already set up)
                return brg[brgnum];
            }
        }
    }
    // Not currently assigned, try and find a free one
    for (brgnum = 0;  brgnum < BRG_MAX;  brgnum++) {
        if (alloc[brgnum] == BRG_FREE) {
            // Allocate to this port.
            alloc[brgnum] = port;
            break;
        }
    }
    CYG_ASSERT(brgnum < BRG_MAX, "Out of baud rate generators!");
    // If no generator found - punt!
    if (brgnum == BRG_MAX) {
        brgnum = BRG_MAX-1;
    }
    // Set up clock routing for new assignment
    switch (port) {
#if CYGNUM_HAL_QUICC_SMC1 > 0
    case QUICC_CPM_SMC1:
        eppc->si_simode = (eppc->si_simode & ~(0x07<<12)) | (brgnum<<12);
        break;
#endif
#if CYGNUM_HAL_QUICC_SMC2 > 0
    case QUICC_CPM_SMC2:
        eppc->si_simode = (eppc->si_simode & ~(0x07<<28)) | (brgnum<<28);
        break;
#endif
#if CYGNUM_HAL_QUICC_SCC1 > 0
    case QUICC_CPM_SCC1:
        eppc->si_sicr = (eppc->si_sicr & ~(0xFF<<0)) | (((brgnum<<3)|(brgnum<<0))<<0);
        break;
#endif
#if CYGNUM_HAL_QUICC_SCC2 > 0
    case QUICC_CPM_SCC2:
        eppc->si_sicr = (eppc->si_sicr & ~(0xFF<<8)) | (((brgnum<<3)|(brgnum<<0))<<8);
        break;
#endif
#if CYGNUM_HAL_QUICC_SCC3 > 0
    case QUICC_CPM_SCC3:
        eppc->si_sicr = (eppc->si_sicr & ~(0xFF<<16)) | (((brgnum<<3)|(brgnum<<0))<<16);
        break;
#endif
#if CYGNUM_HAL_QUICC_SCC4 > 0
    case QUICC_CPM_SCC4:
        eppc->si_sicr = (eppc->si_sicr & ~(0xFF<<24)) | (((brgnum<<3)|(brgnum<<0))<<24);
        break;
#endif
    }
    return brg[brgnum];
}

// EOF cpm.c
